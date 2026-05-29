/**
 * @ExportModel({
 *   ios: 'SCPoint',
 *   android: 'com.snap.modules.valdi_tsx.Point'
 * })
 */
export interface Point {
  x: number;
  y: number;
}

/**
 * @ExportModel({
 *   ios: 'SCSize',
 *   android: 'com.snap.modules.valdi_tsx.Size'
 * })
 */
export interface Size {
  width: number;
  height: number;
}

export interface Vector {
  dx: number;
  dy: number;
}

/**
 * @ExportModel({
 *   ios: 'SCElementFrame',
 *   android: 'com.snap.modules.valdi_tsx.ElementFrame'
 * })
 */
export interface ElementFrame {
  x: number;
  y: number;
  width: number;
  height: number;
}
