package com.snap.valdi.views.touches

import android.content.Context
import android.view.MotionEvent
import android.view.View
import android.widget.FrameLayout
import androidx.test.core.app.ApplicationProvider.getApplicationContext
import com.snap.valdi.views.ValdiTouchEventResult
import com.snap.valdi.views.ValdiTouchTarget
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

/**
 * Tests for the [ValdiTouchTarget.allowSiblingCaptureBelow] opt-in: a touch target that opts in
 * must not shadow lower-z siblings during touch-down candidate capture, so an entity visually
 * behind it (e.g. a caption under the filter carousel) still receives the touch. Without the
 * opt-in, the topmost target wins and the sibling below is skipped.
 */
@RunWith(RobolectricTestRunner::class)
@Config(sdk = [28], manifest = Config.NONE)
class TouchDispatcherSiblingCaptureTest {

    /**
     * A [ValdiTouchTarget] view that records whether it was asked to process a touch and never
     * consumes it, so the dispatcher keeps offering the event to remaining candidates.
     */
    private class FakeTouchTargetView(
        context: Context,
        private val allowBelow: Boolean,
    ) : View(context), ValdiTouchTarget {
        var processed = false

        override fun processTouchEvent(event: MotionEvent): ValdiTouchEventResult {
            processed = true
            return ValdiTouchEventResult.IgnoreEvent
        }

        override val allowSiblingCaptureBelow: Boolean = allowBelow
    }

    private fun makeDownEvent(x: Float, y: Float): MotionEvent =
        MotionEvent.obtain(0L, 0L, MotionEvent.ACTION_DOWN, x, y, 0)

    /**
     * Two full-bleed, overlapping touch targets under a common parent. [lower] is added first
     * (bottom of the z-order), [upper] last (top). Both cover the touch point.
     */
    private fun buildOverlappingTargets(
        upperAllowsBelow: Boolean,
    ): Triple<FrameLayout, FakeTouchTargetView, FakeTouchTargetView> {
        val context = getApplicationContext<Context>()
        val root = FrameLayout(context)
        val lower = FakeTouchTargetView(context, allowBelow = false)
        val upper = FakeTouchTargetView(context, allowBelow = upperAllowsBelow)
        root.addView(lower)
        root.addView(upper)
        root.layout(0, 0, 1000, 1000)
        lower.layout(0, 0, 1000, 1000)
        upper.layout(0, 0, 1000, 1000)
        return Triple(root, lower, upper)
    }

    @Test
    fun `opted-in target does not shadow the sibling below it`() {
        val (root, lower, upper) = buildOverlappingTargets(upperAllowsBelow = true)
        val dispatcher = TouchDispatcher.create(
            root,
            DisallowInterceptTouchEventMode.DISALLOW_WHEN_GESTURE_POSSIBLE,
            null,
        )

        val event = makeDownEvent(500f, 500f)
        dispatcher.dispatchTouch(event)

        assertTrue("topmost target should be captured", upper.processed)
        assertTrue("sibling below should still be captured when opt-in is on", lower.processed)
        event.recycle()
    }

    @Test
    fun `default target shadows the sibling below it`() {
        val (root, lower, upper) = buildOverlappingTargets(upperAllowsBelow = false)
        val dispatcher = TouchDispatcher.create(
            root,
            DisallowInterceptTouchEventMode.DISALLOW_WHEN_GESTURE_POSSIBLE,
            null,
        )

        val event = makeDownEvent(500f, 500f)
        dispatcher.dispatchTouch(event)

        assertTrue("topmost target should be captured", upper.processed)
        assertFalse("sibling below should be shadowed by default", lower.processed)
        event.recycle()
    }

    @Test
    fun `opt-in reaches a sibling subtree but keeps topmost-wins inside it`() {
        val context = getApplicationContext<Context>()
        val root = FrameLayout(context)

        // z-bottom sibling: a container holding two overlapping targets (childAbove on top).
        val siblingContainer = FrameLayout(context)
        val childBelow = FakeTouchTargetView(context, allowBelow = false)
        val childAbove = FakeTouchTargetView(context, allowBelow = false)
        siblingContainer.addView(childBelow)
        siblingContainer.addView(childAbove)

        // z-top sibling: opts into capturing lower-z siblings.
        val optIn = FakeTouchTargetView(context, allowBelow = true)

        root.addView(siblingContainer)
        root.addView(optIn)
        root.layout(0, 0, 1000, 1000)
        siblingContainer.layout(0, 0, 1000, 1000)
        childBelow.layout(0, 0, 1000, 1000)
        childAbove.layout(0, 0, 1000, 1000)
        optIn.layout(0, 0, 1000, 1000)

        val dispatcher = TouchDispatcher.create(
            root,
            DisallowInterceptTouchEventMode.DISALLOW_WHEN_GESTURE_POSSIBLE,
            null,
        )

        val event = makeDownEvent(500f, 500f)
        dispatcher.dispatchTouch(event)

        assertTrue("opted-in target captured", optIn.processed)
        assertTrue("topmost child of the reached sibling subtree captured", childAbove.processed)
        assertFalse(
            "lower child of the sibling subtree must not be captured (opt-in must not force capture-all)",
            childBelow.processed,
        )
        event.recycle()
    }

    @Test
    fun `a solid sibling below the opt-in stops capture leaking to even-lower siblings`() {
        val context = getApplicationContext<Context>()
        val root = FrameLayout(context)

        // Bottom-to-top z-order: evenLower, solidMid, optIn. Only optIn opts in.
        val evenLower = FakeTouchTargetView(context, allowBelow = false)
        val solidMid = FakeTouchTargetView(context, allowBelow = false)
        val optIn = FakeTouchTargetView(context, allowBelow = true)
        root.addView(evenLower)
        root.addView(solidMid)
        root.addView(optIn)
        root.layout(0, 0, 1000, 1000)
        evenLower.layout(0, 0, 1000, 1000)
        solidMid.layout(0, 0, 1000, 1000)
        optIn.layout(0, 0, 1000, 1000)

        val dispatcher = TouchDispatcher.create(
            root,
            DisallowInterceptTouchEventMode.DISALLOW_WHEN_GESTURE_POSSIBLE,
            null,
        )

        val event = makeDownEvent(500f, 500f)
        dispatcher.dispatchTouch(event)

        assertTrue("opted-in target captured", optIn.processed)
        assertTrue("first solid sibling below the opt-in captured", solidMid.processed)
        assertFalse(
            "solid sibling must break the loop; capture must not leak to even-lower siblings",
            evenLower.processed,
        )
        event.recycle()
    }
}
