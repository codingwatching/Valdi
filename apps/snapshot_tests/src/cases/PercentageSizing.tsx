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
  cell: { borderRadius: 4 },
};

export class PercentageSizing extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="50/50 split" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section} height={30}>
        <view width={'50%'} backgroundColor="#2196F3" {...styles.cell} />
        <view width={'50%'} backgroundColor="#4CAF50" {...styles.cell} />
      </layout>

      <label value="33/33/33 grid" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section} height={30}>
        <view width={'33.33%'} backgroundColor="#FF9800" marginRight={2} {...styles.cell} />
        <view width={'33.33%'} backgroundColor="#E91E63" marginRight={2} {...styles.cell} />
        <view width={'33.33%'} backgroundColor="#9C27B0" {...styles.cell} />
      </layout>

      <label value="25/75 uneven" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section} height={30}>
        <view width={'25%'} backgroundColor="#00BCD4" marginRight={2} {...styles.cell} />
        <view width={'75%'} backgroundColor="#FF5722" {...styles.cell} />
      </layout>

      <label value="% height in fixed parent" font={testFont(9)} {...styles.sectionLabel} />
      <view height={60} backgroundColor="#F5F5F5" borderRadius={4} {...styles.section}>
        <view height={'50%'} width={'100%'} backgroundColor="#3F51B5" borderRadius={4} />
      </view>

      <label value="% padding + margin" font={testFont(9)} {...styles.sectionLabel} />
      <view backgroundColor="#E8F5E9" borderRadius={4} padding={0} {...styles.section}>
        <view marginLeft={'10%'} marginRight={'10%'} height={25} backgroundColor="#4CAF50" borderRadius={4} />
      </view>

      <label value="nested % sizing" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} flexGrow={1}>
        <view width={'40%'} backgroundColor="#FFF3E0" marginRight={2} borderRadius={4} padding={4}>
          <view width={'50%'} height={'100%'} backgroundColor="#FF9800" borderRadius={2} />
        </view>
        <view width={'60%'} backgroundColor="#E3F2FD" borderRadius={4} padding={4}>
          <view width={'75%'} height={'100%'} backgroundColor="#2196F3" borderRadius={2} />
        </view>
      </layout>
    </view>;
  }
}
