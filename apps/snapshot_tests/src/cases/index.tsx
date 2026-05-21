export interface SnapshotTestCase {
  name: string;
  width: number;
  height: number;
  render: () => void;
}

import { BasicLayout } from './BasicLayout';
import { TextRendering } from './TextRendering';
import { NestedViews } from './NestedViews';
import { FlexBasisVariants } from './FlexBasisVariants';
import { AbsolutePositioning } from './AbsolutePositioning';
import { CustomMeasure } from './CustomMeasure';
import { MinMaxConstraints } from './MinMaxConstraints';
import { AspectRatioLayout } from './AspectRatioLayout';
import { PercentageSizing } from './PercentageSizing';
import { AlignSelfOverrides } from './AlignSelfOverrides';
import { NegativeMargins } from './NegativeMargins';
import { ReverseDirections } from './ReverseDirections';
import { AlignContentWrap } from './AlignContentWrap';
import { DisplayNone } from './DisplayNone';

export const testCases: SnapshotTestCase[] = [
  { name: 'BasicLayout', width: 200, height: 150, render: () => { <BasicLayout />; } },
  { name: 'TextRendering', width: 200, height: 100, render: () => { <TextRendering />; } },
  { name: 'NestedViews', width: 200, height: 200, render: () => { <NestedViews />; } },
  { name: 'FlexBasisVariants', width: 200, height: 200, render: () => { <FlexBasisVariants />; } },
  { name: 'AbsolutePositioning', width: 200, height: 200, render: () => { <AbsolutePositioning />; } },
  { name: 'CustomMeasure', width: 200, height: 300, render: () => { <CustomMeasure />; } },
  { name: 'MinMaxConstraints', width: 200, height: 220, render: () => { <MinMaxConstraints />; } },
  { name: 'AspectRatioLayout', width: 200, height: 200, render: () => { <AspectRatioLayout />; } },
  { name: 'PercentageSizing', width: 200, height: 230, render: () => { <PercentageSizing />; } },
  { name: 'AlignSelfOverrides', width: 200, height: 220, render: () => { <AlignSelfOverrides />; } },
  { name: 'NegativeMargins', width: 200, height: 220, render: () => { <NegativeMargins />; } },
  { name: 'ReverseDirections', width: 200, height: 230, render: () => { <ReverseDirections />; } },
  { name: 'AlignContentWrap', width: 200, height: 280, render: () => { <AlignContentWrap />; } },
  { name: 'DisplayNone', width: 200, height: 250, render: () => { <DisplayNone />; } },
];
