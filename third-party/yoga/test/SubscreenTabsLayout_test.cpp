#include <yoga/Yoga.h>
#include <yoga/YGEnums.h>
#include <yoga/YGNode.h>
#include <gtest/gtest.h>

// Tests nested subscreen layout where an outer SubscreenTabs wraps tab pages,
// and each page renders its own inner Subscreen. The inner Subscreen root is
// position:absolute with all edges 0 — it depends on its parent for height.
// When the parent (the TabsContent page content wrapper) has no explicit
// height, the absolute child collapses to zero.
//
// A <view width=100% height=100%> wrapper doesn't help: height:100% of a
// zero-height parent is still zero.
//
// The fix: TabsContent must set height on its page content wrapper so that
// percentage-based children can resolve against it.

class SubscreenTabsLayoutTest : public ::testing::Test {
protected:
    YGConfigRef config;

    void SetUp() override {
        config = YGConfigNew();
        YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, true);
        YGConfigSetPointScaleFactor(config, 3.0f);
    }

    void TearDown() override { YGConfigFree(config); }
    YGNodeRef makeNode() { return YGNodeNewWithConfig(config); }

    struct FullLayout {
        YGNodeRef outerRoot, outerBody, outerScroll, tabsContent;
        YGNodeRef horizontalScroll, page, pageContent, navPageWrapper;
        YGNodeRef innerRoot, innerBody, innerScroll, innerContent, innerHeader;
        YGNodeRef outerHeader;
    };

    FullLayout buildLayout(bool focusedPageContentAbsolute) {
        FullLayout l;

        l.outerRoot = makeNode();
        YGNodeStyleSetFlexDirection(l.outerRoot, YGFlexDirectionColumnReverse);
        YGNodeStyleSetPositionType(l.outerRoot, YGPositionTypeAbsolute);
        YGNodeStyleSetPosition(l.outerRoot, YGEdgeTop, 0);
        YGNodeStyleSetPosition(l.outerRoot, YGEdgeRight, 0);
        YGNodeStyleSetPosition(l.outerRoot, YGEdgeBottom, 0);
        YGNodeStyleSetPosition(l.outerRoot, YGEdgeLeft, 0);

        l.outerBody = makeNode();
        YGNodeStyleSetFlexGrow(l.outerBody, 1);
        YGNodeStyleSetFlexShrink(l.outerBody, 1);

        l.outerScroll = makeNode();
        YGNodeStyleSetOverflow(l.outerScroll, YGOverflowScroll);
        YGNodeStyleSetHeightPercent(l.outerScroll, 100);
        YGNodeStyleSetWidthPercent(l.outerScroll, 100);

        l.tabsContent = makeNode();
        YGNodeStyleSetFlexDirection(l.tabsContent, YGFlexDirectionColumnReverse);
        // Runtime sets minHeight = outerScroll height after first layout pass.
        // Pre-set it here to simulate the steady-state.
        YGNodeStyleSetMinHeight(l.tabsContent, 788);

        l.horizontalScroll = makeNode();
        YGNodeStyleSetOverflow(l.horizontalScroll, YGOverflowScroll);
        YGNodeStyleSetFlexGrow(l.horizontalScroll, 1);
        YGNodeStyleSetFlexDirection(l.horizontalScroll, YGFlexDirectionRow);

        // styles.page = { width: '100%' }
        l.page = makeNode();
        YGNodeStyleSetWidthPercent(l.page, 100);

        // styles.content = { width: '100%' }
        // Focused: position=static; Unfocused: position=absolute
        l.pageContent = makeNode();
        YGNodeStyleSetWidthPercent(l.pageContent, 100);
        if (focusedPageContentAbsolute) {
            YGNodeStyleSetPositionType(l.pageContent, YGPositionTypeAbsolute);
        }

        // NavigationPageComponent: <view width=100% height=100%>
        l.navPageWrapper = makeNode();
        YGNodeStyleSetWidthPercent(l.navPageWrapper, 100);
        YGNodeStyleSetHeightPercent(l.navPageWrapper, 100);

        // Inner Subscreen (nested page content)
        l.innerRoot = makeNode();
        YGNodeStyleSetFlexDirection(l.innerRoot, YGFlexDirectionColumnReverse);
        YGNodeStyleSetPositionType(l.innerRoot, YGPositionTypeAbsolute);
        YGNodeStyleSetPosition(l.innerRoot, YGEdgeTop, 0);
        YGNodeStyleSetPosition(l.innerRoot, YGEdgeRight, 0);
        YGNodeStyleSetPosition(l.innerRoot, YGEdgeBottom, 0);
        YGNodeStyleSetPosition(l.innerRoot, YGEdgeLeft, 0);

        l.innerBody = makeNode();
        YGNodeStyleSetFlexGrow(l.innerBody, 1);
        YGNodeStyleSetFlexShrink(l.innerBody, 1);

        l.innerScroll = makeNode();
        YGNodeStyleSetOverflow(l.innerScroll, YGOverflowScroll);
        YGNodeStyleSetHeightPercent(l.innerScroll, 100);
        YGNodeStyleSetWidthPercent(l.innerScroll, 100);

        l.innerContent = makeNode();
        YGNodeStyleSetHeight(l.innerContent, 400);

        l.innerHeader = makeNode();
        YGNodeStyleSetHeight(l.innerHeader, 48);

        l.outerHeader = makeNode();
        YGNodeStyleSetHeight(l.outerHeader, 56);

        // Assemble
        YGNodeInsertChild(l.innerScroll, l.innerContent, 0);
        YGNodeInsertChild(l.innerBody, l.innerScroll, 0);
        YGNodeInsertChild(l.innerRoot, l.innerBody, 0);
        YGNodeInsertChild(l.innerRoot, l.innerHeader, 1);
        YGNodeInsertChild(l.navPageWrapper, l.innerRoot, 0);
        YGNodeInsertChild(l.pageContent, l.navPageWrapper, 0);
        YGNodeInsertChild(l.page, l.pageContent, 0);
        YGNodeInsertChild(l.horizontalScroll, l.page, 0);
        YGNodeInsertChild(l.tabsContent, l.horizontalScroll, 0);
        YGNodeInsertChild(l.outerScroll, l.tabsContent, 0);
        YGNodeInsertChild(l.outerBody, l.outerScroll, 0);
        YGNodeInsertChild(l.outerRoot, l.outerBody, 0);
        YGNodeInsertChild(l.outerRoot, l.outerHeader, 1);

        return l;
    }
};

// The focused tab: pageContent is in normal flow (position=static).
// BUG: pageContent has no explicit height, navPageWrapper's height:100%
// resolves to 0, inner Subscreen collapses.
TEST_F(SubscreenTabsLayoutTest, focusedTabInnerSubscreenGetsHeight) {
    auto l = buildLayout(false);
    YGNodeCalculateLayout(l.outerRoot, 390, 844, YGDirectionLTR);

    EXPECT_FLOAT_EQ(788, YGNodeLayoutGetHeight(l.page));

    EXPECT_GT(YGNodeLayoutGetHeight(l.pageContent), 0)
        << "pageContent collapsed — absolute children have no containing height";

    EXPECT_GT(YGNodeLayoutGetHeight(l.innerRoot), 0)
        << "Inner Subscreen collapsed to zero — blank screen";

    EXPECT_GT(YGNodeLayoutGetHeight(l.innerBody), 0)
        << "Inner body collapsed — content invisible";

    YGNodeFreeRecursive(l.outerRoot);
}

// Verify the fix: setting height=100% on pageContent propagates height down
TEST_F(SubscreenTabsLayoutTest, focusedTabWorksWithHeightOnPageContent) {
    auto l = buildLayout(false);

    // THE FIX: set height=100% on the page content wrapper
    YGNodeStyleSetHeightPercent(l.pageContent, 100);

    YGNodeCalculateLayout(l.outerRoot, 390, 844, YGDirectionLTR);

    EXPECT_FLOAT_EQ(788, YGNodeLayoutGetHeight(l.page));
    EXPECT_FLOAT_EQ(788, YGNodeLayoutGetHeight(l.pageContent));
    EXPECT_FLOAT_EQ(788, YGNodeLayoutGetHeight(l.navPageWrapper));
    EXPECT_FLOAT_EQ(788, YGNodeLayoutGetHeight(l.innerRoot));
    EXPECT_FLOAT_EQ(740, YGNodeLayoutGetHeight(l.innerBody));
    EXPECT_FLOAT_EQ(400, YGNodeLayoutGetHeight(l.innerContent));

    YGNodeFreeRecursive(l.outerRoot);
}

// Same tests but with FixFlexBasisFitContent DISABLED (production default).
TEST_F(SubscreenTabsLayoutTest, focusedTabWithFeatureOff_NoFix) {
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, false);
    auto l = buildLayout(false);

    YGNodeCalculateLayout(l.outerRoot, 390, 844, YGDirectionLTR);

    printf("=== FixFlexBasisFitContent OFF, NO height fix ===\n");
    printf("page height: %f\n", YGNodeLayoutGetHeight(l.page));
    printf("pageContent height: %f\n", YGNodeLayoutGetHeight(l.pageContent));
    printf("navPageWrapper height: %f\n", YGNodeLayoutGetHeight(l.navPageWrapper));
    printf("innerRoot height: %f\n", YGNodeLayoutGetHeight(l.innerRoot));
    printf("innerBody height: %f\n", YGNodeLayoutGetHeight(l.innerBody));

    YGNodeFreeRecursive(l.outerRoot);
}

TEST_F(SubscreenTabsLayoutTest, focusedTabWithFeatureOff_WithFix) {
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureFixFlexBasisFitContent, false);
    auto l = buildLayout(false);

    // Apply the fix
    YGNodeStyleSetHeightPercent(l.pageContent, 100);

    YGNodeCalculateLayout(l.outerRoot, 390, 844, YGDirectionLTR);

    printf("=== FixFlexBasisFitContent OFF, WITH height fix ===\n");
    printf("page height: %f\n", YGNodeLayoutGetHeight(l.page));
    printf("pageContent height: %f\n", YGNodeLayoutGetHeight(l.pageContent));
    printf("navPageWrapper height: %f\n", YGNodeLayoutGetHeight(l.navPageWrapper));
    printf("innerRoot height: %f\n", YGNodeLayoutGetHeight(l.innerRoot));
    printf("innerBody height: %f\n", YGNodeLayoutGetHeight(l.innerBody));

    EXPECT_GT(YGNodeLayoutGetHeight(l.page), 0) << "page collapsed";
    EXPECT_GT(YGNodeLayoutGetHeight(l.pageContent), 0) << "pageContent collapsed";
    EXPECT_GT(YGNodeLayoutGetHeight(l.innerRoot), 0) << "innerRoot collapsed — blank screen";

    YGNodeFreeRecursive(l.outerRoot);
}

// Models the music picker layout (MUSIC-12367 / MUSIC-12365).
// TabsContent column-reverse with enableStickySlot=true, where page content
// is tall (song list) and has intrinsic height rather than absolute children.
//
// The sticky header (tab bar: Featured / My Favorites / Trending) must remain
// at y >= 0 inside the column-reverse container. A regression caused the
// header to land at y < 0 (clipped above the scroll viewport).
class MusicPickerTabsLayoutTest : public ::testing::Test {
protected:
    YGConfigRef config;

    void SetUp() override {
        config = YGConfigNew();
        YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetPointScaleFactor(config, 3.0f);
    }

    void TearDown() override { YGConfigFree(config); }
    YGNodeRef makeNode() { return YGNodeNewWithConfig(config); }

    struct Layout {
        YGNodeRef outerScroll, tabsContent;
        YGNodeRef horizontalScroll, page, pageContent;
        YGNodeRef musicContent;
        YGNodeRef stickyHeader;
    };

    Layout buildLayout(float viewportHeight, float contentHeight,
                       float stickyHeight) {
        Layout l;

        // LayoutRoot outer vertical scroll
        l.outerScroll = makeNode();
        YGNodeStyleSetOverflow(l.outerScroll, YGOverflowScroll);
        YGNodeStyleSetWidth(l.outerScroll, 390);
        YGNodeStyleSetHeight(l.outerScroll, viewportHeight);

        // TabsContent: column-reverse, minHeight = viewport
        l.tabsContent = makeNode();
        YGNodeStyleSetFlexDirection(l.tabsContent, YGFlexDirectionColumnReverse);
        YGNodeStyleSetMinHeight(l.tabsContent, viewportHeight);

        // Horizontal paging scroll: flexGrow:1, horizontal, overflow:scroll
        l.horizontalScroll = makeNode();
        YGNodeStyleSetOverflow(l.horizontalScroll, YGOverflowScroll);
        YGNodeStyleSetFlexGrow(l.horizontalScroll, 1);
        YGNodeStyleSetFlexDirection(l.horizontalScroll, YGFlexDirectionRow);

        // styles.page = { width: '100%' }
        l.page = makeNode();
        YGNodeStyleSetWidthPercent(l.page, 100);

        // styles.content = { width: '100%' }
        // Focused tab: position=static, no explicit height (original)
        l.pageContent = makeNode();
        YGNodeStyleSetWidthPercent(l.pageContent, 100);

        // Music song list — tall intrinsic content
        l.musicContent = makeNode();
        YGNodeStyleSetHeight(l.musicContent, contentHeight);

        // Sticky header (tab bar: Featured / My Favorites / Trending)
        l.stickyHeader = makeNode();
        YGNodeStyleSetHeight(l.stickyHeader, stickyHeight);

        // Assemble
        YGNodeInsertChild(l.pageContent, l.musicContent, 0);
        YGNodeInsertChild(l.page, l.pageContent, 0);
        YGNodeInsertChild(l.horizontalScroll, l.page, 0);
        YGNodeInsertChild(l.tabsContent, l.horizontalScroll, 0);
        YGNodeInsertChild(l.tabsContent, l.stickyHeader, 1);
        YGNodeInsertChild(l.outerScroll, l.tabsContent, 0);

        return l;
    }
};

// Baseline: without height='100%' on pageContent, sticky header is visible.
TEST_F(MusicPickerTabsLayoutTest, stickyHeaderVisibleWithoutHeightFix) {
    auto l = buildLayout(536.667f, 1739.f, 39.f);
    YGNodeCalculateLayout(l.outerScroll, 390, 536.667f, YGDirectionLTR);

    float stickyTop = YGNodeLayoutGetTop(l.stickyHeader);
    float stickyHeight = YGNodeLayoutGetHeight(l.stickyHeader);
    float scrollTop = YGNodeLayoutGetTop(l.horizontalScroll);

    EXPECT_GE(stickyTop, 0)
        << "Sticky header at y=" << stickyTop
        << " — tab bar clipped above viewport (MUSIC-12367)";
    EXPECT_FLOAT_EQ(39, stickyHeight);
    EXPECT_GE(scrollTop, stickyHeight)
        << "Horizontal scroll should start below the sticky header";

    YGNodeFreeRecursive(l.outerScroll);
}

// Regression: height='100%' on focused pageContent pushes sticky to y < 0.
TEST_F(MusicPickerTabsLayoutTest, stickyHeaderWithHeightPercentOnContent) {
    auto l = buildLayout(536.667f, 1739.f, 39.f);

    // Apply the LENS-54799 fix that caused the regression
    YGNodeStyleSetHeightPercent(l.pageContent, 100);

    YGNodeCalculateLayout(l.outerScroll, 390, 536.667f, YGDirectionLTR);

    float stickyTop = YGNodeLayoutGetTop(l.stickyHeader);

    // Document the regression: height='100%' causes sticky to go negative
    printf("=== height='100%%' on pageContent ===\n");
    printf("stickyHeader top: %f\n", stickyTop);
    printf("stickyHeader height: %f\n", YGNodeLayoutGetHeight(l.stickyHeader));
    printf("horizontalScroll top: %f\n", YGNodeLayoutGetTop(l.horizontalScroll));
    printf("horizontalScroll height: %f\n", YGNodeLayoutGetHeight(l.horizontalScroll));
    printf("page height: %f\n", YGNodeLayoutGetHeight(l.page));
    printf("pageContent height: %f\n", YGNodeLayoutGetHeight(l.pageContent));
    printf("tabsContent height: %f\n", YGNodeLayoutGetHeight(l.tabsContent));

    YGNodeFreeRecursive(l.outerScroll);
}

// minHeight='100%' on pageContent — same regression as height='100%'.
TEST_F(MusicPickerTabsLayoutTest, stickyHeaderWithMinHeightPercentOnContent) {
    auto l = buildLayout(536.667f, 1739.f, 39.f);

    YGNodeStyleSetMinHeightPercent(l.pageContent, 100);

    YGNodeCalculateLayout(l.outerScroll, 390, 536.667f, YGDirectionLTR);

    float stickyTop = YGNodeLayoutGetTop(l.stickyHeader);

    printf("=== minHeight='100%%' on pageContent ===\n");
    printf("stickyHeader top: %f\n", stickyTop);
    printf("horizontalScroll top: %f\n", YGNodeLayoutGetTop(l.horizontalScroll));
    printf("horizontalScroll height: %f\n", YGNodeLayoutGetHeight(l.horizontalScroll));
    printf("tabsContent height: %f\n", YGNodeLayoutGetHeight(l.tabsContent));

    YGNodeFreeRecursive(l.outerScroll);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SubscreenTabsLayoutTest);
