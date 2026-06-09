/**
 * @ExportProxy({ios: 'SCValdiFoundationCancelable', android: 'com.snap.modules.foundation.Cancelable'})
 */
export interface ICancelable {
  cancel(): void;
}
