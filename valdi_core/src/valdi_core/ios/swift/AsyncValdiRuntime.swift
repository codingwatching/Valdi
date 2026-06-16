import Foundation
import valdi_core

public class AsyncValdiRuntimeProvider: NSObject, AsyncValdiRuntimeProviding, SCAsyncValdiRuntimeProviding {

    private let factory: @Sendable () async -> SCValdiRuntimeProtocol

    @ValdiActor private var cachedRuntime: SCValdiRuntimeProtocol?
    @ValdiActor private var initializationTask: Task<SCValdiRuntimeProtocol, Never>?

    public init(factory: @escaping @Sendable () async -> SCValdiRuntimeProtocol) {
        self.factory = factory
        super.init()
    }

    // MARK: - ObjC Bridge

    @objc(getRuntime:)
    public func getRuntime(completion: @escaping (SCValdiRuntimeProtocol) -> Void) {
        Task { @ValdiActor in
            completion(await self.runtime)
        }
    }

    @objc(getJSRuntime:)
    public func getJSRuntime(completion: @escaping (SCValdiJSRuntime?) -> Void) {
        Task { @ValdiActor in
            let runtime = await self.runtime
            runtime.getJSRuntime { jsRuntime in
                completion(jsRuntime)
            }
        }
    }

    // MARK: - Swift Async API

    @ValdiActor public var runtime: SCValdiRuntimeProtocol {
        get async {
            if let cachedRuntime {
                return cachedRuntime
            }

            if let initializationTask {
                return await initializationTask.value
            }

            let factory = self.factory
            let initializationTask = Task {
                await factory()
            }
            self.initializationTask = initializationTask

            let runtime = await initializationTask.value
            self.cachedRuntime = runtime
            self.initializationTask = nil
            return runtime
        }
    }

    @ValdiActor public var jsRuntime: SCValdiJSRuntime? {
        get async {
            let runtime = await self.runtime
            return await runtime.jsRuntime()
        }
    }
}
