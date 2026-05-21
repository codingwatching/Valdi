import { Component } from 'valdi_core/src/Component';

const styles = {
  root: {
    width: '100%' as const,
    height: '100%' as const,
    backgroundColor: '#FFFFFF',
    flexDirection: 'column' as const,
    padding: 16,
  },
  topRow: {
    flexDirection: 'row' as const,
    justifyContent: 'space-between' as const,
    alignItems: 'center' as const,
    height: 60,
  },
  redBox: { width: 60, height: 60, backgroundColor: '#FF0000', borderRadius: 8 },
  greenBox: { width: 60, height: 60, backgroundColor: '#00FF00', borderRadius: 8 },
  blueBox: { width: 60, height: 60, backgroundColor: '#0000FF', borderRadius: 8 },
  growRow: {
    flexDirection: 'row' as const,
    height: 80,
    justifyContent: 'center' as const,
    alignItems: 'stretch' as const,
  },
  goldCell: { flexGrow: 1, backgroundColor: '#FFD700', borderRadius: 16, marginRight: 8 },
  purpleCell: { flexGrow: 2, backgroundColor: '#8A2BE2', borderRadius: 16, marginLeft: 8 },
  wrapRow: {
    flexDirection: 'row' as const,
    flexWrap: 'wrap' as const,
    justifyContent: 'flex-start' as const,
  },
  tile: { width: 50, height: 50, margin: 4 },
};

export class BasicLayout extends Component {
  onRender(): void {
    <view {...styles.root}>
      <layout {...styles.topRow}>
        <view {...styles.redBox} />
        <view {...styles.greenBox} />
        <view {...styles.blueBox} />
      </layout>

      <layout height={16} />

      <layout {...styles.growRow}>
        <view {...styles.goldCell} />
        <view {...styles.purpleCell} />
      </layout>

      <layout height={16} />

      <layout {...styles.wrapRow}>
        <view {...styles.tile} backgroundColor="#FF6347" />
        <view {...styles.tile} backgroundColor="#4682B4" />
        <view {...styles.tile} backgroundColor="#32CD32" />
        <view {...styles.tile} backgroundColor="#FF69B4" />
        <view {...styles.tile} backgroundColor="#DDA0DD" />
        <view {...styles.tile} backgroundColor="#20B2AA" />
      </layout>
    </view>;
  }
}
