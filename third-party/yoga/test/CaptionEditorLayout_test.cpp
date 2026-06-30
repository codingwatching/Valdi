#include <yoga/Yoga.h>
#include <yoga/YGEnums.h>
#include <yoga/YGNode.h>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>

// Tests caption text editor overlay layout under Yoga 2.x.
//
// The editing layout is:
//   editorRoot (column, flexGrow:1, marginTop: safeArea)
//     ├── dismiss tap target (width:100%, height:100%)
//     └── captionContainer (absolute, bottom:X, width:390, justifyContent:center)
//          └── textViewContainer (width:390, alignSelf:center, justifyContent:center)
//               └── textViewWrapper (marginL:8, marginR:8, height:100%, measureFunc)
//
// The Classic caption style sets width on the textViewContainer to match
// the root entity width (390pt on iPhone). The textViewWrapper has 8pt
// margins on each side and a native text measure function.
//
// Under Yoga 2.x the text may be clipped at the left edge, suggesting
// the textViewWrapper's position or width is computed incorrectly.

struct CaptionMeasureRecord {
    float width;
    YGMeasureMode widthMode;
    float height;
    YGMeasureMode heightMode;
};

static thread_local std::vector<CaptionMeasureRecord> g_captionMeasures;

// Simulates a multi-line caption measuring "Good morning everyone!"
// Text wraps within the given width constraint.
static YGSize captionTextMeasure(
    YGNodeConstRef /*node*/,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode) {
    g_captionMeasures.push_back({width, widthMode, height, heightMode});

    // Simulated text metrics for "Good morning everyone!" in 17pt font:
    // Single line intrinsic width ~200px, wraps at narrower widths.
    const float intrinsicWidth = 200.0f;
    const float lineHeight = 22.0f;

    float resultWidth;
    if (widthMode == YGMeasureModeExactly) {
        resultWidth = width;
    } else if (widthMode == YGMeasureModeAtMost) {
        resultWidth = std::min(width, intrinsicWidth);
    } else {
        resultWidth = intrinsicWidth;
    }

    // Number of lines: ceil(intrinsicWidth / resultWidth) if wrapping
    float numLines = resultWidth > 0 ? std::ceil(intrinsicWidth / resultWidth) : 1;
    float resultHeight = numLines * lineHeight;

    return {resultWidth, resultHeight};
}

// Simulates longer text that always wraps to 2+ lines
static YGSize longCaptionTextMeasure(
    YGNodeConstRef /*node*/,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode) {
    g_captionMeasures.push_back({width, widthMode, height, heightMode});

    const float intrinsicWidth = 600.0f;
    const float lineHeight = 22.0f;

    float resultWidth;
    if (widthMode == YGMeasureModeExactly) {
        resultWidth = width;
    } else if (widthMode == YGMeasureModeAtMost) {
        resultWidth = std::min(width, intrinsicWidth);
    } else {
        resultWidth = intrinsicWidth;
    }

    float numLines = resultWidth > 0 ? std::ceil(intrinsicWidth / resultWidth) : 1;
    float resultHeight = numLines * lineHeight;

    return {resultWidth, resultHeight};
}

class CaptionEditorLayoutTest : public ::testing::Test {
protected:
    YGConfigRef config;

    void SetUp() override {
        config = YGConfigNew();
        YGConfigSetExperimentalFeatureEnabled(
            config, YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetExperimentalFeatureEnabled(
            config, YGExperimentalFeatureFixFlexBasisFitContent, true);
        YGConfigSetPointScaleFactor(config, 3.0f);
        g_captionMeasures.clear();
    }

    void TearDown() override { YGConfigFree(config); }
    YGNodeRef makeNode() { return YGNodeNewWithConfig(config); }

    static constexpr float kScreenWidth = 390.0f;
    static constexpr float kScreenHeight = 844.0f;
    static constexpr float kSafeAreaTop = 47.0f;
    static constexpr float kKeyboardHeight = 300.0f;
    static constexpr float kCarouselHeight = 44.0f;
    static constexpr float kCaptionMargin = 8.0f;

    struct CaptionLayout {
        YGNodeRef root;
        YGNodeRef editorRoot;
        YGNodeRef dismissTap;
        YGNodeRef captionContainer;
        YGNodeRef textViewContainer;
        YGNodeRef textViewWrapper;
    };

    // Builds the caption editor layout in "above keyboard" editing mode
    CaptionLayout buildAboveKeyboard(YGMeasureFunc measureFn) {
        CaptionLayout l;

        // Root container (provided by the Valdi context host)
        l.root = makeNode();

        // CaptionEditor root view: column, flexGrow:1, marginTop: safeArea
        l.editorRoot = makeNode();
        YGNodeStyleSetFlexGrow(l.editorRoot, 1);
        YGNodeStyleSetMargin(l.editorRoot, YGEdgeTop, kSafeAreaTop);

        // Dismiss tap target: width:100%, height:100%
        l.dismissTap = makeNode();
        YGNodeStyleSetWidthPercent(l.dismissTap, 100);
        YGNodeStyleSetHeightPercent(l.dismissTap, 100);

        // Caption container: absolute, bottom:keyboardH+carouselH, width:390
        // justifyContent:center (from captionContainerStyle)
        l.captionContainer = makeNode();
        YGNodeStyleSetPositionType(l.captionContainer, YGPositionTypeAbsolute);
        YGNodeStyleSetPosition(l.captionContainer, YGEdgeBottom,
                               kKeyboardHeight + kCarouselHeight);
        YGNodeStyleSetWidth(l.captionContainer, kScreenWidth);
        YGNodeStyleSetJustifyContent(l.captionContainer, YGJustifyCenter);

        // TextViewContainer (from CaptionTextView outer <view>):
        // Classic style: width:390, alignSelf:center, backgroundColor, justifyContent:center
        l.textViewContainer = makeNode();
        YGNodeStyleSetWidth(l.textViewContainer, kScreenWidth);
        YGNodeStyleSetAlignSelf(l.textViewContainer, YGAlignCenter);
        YGNodeStyleSetJustifyContent(l.textViewContainer, YGJustifyCenter);

        // TextViewWrapper (leaf with measure func):
        // marginLeft:8, marginRight:8, height:100%
        l.textViewWrapper = makeNode();
        YGNodeStyleSetMargin(l.textViewWrapper, YGEdgeLeft, kCaptionMargin);
        YGNodeStyleSetMargin(l.textViewWrapper, YGEdgeRight, kCaptionMargin);
        YGNodeStyleSetHeightPercent(l.textViewWrapper, 100);
        YGNodeSetMeasureFunc(l.textViewWrapper, measureFn);

        // Assemble
        YGNodeInsertChild(l.textViewContainer, l.textViewWrapper, 0);
        YGNodeInsertChild(l.captionContainer, l.textViewContainer, 0);
        YGNodeInsertChild(l.editorRoot, l.dismissTap, 0);
        YGNodeInsertChild(l.editorRoot, l.captionContainer, 1);
        YGNodeInsertChild(l.root, l.editorRoot, 0);

        return l;
    }

    void dumpLayout(const CaptionLayout& l) {
        auto dump = [](const char* name, YGNodeRef node) {
            fprintf(stderr, "%s: pos=(%.1f, %.1f) size=%.1f x %.1f\n",
                    name,
                    YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node),
                    YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));
        };
        fprintf(stderr, "\n=== Caption Editor Layout ===\n");
        dump("root", l.root);
        dump("editorRoot", l.editorRoot);
        dump("captionContainer", l.captionContainer);
        dump("textViewContainer", l.textViewContainer);
        dump("textViewWrapper", l.textViewWrapper);

        fprintf(stderr, "Measure calls:\n");
        for (size_t i = 0; i < g_captionMeasures.size(); i++) {
            const auto& r = g_captionMeasures[i];
            const char* wMode = r.widthMode == YGMeasureModeExactly ? "Exactly" :
                               r.widthMode == YGMeasureModeAtMost ? "AtMost" : "Undefined";
            const char* hMode = r.heightMode == YGMeasureModeExactly ? "Exactly" :
                               r.heightMode == YGMeasureModeAtMost ? "AtMost" : "Undefined";
            fprintf(stderr, "  [%zu]: w=%.1f(%s) h=%.1f(%s)\n",
                    i, r.width, wMode, r.height, hMode);
        }
    }
};

// Test 1: Basic caption editing layout — textViewWrapper position and size
TEST_F(CaptionEditorLayoutTest, textViewWrapperPositionedCorrectly) {
    auto l = buildAboveKeyboard(captionTextMeasure);
    g_captionMeasures.clear();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    // The textViewWrapper should be at x=8 (marginLeft) within the textViewContainer
    float wrapperLeft = YGNodeLayoutGetLeft(l.textViewWrapper);
    float wrapperWidth = YGNodeLayoutGetWidth(l.textViewWrapper);
    float containerWidth = YGNodeLayoutGetWidth(l.textViewContainer);

    EXPECT_FLOAT_EQ(kCaptionMargin, wrapperLeft)
        << "TextViewWrapper left should be 8 (marginLeft). "
           "Left-edge clipping means it's positioned incorrectly.";

    EXPECT_FLOAT_EQ(kScreenWidth - 2 * kCaptionMargin, wrapperWidth)
        << "TextViewWrapper width should be 390 - 16 = 374. "
           "Wrong width causes text to render outside bounds.";

    EXPECT_FLOAT_EQ(kScreenWidth, containerWidth)
        << "TextViewContainer should be full width (390).";

    // The textViewContainer should be at x=0 within the captionContainer
    // (both are 390px wide, centering a 390px child in 390px parent = 0 offset)
    float containerLeft = YGNodeLayoutGetLeft(l.textViewContainer);
    EXPECT_FLOAT_EQ(0.0f, containerLeft)
        << "TextViewContainer should be at x=0 (same width as parent, centering is no-op).";

    YGNodeFreeRecursive(l.root);
}

// Test 2: Long text that wraps to multiple lines
TEST_F(CaptionEditorLayoutTest, longTextWrapsWithCorrectBounds) {
    auto l = buildAboveKeyboard(longCaptionTextMeasure);
    g_captionMeasures.clear();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    float wrapperLeft = YGNodeLayoutGetLeft(l.textViewWrapper);
    float wrapperWidth = YGNodeLayoutGetWidth(l.textViewWrapper);

    EXPECT_FLOAT_EQ(kCaptionMargin, wrapperLeft)
        << "Long text: wrapper should still start at marginLeft=8.";

    EXPECT_FLOAT_EQ(kScreenWidth - 2 * kCaptionMargin, wrapperWidth)
        << "Long text: wrapper width should be 374 regardless of text length.";

    float wrapperHeight = YGNodeLayoutGetHeight(l.textViewWrapper);
    EXPECT_GT(wrapperHeight, 0)
        << "Long text: wrapper should have non-zero height for multi-line content.";

    YGNodeFreeRecursive(l.root);
}

// Test 3: textViewWrapper inside absolute container gets non-zero dimensions
TEST_F(CaptionEditorLayoutTest, textViewWrapperHasNonZeroDimensions) {
    auto l = buildAboveKeyboard(captionTextMeasure);
    g_captionMeasures.clear();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    EXPECT_GT(YGNodeLayoutGetWidth(l.textViewWrapper), 0)
        << "TextViewWrapper has zero width — text invisible.";
    EXPECT_GT(YGNodeLayoutGetHeight(l.textViewWrapper), 0)
        << "TextViewWrapper has zero height — text invisible.";
    EXPECT_GT(YGNodeLayoutGetWidth(l.textViewContainer), 0)
        << "TextViewContainer has zero width.";
    EXPECT_GT(YGNodeLayoutGetHeight(l.textViewContainer), 0)
        << "TextViewContainer has zero height.";

    YGNodeFreeRecursive(l.root);
}

// Test 4: The caption container's absolute position doesn't push content off-screen
TEST_F(CaptionEditorLayoutTest, captionContainerAbsolutePositionValid) {
    auto l = buildAboveKeyboard(captionTextMeasure);
    g_captionMeasures.clear();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    float containerLeft = YGNodeLayoutGetLeft(l.captionContainer);
    float containerTop = YGNodeLayoutGetTop(l.captionContainer);

    // The absolute container with bottom:344 and no left should be at x=0
    EXPECT_GE(containerLeft, 0.0f)
        << "Caption container has negative left — content may be clipped.";

    // Top should be computed from bottom + height
    float containerHeight = YGNodeLayoutGetHeight(l.captionContainer);
    float parentHeight = YGNodeLayoutGetHeight(l.editorRoot);
    float expectedTop = parentHeight - (kKeyboardHeight + kCarouselHeight) - containerHeight;
    EXPECT_NEAR(expectedTop, containerTop, 1.0f)
        << "Caption container top position is wrong.";

    YGNodeFreeRecursive(l.root);
}

// Test 5: Non-Classic style where textViewContainer has NO explicit width
// (DropShadow, Outline, etc. don't set width on the container)
TEST_F(CaptionEditorLayoutTest, noExplicitWidthOnTextViewContainer) {
    auto l = buildAboveKeyboard(longCaptionTextMeasure);
    g_captionMeasures.clear();

    // Remove the explicit width from textViewContainer to simulate non-Classic styles
    YGNodeStyleSetWidth(l.textViewContainer, YGUndefined);

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    float wrapperLeft = YGNodeLayoutGetLeft(l.textViewWrapper);
    float wrapperWidth = YGNodeLayoutGetWidth(l.textViewWrapper);
    float containerLeft = YGNodeLayoutGetLeft(l.textViewContainer);
    float containerWidth = YGNodeLayoutGetWidth(l.textViewContainer);

    // With alignSelf:center and no explicit width, the container should size
    // to its content (textViewWrapper + margins) and then center
    EXPECT_GT(containerWidth, 0)
        << "TextViewContainer with no explicit width should still have content-based width.";

    EXPECT_GT(wrapperWidth, 0)
        << "TextViewWrapper should have non-zero width even without explicit parent width.";

    EXPECT_FLOAT_EQ(kCaptionMargin, wrapperLeft)
        << "TextViewWrapper left should be marginLeft=8 regardless of parent sizing.";

    (void)containerLeft;

    YGNodeFreeRecursive(l.root);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(CaptionEditorLayoutTest);
