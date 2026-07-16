//
//  SCValdiTouches.mm
//  Valdi
//
//  Created by David Li on 06/07/24.
//  Copyright © 2024 Snap Inc. All rights reserved.
//

#import "valdi_core/SCValdiTouches.h"
#import "valdi_core/cpp/Events/TouchEvents.hpp"
#import "valdi_core/SCValdiFunctionWithCPPFunction+CPP.h"
#import "valdi_core/SCValdiLogger.h"
#import "valdi_core/SCValdiValueUtils.h"
#import "valdi_core/SCValdiMarshaller+CPP.h"
#import "valdi_core/SCValdiError.h"

BOOL SCValdiCallActionWithEvent(id<SCValdiFunction> action, const Valdi::Value &event, Valdi::ValueFunctionFlags flags)
{
    Valdi::SimpleExceptionTracker exceptionTracker;
    Valdi::Marshaller params(exceptionTracker);
    params.push(event);

    bool hasResult;

    if ([action respondsToSelector:@selector(performWithMarshaller:flags:)]) {
        // Bypass ObjC conversion if the action is implemented by C++

        hasResult = [(SCValdiFunctionWithCPPFunction *)action performWithMarshaller:params flags:flags];
    } else {
        SCValdiMarshallerRef marshaller = SCValdiMarshallerWrap(&params);
        hasResult = [action performWithMarshaller:marshaller];
    }

    BOOL ret = NO;
    if (exceptionTracker && hasResult) {
        ret = params.getOrUndefined(-1).toBool();
    }

    return ret;
}

BOOL SCValdiForwardTouchAndCallAction(UIView *view,
                                                UIEvent *uiEvent,
                                                SCValdiGestureType gestureType,
                                                UIGestureRecognizerState state,
                                                id<SCValdiFunction> action,
                                                const Valdi::Value &event,
                                                Valdi::ValueFunctionFlags flags)
{
    BOOL result =  SCValdiCallActionWithEvent(action, event, flags);

    if (uiEvent) {
        id<SCValdiGestureListener> gestureListener = view.valdiContext.gestureListener;
        if (gestureListener) {
            [gestureListener onGestureType:gestureType didUpdateWithState:state event:uiEvent];
        }
    }

    return result;
}

Valdi::TouchEventState SCValdiMakeTouchState(UIGestureRecognizerState state)
{
    switch (state) {
        case UIGestureRecognizerStateBegan:
            return Valdi::TouchEventStateStarted;
        case UIGestureRecognizerStateChanged:
            return Valdi::TouchEventStateChanged;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateFailed:
        case UIGestureRecognizerStateCancelled:
            return Valdi::TouchEventStateEnded;
        case UIGestureRecognizerStatePossible:
            return Valdi::TouchEventStatePossible;
    }
}

SCValdiGestureLocation SCValdiGetGestureLocation(UIView *view, CGPoint gestureLocation)
{
    SCValdiGestureLocation location;
    location.relative = gestureLocation;
    location.absolute = gestureLocation;

    id<SCValdiViewNodeProtocol> viewNode = view.valdiViewNode;

    if (viewNode) {
        location.relative = [viewNode relativeDirectionAgnosticPointFromPoint:gestureLocation];
        location.absolute = [viewNode absoluteDirectionAgnosticPointFromPoint:gestureLocation];
    }

    return location;
}

Valdi::ValueFunctionFlags SCValdiGetCallFlags(UIGestureRecognizerState gestureState)
{
    if (gestureState == UIGestureRecognizerStateChanged) {
        return Valdi::ValueFunctionFlagsAllowThrottling;
    } else {
        return Valdi::ValueFunctionFlagsNone;
    }
}

Valdi::Value SCValdiMakeTouchEvent(UIView *view, CGPoint gestureLocation, UIGestureRecognizerState gestureState, Valdi::TouchEvents::PointerLocations pointerLocations)
{
    auto state = SCValdiMakeTouchState(gestureState);

    SCValdiGestureLocation location = SCValdiGetGestureLocation(view, gestureLocation);

    return Valdi::TouchEvents::makeTapEvent(state, location.relative.x, location.relative.y, location.absolute.x, location.absolute.y, pointerLocations.size(), pointerLocations);
}

Valdi::TouchEvents::PointerLocations SCValdiGetPointerDataFromEvent(UIEvent *uiEvent)
{
    Valdi::TouchEvents::PointerLocations pointerLocations;

    // we filter out historical touches - touches in UITouchPhaseCancelled/UITouchPhaseEnded, as they aren't active pointers
    for (UITouch *touch in [uiEvent allTouches]) {
        if (touch.phase == UITouchPhaseBegan ||
               touch.phase == UITouchPhaseMoved ||
               touch.phase == UITouchPhaseStationary
            ) {
            CGPoint location = [touch locationInView:touch.view];
            // use memory address of the touch as an unique ID - they are stable across a continous touch event
            pointerLocations.emplace_back(Valdi::TouchEvents::PointerData(location.x, location.y, (uintptr_t)touch));
        }
    }

    return pointerLocations;
}

 Valdi::TouchEvents::PointerLocations SCValdiGetPointerDataFromGestureRecognizer(UIGestureRecognizer *gestureRecognizer)
 {
    Valdi::TouchEvents::PointerLocations pointerLocations;

    NSUInteger numTouches = [gestureRecognizer numberOfTouches];
    for (NSUInteger i = 0; i < numTouches; i++) {
        CGPoint location = [gestureRecognizer locationOfTouch:i inView:gestureRecognizer.view];

        // use pointer Index unique ID for gesture based ones, as we don't have access for UITouches
        // TODO(2965) fix once we centralize iOS rendering
        pointerLocations.emplace_back(Valdi::TouchEvents::PointerData(location.x, location.y, i));
    }

    return pointerLocations;

}

BOOL SCValdiCallPredicateWithEvent(id<SCValdiFunction> predicate,
                                   UIView *view,
                                   const Valdi::Value &event,
                                   const char *callContext)
{
    if ([predicate respondsToSelector:@selector(performWithMarshaller:flags:)]) {
        // Use callSyncWithDeadline instead of dispatch_sync to prevent the main thread from
        // blocking indefinitely when the JS queue is busy. On timeout, returning NO safely
        // drops the hit test or gesture begin. VALDI_DISABLE_HIT_TEST_SYNC_DEADLINE reverts
        // to the legacy path.
        if (!view.valdiContext.disableHitTestSyncDeadline) {
            const auto& fn = [(SCValdiFunctionWithCPPFunction *)predicate getFunction];

            NSInteger simulatedHangMs = view.valdiContext.hitTestSyncDeadlineSimulatedHangMs;
            // Once-per-launch proof-of-life so log bundles can distinguish "path never ran"
            // from "path ran with no abnormal outcomes".
            static dispatch_once_t sDeadlinePathLogOnce;
            dispatch_once(&sDeadlinePathLogOnce, ^{
                SCLogValdiInfo(@"[ValdiTouchDeadline] deadline path active (250ms), first call site: %s, "
                               @"simulated hang: %ldms",
                               callContext, (long)simulatedHangMs);
            });
            if (simulatedHangMs > 0) {
                SCLogValdiWarning(
                    @"[ValdiTouchDeadline] %s: injecting simulated %ldms JS-thread hang "
                    @"(VALDI_HIT_TEST_SYNC_DEADLINE_SIMULATED_HANG_MS)",
                    callContext, (long)simulatedHangMs);
                fn->enqueueSimulatedHangForTesting(std::chrono::milliseconds(simulatedHangMs));
            }

            Valdi::Value params[] = { event };
            // steady_clock (not wall clock) so an NTP / leap-second jump can't log a
            // negative or bogus elapsed time; matches the clock callSyncWithDeadline uses.
            auto startTime = std::chrono::steady_clock::now();
            auto result = fn->callSyncWithDeadline(std::chrono::milliseconds(250), params, 1);
            double elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
            if (!result.success()) {
                SCLogValdiError(
                    @"[ValdiTouchDeadline] %s: sync call exceeded the 250ms deadline (waited %.0fms), dropping event",
                    callContext, elapsedMs);
                return NO;
            }
            if (elapsedMs > 100) {
                SCLogValdiWarning(@"[ValdiTouchDeadline] %s: sync call slow, took %.0fms", callContext, elapsedMs);
            }
            return result.value().toBool();
        }
    }

    // Legacy path: non-C++ ValueFunction implementations, or when sync deadline is disabled.
    auto startTime = std::chrono::steady_clock::now();
    BOOL ret = SCValdiCallActionWithEvent(predicate, event, Valdi::ValueFunctionFlagsCallSync);
    double elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
    if (elapsedMs > 250) {
        SCLogValdiWarning(@"[ValdiTouchDeadline] %s: legacy unbounded sync call blocked main for %.0fms",
                          callContext, elapsedMs);
    }
    return ret;
}

BOOL SCValdiCallSyncActionWithUIEventAndView(id<SCValdiFunction> action, CGPoint location, UIEvent* uiEvent, UIView* view)
{
    auto event = SCValdiMakeTouchEvent(view, location, UIGestureRecognizerStatePossible, SCValdiGetPointerDataFromEvent(uiEvent));

    return SCValdiCallPredicateWithEvent(action, view, event, "hit_test");
}
