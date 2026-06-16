import XCTest
import ValdiCoreSwift
import valdi_core

class MockValdiRuntime: NSObject, SCValdiRuntimeProtocol {
    var jsRuntimeRequestCount = 0

    func registerNativeModuleFactory(_ moduleFactory: SCNValdiCoreModuleFactory) {}
    func getJSRuntime(_ block: (@Sendable ((any SCValdiJSRuntime)?) -> Void)) {
        jsRuntimeRequestCount += 1
        block(nil)
    }
    func jsRuntime() -> (any SCValdiJSRuntime)? {
        jsRuntimeRequestCount += 1
        return nil
    }
    func inflateView(_ view: UIView & SCValdiRootViewProtocol, owner: Any?, viewModel: Any?, componentContext: Any?) {}
    func inflateView(_ view: UIView & SCValdiRootViewProtocol, owner: Any?, cppMarshaller: UnsafeMutableRawPointer) {}
    func createContext(withComponentPath componentPath: String, viewModel: Any?, componentContext: Any?) -> SCValdiContextProtocol? { nil }
    func createContext(withViewClass viewClass: AnyClass, viewModel: Any?, componentContext: Any?) -> SCValdiContextProtocol? { nil }
    func loadView(withComponentPath componentPath: String, owner: Any?) throws -> UIView & SCValdiRootViewProtocol { fatalError() }
    func loadView(withComponentPath componentPath: String, owner: Any?, viewModel: Any?, componentContext: Any?) throws -> UIView & SCValdiRootViewProtocol { fatalError() }
    func executeMainThreadBatch(_ function: @escaping () -> Void) {}
    func dumpLogMetadata() -> String { "" }
    func dumpLogs() -> String { "" }
    func currentContext() -> SCValdiContextProtocol? { nil }
    func makeViewFactory(_ viewFactory: SCValdiViewFactoryBlock, attributesBinder: SCValdiBindAttributesCallback?, for viewClass: AnyClass) -> (any SCValdiViewFactory) { fatalError() }
    func manager() -> SCValdiRuntimeManagerProtocol? { nil }
    func asset(withModuleName moduleName: String, path: String) -> SCNValdiCoreAsset? { nil }
    func asset(withURL url: String) -> SCNValdiCoreAsset? { nil }
}

private final class ThreadSafeCounter: @unchecked Sendable {
    private let lock = NSLock()
    private var value = 0

    func increment() {
        lock.lock()
        value += 1
        lock.unlock()
    }

    var currentValue: Int {
        lock.lock()
        let value = self.value
        lock.unlock()
        return value
    }
}

private final class AsyncGate: @unchecked Sendable {
    private let lock = NSLock()
    private var continuation: CheckedContinuation<Void, Never>?
    private var isOpen = false

    func wait() async {
        await withCheckedContinuation { continuation in
            lock.lock()
            if isOpen {
                lock.unlock()
                continuation.resume()
                return
            }

            self.continuation = continuation
            lock.unlock()
        }
    }

    func open() {
        lock.lock()
        guard !isOpen else {
            lock.unlock()
            return
        }

        isOpen = true
        let continuation = self.continuation
        self.continuation = nil
        lock.unlock()

        continuation?.resume()
    }
}

class AsyncValdiRuntimeTests: XCTestCase {

    /// Core test: getRuntime must not block the caller
    func testGetRuntimeDoesNotBlock() throws {
        let factoryGate = AsyncGate()

        let asyncRuntime = AsyncValdiRuntimeProvider(
            factory: { () async -> SCValdiRuntimeProtocol in
                await factoryGate.wait()
                return MockValdiRuntime()
            }
        )

        let startTime = Date()
        let expectation = self.expectation(description: "Callback called")

        asyncRuntime.getRuntime { _ in
            expectation.fulfill()
        }

        let elapsed = Date().timeIntervalSince(startTime)

        // Call should return immediately, not wait for factory
        XCTAssertLessThan(elapsed, 0.1)

        factoryGate.open()
        waitForExpectations(timeout: 1.0)
    }

    /// The runtime must not be published before the factory finishes its readiness work.
    func testGetRuntimeDoesNotCompleteBeforeFactoryFinishes() throws {
        let factoryStarted = self.expectation(description: "Factory started")
        let completion = self.expectation(description: "Callback called")
        let factoryGate = AsyncGate()
        let lock = NSLock()
        var didComplete = false

        let asyncRuntime = AsyncValdiRuntimeProvider(
            factory: { () async -> SCValdiRuntimeProtocol in
                factoryStarted.fulfill()
                await factoryGate.wait()
                return MockValdiRuntime()
            }
        )

        asyncRuntime.getRuntime { _ in
            lock.lock()
            didComplete = true
            lock.unlock()
            completion.fulfill()
        }

        wait(for: [factoryStarted], timeout: 1.0)

        let completedBeforeFactoryFinished: Bool = {
            lock.lock()
            defer { lock.unlock() }
            return didComplete
        }()
        XCTAssertFalse(completedBeforeFactoryFinished)

        factoryGate.open()
        wait(for: [completion], timeout: 1.0)
    }

    /// Concurrent runtime and JS runtime requests must coalesce to one factory invocation.
    func testConcurrentCallersShareFactoryInvocation() throws {
        let factoryStarted = self.expectation(description: "Factory started")
        let runtimeCompletion = self.expectation(description: "Runtime callback")
        let jsRuntimeCompletion = self.expectation(description: "JS runtime callback")
        let factoryGate = AsyncGate()
        let factoryCallCounter = ThreadSafeCounter()

        let runtime = MockValdiRuntime()
        let asyncRuntime = AsyncValdiRuntimeProvider(
            factory: { () async -> SCValdiRuntimeProtocol in
                factoryCallCounter.increment()
                factoryStarted.fulfill()
                await factoryGate.wait()
                return runtime
            }
        )

        asyncRuntime.getRuntime { _ in
            runtimeCompletion.fulfill()
        }
        asyncRuntime.getJSRuntime { _ in
            jsRuntimeCompletion.fulfill()
        }

        wait(for: [factoryStarted], timeout: 1.0)

        XCTAssertEqual(factoryCallCounter.currentValue, 1)

        factoryGate.open()
        wait(for: [runtimeCompletion, jsRuntimeCompletion], timeout: 1.0)

        XCTAssertEqual(factoryCallCounter.currentValue, 1)
        XCTAssertEqual(runtime.jsRuntimeRequestCount, 1)
    }
}
