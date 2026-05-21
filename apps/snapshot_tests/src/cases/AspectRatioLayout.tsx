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
  row: { flexDirection: 'row' as const },
};

export class AspectRatioLayout extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="1:1 squares in row" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section} height={50}>
        <view aspectRatio={1} backgroundColor="#2196F3" marginRight={4} borderRadius={4} />
        <view aspectRatio={1} backgroundColor="#4CAF50" marginRight={4} borderRadius={4} />
        <view aspectRatio={1} backgroundColor="#FF9800" borderRadius={4} />
      </layout>

      <label value="16:9 + flex-grow" font={testFont(9)} {...styles.sectionLabel} />
      <view {...styles.section}>
        <view width={'100%'} aspectRatio={16 / 9} backgroundColor="#E91E63" borderRadius={4} />
      </view>

      <label value="mixed ratios in row" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section} height={60}>
        <view aspectRatio={1} backgroundColor="#9C27B0" marginRight={4} borderRadius={4} />
        <view aspectRatio={2} backgroundColor="#00BCD4" marginRight={4} borderRadius={4} />
        <view aspectRatio={0.5} backgroundColor="#FF5722" borderRadius={4} />
      </layout>

      <label value="aspect + flexGrow" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} flexGrow={1}>
        <view flexGrow={1} aspectRatio={1} backgroundColor="#3F51B5" marginRight={4} borderRadius={8} />
        <view flexGrow={2} aspectRatio={1} backgroundColor="#009688" borderRadius={8} />
      </layout>
    </view>;
  }
}
