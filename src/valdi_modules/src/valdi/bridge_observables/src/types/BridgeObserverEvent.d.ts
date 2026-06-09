/**
 * @ExportEnum({
 *  ios: 'SCBridgeObserverEvent',
 *  swift: 'BridgeObserverEvent',
 *  android: 'com.snap.modules.bridge_observables.BridgeObserverEvent'
 * })
 */
export const enum BridgeObserverEvent {
  RECEIVE_SUBSCRIPTION,
  NEXT,
  ERROR,
  COMPLETE,
}
