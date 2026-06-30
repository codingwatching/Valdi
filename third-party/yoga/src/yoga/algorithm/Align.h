/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <yoga/Yoga.h>

#include <yoga/algorithm/FlexDirection.h>
#include <yoga/node/Node.h>

namespace facebook::yoga {

inline Align resolveChildAlignment(
    const yoga::Node* node,
    const yoga::Node* child) {
  const Align align = child->style().alignSelf() == Align::Auto
      ? node->style().alignItems()
      : child->style().alignSelf();

  if (node->style().display() == Display::Flex && align == Align::Baseline &&
      isColumn(node->style().flexDirection())) {
    return Align::FlexStart;
  }

  return align;
}

inline Justify resolveChildJustification(
    const yoga::Node* node,
    const yoga::Node* child) {
  return child->style().justifySelf() == Justify::Auto
      ? node->style().justifyItems()
      : child->style().justifySelf();
}

/**
 * Fallback alignment to use on overflow
 * https://www.w3.org/TR/css-align-3/#distribution-values
 */
constexpr Align fallbackAlignment(Align align) {
  switch (align) {
      // Fallback to flex-start
    case Align::SpaceBetween:
    case Align::Stretch:
      return Align::FlexStart;

    // Fallback to safe center. TODO (T208209388): This should be aligned to
    // Start instead of FlexStart (for row-reverse containers)
    case Align::SpaceAround:
    case Align::SpaceEvenly:
      return Align::FlexStart;
    default:
      return align;
  }
}

/**
 * Fallback alignment to use on overflow
 * https://www.w3.org/TR/css-align-3/#distribution-values
 *
 * Yoga 1.x clamped space-between to max(0, remaining) on overflow,
 * equivalent to falling back to FlexStart. But space-around and
 * space-evenly were NOT clamped — they produced negative spacing,
 * causing items to overlap. Preserve that mixed behavior so existing
 * layouts are not disrupted by the Yoga 2.x upgrade.
 */
constexpr Justify fallbackAlignment(Justify align) {
  switch (align) {
    case Justify::SpaceBetween:
      return Justify::FlexStart;
    case Justify::SpaceAround:
    case Justify::SpaceEvenly:
      return align;
    default:
      return align;
  }
}

} // namespace facebook::yoga
