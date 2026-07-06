#include <yoga/Yoga.h>
#include <yoga/YGEnums.h>
#include <yoga/YGNode.h>
#include <gtest/gtest.h>

// Models the SegmentedProgressBar in batch capture preview (PREVIEW-30429).
//
// Layout hierarchy (from BottomFloatingToolbar down to progress bar):
//   toolbar: column-reverse, alignItems: flex-start, width: 100%, height: 100%
//     thumbnailList container: paddingBottom: 4 (no explicit width!)
//       scroll: overflow: scroll, horizontal (flexDirection: row), no explicit width
//         padding layout: padding: 10
//           rootEntity: width: 70 (stacked), height: 75
//       progressBar: position: absolute, width: 100%, height: 4, flexDirection: row
//         segments (flexGrow: 1) + gaps (width: 5)
//
// The key question: does the thumbnailList container stretch to the toolbar's
// full width, or collapse to the scroll view's content width?
// In Yoga 1.x it may have stretched (bug); in Yoga 2.x with flex-start it
// correctly takes content width, causing the progress bar to compress.

class SegmentedProgressBarLayoutTest : public ::testing::Test {
protected:
    YGConfigRef config;

    void SetUp() override {
        config = YGConfigNew();
        YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetPointScaleFactor(config, 3.0f);
    }

    void TearDown() override { YGConfigFree(config); }
    YGNodeRef makeNode() { return YGNodeNewWithConfig(config); }

    static constexpr float SCREEN_WIDTH = 393.0f;
    static constexpr float SCREEN_HEIGHT = 852.0f;
    static constexpr int NUM_SEGMENTS = 4;
    static constexpr float SEGMENT_GAP_WIDTH = 5.0f;
    static constexpr float PROGRESS_BAR_HEIGHT = 4.0f;
    static constexpr float SEGMENT_HEIGHT = 2.0f;
    static constexpr float PADDING_H = 10.0f;
    static constexpr float THUMBNAIL_WIDTH = 70.0f;
    static constexpr float ENTITY_HEIGHT = 75.0f;
    static constexpr float THUMBNAIL_LIST_PADDING = 10.0f;

    struct Layout {
        YGNodeRef root;
        YGNodeRef toolbar;
        YGNodeRef thumbnailContainer;
        YGNodeRef scroll;
        YGNodeRef paddingLayout;
        YGNodeRef rootEntity;
        YGNodeRef progressBar;
        YGNodeRef segments[NUM_SEGMENTS];
    };

    Layout buildFullHierarchy() {
        Layout l;

        // Root: screen-sized
        l.root = makeNode();
        YGNodeStyleSetWidth(l.root, SCREEN_WIDTH);
        YGNodeStyleSetHeight(l.root, SCREEN_HEIGHT);

        // BottomFloatingToolbar: column-reverse, alignItems: flex-start
        l.toolbar = makeNode();
        YGNodeStyleSetFlexDirection(l.toolbar, YGFlexDirectionColumnReverse);
        YGNodeStyleSetAlignItems(l.toolbar, YGAlignFlexStart);
        YGNodeStyleSetWidthPercent(l.toolbar, 100);
        YGNodeStyleSetHeightPercent(l.toolbar, 100);
        YGNodeStyleSetPosition(l.toolbar, YGEdgeLeft, 0);
        YGNodeInsertChild(l.root, l.toolbar, 0);

        // ThumbnailList container: paddingBottom only, NO explicit width
        l.thumbnailContainer = makeNode();
        YGNodeStyleSetPadding(l.thumbnailContainer, YGEdgeBottom, 4);
        YGNodeInsertChild(l.toolbar, l.thumbnailContainer, 0);

        // Horizontal scroll view
        l.scroll = makeNode();
        YGNodeStyleSetOverflow(l.scroll, YGOverflowScroll);
        YGNodeStyleSetFlexDirection(l.scroll, YGFlexDirectionRow);
        YGNodeStyleSetAlignItems(l.scroll, YGAlignFlexStart);
        YGNodeStyleSetPadding(l.scroll, YGEdgeTop, 2);
        YGNodeStyleSetPadding(l.scroll, YGEdgeRight, 2);
        YGNodeInsertChild(l.thumbnailContainer, l.scroll, 0);

        // Padding layout inside scroll
        l.paddingLayout = makeNode();
        YGNodeStyleSetPadding(l.paddingLayout, YGEdgeAll, THUMBNAIL_LIST_PADDING);
        YGNodeInsertChild(l.scroll, l.paddingLayout, 0);

        // ThumbnailListRootEntity: fixed size (stacked mode)
        l.rootEntity = makeNode();
        YGNodeStyleSetWidth(l.rootEntity, THUMBNAIL_WIDTH);
        YGNodeStyleSetHeight(l.rootEntity, ENTITY_HEIGHT);
        YGNodeInsertChild(l.paddingLayout, l.rootEntity, 0);

        // SegmentedProgressBar: absolute positioned
        l.progressBar = makeNode();
        YGNodeStyleSetPositionType(l.progressBar, YGPositionTypeAbsolute);
        YGNodeStyleSetPosition(l.progressBar, YGEdgeBottom, 0);
        YGNodeStyleSetPosition(l.progressBar, YGEdgeLeft, 0);
        YGNodeStyleSetWidthPercent(l.progressBar, 100);
        YGNodeStyleSetHeight(l.progressBar, PROGRESS_BAR_HEIGHT);
        YGNodeStyleSetFlexDirection(l.progressBar, YGFlexDirectionRow);
        YGNodeStyleSetPadding(l.progressBar, YGEdgeLeft, PADDING_H);
        YGNodeStyleSetPadding(l.progressBar, YGEdgeRight, PADDING_H);
        YGNodeInsertChild(l.thumbnailContainer, l.progressBar, 1);

        // 4 segments with gaps
        for (int i = 0; i < NUM_SEGMENTS; i++) {
            l.segments[i] = makeNode();
            YGNodeStyleSetFlexGrow(l.segments[i], 1);
            YGNodeStyleSetHeight(l.segments[i], SEGMENT_HEIGHT);
            YGNodeInsertChild(l.progressBar, l.segments[i], YGNodeGetChildCount(l.progressBar));

            YGNodeRef gap = makeNode();
            YGNodeStyleSetWidth(gap, SEGMENT_GAP_WIDTH);
            YGNodeInsertChild(l.progressBar, gap, YGNodeGetChildCount(l.progressBar));
        }

        return l;
    }
};

TEST_F(SegmentedProgressBarLayoutTest, ReproducesCompressedProgressBar) {
    auto l = buildFullHierarchy();
    YGNodeCalculateLayout(l.root, SCREEN_WIDTH, SCREEN_HEIGHT, YGDirectionLTR);

    float toolbarWidth = YGNodeLayoutGetWidth(l.toolbar);
    float containerWidth = YGNodeLayoutGetWidth(l.thumbnailContainer);
    float scrollWidth = YGNodeLayoutGetWidth(l.scroll);
    float progressBarWidth = YGNodeLayoutGetWidth(l.progressBar);

    printf("Yoga 2.x (no errata):\n");
    printf("  toolbar width:   %.1f\n", toolbarWidth);
    printf("  container width: %.1f\n", containerWidth);
    printf("  scroll width:    %.1f\n", scrollWidth);
    printf("  progressBar width: %.1f\n", progressBarWidth);

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        float w = YGNodeLayoutGetWidth(l.segments[i]);
        printf("  segment[%d] width: %.1f\n", i, w);
    }

    // The bug: container collapses to content width instead of full toolbar width
    // because alignItems: flex-start doesn't stretch children
    bool isCompressed = containerWidth < SCREEN_WIDTH * 0.5f;
    EXPECT_TRUE(isCompressed) << "Expected container to be compressed to < 50% screen width to confirm the bug exists";
    if (isCompressed) {
        printf("\n  ** BUG CONFIRMED: container is compressed to %.1f (< half screen)\n",
               containerWidth);
    }

    YGNodeFreeRecursive(l.root);
}

TEST_F(SegmentedProgressBarLayoutTest, ClassicErrataComparison) {
    YGConfigSetErrata(config, YGErrataClassic);
    auto l = buildFullHierarchy();
    YGNodeCalculateLayout(l.root, SCREEN_WIDTH, SCREEN_HEIGHT, YGDirectionLTR);

    float toolbarWidth = YGNodeLayoutGetWidth(l.toolbar);
    float containerWidth = YGNodeLayoutGetWidth(l.thumbnailContainer);
    float scrollWidth = YGNodeLayoutGetWidth(l.scroll);
    float progressBarWidth = YGNodeLayoutGetWidth(l.progressBar);

    printf("Yoga 2.x (Classic errata / Yoga 1.x compat):\n");
    printf("  toolbar width:   %.1f\n", toolbarWidth);
    printf("  container width: %.1f\n", containerWidth);
    printf("  scroll width:    %.1f\n", scrollWidth);
    printf("  progressBar width: %.1f\n", progressBarWidth);

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        float w = YGNodeLayoutGetWidth(l.segments[i]);
        printf("  segment[%d] width: %.1f\n", i, w);
    }

    bool isFullWidth = containerWidth > SCREEN_WIDTH * 0.9f;
    if (isFullWidth) {
        printf("\n  ** Classic errata: container stretches to %.1f (full width)\n",
               containerWidth);
    }

    YGNodeFreeRecursive(l.root);
}

TEST_F(SegmentedProgressBarLayoutTest, FixWithAlignSelfStretch) {
    auto l = buildFullHierarchy();

    // Potential fix: add alignSelf: stretch to the ThumbnailList container
    YGNodeStyleSetAlignSelf(l.thumbnailContainer, YGAlignStretch);

    YGNodeCalculateLayout(l.root, SCREEN_WIDTH, SCREEN_HEIGHT, YGDirectionLTR);

    float containerWidth = YGNodeLayoutGetWidth(l.thumbnailContainer);
    float progressBarWidth = YGNodeLayoutGetWidth(l.progressBar);

    printf("With alignSelf: stretch fix:\n");
    printf("  container width: %.1f\n", containerWidth);
    printf("  progressBar width: %.1f\n", progressBarWidth);

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        float w = YGNodeLayoutGetWidth(l.segments[i]);
        printf("  segment[%d] width: %.1f\n", i, w);
    }

    EXPECT_GT(containerWidth, SCREEN_WIDTH * 0.9f)
        << "alignSelf: stretch should make container full width";
    EXPECT_GT(progressBarWidth, SCREEN_WIDTH * 0.9f)
        << "progress bar should be full width with the fix";

    YGNodeFreeRecursive(l.root);
}

TEST_F(SegmentedProgressBarLayoutTest, FixWithExplicitWidth100Percent) {
    auto l = buildFullHierarchy();

    // Alternative fix: add width: 100% to the ThumbnailList container
    YGNodeStyleSetWidthPercent(l.thumbnailContainer, 100);

    YGNodeCalculateLayout(l.root, SCREEN_WIDTH, SCREEN_HEIGHT, YGDirectionLTR);

    float containerWidth = YGNodeLayoutGetWidth(l.thumbnailContainer);
    float progressBarWidth = YGNodeLayoutGetWidth(l.progressBar);

    printf("With width: 100%% fix:\n");
    printf("  container width: %.1f\n", containerWidth);
    printf("  progressBar width: %.1f\n", progressBarWidth);

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        float w = YGNodeLayoutGetWidth(l.segments[i]);
        printf("  segment[%d] width: %.1f\n", i, w);
    }

    EXPECT_GT(containerWidth, SCREEN_WIDTH * 0.9f)
        << "width: 100% should make container full width";
    EXPECT_GT(progressBarWidth, SCREEN_WIDTH * 0.9f)
        << "progress bar should be full width with the fix";

    YGNodeFreeRecursive(l.root);
}
