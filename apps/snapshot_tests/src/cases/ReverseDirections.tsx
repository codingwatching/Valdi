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

export class ReverseDirections extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="row (normal)" font={testFont(9)} {...styles.sectionLabel} />
      <layout {...styles.row} {...styles.section}>
        <view flexGrow={1} backgroundColor="#F44336" marginRight={4} {...styles.cell}>
          <label value="1" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
        <view flexGrow={1} backgroundColor="#4CAF50" marginRight={4} {...styles.cell}>
          <label value="2" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
        <view flexGrow={1} backgroundColor="#2196F3" {...styles.cell}>
          <label value="3" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
      </layout>

      <label value="row-reverse" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="row-reverse" height={30} {...styles.section}>
        <view flexGrow={1} backgroundColor="#F44336" marginLeft={4} {...styles.cell}>
          <label value="1" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
        <view flexGrow={1} backgroundColor="#4CAF50" marginLeft={4} {...styles.cell}>
          <label value="2" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
        <view flexGrow={1} backgroundColor="#2196F3" {...styles.cell}>
          <label value="3" font={testBoldFont(10)} color="#FFFFFF" />
        </view>
      </layout>

      <label value="column-reverse" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="column-reverse" height={70} {...styles.section}>
        <view height={18} backgroundColor="#FF9800" marginTop={4} {...styles.cell}>
          <label value="1 (first)" font={testFont(8)} color="#FFFFFF" />
        </view>
        <view height={18} backgroundColor="#9C27B0" marginTop={4} {...styles.cell}>
          <label value="2" font={testFont(8)} color="#FFFFFF" />
        </view>
        <view height={18} backgroundColor="#00BCD4" {...styles.cell}>
          <label value="3 (last)" font={testFont(8)} color="#FFFFFF" />
        </view>
      </layout>

      <label value="wrap-reverse" font={testFont(9)} {...styles.sectionLabel} />
      <layout flexDirection="row" flexWrap="wrap-reverse" flexGrow={1}>
        <view width={50} height={25} backgroundColor="#795548" margin={2} {...styles.cell}>
          <label value="A" font={testBoldFont(9)} color="#FFFFFF" />
        </view>
        <view width={50} height={25} backgroundColor="#607D8B" margin={2} {...styles.cell}>
          <label value="B" font={testBoldFont(9)} color="#FFFFFF" />
        </view>
        <view width={50} height={25} backgroundColor="#CDDC39" margin={2} {...styles.cell}>
          <label value="C" font={testBoldFont(9)} color="#333333" />
        </view>
        <view width={50} height={25} backgroundColor="#FF4081" margin={2} {...styles.cell}>
          <label value="D" font={testBoldFont(9)} color="#FFFFFF" />
        </view>
        <view width={50} height={25} backgroundColor="#3F51B5" margin={2} {...styles.cell}>
          <label value="E" font={testBoldFont(9)} color="#FFFFFF" />
        </view>
      </layout>
    </view>;
  }
}
