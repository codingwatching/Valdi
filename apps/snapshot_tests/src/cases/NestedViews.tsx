import { Component } from 'valdi_core/src/Component';
import { testBoldFont } from './TestFont';

const styles = {
  root: {
    width: '100%' as const,
    height: '100%' as const,
    backgroundColor: '#F0F0F0',
    padding: 16,
  },
  card: {
    flexGrow: 1,
    backgroundColor: '#FFFFFF',
    borderRadius: 16,
    borderWidth: 2,
    borderColor: '#CCCCCC',
    padding: 16,
    flexDirection: 'column' as const,
  },
  header: {
    height: 80,
    backgroundColor: '#E8F0FE',
    borderRadius: 12,
    padding: 12,
    flexDirection: 'row' as const,
    alignItems: 'center' as const,
  },
  avatar: { width: 56, height: 56, backgroundColor: '#4285F4', borderRadius: '50%' as const },
  titleColumn: { flexDirection: 'column' as const, flexGrow: 1 },
  body: {
    flexGrow: 1,
    backgroundColor: '#FFF3E0',
    borderRadius: 8,
    padding: 12,
    flexDirection: 'column' as const,
  },
  bodyRow: {
    flexDirection: 'row' as const,
    justifyContent: 'space-between' as const,
  },
  bodyTile: { width: 80, height: 80, borderRadius: 8 },
  bodyBar: { height: 40, backgroundColor: '#FFCC80', borderRadius: 20 },
  footerRow: {
    flexDirection: 'row' as const,
    justifyContent: 'space-around' as const,
    height: 50,
  },
  footerCircle: { width: 50, height: 50, borderRadius: '50%' as const },
};

export class NestedViews extends Component {
  onRender(): void {
    <view {...styles.root}>
      <view {...styles.card}>
        <view {...styles.header}>
          <view {...styles.avatar} />
          <layout width={12} />
          <layout {...styles.titleColumn}>
            <label value="Card Title" font={testBoldFont(16)} color="#202124" />
            <layout height={4} />
            <label value="Subtitle text" font={testBoldFont(12)} color="#5F6368" />
          </layout>
        </view>

        <layout height={16} />

        <view {...styles.body}>
          <layout {...styles.bodyRow}>
            <view {...styles.bodyTile} backgroundColor="#FF9800" />
            <view {...styles.bodyTile} backgroundColor="#F44336" />
            <view {...styles.bodyTile} backgroundColor="#9C27B0" />
          </layout>
          <layout height={12} />
          <view {...styles.bodyBar} />
        </view>

        <layout height={16} />

        <layout {...styles.footerRow}>
          <view {...styles.footerCircle} backgroundColor="#4CAF50" />
          <view {...styles.footerCircle} backgroundColor="#2196F3" />
          <view {...styles.footerCircle} backgroundColor="#FF5722" />
        </layout>
      </view>
    </view>;
  }
}
