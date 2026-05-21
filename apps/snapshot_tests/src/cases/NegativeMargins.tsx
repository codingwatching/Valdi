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
  section: { marginBottom: 8 },
  card: {
    width: 60,
    height: 40,
    borderRadius: 8,
    borderWidth: 2,
    borderColor: '#FFFFFF',
  },
};

export class NegativeMargins extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="overlapping cards" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="row" {...styles.section} height={44}>
        <view {...styles.card} backgroundColor="#2196F3" />
        <view {...styles.card} backgroundColor="#4CAF50" marginLeft={-16} />
        <view {...styles.card} backgroundColor="#FF9800" marginLeft={-16} />
        <view {...styles.card} backgroundColor="#E91E63" marginLeft={-16} />
      </layout>

      <label value="negative top margin (pull up)" font={testFont(9)} {...styles.sectionLabel} />
      <view {...styles.section}>
        <view height={30} backgroundColor="#E3F2FD" borderRadius={4} />
        <view height={20} width={80} backgroundColor="#1565C0" borderRadius={4} marginTop={-10} marginLeft={10} />
      </view>

      <label value="negative margin + padding" font={testFont(9)} {...styles.sectionLabel} />
      <view backgroundColor="#F5F5F5" borderRadius={4} padding={12} {...styles.section}>
        <view height={25} backgroundColor="#9C27B0" borderRadius={4} marginLeft={-8} marginRight={-8} />
      </view>

      <label value="negative margin in wrap" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="row" flexWrap="wrap" flexGrow={1}>
        <view width={55} height={30} backgroundColor="#FFF3E0" borderRadius={4} margin={2}>
          <view width={30} height={20} backgroundColor="#FF5722" borderRadius={4} marginTop={-6} />
        </view>
        <view width={55} height={30} backgroundColor="#E8F5E9" borderRadius={4} margin={2}>
          <view width={30} height={20} backgroundColor="#4CAF50" borderRadius={4} marginTop={-6} />
        </view>
        <view width={55} height={30} backgroundColor="#E3F2FD" borderRadius={4} margin={2}>
          <view width={30} height={20} backgroundColor="#2196F3" borderRadius={4} marginTop={-6} />
        </view>
      </layout>
    </view>;
  }
}
