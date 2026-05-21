import { Component } from 'valdi_core/src/Component';
import { testFont } from './TestFont';

const styles = {
  root: {
    width: '100%' as const,
    height: '100%' as const,
    backgroundColor: '#FFFFFF',
    flexDirection: 'column' as const,
    padding: 12,
  },
  sectionLabel: { color: '#666666' },
  row: { flexDirection: 'row' as const, height: 40, marginBottom: 8 },
  cell: { borderRadius: 4 },
  wrapColumn: {
    flexDirection: 'column' as const,
    flexWrap: 'wrap' as const,
    height: 100,
    marginBottom: 8,
  },
  wrapCell: { flexBasis: 40, width: 80, margin: 2, borderRadius: 4 },
  nestedRow: { flexDirection: 'row' as const, flexGrow: 1 },
  nestedCol: { flexDirection: 'column' as const, flexGrow: 1, marginRight: 4 },
  nestedColRight: { flexDirection: 'column' as const, flexGrow: 2 },
};

export class FlexBasisVariants extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="flex-basis: auto" font={testFont(11)} {...styles.sectionLabel} />
      <layout {...styles.row}>
        <view flexBasis={'auto'} flexGrow={1} backgroundColor="#2196F3" marginRight={4} {...styles.cell} />
        <view flexBasis={'auto'} flexGrow={2} backgroundColor="#4CAF50" marginRight={4} {...styles.cell} />
        <view flexBasis={'auto'} flexGrow={1} backgroundColor="#FF9800" {...styles.cell} />
      </layout>

      <label value="flex-basis: fixed (100px)" font={testFont(11)} {...styles.sectionLabel} />
      <layout {...styles.row}>
        <view flexBasis={100} backgroundColor="#E91E63" marginRight={4} {...styles.cell} />
        <view flexGrow={1} backgroundColor="#9C27B0" {...styles.cell} />
      </layout>

      <label value="flex-basis: 0 + flex-grow" font={testFont(11)} {...styles.sectionLabel} />
      <layout {...styles.row}>
        <view flexBasis={0} flexGrow={1} backgroundColor="#00BCD4" marginRight={4} {...styles.cell} />
        <view flexBasis={0} flexGrow={3} backgroundColor="#FF5722" {...styles.cell} />
      </layout>

      <label value="column flex-basis + wrap" font={testFont(11)} {...styles.sectionLabel} />
      <layout {...styles.wrapColumn}>
        <view {...styles.wrapCell} backgroundColor="#795548" />
        <view {...styles.wrapCell} backgroundColor="#607D8B" />
        <view {...styles.wrapCell} backgroundColor="#CDDC39" />
        <view {...styles.wrapCell} backgroundColor="#FF4081" />
      </layout>

      <label value="nested flex containers" font={testFont(11)} {...styles.sectionLabel} />
      <layout {...styles.nestedRow}>
        <layout {...styles.nestedCol}>
          <view flexGrow={1} backgroundColor="#3F51B5" marginBottom={4} {...styles.cell} />
          <view flexGrow={2} backgroundColor="#009688" {...styles.cell} />
        </layout>
        <layout {...styles.nestedColRight}>
          <view flexGrow={2} backgroundColor="#FFC107" marginBottom={4} {...styles.cell} />
          <view flexGrow={1} backgroundColor="#8BC34A" {...styles.cell} />
        </layout>
      </layout>
    </view>;
  }
}
