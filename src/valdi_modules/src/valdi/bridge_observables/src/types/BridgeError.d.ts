/**
 * @ExportModel({
 *  ios: 'SCBridgeError',
 *  swift: 'BridgeError',
 *  android: 'com.snap.modules.bridge_observables.BridgeError'
 * })
 */
export interface BridgeError {
  message: string;
  stack?: string;
}
