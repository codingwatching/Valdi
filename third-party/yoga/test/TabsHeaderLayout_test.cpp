#include <yoga/Yoga.h>
#include <yoga/YGEnums.h>
#include <yoga/YGNode.h>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>

// Tests tab bar label layout under Yoga 2.x: percentage-width flex children
// inside a horizontal scroll view should resolve to non-zero widths.
//
// The tab bar uses percentage-width items (e.g. 50% each for 2 tabs) inside
// a horizontal scroll view with space-evenly justification. Under Yoga 2.x,
// these percentage-width children may resolve to zero if the containing block
// or available width changes.

struct MeasureRecord {
    float width;
    YGMeasureMode widthMode;
    float height;
    YGMeasureMode heightMode;
};

static thread_local std::vector<MeasureRecord> g_measureRecords;

// Simulates a label with "AvenirNext-DemiBold 14" measuring "Stories"
static YGSize labelMeasure(
    YGNodeConstRef /*node*/,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode) {
    g_measureRecords.push_back({width, widthMode, height, heightMode});
    return {76.7f, 19.3f};
}

static YGSize labelMeasure2(
    YGNodeConstRef /*node*/,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode) {
    g_measureRecords.push_back({width, widthMode, height, heightMode});
    return {84.3f, 19.3f};
}

class TabBarLayoutTest : public ::testing::Test {
protected:
    YGConfigRef config;

    void SetUp() override {
        config = YGConfigNew();
        YGConfigSetExperimentalFeatureEnabled(
            config, YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetExperimentalFeatureEnabled(
            config, YGExperimentalFeatureFixFlexBasisFitContent, true);
        YGConfigSetPointScaleFactor(config, 3.0f);
        g_measureRecords.clear();
    }

    void TearDown() override { YGConfigFree(config); }
    YGNodeRef makeNode() { return YGNodeNewWithConfig(config); }

    struct TabBarLayout {
        YGNodeRef root;        // outer container
        YGNodeRef scrollView;  // ScrollView (height:100%, width:100%, horizontal)
        YGNodeRef itemContainer; // Layout (flexDir:row, minWidth:100%, justifyContent:space-evenly)
        YGNodeRef item0;       // View (width:50% for 2 items)
        YGNodeRef label0;      // Label (width:100%, measure func)
        YGNodeRef item1;       // View (width:50% for 2 items)
        YGNodeRef label1;      // Label (width:100%, measure func)
        YGNodeRef highlightBar; // View (position:absolute)
    };

    TabBarLayout buildTabBar(int numItems) {
        TabBarLayout l;

        l.root = makeNode();

        // ScrollView: height:100%, width:100%, horizontal
        // In Valdi, horizontal scroll sets flexDirection:row on the content node
        l.scrollView = makeNode();
        YGNodeStyleSetOverflow(l.scrollView, YGOverflowScroll);
        YGNodeStyleSetFlexDirection(l.scrollView, YGFlexDirectionRow);
        YGNodeStyleSetHeightPercent(l.scrollView, 100);
        YGNodeStyleSetWidthPercent(l.scrollView, 100);

        // itemContainer: flexDir:row, min-width:100%, justify-content:space-evenly
        l.itemContainer = makeNode();
        YGNodeStyleSetFlexDirection(l.itemContainer, YGFlexDirectionRow);
        YGNodeStyleSetMinWidthPercent(l.itemContainer, 100);
        YGNodeStyleSetJustifyContent(l.itemContainer, YGJustifySpaceEvenly);
        YGNodeStyleSetPadding(l.itemContainer, YGEdgeBottom, 2);

        // item0: width = (100/numItems)%, justify-content:center
        float pct = 100.0f / numItems;
        l.item0 = makeNode();
        YGNodeStyleSetWidthPercent(l.item0, pct);
        YGNodeStyleSetJustifyContent(l.item0, YGJustifyCenter);

        // label0: width:100%, leaf with measure func
        l.label0 = makeNode();
        YGNodeStyleSetWidthPercent(l.label0, 100);
        YGNodeSetMeasureFunc(l.label0, labelMeasure);

        // item1
        l.item1 = makeNode();
        YGNodeStyleSetWidthPercent(l.item1, pct);
        YGNodeStyleSetJustifyContent(l.item1, YGJustifyCenter);

        // label1
        l.label1 = makeNode();
        YGNodeStyleSetWidthPercent(l.label1, 100);
        YGNodeSetMeasureFunc(l.label1, labelMeasure2);

        // highlightBar: position:absolute
        l.highlightBar = makeNode();
        YGNodeStyleSetPositionType(l.highlightBar, YGPositionTypeAbsolute);
        YGNodeStyleSetHeight(l.highlightBar, 1.66f);
        YGNodeStyleSetPosition(l.highlightBar, YGEdgeLeft, 0);
        YGNodeStyleSetPosition(l.highlightBar, YGEdgeBottom, 0);

        // Assemble
        YGNodeInsertChild(l.item0, l.label0, 0);
        YGNodeInsertChild(l.item1, l.label1, 0);
        YGNodeInsertChild(l.itemContainer, l.item0, 0);
        YGNodeInsertChild(l.itemContainer, l.item1, 1);
        YGNodeInsertChild(l.scrollView, l.itemContainer, 0);
        YGNodeInsertChild(l.scrollView, l.highlightBar, 1);
        YGNodeInsertChild(l.root, l.scrollView, 0);

        return l;
    }
};

// Core test: labels inside percentage-width flex children should have
// non-zero width after layout. If they don't, the tab labels are invisible.
TEST_F(TabBarLayoutTest, tabLabelsGetNonZeroWidth) {
    auto l = buildTabBar(2);
    g_measureRecords.clear();

    // Parent container size: iPhone 14 Pro (393×44 for tab bar area)
    YGNodeCalculateLayout(l.root, 393, 44, YGDirectionLTR);

    float label0Width = YGNodeLayoutGetWidth(l.label0);
    float label1Width = YGNodeLayoutGetWidth(l.label1);

    // Dump measure constraints for debugging
    for (size_t i = 0; i < g_measureRecords.size(); i++) {
        const auto& r = g_measureRecords[i];
        const char* wMode = r.widthMode == YGMeasureModeExactly ? "Exactly" :
                           r.widthMode == YGMeasureModeAtMost ? "AtMost" : "Undefined";
        const char* hMode = r.heightMode == YGMeasureModeExactly ? "Exactly" :
                           r.heightMode == YGMeasureModeAtMost ? "AtMost" : "Undefined";
        fprintf(stderr, "Measure[%zu]: w=%.1f(%s) h=%.1f(%s)\n",
                i, r.width, wMode, r.height, hMode);
    }

    fprintf(stderr, "label0: %.1f x %.1f\n", label0Width, YGNodeLayoutGetHeight(l.label0));
    fprintf(stderr, "label1: %.1f x %.1f\n", label1Width, YGNodeLayoutGetHeight(l.label1));
    fprintf(stderr, "item0: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.item0), YGNodeLayoutGetHeight(l.item0));
    fprintf(stderr, "itemContainer: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.itemContainer), YGNodeLayoutGetHeight(l.itemContainer));
    fprintf(stderr, "scrollView: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.scrollView), YGNodeLayoutGetHeight(l.scrollView));

    EXPECT_GT(label0Width, 0)
        << "Label 0 has zero width — tab name invisible.";
    EXPECT_GT(label1Width, 0)
        << "Label 1 has zero width — tab name invisible.";
}

// Verify measure function is NOT called with zero-width constraints
TEST_F(TabBarLayoutTest, measureFunctionNeverGetsZeroWidthConstraint) {
    auto l = buildTabBar(2);
    g_measureRecords.clear();

    YGNodeCalculateLayout(l.root, 393, 44, YGDirectionLTR);

    bool gotZeroWidth = false;
    for (const auto& rec : g_measureRecords) {
        if (rec.width == 0.0f &&
            (rec.widthMode == YGMeasureModeAtMost || rec.widthMode == YGMeasureModeExactly)) {
            gotZeroWidth = true;
        }
    }

    EXPECT_FALSE(gotZeroWidth)
        << "Measure called with width=0 — labels will be clamped to zero width.";

    YGNodeFreeRecursive(l.root);
}

// Test with the exact node hierarchy from the runtime:
// The TabBar is inside a StickyHeaderSection which is inside the profile page.
// The profile page has a vertical scroll with the tab bar as a sticky header.
TEST_F(TabBarLayoutTest, tabLabelsInProfileHierarchy) {
    // Profile page root (subscreen root)
    YGNodeRef profileRoot = makeNode();
    YGNodeStyleSetPositionType(profileRoot, YGPositionTypeAbsolute);
    YGNodeStyleSetPosition(profileRoot, YGEdgeTop, 0);
    YGNodeStyleSetPosition(profileRoot, YGEdgeRight, 0);
    YGNodeStyleSetPosition(profileRoot, YGEdgeBottom, 0);
    YGNodeStyleSetPosition(profileRoot, YGEdgeLeft, 0);
    YGNodeStyleSetFlexDirection(profileRoot, YGFlexDirectionColumnReverse);

    YGNodeRef body = makeNode();
    YGNodeStyleSetFlexGrow(body, 1);
    YGNodeStyleSetFlexShrink(body, 1);

    YGNodeRef profileScroll = makeNode();
    YGNodeStyleSetOverflow(profileScroll, YGOverflowScroll);
    YGNodeStyleSetWidthPercent(profileScroll, 100);
    YGNodeStyleSetHeightPercent(profileScroll, 100);

    // StickyHeaderSection container for the tab bar
    YGNodeRef stickyContainer = makeNode();
    YGNodeStyleSetHeight(stickyContainer, 44);

    // tabBarHeader: height:100%, marginBottom:2
    YGNodeRef tabBarHeader = makeNode();
    YGNodeStyleSetHeightPercent(tabBarHeader, 100);
    YGNodeStyleSetMargin(tabBarHeader, YGEdgeBottom, 2);

    // Now build the TabBar inside
    auto l = buildTabBar(2);

    // Assemble profile hierarchy
    YGNodeInsertChild(tabBarHeader, l.root, 0);
    YGNodeInsertChild(stickyContainer, tabBarHeader, 0);
    YGNodeInsertChild(profileScroll, stickyContainer, 0);
    YGNodeInsertChild(body, profileScroll, 0);
    YGNodeInsertChild(profileRoot, body, 0);

    g_measureRecords.clear();
    YGNodeCalculateLayout(profileRoot, 393, 852, YGDirectionLTR);

    float label0Width = YGNodeLayoutGetWidth(l.label0);
    float label1Width = YGNodeLayoutGetWidth(l.label1);

    fprintf(stderr, "\n=== Profile hierarchy ===\n");
    fprintf(stderr, "label0: %.1f x %.1f\n", label0Width, YGNodeLayoutGetHeight(l.label0));
    fprintf(stderr, "label1: %.1f x %.1f\n", label1Width, YGNodeLayoutGetHeight(l.label1));
    fprintf(stderr, "item0: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.item0), YGNodeLayoutGetHeight(l.item0));
    fprintf(stderr, "scrollView: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.scrollView), YGNodeLayoutGetHeight(l.scrollView));
    for (size_t i = 0; i < g_measureRecords.size(); i++) {
        const auto& r = g_measureRecords[i];
        const char* wMode = r.widthMode == YGMeasureModeExactly ? "Exactly" :
                           r.widthMode == YGMeasureModeAtMost ? "AtMost" : "Undefined";
        fprintf(stderr, "Measure[%zu]: w=%.1f(%s)\n", i, r.width, wMode);
    }

    EXPECT_GT(label0Width, 0)
        << "In full profile hierarchy, label 0 has zero width.";
    EXPECT_GT(label1Width, 0)
        << "In full profile hierarchy, label 1 has zero width.";

    YGNodeFreeRecursive(profileRoot);
}

// Exact hierarchy from the runtime: StickyHeaderSection has a position:absolute
// header wrapper. The TabBar is INSIDE this absolute wrapper.
TEST_F(TabBarLayoutTest, tabLabelsInsideAbsoluteHeader) {
    // Profile page root (subscreen root) - position:absolute, all edges 0
    YGNodeRef profileRoot = makeNode();
    YGNodeStyleSetPositionType(profileRoot, YGPositionTypeAbsolute);
    YGNodeStyleSetPosition(profileRoot, YGEdgeTop, 0);
    YGNodeStyleSetPosition(profileRoot, YGEdgeRight, 0);
    YGNodeStyleSetPosition(profileRoot, YGEdgeBottom, 0);
    YGNodeStyleSetPosition(profileRoot, YGEdgeLeft, 0);

    YGNodeRef bodyOuter = makeNode();
    YGNodeStyleSetFlexGrow(bodyOuter, 1);
    YGNodeStyleSetFlexShrink(bodyOuter, 1);

    YGNodeRef profileScroll = makeNode();
    YGNodeStyleSetOverflow(profileScroll, YGOverflowScroll);
    YGNodeStyleSetWidthPercent(profileScroll, 100);
    YGNodeStyleSetHeightPercent(profileScroll, 100);

    // StickyHeaderSection root (is-layout="false" means View, no explicit size)
    YGNodeRef stickyRoot = makeNode();

    // StickyHeaderSection.body (Layout, width: 100%)
    YGNodeRef stickyBody = makeNode();
    YGNodeStyleSetWidthPercent(stickyBody, 100);
    // body.marginTop is set dynamically to header height; simulate with 44
    YGNodeStyleSetMargin(stickyBody, YGEdgeTop, 44);

    // Some content in the body to give the section height
    YGNodeRef bodyContent = makeNode();
    YGNodeStyleSetHeight(bodyContent, 400);

    // StickyHeaderSection.header (View, position: absolute, width: 100%, top: 0, left: 0)
    YGNodeRef stickyHeader = makeNode();
    YGNodeStyleSetPositionType(stickyHeader, YGPositionTypeAbsolute);
    YGNodeStyleSetWidthPercent(stickyHeader, 100);
    YGNodeStyleSetPosition(stickyHeader, YGEdgeTop, 0);
    YGNodeStyleSetPosition(stickyHeader, YGEdgeLeft, 0);

    // gradient (position: absolute) inside header
    YGNodeRef gradient = makeNode();
    YGNodeStyleSetPositionType(gradient, YGPositionTypeAbsolute);
    YGNodeStyleSetPosition(gradient, YGEdgeTop, 0);  // top: 100% in CSS but simplified
    YGNodeStyleSetWidthPercent(gradient, 100);

    // background (position: absolute) inside header
    YGNodeRef background = makeNode();
    YGNodeStyleSetPositionType(background, YGPositionTypeAbsolute);
    YGNodeStyleSetWidthPercent(background, 100);
    YGNodeStyleSetHeightPercent(background, 100);

    // tabBarHeader wrapper (view with height:100%, marginBottom:2)
    YGNodeRef tabBarHeader = makeNode();
    YGNodeStyleSetHeightPercent(tabBarHeader, 100);
    YGNodeStyleSetMargin(tabBarHeader, YGEdgeBottom, 2);

    // Build the TabBar
    auto l = buildTabBar(2);

    // Assemble StickyHeaderSection
    YGNodeInsertChild(stickyBody, bodyContent, 0);
    YGNodeInsertChild(tabBarHeader, l.root, 0);
    YGNodeInsertChild(stickyHeader, gradient, 0);
    YGNodeInsertChild(stickyHeader, background, 1);
    YGNodeInsertChild(stickyHeader, tabBarHeader, 2);

    YGNodeInsertChild(stickyRoot, stickyBody, 0);
    YGNodeInsertChild(stickyRoot, stickyHeader, 1);

    // Profile hierarchy
    YGNodeInsertChild(profileScroll, stickyRoot, 0);
    YGNodeInsertChild(bodyOuter, profileScroll, 0);
    YGNodeInsertChild(profileRoot, bodyOuter, 0);

    g_measureRecords.clear();
    YGNodeCalculateLayout(profileRoot, 393, 852, YGDirectionLTR);

    float label0Width = YGNodeLayoutGetWidth(l.label0);
    float label1Width = YGNodeLayoutGetWidth(l.label1);

    fprintf(stderr, "\n=== Absolute header hierarchy ===\n");
    fprintf(stderr, "profileRoot: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(profileRoot), YGNodeLayoutGetHeight(profileRoot));
    fprintf(stderr, "profileScroll: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(profileScroll), YGNodeLayoutGetHeight(profileScroll));
    fprintf(stderr, "stickyRoot: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(stickyRoot), YGNodeLayoutGetHeight(stickyRoot));
    fprintf(stderr, "stickyHeader: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(stickyHeader), YGNodeLayoutGetHeight(stickyHeader));
    fprintf(stderr, "tabBarHeader: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(tabBarHeader), YGNodeLayoutGetHeight(tabBarHeader));
    fprintf(stderr, "scrollView: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.scrollView), YGNodeLayoutGetHeight(l.scrollView));
    fprintf(stderr, "itemContainer: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.itemContainer), YGNodeLayoutGetHeight(l.itemContainer));
    fprintf(stderr, "item0: %.1f x %.1f\n",
            YGNodeLayoutGetWidth(l.item0), YGNodeLayoutGetHeight(l.item0));
    fprintf(stderr, "label0: %.1f x %.1f\n", label0Width, YGNodeLayoutGetHeight(l.label0));
    fprintf(stderr, "label1: %.1f x %.1f\n", label1Width, YGNodeLayoutGetHeight(l.label1));

    for (size_t i = 0; i < g_measureRecords.size(); i++) {
        const auto& r = g_measureRecords[i];
        const char* wMode = r.widthMode == YGMeasureModeExactly ? "Exactly" :
                           r.widthMode == YGMeasureModeAtMost ? "AtMost" : "Undefined";
        fprintf(stderr, "Measure[%zu]: w=%.1f(%s) h=%.1f(%s)\n",
                i, r.width, wMode, r.height,
                r.heightMode == YGMeasureModeExactly ? "Exactly" :
                r.heightMode == YGMeasureModeAtMost ? "AtMost" : "Undefined");
    }

    EXPECT_GT(label0Width, 0)
        << "Tab label 0 has zero width inside absolute header.";
    EXPECT_GT(label1Width, 0)
        << "Tab label 1 has zero width inside absolute header.";

    YGNodeFreeRecursive(profileRoot);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TabBarLayoutTest);
