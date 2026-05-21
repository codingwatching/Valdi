import { Component } from 'valdi_core/src/Component';
import { testFont } from './TestFont';

const styles = {
  root: {
    width: '100%' as const,
    height: '100%' as const,
    backgroundColor: '#FFFFFF',
    flexDirection: 'column' as const,
    padding: 8,
  },
  sectionLabel: { color: '#666666' },
  section: { marginBottom: 6 },
  row: { flexDirection: 'row' as const, height: 30 },
  cell: { borderRadius: 4 },
};

export class MinMaxConstraints extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="maxWidth stops grow" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section}>
        <view flexGrow={1} maxWidth={60} backgroundColor="#2196F3" marginRight={4} {...styles.cell} />
        <view flexGrow={1} maxWidth={60} backgroundColor="#4CAF50" marginRight={4} {...styles.cell} />
        <view flexGrow={1} backgroundColor="#FF9800" {...styles.cell} />
      </layout>

      <label value="minWidth stops shrink" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section}>
        <view flexShrink={1} minWidth={80} width={200} backgroundColor="#E91E63" marginRight={4} {...styles.cell} />
        <view flexShrink={1} minWidth={40} width={200} backgroundColor="#9C27B0" {...styles.cell} />
      </layout>

      <label value="minHeight expands container" font={testFont(9)} {...styles.sectionLabel} />
      <view minHeight={50} backgroundColor="#E3F2FD" borderRadius={4} padding={4} {...styles.section}>
        <label value="short" font={testFont(9)} color="#333333" />
      </view>

      <label value="maxHeight + flex-basis" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="column" flexGrow={1} {...styles.section}>
        <view flexGrow={1} maxHeight={25} backgroundColor="#00BCD4" marginBottom={4} {...styles.cell} />
        <view flexGrow={2} maxHeight={25} backgroundColor="#FF5722" marginBottom={4} {...styles.cell} />
        <view flexGrow={3} backgroundColor="#8BC34A" {...styles.cell} />
      </layout>

      <label value="min/max width + %" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row}>
        <view width={'60%'} maxWidth={80} backgroundColor="#795548" marginRight={4} {...styles.cell} />
        <view width={'20%'} minWidth={60} backgroundColor="#607D8B" {...styles.cell} />
      </layout>
    </view>;
  }
}
