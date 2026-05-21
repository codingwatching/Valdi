import { Component } from 'valdi_core/src/Component';
import { testFont, testBoldFont } from './TestFont';

const styles = {
  root: {
    width: '100%' as const,
    height: '100%' as const,
    backgroundColor: '#FFFFFF',
    flexDirection: 'column' as const,
    padding: 16,
  },
};

export class TextRendering extends Component {
  onRender(): void {
    <view {...styles.root}>
      <label value="Heading Bold 24" font={testBoldFont(24)} color="#000000" />
      <layout height={8} />
      <label value="Body Regular 16" font={testFont(16)} color="#333333" />
      <layout height={8} />
      <label value="Caption 12" font={testFont(12)} color="#888888" />
      <layout height={16} />
      <label value="Red Text" font={testBoldFont(18)} color="#FF0000" />
      <label value="Blue Text" font={testBoldFont(18)} color="#0000FF" />
      <label value="Green Text" font={testBoldFont(18)} color="#00AA00" />
    </view>;
  }
}
