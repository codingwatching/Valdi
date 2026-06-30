#include <yoga/Yoga.h>
#include <yoga/YGEnums.h>
#include <yoga/YGNode.h>
#include <gtest/gtest.h>
#include <cmath>

// Tests a horizontal style carousel layout under Yoga 2.x.
//
// Layout hierarchy:
//   editorRoot (flexGrow:1, marginTop:safeArea)
//     └── carouselLayout (height:60, position:absolute, bottom:keyboardH, left:0, right:0)
//         └── carouselContainer (flexGrow:1, gradient)
//             └── pluginWrapper (position:absolute, width:100%, height:100%)
//                 └── styleContainer (flexGrow:1, justifyContent:flex-start)
//                     └── layoutContainer (flexDirection:row, flexGrow:1)
//                         ├── toolbarWrapper > toolbarLayout (row, alignItems:center)
//                         │   └── items (40x40)
//                         └── ScrollView (horizontal, flexShrink:1, alignItems:center, overflow:scroll)
//                             └── styleItem (w:60, h:100%, justifyContent:center)
//                                 └── inner (w:100%, h:55%)
//
// The carousel parent is constrained to 60px height. Under Yoga 2.x with
// FixFlexBasisFitContent, height:100% inside the horizontal scroll view
// may resolve incorrectly, causing items to not fill the 60px bar height.

class StyleCarouselLayoutTest : public ::testing::Test {
protected:
    YGConfigRef config;

    void SetUp() override {
        config = YGConfigNew();
        YGConfigSetExperimentalFeatureEnabled(
            config, YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetExperimentalFeatureEnabled(
            config, YGExperimentalFeatureFixFlexBasisFitContent, true);
        YGConfigSetPointScaleFactor(config, 3.0f);
    }

    void TearDown() override { YGConfigFree(config); }
    YGNodeRef makeNode() { return YGNodeNewWithConfig(config); }

    static constexpr float kScreenWidth = 390.0f;
    static constexpr float kScreenHeight = 844.0f;
    static constexpr float kSafeAreaTop = 47.0f;
    static constexpr float kKeyboardHeight = 300.0f;
    static constexpr float kCarouselHeight = 60.0f;
    static constexpr float kToolbarItemSize = 40.0f;
    static constexpr float kToolbarRightMargin = 8.0f;
    static constexpr float kToolbarLeftMargin = 10.0f;
    static constexpr int kToolbarItemCount = 3;
    static constexpr float kStyleItemWidth = 60.0f;
    static constexpr float kStyleItemPadding = 5.0f;
    static constexpr int kStyleItemCount = 5;

    struct CarouselLayout {
        YGNodeRef root;
        YGNodeRef editorRoot;
        YGNodeRef carouselLayout;
        YGNodeRef carouselContainer;
        YGNodeRef pluginWrapper;
        YGNodeRef styleContainer;
        YGNodeRef layoutContainer;
        YGNodeRef toolbarWrapper;
        YGNodeRef toolbarLayout;
        YGNodeRef toolbarItems[3];
        YGNodeRef scrollView;
        YGNodeRef styleItems[5];
        YGNodeRef styleInners[5];
    };

    CarouselLayout buildCarousel() {
        CarouselLayout l;

        // Root container (screen)
        l.root = makeNode();

        // CaptionEditor root: flexGrow:1, marginTop:safeArea
        l.editorRoot = makeNode();
        YGNodeStyleSetFlexGrow(l.editorRoot, 1);
        YGNodeStyleSetMargin(l.editorRoot, YGEdgeTop, kSafeAreaTop);

        // carouselLayout: height:60, position:absolute, bottom:keyboardH, left:0, right:0
        l.carouselLayout = makeNode();
        YGNodeStyleSetHeight(l.carouselLayout, kCarouselHeight);
        YGNodeStyleSetPositionType(l.carouselLayout, YGPositionTypeAbsolute);
        YGNodeStyleSetPosition(l.carouselLayout, YGEdgeBottom, kKeyboardHeight);
        YGNodeStyleSetPosition(l.carouselLayout, YGEdgeLeft, 0);
        YGNodeStyleSetPosition(l.carouselLayout, YGEdgeRight, 0);

        // CaptionCarousel container: flexGrow:1
        l.carouselContainer = makeNode();
        YGNodeStyleSetFlexGrow(l.carouselContainer, 1);

        // pluginWrapper: position:absolute, width:100%, height:100%
        l.pluginWrapper = makeNode();
        YGNodeStyleSetPositionType(l.pluginWrapper, YGPositionTypeAbsolute);
        YGNodeStyleSetWidthPercent(l.pluginWrapper, 100);
        YGNodeStyleSetHeightPercent(l.pluginWrapper, 100);

        // styleContainer: flexGrow:1, justifyContent:flex-start
        l.styleContainer = makeNode();
        YGNodeStyleSetFlexGrow(l.styleContainer, 1);
        YGNodeStyleSetJustifyContent(l.styleContainer, YGJustifyFlexStart);

        // layoutContainer: flexDirection:row, flexGrow:1
        l.layoutContainer = makeNode();
        YGNodeStyleSetFlexDirection(l.layoutContainer, YGFlexDirectionRow);
        YGNodeStyleSetFlexGrow(l.layoutContainer, 1);

        // Toolbar wrapper <view>
        l.toolbarWrapper = makeNode();

        // Toolbar layout: flexGrow:1, row, alignItems:center, marginLeft:10
        l.toolbarLayout = makeNode();
        YGNodeStyleSetFlexGrow(l.toolbarLayout, 1);
        YGNodeStyleSetFlexDirection(l.toolbarLayout, YGFlexDirectionRow);
        YGNodeStyleSetAlignItems(l.toolbarLayout, YGAlignCenter);
        YGNodeStyleSetJustifyContent(l.toolbarLayout, YGJustifyFlexStart);
        YGNodeStyleSetMargin(l.toolbarLayout, YGEdgeLeft, kToolbarLeftMargin);

        for (int i = 0; i < kToolbarItemCount; i++) {
            l.toolbarItems[i] = makeNode();
            YGNodeStyleSetWidth(l.toolbarItems[i], kToolbarItemSize);
            YGNodeStyleSetHeight(l.toolbarItems[i], kToolbarItemSize);
            YGNodeStyleSetAlignItems(l.toolbarItems[i], YGAlignCenter);
            YGNodeStyleSetJustifyContent(l.toolbarItems[i], YGJustifyCenter);
            YGNodeStyleSetMargin(l.toolbarItems[i], YGEdgeRight, kToolbarRightMargin);
            YGNodeInsertChild(l.toolbarLayout, l.toolbarItems[i], i);
        }

        YGNodeInsertChild(l.toolbarWrapper, l.toolbarLayout, 0);

        // ScrollView: horizontal, flexShrink:1, alignItems:center, overflow:scroll
        l.scrollView = makeNode();
        YGNodeStyleSetFlexShrink(l.scrollView, 1);
        YGNodeStyleSetAlignItems(l.scrollView, YGAlignCenter);
        YGNodeStyleSetFlexDirection(l.scrollView, YGFlexDirectionRow);
        YGNodeStyleSetOverflow(l.scrollView, YGOverflowScroll);

        for (int i = 0; i < kStyleItemCount; i++) {
            l.styleItems[i] = makeNode();
            YGNodeStyleSetWidth(l.styleItems[i], kStyleItemWidth);
            YGNodeStyleSetHeightPercent(l.styleItems[i], 100);
            YGNodeStyleSetAlignItems(l.styleItems[i], YGAlignCenter);
            YGNodeStyleSetJustifyContent(l.styleItems[i], YGJustifyCenter);
            YGNodeStyleSetPadding(l.styleItems[i], YGEdgeLeft, kStyleItemPadding);
            YGNodeStyleSetPadding(l.styleItems[i], YGEdgeRight, kStyleItemPadding);

            l.styleInners[i] = makeNode();
            YGNodeStyleSetWidthPercent(l.styleInners[i], 100);
            YGNodeStyleSetHeightPercent(l.styleInners[i], 55);

            YGNodeInsertChild(l.styleItems[i], l.styleInners[i], 0);
            YGNodeInsertChild(l.scrollView, l.styleItems[i], i);
        }

        // Assemble
        YGNodeInsertChild(l.layoutContainer, l.toolbarWrapper, 0);
        YGNodeInsertChild(l.layoutContainer, l.scrollView, 1);
        YGNodeInsertChild(l.styleContainer, l.layoutContainer, 0);
        YGNodeInsertChild(l.pluginWrapper, l.styleContainer, 0);
        YGNodeInsertChild(l.carouselContainer, l.pluginWrapper, 0);
        YGNodeInsertChild(l.carouselLayout, l.carouselContainer, 0);
        YGNodeInsertChild(l.editorRoot, l.carouselLayout, 0);
        YGNodeInsertChild(l.root, l.editorRoot, 0);

        return l;
    }

    void dumpLayout(const CarouselLayout& l) {
        auto dump = [](const char* name, YGNodeRef node) {
            fprintf(stderr, "%s: pos=(%.1f, %.1f) size=%.1f x %.1f\n",
                    name,
                    YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node),
                    YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));
        };
        fprintf(stderr, "\n=== Style Carousel Layout ===\n");
        dump("root", l.root);
        dump("editorRoot", l.editorRoot);
        dump("carouselLayout", l.carouselLayout);
        dump("carouselContainer", l.carouselContainer);
        dump("pluginWrapper", l.pluginWrapper);
        dump("styleContainer", l.styleContainer);
        dump("layoutContainer", l.layoutContainer);
        dump("toolbarWrapper", l.toolbarWrapper);
        dump("toolbarLayout", l.toolbarLayout);
        dump("toolbarItem[0]", l.toolbarItems[0]);
        dump("scrollView", l.scrollView);
        for (int i = 0; i < kStyleItemCount; i++) {
            char buf[32];
            snprintf(buf, sizeof(buf), "styleItem[%d]", i);
            dump(buf, l.styleItems[i]);
            snprintf(buf, sizeof(buf), "styleInner[%d]", i);
            dump(buf, l.styleInners[i]);
        }
    }
};

// Style items with height:100% should fill the scroll view's 60px cross-axis
TEST_F(StyleCarouselLayoutTest, styleItemsFillCarouselHeight) {
    auto l = buildCarousel();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    float carouselLayoutHeight = YGNodeLayoutGetHeight(l.carouselLayout);
    ASSERT_FLOAT_EQ(kCarouselHeight, carouselLayoutHeight)
        << "Carousel layout should be 60px";

    float scrollViewHeight = YGNodeLayoutGetHeight(l.scrollView);
    ASSERT_GT(scrollViewHeight, 0) << "ScrollView should have non-zero height";
    EXPECT_FLOAT_EQ(kCarouselHeight, scrollViewHeight)
        << "ScrollView should fill the 60px carousel height";

    for (int i = 0; i < kStyleItemCount; i++) {
        float itemHeight = YGNodeLayoutGetHeight(l.styleItems[i]);
        EXPECT_FLOAT_EQ(scrollViewHeight, itemHeight)
            << "Style item " << i << " with height:100% should equal scroll view height ("
            << scrollViewHeight << "), got " << itemHeight;
    }

    YGNodeFreeRecursive(l.root);
}

// The inner container (height:55%) should be vertically centered in the 60px bar
TEST_F(StyleCarouselLayoutTest, innerContainerVerticallyCentered) {
    auto l = buildCarousel();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    for (int i = 0; i < kStyleItemCount; i++) {
        float itemHeight = YGNodeLayoutGetHeight(l.styleItems[i]);
        float innerHeight = YGNodeLayoutGetHeight(l.styleInners[i]);
        float innerTop = YGNodeLayoutGetTop(l.styleInners[i]);

        ASSERT_GT(itemHeight, 0) << "Style item " << i << " should have non-zero height";
        ASSERT_GT(innerHeight, 0) << "Inner container " << i << " should have non-zero height";

        float expectedTop = (itemHeight - innerHeight) / 2.0f;
        EXPECT_NEAR(expectedTop, innerTop, 1.0f)
            << "Inner container " << i << " should be vertically centered. "
            << "Expected top ~" << expectedTop << " but got " << innerTop
            << " (item height=" << itemHeight << ", inner height=" << innerHeight << ")";
    }

    YGNodeFreeRecursive(l.root);
}

// Toolbar items (40x40) should be vertically centered in the 60px bar
TEST_F(StyleCarouselLayoutTest, toolbarItemsVerticallyCentered) {
    auto l = buildCarousel();

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    dumpLayout(l);

    float toolbarLayoutHeight = YGNodeLayoutGetHeight(l.toolbarLayout);
    for (int i = 0; i < kToolbarItemCount; i++) {
        float itemTop = YGNodeLayoutGetTop(l.toolbarItems[i]);
        float expectedTop = (toolbarLayoutHeight - kToolbarItemSize) / 2.0f;
        EXPECT_NEAR(expectedTop, itemTop, 1.0f)
            << "Toolbar item " << i << " should be vertically centered in "
            << toolbarLayoutHeight << "px bar";
    }

    YGNodeFreeRecursive(l.root);
}

// Minimal repro: height:100% child inside a fixed-height horizontal scroll view
TEST_F(StyleCarouselLayoutTest, percentHeightInFixedHeightHorizontalScrollView) {
    // Outer container with fixed height (simulates the 60px carousel bar)
    YGNodeRef root = makeNode();
    YGNodeStyleSetWidth(root, kScreenWidth);
    YGNodeStyleSetHeight(root, kCarouselHeight);

    YGNodeRef scrollView = makeNode();
    YGNodeStyleSetFlexDirection(scrollView, YGFlexDirectionRow);
    YGNodeStyleSetOverflow(scrollView, YGOverflowScroll);
    YGNodeStyleSetAlignItems(scrollView, YGAlignCenter);
    YGNodeStyleSetFlexGrow(scrollView, 1);

    YGNodeRef child = makeNode();
    YGNodeStyleSetWidth(child, 60);
    YGNodeStyleSetHeightPercent(child, 100);

    YGNodeInsertChild(scrollView, child, 0);
    YGNodeInsertChild(root, scrollView, 0);

    YGNodeCalculateLayout(root, kScreenWidth, kCarouselHeight, YGDirectionLTR);

    float scrollViewHeight = YGNodeLayoutGetHeight(scrollView);
    float childHeight = YGNodeLayoutGetHeight(child);
    float childTop = YGNodeLayoutGetTop(child);

    fprintf(stderr, "\n=== Minimal Scroll Repro ===\n");
    fprintf(stderr, "root: size=%.1f x %.1f\n",
            YGNodeLayoutGetWidth(root), YGNodeLayoutGetHeight(root));
    fprintf(stderr, "scrollView: size=%.1f x %.1f\n",
            YGNodeLayoutGetWidth(scrollView), scrollViewHeight);
    fprintf(stderr, "child: pos=(%.1f, %.1f) size=%.1f x %.1f\n",
            YGNodeLayoutGetLeft(child), childTop,
            YGNodeLayoutGetWidth(child), childHeight);

    EXPECT_FLOAT_EQ(kCarouselHeight, scrollViewHeight)
        << "ScrollView should be 60px tall";
    EXPECT_FLOAT_EQ(kCarouselHeight, childHeight)
        << "Child with height:100% should fill 60px scroll view, got " << childHeight;
    EXPECT_FLOAT_EQ(0.0f, childTop)
        << "Child filling parent should start at top=0";

    YGNodeFreeRecursive(root);
}

// Compare layout with vs without FixFlexBasisFitContent
TEST_F(StyleCarouselLayoutTest, compareWithAndWithoutFixFlexBasis) {
    // First: WITH FixFlexBasisFitContent (current config)
    auto l1 = buildCarousel();
    YGNodeCalculateLayout(l1.root, kScreenWidth, kScreenHeight, YGDirectionLTR);

    float withFix_scrollH = YGNodeLayoutGetHeight(l1.scrollView);
    float withFix_item0H = YGNodeLayoutGetHeight(l1.styleItems[0]);
    float withFix_item0Top = YGNodeLayoutGetTop(l1.styleItems[0]);
    float withFix_inner0H = YGNodeLayoutGetHeight(l1.styleInners[0]);
    float withFix_inner0Top = YGNodeLayoutGetTop(l1.styleInners[0]);

    fprintf(stderr, "\n=== WITH FixFlexBasisFitContent ===\n");
    dumpLayout(l1);
    YGNodeFreeRecursive(l1.root);

    // Second: WITHOUT FixFlexBasisFitContent
    YGConfigSetExperimentalFeatureEnabled(
        config, YGExperimentalFeatureFixFlexBasisFitContent, false);

    auto l2 = buildCarousel();
    YGNodeCalculateLayout(l2.root, kScreenWidth, kScreenHeight, YGDirectionLTR);

    float withoutFix_scrollH = YGNodeLayoutGetHeight(l2.scrollView);
    float withoutFix_item0H = YGNodeLayoutGetHeight(l2.styleItems[0]);
    float withoutFix_item0Top = YGNodeLayoutGetTop(l2.styleItems[0]);
    float withoutFix_inner0H = YGNodeLayoutGetHeight(l2.styleInners[0]);
    float withoutFix_inner0Top = YGNodeLayoutGetTop(l2.styleInners[0]);

    fprintf(stderr, "\n=== WITHOUT FixFlexBasisFitContent ===\n");
    dumpLayout(l2);
    YGNodeFreeRecursive(l2.root);

    // Re-enable for other tests
    YGConfigSetExperimentalFeatureEnabled(
        config, YGExperimentalFeatureFixFlexBasisFitContent, true);

    // Report differences
    fprintf(stderr, "\n=== COMPARISON ===\n");
    fprintf(stderr, "scrollView height: with=%.1f, without=%.1f\n", withFix_scrollH, withoutFix_scrollH);
    fprintf(stderr, "item[0] height:    with=%.1f, without=%.1f\n", withFix_item0H, withoutFix_item0H);
    fprintf(stderr, "item[0] top:       with=%.1f, without=%.1f\n", withFix_item0Top, withoutFix_item0Top);
    fprintf(stderr, "inner[0] height:   with=%.1f, without=%.1f\n", withFix_inner0H, withoutFix_inner0H);
    fprintf(stderr, "inner[0] top:      with=%.1f, without=%.1f\n", withFix_inner0Top, withoutFix_inner0Top);

    if (withFix_scrollH != withoutFix_scrollH ||
        withFix_item0H != withoutFix_item0H ||
        withFix_inner0Top != withoutFix_inner0Top) {
        fprintf(stderr, "*** LAYOUTS DIFFER — FixFlexBasisFitContent causes regression ***\n");
    } else {
        fprintf(stderr, "*** LAYOUTS IDENTICAL — issue is not FixFlexBasisFitContent ***\n");
    }
}

// Compare Yoga 2.x layout with experimental features fully disabled
TEST_F(StyleCarouselLayoutTest, compareWithAllExperimentalFeaturesDisabled) {
    // WITH experiments
    auto l1 = buildCarousel();
    YGNodeCalculateLayout(l1.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    fprintf(stderr, "\n=== ALL EXPERIMENTS ON ===\n");
    dumpLayout(l1);

    float with_item0H = YGNodeLayoutGetHeight(l1.styleItems[0]);
    float with_inner0Top = YGNodeLayoutGetTop(l1.styleInners[0]);
    float with_toolbarTop = YGNodeLayoutGetTop(l1.toolbarItems[0]);
    YGNodeFreeRecursive(l1.root);

    // WITHOUT experiments
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, false);
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, false);

    auto l2 = buildCarousel();
    YGNodeCalculateLayout(l2.root, kScreenWidth, kScreenHeight, YGDirectionLTR);
    fprintf(stderr, "\n=== ALL EXPERIMENTS OFF ===\n");
    dumpLayout(l2);

    float without_item0H = YGNodeLayoutGetHeight(l2.styleItems[0]);
    float without_inner0Top = YGNodeLayoutGetTop(l2.styleInners[0]);
    float without_toolbarTop = YGNodeLayoutGetTop(l2.toolbarItems[0]);
    YGNodeFreeRecursive(l2.root);

    // Restore
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, true);

    fprintf(stderr, "\n=== COMPARISON ===\n");
    fprintf(stderr, "item[0] height:    with=%.1f, without=%.1f\n", with_item0H, without_item0H);
    fprintf(stderr, "inner[0] top:      with=%.1f, without=%.1f\n", with_inner0Top, without_inner0Top);
    fprintf(stderr, "toolbar[0] top:    with=%.1f, without=%.1f\n", with_toolbarTop, without_toolbarTop);
}

// Test with deeper nesting inside style items (auto-height intermediate view):
//   tappableContainer (w:60, h:100%, column, alignItems:center, justifyContent:center)
//     └── container (w:100%, h:55%)
//         └── autoHeightOuter (column, justifyContent:center, NO explicit size)
//             └── textViewWrapper (h:100%)
TEST_F(StyleCarouselLayoutTest, withDeepNestedAutoHeightView) {
    // Build carousel but with deeper nesting in style items
    auto l = buildCarousel();

    YGNodeRef captionOuters[5];
    YGNodeRef textWrappers[5];

    for (int i = 0; i < kStyleItemCount; i++) {
        captionOuters[i] = makeNode();
        YGNodeStyleSetJustifyContent(captionOuters[i], YGJustifyCenter);

        textWrappers[i] = makeNode();
        YGNodeStyleSetHeightPercent(textWrappers[i], 100);

        YGNodeInsertChild(captionOuters[i], textWrappers[i], 0);
        YGNodeInsertChild(l.styleInners[i], captionOuters[i], 0);
    }

    YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);

    fprintf(stderr, "\n=== With Deep Nested Auto-Height View ===\n");
    dumpLayout(l);

    for (int i = 0; i < kStyleItemCount; i++) {
        float innerH = YGNodeLayoutGetHeight(l.styleInners[i]);
        float outerH = YGNodeLayoutGetHeight(captionOuters[i]);
        float outerTop = YGNodeLayoutGetTop(captionOuters[i]);
        float wrapperH = YGNodeLayoutGetHeight(textWrappers[i]);
        float wrapperTop = YGNodeLayoutGetTop(textWrappers[i]);

        fprintf(stderr, "item[%d]: inner=%.1f, captionOuter=(top=%.1f, h=%.1f), wrapper=(top=%.1f, h=%.1f)\n",
                i, innerH, outerTop, outerH, wrapperTop, wrapperH);

        EXPECT_GT(outerH, 0)
            << "Auto-height outer view " << i << " should have non-zero height";
        EXPECT_GT(wrapperH, 0)
            << "Text wrapper " << i << " should have non-zero height";
    }

    YGNodeFreeRecursive(l.root);
}

// Same test but also comparing with/without experimental features
TEST_F(StyleCarouselLayoutTest, deepNestingExperimentComparison) {
    auto buildAndMeasure = [&](bool enableExperiments) {
        if (!enableExperiments) {
            YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, false);
            YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, false);
        }

        auto l = buildCarousel();
        YGNodeRef captionOuters[5];
        YGNodeRef textWrappers[5];

        for (int i = 0; i < kStyleItemCount; i++) {
            captionOuters[i] = makeNode();
            YGNodeStyleSetJustifyContent(captionOuters[i], YGJustifyCenter);
            textWrappers[i] = makeNode();
            YGNodeStyleSetHeightPercent(textWrappers[i], 100);
            YGNodeInsertChild(captionOuters[i], textWrappers[i], 0);
            YGNodeInsertChild(l.styleInners[i], captionOuters[i], 0);
        }

        YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);

        float itemH = YGNodeLayoutGetHeight(l.styleItems[0]);
        float innerH = YGNodeLayoutGetHeight(l.styleInners[0]);
        float outerH = YGNodeLayoutGetHeight(captionOuters[0]);
        float outerTop = YGNodeLayoutGetTop(captionOuters[0]);
        float wrapperH = YGNodeLayoutGetHeight(textWrappers[0]);

        fprintf(stderr, "%s: item=%.1f inner=%.1f captionOuter=(top=%.1f,h=%.1f) wrapper=%.1f\n",
                enableExperiments ? "WITH experiments" : "WITHOUT experiments",
                itemH, innerH, outerTop, outerH, wrapperH);

        YGNodeFreeRecursive(l.root);

        if (!enableExperiments) {
            YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
            YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, true);
        }
    };

    fprintf(stderr, "\n=== Deep Nesting Experiment Comparison ===\n");
    buildAndMeasure(true);
    buildAndMeasure(false);
}

// Isolate which experimental feature causes the regression
TEST_F(StyleCarouselLayoutTest, isolateRegressionToExperiment) {
    auto buildAndMeasure = [&](const char* label) {
        auto l = buildCarousel();
        YGNodeRef captionOuters[5];
        YGNodeRef textWrappers[5];

        for (int i = 0; i < kStyleItemCount; i++) {
            captionOuters[i] = makeNode();
            YGNodeStyleSetJustifyContent(captionOuters[i], YGJustifyCenter);
            textWrappers[i] = makeNode();
            YGNodeStyleSetHeightPercent(textWrappers[i], 100);
            YGNodeInsertChild(captionOuters[i], textWrappers[i], 0);
            YGNodeInsertChild(l.styleInners[i], captionOuters[i], 0);
        }

        YGNodeCalculateLayout(l.root, kScreenWidth, kScreenHeight, YGDirectionLTR);

        float outerH = YGNodeLayoutGetHeight(captionOuters[0]);
        float wrapperH = YGNodeLayoutGetHeight(textWrappers[0]);
        fprintf(stderr, "%s: captionOuter.h=%.1f wrapper.h=%.1f %s\n",
                label, outerH, wrapperH,
                outerH == 0 ? "*** BUG ***" : "OK");
        YGNodeFreeRecursive(l.root);
    };

    // Both on
    fprintf(stderr, "\n=== Isolating Experiment ===\n");
    buildAndMeasure("WebFlexBasis=ON  FixFlexBasis=ON ");

    // Only WebFlexBasis
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, false);
    buildAndMeasure("WebFlexBasis=ON  FixFlexBasis=OFF");

    // Only FixFlexBasisFitContent
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, false);
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, true);
    buildAndMeasure("WebFlexBasis=OFF FixFlexBasis=ON ");

    // Both off
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, false);
    buildAndMeasure("WebFlexBasis=OFF FixFlexBasis=OFF");

    // Restore
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, true);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(StyleCarouselLayoutTest);
