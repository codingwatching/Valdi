import { Component } from 'valdi_core/src/Component';
import { testFont } from './TestFont';

const styles = {
  root: {
    width: '100%' as const,
    height: '100%' as const,
    backgroundColor: '#FFFFFF',
    padding: 12,
  },
  sectionLabel: { color: '#666666' },
  flexOverlay: {
    height: 150,
    backgroundColor: '#F5F5F5',
    borderRadius: 8,
    marginBottom: 12,
  },
  flexRow: {
    flexDirection: 'row' as const,
    height: '100%' as const,
    padding: 8,
  },
  flexChild: { flexGrow: 1, marginRight: 4, borderRadius: 4 },
  floatingCircle: {
    position: 'absolute' as const,
    top: 20,
    right: 20,
    width: 60,
    height: 60,
    backgroundColor: '#F44336',
    borderRadius: '50%' as const,
    borderWidth: 3,
    borderColor: '#FFFFFF',
  },
  insetContainer: {
    height: 120,
    backgroundColor: '#ECEFF1',
    borderRadius: 8,
    marginBottom: 12,
  },
  insetOuter: {
    position: 'absolute' as const,
    top: 8,
    left: 8,
    right: 8,
    bottom: 8,
    backgroundColor: '#1565C0',
    borderRadius: 4,
  },
  insetInner: {
    position: 'absolute' as const,
    top: 16,
    left: 16,
    right: 16,
    bottom: 16,
    backgroundColor: '#42A5F5',
    borderRadius: 4,
  },
  mixedContainer: {
    flexGrow: 1,
    backgroundColor: '#F5F5F5',
    borderRadius: 8,
  },
  mixedColumn: {
    flexDirection: 'column' as const,
    padding: 8,
  },
  mixedBar: { height: 30, borderRadius: 4, marginBottom: 4 },
  fabLeft: {
    position: 'absolute' as const,
    bottom: 8,
    left: 8,
    width: 40,
    height: 40,
    backgroundColor: '#D32F2F',
    borderRadius: 20,
  },
  fabRight: {
    position: 'absolute' as const,
    bottom: 8,
    right: 8,
    width: 40,
    height: 40,
    backgroundColor: '#388E3C',
    borderRadius: 20,
  },
};

export class AbsolutePositioning extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="absolute over flex" font={testFont(11)} {...styles.sectionLabel} />
      <view {...styles.flexOverlay}>
        <layout {...styles.flexRow}>
          <view {...styles.flexChild} backgroundColor="#E3F2FD" />
          <view {...styles.flexChild} backgroundColor="#E8F5E9" />
          <view flexGrow={1} backgroundColor="#FFF3E0" borderRadius={4} />
        </layout>
        <view {...styles.floatingCircle} />
      </view>

      <label value="inset absolute" font={testFont(11)} {...styles.sectionLabel} />
      <view {...styles.insetContainer}>
        <view {...styles.insetOuter} />
        <view {...styles.insetInner} />
      </view>

      <label value="mixed flow + absolute" font={testFont(11)} {...styles.sectionLabel} />
      <view {...styles.mixedContainer}>
        <layout {...styles.mixedColumn}>
          <view {...styles.mixedBar} backgroundColor="#CE93D8" />
          <view {...styles.mixedBar} backgroundColor="#80CBC4" />
          <view height={30} backgroundColor="#FFCC80" borderRadius={4} />
        </layout>
        <view {...styles.fabLeft} />
        <view {...styles.fabRight} />
      </view>
    </view>;
  }
}
