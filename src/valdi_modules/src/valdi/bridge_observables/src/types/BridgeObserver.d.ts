import { BridgeError } from './BridgeError';
import { BridgeObserverEvent } from './BridgeObserverEvent';

/**
 * @ExportModel({
 *  ios: 'SCBridgeObserver',
 *  swift: 'BridgeObserver',
 *  android: 'com.snap.modules.bridge_observables.BridgeObserver'
 * })
 */
export interface BridgeObserver<T> {
  // @WorkerThread
  onEvent: (
    type: BridgeObserverEvent,
    subscription: (() => void) | undefined,
    value: T | undefined,
    error: BridgeError | undefined,
  ) => void;
}
