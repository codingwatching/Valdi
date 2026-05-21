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
  container: {
    flexDirection: 'row' as const,
    flexWrap: 'wrap' as const,
    backgroundColor: '#F5F5F5',
    borderRadius: 4,
    padding: 2,
    marginBottom: 4,
  },
  tile: { width: 35, height: 20, borderRadius: 3, margin: 1 },
};

export class AlignContentWrap extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="alignContent: flex-start" font={testFont(8)} {...styles.sectionLabel} />
      <layout {...styles.container} height={55} alignContent="flex-start">
        <view {...styles.tile} backgroundColor="#F44336" />
        <view {...styles.tile} backgroundColor="#E91E63" />
        <view {...styles.tile} backgroundColor="#9C27B0" />
        <view {...styles.tile} backgroundColor="#673AB7" />
        <view {...styles.tile} backgroundColor="#3F51B5" />
      </layout>

      <label value="alignContent: center" font={testFont(8)} {...styles.sectionLabel} />
      <layout {...styles.container} height={55} alignContent="center">
        <view {...styles.tile} backgroundColor="#2196F3" />
        <view {...styles.tile} backgroundColor="#03A9F4" />
        <view {...styles.tile} backgroundColor="#00BCD4" />
        <view {...styles.tile} backgroundColor="#009688" />
        <view {...styles.tile} backgroundColor="#4CAF50" />
      </layout>

      <label value="alignContent: space-between" font={testFont(8)} {...styles.sectionLabel} />
      <layout {...styles.container} height={55} alignContent="space-between">
        <view {...styles.tile} backgroundColor="#8BC34A" />
        <view {...styles.tile} backgroundColor="#CDDC39" />
        <view {...styles.tile} backgroundColor="#FFC107" />
        <view {...styles.tile} backgroundColor="#FF9800" />
        <view {...styles.tile} backgroundColor="#FF5722" />
      </layout>

      <label value="alignContent: stretch" font={testFont(8)} {...styles.sectionLabel} />
      <layout {...styles.container} flexGrow={1} alignContent="stretch">
        <view width={35} borderRadius={3} margin={1} backgroundColor="#795548" />
        <view width={35} borderRadius={3} margin={1} backgroundColor="#607D8B" />
        <view width={35} borderRadius={3} margin={1} backgroundColor="#9E9E9E" />
        <view width={35} borderRadius={3} margin={1} backgroundColor="#455A64" />
        <view width={35} borderRadius={3} margin={1} backgroundColor="#BF360C" />
      </layout>
    </view>;
  }
}
