import { createBitmapWithBuffer } from 'drawing/src/BitmapFactory';
import { BitmapAlphaType, BitmapColorType, IBitmap, ImageEncoding } from 'drawing/src/IBitmap';

export function generateDiffImage(baseline: IBitmap, actual: IBitmap, tolerance: number): ArrayBuffer {
  const baseInfo = baseline.getInfo();
  const actInfo = actual.getInfo();
  const width = baseInfo.width;
  const height = baseInfo.height;
  const baseRowBytes = baseInfo.rowBytes;
  const actRowBytes = actInfo.rowBytes;
  const panelWidth = width * 3;
  const bytesPerPixel = 4;
  const diffRowBytes = panelWidth * bytesPerPixel;
  const buffer = new ArrayBuffer(diffRowBytes * height);

  const diffBitmap = createBitmapWithBuffer(
    {
      width: panelWidth,
      height,
      colorType: BitmapColorType.RGBA8888,
      alphaType: BitmapAlphaType.Premul,
      rowBytes: diffRowBytes,
    },
    buffer,
  );

  try {
    baseline.accessPixels(baselineView => {
      actual.accessPixels(actualView => {
        diffBitmap.accessPixels(diffView => {
          for (let y = 0; y < height; y++) {
            for (let x = 0; x < width; x++) {
              const baseOffset = y * baseRowBytes + x * bytesPerPixel;
              const actOffset = y * actRowBytes + x * bytesPerPixel;
              const baseR = baselineView.getUint8(baseOffset);
              const baseG = baselineView.getUint8(baseOffset + 1);
              const baseB = baselineView.getUint8(baseOffset + 2);
              const baseA = baselineView.getUint8(baseOffset + 3);
              const actR = actualView.getUint8(actOffset);
              const actG = actualView.getUint8(actOffset + 1);
              const actB = actualView.getUint8(actOffset + 2);
              const actA = actualView.getUint8(actOffset + 3);

              const diffRowOffset = y * diffRowBytes;

              // Left panel: baseline
              const leftOffset = diffRowOffset + x * bytesPerPixel;
              diffView.setUint8(leftOffset, baseR);
              diffView.setUint8(leftOffset + 1, baseG);
              diffView.setUint8(leftOffset + 2, baseB);
              diffView.setUint8(leftOffset + 3, baseA);

              // Center panel: actual
              const centerOffset = diffRowOffset + (width + x) * bytesPerPixel;
              diffView.setUint8(centerOffset, actR);
              diffView.setUint8(centerOffset + 1, actG);
              diffView.setUint8(centerOffset + 2, actB);
              diffView.setUint8(centerOffset + 3, actA);

              // Right panel: diff overlay
              const rightOffset = diffRowOffset + (width * 2 + x) * bytesPerPixel;
              const rDiff = Math.abs(baseR - actR);
              const gDiff = Math.abs(baseG - actG);
              const bDiff = Math.abs(baseB - actB);
              const aDiff = Math.abs(baseA - actA);
              const pixelMax = Math.max(rDiff, gDiff, bDiff, aDiff);

              if (pixelMax > tolerance) {
                diffView.setUint8(rightOffset, 255);
                diffView.setUint8(rightOffset + 1, 0);
                diffView.setUint8(rightOffset + 2, 255);
                diffView.setUint8(rightOffset + 3, 255);
              } else {
                const gray = Math.round((baseR * 0.299 + baseG * 0.587 + baseB * 0.114) * 0.25);
                diffView.setUint8(rightOffset, gray);
                diffView.setUint8(rightOffset + 1, gray);
                diffView.setUint8(rightOffset + 2, gray);
                diffView.setUint8(rightOffset + 3, 255);
              }
            }
          }
        });
      });
    });

    return diffBitmap.encode(ImageEncoding.PNG, 1.0);
  } finally {
    diffBitmap.dispose();
  }
}
