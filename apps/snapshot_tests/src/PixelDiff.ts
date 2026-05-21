import { IBitmap } from 'drawing/src/IBitmap';

export interface PixelDiffResult {
  totalPixels: number;
  mismatchedPixels: number;
  mismatchPercent: number;
  maxDifference: number;
}

export function pixelDiff(baseline: IBitmap, actual: IBitmap, tolerance: number): PixelDiffResult {
  const baselineInfo = baseline.getInfo();
  const actualInfo = actual.getInfo();

  if (baselineInfo.width !== actualInfo.width || baselineInfo.height !== actualInfo.height) {
    return {
      totalPixels: baselineInfo.width * baselineInfo.height,
      mismatchedPixels: baselineInfo.width * baselineInfo.height,
      mismatchPercent: 100,
      maxDifference: 255,
    };
  }

  const { width, height, rowBytes } = baselineInfo;
  const actualRowBytes = actualInfo.rowBytes;
  const bytesPerPixel = 4;
  const totalPixels = width * height;
  let mismatchedPixels = 0;
  let maxDifference = 0;

  baseline.accessPixels(baselineView => {
    actual.accessPixels(actualView => {
      for (let y = 0; y < height; y++) {
        for (let x = 0; x < width; x++) {
          const baseOffset = y * rowBytes + x * bytesPerPixel;
          const actOffset = y * actualRowBytes + x * bytesPerPixel;
          const rDiff = Math.abs(baselineView.getUint8(baseOffset) - actualView.getUint8(actOffset));
          const gDiff = Math.abs(baselineView.getUint8(baseOffset + 1) - actualView.getUint8(actOffset + 1));
          const bDiff = Math.abs(baselineView.getUint8(baseOffset + 2) - actualView.getUint8(actOffset + 2));
          const aDiff = Math.abs(baselineView.getUint8(baseOffset + 3) - actualView.getUint8(actOffset + 3));

          const pixelMax = Math.max(rDiff, gDiff, bDiff, aDiff);
          if (pixelMax > tolerance) {
            mismatchedPixels++;
          }
          if (pixelMax > maxDifference) {
            maxDifference = pixelMax;
          }
        }
      }
    });
  });

  return {
    totalPixels,
    mismatchedPixels,
    mismatchPercent: totalPixels > 0 ? (mismatchedPixels / totalPixels) * 100 : 0,
    maxDifference,
  };
}
