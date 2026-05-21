import { Component } from 'valdi_core/src/Component';
import { testFont, testBoldFont } from './TestFont';

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
  cell: { borderRadius: 4, padding: 4 },
};

export class DisplayNone extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="hidden middle child" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section}>
        <view flexGrow={1} backgroundColor="#2196F3" marginRight={4} {...styles.cell}>
          <label value="A" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
        <view flexGrow={1} backgroundColor="#F44336" marginRight={4} display="none" {...styles.cell}>
          <label value="B" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
        <view flexGrow={1} backgroundColor="#4CAF50" {...styles.cell}>
          <label value="C" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
      </layout>

      <label value="hidden in column" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="column" {...styles.section}>
        <view height={25} backgroundColor="#FF9800" marginBottom={4} {...styles.cell}>
          <label value="visible" font={testFont(8)} color="#FFFFFF" />
        </view>
        <view height={25} backgroundColor="#9C27B0" marginBottom={4} display="none" {...styles.cell} />
        <view height={25} backgroundColor="#00BCD4" {...styles.cell}>
          <label value="visible" font={testFont(8)} color="#FFFFFF" />
        </view>
      </layout>

      <label value="hidden affects flexGrow" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section}>
        <view flexGrow={1} backgroundColor="#E91E63" {...styles.cell} />
        <view flexGrow={2} backgroundColor="#795548" display="none" {...styles.cell} />
        <view flexGrow={1} backgroundColor="#607D8B" {...styles.cell} />
      </layout>

      <label value="all visible (reference)" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section}>
        <view flexGrow={1} backgroundColor="#E91E63" marginRight={4} {...styles.cell} />
        <view flexGrow={2} backgroundColor="#795548" marginRight={4} {...styles.cell} />
        <view flexGrow={1} backgroundColor="#607D8B" {...styles.cell} />
      </layout>

      <label value="hidden in wrap" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="row" flexWrap="wrap" flexGrow={1}>
        <view width={50} height={25} backgroundColor="#3F51B5" margin={2} {...styles.cell} />
        <view width={50} height={25} backgroundColor="#F44336" margin={2} display="none" {...styles.cell} />
        <view width={50} height={25} backgroundColor="#009688" margin={2} {...styles.cell} />
        <view width={50} height={25} backgroundColor="#FFC107" margin={2} {...styles.cell} />
        <view width={50} height={25} backgroundColor="#8BC34A" margin={2} {...styles.cell} />
      </layout>
    </view>;
  }
}
