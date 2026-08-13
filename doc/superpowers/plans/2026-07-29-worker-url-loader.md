# Worker URL Loader Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let `new Worker(url)` load a worker's script by URL through an
integrator-provided `IWorkerSetup` that returns a `jsi::Buffer` of
bytecode or source, plus hooks to configure the worker `RuntimeConfig` and run
JS setup in the worker runtime; add opt-in `data:` URL decoding.

**Architecture:** Two new `jsi::ICast` interfaces in `hermes-interfaces.h`
(`IWorkerSetup`, `ISetWorkerSetup`). `HermesRuntimeImpl` stores
an opaque `jsi::ICast*` and delegates any unrecognized `castInterface` UUID to it,
so the provider (and future provider interfaces) are reachable via
`castInterface<IWorkerSetup>(&runtime)` with no future runtime changes.
`Worker.cpp` looks up the provider, classifies the constructor argument, resolves
URLs on the worker thread, and runs resolve → init → evaluate.

**Tech Stack:** C++17 (no exceptions/RTTI in VM; the API layer *does* use
exceptions — `Worker.cpp` already throws/catches `jsi::JSError`), JSI, CMake +
Ninja, GoogleTest (APITests), LLVM-lit (`test/hermes/worker/`).

**Design spec:** `doc/superpowers/specs/2026-07-29-worker-url-loader-design.md`.
Read it before starting; this plan implements it verbatim.

## Global Constraints

- **C++17**, no RTTI. Copyright header on every new file (see `CLAUDE.md`).
- **80-column** lines, **2-space** indent. Doc comment on every declaration.
- **GC-safe coding:** `Worker.cpp` / `hermes.cpp` are API-layer JSI code (they use
  the stable JSI API, not internal VM handles), so the Locals/Handle rules do not
  apply here; follow existing `Worker.cpp` patterns.
- **Build (default ASan+Debug -O1):** configure per `CLAUDE.md`. Build targets:
  `cmake --build cmake-build-asan --target hermes` (rebuilds the extensions
  bytecode, which is generated from the `*.js` files at build time), and
  `cmake --build cmake-build-asan --target APITests`.
- **Run APITests:** `cmake-build-asan/unittests/API/APITests --gtest_filter='<F>'`
- **Run one LIT test (macOS/Claude sandbox):**
  `LIT_OPTS="-j1" LIT_FILTER="worker/<name>" cmake --build cmake-build-asan --target check-hermes`
- **After editing any `.cpp`/`.h`, run `arc f`** before committing.
- **Sapling repo:** commit with `sl commit --reason "<intent> - sl help commit" -m "<msg>"`.
  Commit messages hard-wrapped to 72 cols, subject ≤50. Do NOT `jf submit` or
  amend/fold unless explicitly asked. Implementation tasks each create a new
  commit above the design/plan commit; they may be folded into one diff later.
- **`jsi::ICast` mechanics:** `jsi::castInterface<U>(ptr)` returns
  `static_cast<U*>(ptr->castInterface(U::uuid))` or `nullptr`. An interface
  derived from `jsi::ICast` overrides `jsi::ICast *castInterface(const UUID&)`.
- **UUIDs:** generate two fresh RFC-4122 UUIDs for the new interfaces (e.g.
  `uuidgen`), formatted as the existing `jsi::UUID{0x........, 0x...., 0x....,
  0x...., 0x............}` initializer (see `ISetEventLoopControl` in
  `hermes-interfaces.h`). Never reuse an existing UUID.

---

## File Structure

- **Modify** `xplat/jsi/jsi/hermes-interfaces.h` — add a `RuntimeConfig` forward
  declaration and the two new interfaces beside `IEventLoopControl`.
- **Modify** `API/hermes/hermes.cpp` — `HermesRuntimeImpl`: add the
  `ISetWorkerSetup` base, a `jsi::ICast *workerSetup_` member,
  the setter, and catch-all delegation in `castInterface`.
- **Modify** `API/hermes/extensions/11-Worker.js` — constructor takes `options`
  and forwards the `inline`/`allowData` booleans.
- **Modify** `API/hermes/extensions/Worker.cpp` — provider lookup, argument
  classification (URL/source/buffer/data:/object-coercion), the `jsi::Buffer`
  payload, config + provider-propagation in `startWorker`, and
  resolve → init → evaluate in `startWorkerThread`; `initWorker` arity 4.
- **Modify** `unittests/API/APITest.cpp` — a reusable test provider + new
  `HermesWorkerTest` cases.
- **Create** `test/hermes/worker/worker-data-url.js`,
  `test/hermes/worker/worker-url-object-coercion.js` — LIT tests for the
  provider-independent behavior.

---

### Task 1: Interfaces + runtime plumbing

Add the two interfaces and wire the runtime so a registered provider is reachable
via `castInterface`. Deliverable: registering a provider and reading it back
through the runtime works; the runtime's own interfaces are unaffected.

**Files:**
- Modify: `xplat/jsi/jsi/hermes-interfaces.h`
- Modify: `API/hermes/hermes.cpp` (class base ~line 253-263; method decl
  ~line 725-726; `castInterface` ~line 1633-1637; member ~line 1368; setter impl
  ~line 1734-1741)
- Test: `unittests/API/APITest.cpp`

**Interfaces:**
- Produces:
  - `facebook::hermes::IWorkerSetup : jsi::ICast` with `uuid`,
    `std::shared_ptr<const jsi::Buffer> resolveScript(const std::string &url,
    std::string &error)`, `void initWorkerRuntime(jsi::Runtime &rt)`,
    `void configureWorkerRuntime(::hermes::vm::RuntimeConfig &config)`.
  - `facebook::hermes::ISetWorkerSetup : jsi::ICast` with `uuid` and
    `void setWorkerSetup(jsi::ICast *provider)`.
  - `HermesRuntimeImpl` implements `ISetWorkerSetup` and delegates
    unknown `castInterface` UUIDs to the stored provider.

- [ ] **Step 1: Add the forward declaration and interfaces to `hermes-interfaces.h`**

In `xplat/jsi/jsi/hermes-interfaces.h`, extend the existing top-level forward
declarations (next to `class GCExecTrace;`):

```cpp
namespace hermes::vm {
class GCExecTrace;
class RuntimeConfig;
}
```

Then, inside `namespace facebook::hermes`, immediately after the closing brace of
`ISetEventLoopControl`, add:

```cpp
/// Integrator-provided interface used by the Worker implementation to load
/// worker scripts by URL and to configure/initialize worker runtimes. The
/// integrator implements this (deriving from jsi::ICast) and registers it via
/// ISetWorkerSetup. It must outlive every runtime it is set on. Its
/// methods may be called from worker threads and must be thread-safe.
///
/// URL semantics live entirely here: Hermes has no notion of URL, Blob, or
/// fetch. `new Worker(url)` hands the URL string to resolveScript() and the
/// integrator decides what it means (file path, packager URL, custom scheme).
/// In particular, to support the standard web idiom
/// `new Worker(URL.createObjectURL(new Blob([...])))`, the *host* provides Blob
/// and URL.createObjectURL and backs `blob:` URLs with a resolveScript() that
/// looks up the object-URL registry. (`data:` URLs are the one scheme Hermes can
/// decode itself, opt-in via `new Worker(u, {allowData: true})`; everything else
/// is the integrator's responsibility.)
class JSI_EXPORT IWorkerSetup : public jsi::ICast {
 public:
  static constexpr jsi::UUID uuid{
      /* GENERATE a fresh uuid, formatted like ISetEventLoopControl::uuid */};

  /// Resolve \p url to worker script bytes, returned as an immutable jsi::Buffer
  /// holding either precompiled Hermes bytecode or UTF-8 source (the caller
  /// sniffs the magic number). The buffer may be memory-mapped or otherwise
  /// externally owned; for bytecode it is referenced (not copied) for the life
  /// of the worker runtime, so it must remain valid until the returned
  /// shared_ptr is released. On failure returns nullptr and sets \p error to a
  /// human-readable message. Called on the worker thread; must NOT perform
  /// operations on the worker runtime.
  virtual std::shared_ptr<const jsi::Buffer> resolveScript(
      const std::string& url,
      std::string& error) = 0;

  /// Run one-time JS setup in a freshly created worker runtime \p rt, after the
  /// standard extensions are installed and before the worker script runs.
  /// Called on the worker thread. May throw jsi::JSError (routed to onerror).
  virtual void initWorkerRuntime(jsi::Runtime& rt) = 0;

  /// Adjust the RuntimeConfig used to create a worker runtime. \p config is
  /// seeded with the Hermes default; the integrator may rebuild it in place
  /// (e.g. `config = config.rebuild().withGCConfig(...).build();`). Called on
  /// the constructor thread.
  virtual void configureWorkerRuntime(::hermes::vm::RuntimeConfig& config) = 0;

 protected:
  ~IWorkerSetup() = default;
};

/// Interface for registering an IWorkerSetup on a Runtime. The stored
/// pointer is opaque (jsi::ICast*) so additional provider capabilities can be
/// added later as new cast interfaces without changing this ABI.
struct JSI_EXPORT ISetWorkerSetup : public jsi::ICast {
 public:
  static constexpr jsi::UUID uuid{
      /* GENERATE a second fresh uuid */};

  /// Register \p provider (may be nullptr to clear). The provider must outlive
  /// the runtime. The provider is reachable afterward via
  /// `jsi::castInterface<IWorkerSetup>(&runtime)`.
  virtual void setWorkerSetup(jsi::ICast* provider) = 0;

 protected:
  ~ISetWorkerSetup() = default;
};
```

- [ ] **Step 2: Add the runtime base class, member, and method declaration in `hermes.cpp`**

In the `HermesRuntimeImpl` base list (around line 257), add
`ISetWorkerSetup` next to `ISetEventLoopControl`:

```cpp
                                public ISetEventLoopControl,
                                public ISetWorkerSetup
```

Near the `setEventLoopControl`/`getEventLoopControl` declarations (around line
725), add:

```cpp
  void setWorkerSetup(jsi::ICast *provider) override;
```

Near the `eventLoopControl_` member (around line 1368), add:

```cpp
  /// Integrator-provided worker setup (opaque jsi::ICast*), reachable
  /// via castInterface. Owned by the integrator; may be null.
  jsi::ICast *workerSetup_{nullptr};
```

- [ ] **Step 3: Implement the setter and catch-all delegation in `hermes.cpp`**

Add the setter next to `setEventLoopControl` (around line 1734):

```cpp
void HermesRuntimeImpl::setWorkerSetup(jsi::ICast *provider) {
  workerSetup_ = provider;
}
```

Change the tail of `HermesRuntimeImpl::castInterface` (around line 1633) from:

```cpp
  else if (interfaceUUID == ISetEventLoopControl::uuid) {
    return static_cast<ISetEventLoopControl *>(this);
  }
  return nullptr;
}
```

to:

```cpp
  else if (interfaceUUID == ISetEventLoopControl::uuid) {
    return static_cast<ISetEventLoopControl *>(this);
  } else if (interfaceUUID == ISetWorkerSetup::uuid) {
    return static_cast<ISetWorkerSetup *>(this);
  }
  // Catch-all: delegate any interface the runtime does not implement itself to
  // the registered worker setup, if any. This lets
  // IWorkerSetup (and any future provider interface) be reached via
  // castInterface on the runtime without the runtime knowing its UUID.
  if (workerSetup_) {
    return workerSetup_->castInterface(interfaceUUID);
  }
  return nullptr;
}
```

- [ ] **Step 4: Add the reusable test provider + write the failing registration test**

In `unittests/API/APITest.cpp`, add this test infrastructure near the other
`HermesWorkerTest` code (before `class HermesWorkerTest`). It is used by every
later task, so define it fully now:

```cpp
// Cross-thread signal: the worker thread reports back to the test thread.
struct WorkerTestSignal {
  std::mutex m;
  std::condition_variable cv;
  std::string tag;
  bool fired{false};
  void set(std::string t) {
    std::lock_guard<std::mutex> l(m);
    tag = std::move(t);
    fired = true;
    cv.notify_all();
  }
  // Returns true and sets \p out if signalled within \p timeoutMs.
  bool wait(std::string &out, int timeoutMs) {
    std::unique_lock<std::mutex> l(m);
    if (!cv.wait_for(
            l, std::chrono::milliseconds(timeoutMs), [&] { return fired; }))
      return false;
    out = tag;
    return true;
  }
};

// A jsi::Buffer that records its own destruction and can wrap arbitrary bytes,
// standing in for a memory-mapped / externally owned buffer.
class TrackedBuffer : public jsi::Buffer {
 public:
  TrackedBuffer(std::string bytes, std::shared_ptr<std::atomic<bool>> destroyed)
      : bytes_(std::move(bytes)), destroyed_(std::move(destroyed)) {}
  ~TrackedBuffer() override {
    if (destroyed_)
      destroyed_->store(true);
  }
  size_t size() const override {
    return bytes_.size();
  }
  const uint8_t *data() const override {
    return reinterpret_cast<const uint8_t *>(bytes_.data());
  }

 private:
  std::string bytes_;
  std::shared_ptr<std::atomic<bool>> destroyed_;
};

// Configurable test provider.
class TestWorkerSetup : public IWorkerSetup {
 public:
  // Set by the test: maps a URL to bytes (or nullptr + error on failure).
  std::function<std::shared_ptr<const jsi::Buffer>(
      const std::string &url,
      std::string &error)>
      onResolve;
  std::shared_ptr<WorkerTestSignal> signal;
  std::atomic<bool> configureCalled{false};
  std::atomic<bool> initCalled{false};
  std::string lastUrl;

  jsi::ICast *castInterface(const jsi::UUID &uuid) override {
    if (uuid == IWorkerSetup::uuid)
      return static_cast<IWorkerSetup *>(this);
    return nullptr;
  }

  std::shared_ptr<const jsi::Buffer> resolveScript(
      const std::string &url,
      std::string &error) override {
    lastUrl = url;
    if (onResolve)
      return onResolve(url, error);
    error = "no resolver configured";
    return nullptr;
  }

  void initWorkerRuntime(jsi::Runtime &rt) override {
    initCalled = true;
    // Install __workerRan(tag) so a worker script can signal the test thread.
    auto sig = signal;
    auto fn = jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "__workerRan"),
        1,
        [sig](
            jsi::Runtime &rt,
            const jsi::Value &,
            const jsi::Value *args,
            size_t n) -> jsi::Value {
          if (sig && n > 0)
            sig->set(args[0].asString(rt).utf8(rt));
          return jsi::Value::undefined();
        });
    rt.global().setProperty(rt, "__workerRan", fn);
  }

  void configureWorkerRuntime(::hermes::vm::RuntimeConfig &) override {
    configureCalled = true;
  }
};

// Helper: wrap a std::string of bytes as a jsi::Buffer.
inline std::shared_ptr<const jsi::Buffer> bufferFromString(std::string s) {
  return std::make_shared<jsi::StringBuffer>(std::move(s));
}
```

Ensure the file includes `<atomic>`, `<condition_variable>`, `<functional>`,
`<mutex>` (add any that are missing near the top).

Now add the failing test (registration + delegation), NOT gated on extensions:

```cpp
TEST_F(HermesRuntimeTest, WorkerSetupRegistration) {
  auto *setter = castInterface<ISetWorkerSetup>(rt.get());
  ASSERT_NE(setter, nullptr);

  // Not reachable before registration.
  EXPECT_EQ(castInterface<IWorkerSetup>(rt.get()), nullptr);

  TestWorkerSetup provider;
  setter->setWorkerSetup(&provider);
  // Reachable via the runtime's catch-all delegation.
  EXPECT_EQ(
      castInterface<IWorkerSetup>(rt.get()),
      static_cast<IWorkerSetup *>(&provider));

  // The runtime's own interfaces still resolve.
  EXPECT_NE(castInterface<ISetEventLoopControl>(rt.get()), nullptr);

  // Clearing removes reachability.
  setter->setWorkerSetup(nullptr);
  EXPECT_EQ(castInterface<IWorkerSetup>(rt.get()), nullptr);
}
```

- [ ] **Step 5: Build and run — verify it fails, then passes**

Run: `cmake --build cmake-build-asan --target APITests`
Expected before Steps 1-3 exist: compile error (unknown `ISetWorkerSetup`).
After Steps 1-3: build succeeds.

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*WorkerSetupRegistration*'`
Expected: PASS.

- [ ] **Step 6: `arc f` and commit**

```bash
arc f
sl commit --reason "add worker setup interfaces + runtime plumbing - sl help commit" \
  -m "$(printf '%s' '[SH]: Add IWorkerSetup runtime plumbing

Add IWorkerSetup and ISetWorkerSetup to
hermes-interfaces.h and wire HermesRuntimeImpl to store an opaque
provider and delegate unrecognized castInterface UUIDs to it.')"
```

---

### Task 2: Core URL path + JS glue

Wire `Worker.cpp` so `new Worker(url)` (with a provider registered) resolves the
URL on the worker thread and runs it, with the init hook and provider
propagation. Uses the default `RuntimeConfig` (config hook comes in Task 3). The
worker "script" payload becomes a `std::shared_ptr<const jsi::Buffer>`.

**Files:**
- Modify: `API/hermes/extensions/11-Worker.js`
- Modify: `API/hermes/extensions/Worker.cpp`
- Test: `unittests/API/APITest.cpp`

**Interfaces:**
- Consumes: `IWorkerSetup`, `ISetWorkerSetup`,
  `jsi::castInterface`, `jsi::StringBuffer`, `jsi::Buffer` (Task 1).
- Produces:
  - `Worker.cpp` internal: `struct WorkerScriptSource { std::shared_ptr<const
    jsi::Buffer> eagerBuffer; std::string url; bool needsResolve; };`
  - `void startWorker(jsi::Runtime &rt, jsi::Object self, WorkerScriptSource
    source, IWorkerSetup *provider);`
  - `WorkerNativeState::startWorkerThread(WorkerScriptSource source,
    IWorkerSetup *provider);`
  - `IWorkerSetup *getWorkerSetup(jsi::Runtime &rt);`

- [ ] **Step 1: Update the JS glue for the options argument**

Replace the body of `extensions.Worker` in
`API/hermes/extensions/11-Worker.js` with:

```js
extensions.Worker = function(nativeInit, nativeTerminate, nativePostMessage) {
    class Worker {
        constructor(script, options) {
            // Forward the raw `script` unchanged so native does all type
            // detection; only the option booleans are read here.
            nativeInit(
                this,
                script,
                !!(options && options.inline),
                !!(options && options.allowData));
        }
        terminate() {
            return nativeTerminate.call(this);
        }
        postMessage(...args) {
            return nativePostMessage.call(this, ...args);
        }
    }

    globalThis.Worker = Worker;
};
```

- [ ] **Step 2: Add the payload struct, provider lookup, and change `startWorker` in `Worker.cpp`**

In the anonymous namespace of `API/hermes/extensions/Worker.cpp`, add near the
top:

```cpp
/// The script source handed to the worker thread. Exactly one of the two forms
/// is active: eager bytes already materialized (source string, decoded data:
/// URL, or a copied buffer input), or a URL to resolve on the worker thread.
struct WorkerScriptSource {
  std::shared_ptr<const jsi::Buffer> eagerBuffer;
  std::string url;
  bool needsResolve{false};
};

/// Return the integrator's worker setup registered on \p rt, or
/// nullptr if none. Reachable via the runtime's castInterface delegation.
IWorkerSetup *getWorkerSetup(jsi::Runtime &rt) {
  return jsi::castInterface<IWorkerSetup>(&rt);
}
```

Change `startWorker`'s signature and body. Replace the existing
`void startWorker(jsi::Runtime &rt, jsi::Object self, std::string script)` with:

```cpp
/// Create the Worker runtime/thread and attach state to \p self. \p source is
/// either eager bytes or a URL to resolve on the worker thread; \p provider (may
/// be null) supplies resolveScript/initWorkerRuntime/configureWorkerRuntime.
void startWorker(
    jsi::Runtime &rt,
    jsi::Object self,
    WorkerScriptSource source,
    IWorkerSetup *provider) {
  auto *api = jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  // Default config for now; the config hook is added in a later task.
  auto workerRuntime = api->makeHermesRuntime(::hermes::vm::RuntimeConfig());
  auto workerState = std::make_shared<WorkerState>(rt, self);

  // Propagate the provider to the worker runtime so a nested `new Worker`
  // created inside this worker inherits it. `provider` is an
  // IWorkerSetup*, i.e. a jsi::ICast*; re-casting it on the child still
  // reaches every interface the object implements.
  if (provider) {
    auto *childSetter =
        jsi::castInterface<ISetWorkerSetup>(workerRuntime.get());
    assert(childSetter && "ISetWorkerSetup is not supported");
    childSetter->setWorkerSetup(provider);
  }

  installPostMessageFromWorker(*workerRuntime, workerState);
  installCloseFromWorker(*workerRuntime, workerState);

  auto workerNativeState = std::make_shared<WorkerNativeState>(
      workerState, std::move(workerRuntime));
  self.setNativeState(rt, workerNativeState);

  auto *setEventLoopControlInterface =
      jsi::castInterface<ISetEventLoopControl>(&workerState->parentRuntime);
  assert(
      setEventLoopControlInterface && "ISetEventLoopControl is not supported");
  auto *eventLoopControl = setEventLoopControlInterface->getEventLoopControl();
  if (LLVM_LIKELY(eventLoopControl)) {
    workerState->id = eventLoopControl->registerTaskQueueSource();
  }

  workerNativeState->startWorkerThread(std::move(source), provider);
}
```

- [ ] **Step 3: Change `startWorkerThread` to resolve → init → evaluate**

Update the declaration in `class WorkerNativeState` from
`void startWorkerThread(std::string script);` to:

```cpp
  /// Start the worker thread. \p source is eager bytes or a URL to resolve;
  /// \p provider (may be null) supplies resolveScript/initWorkerRuntime.
  void startWorkerThread(
      WorkerScriptSource source,
      IWorkerSetup *provider);
```

Replace the `startWorkerThread` definition body with:

```cpp
void WorkerNativeState::startWorkerThread(
    WorkerScriptSource source,
    IWorkerSetup *provider) {
  workerThread_ = std::thread([source = std::move(source),
                               provider,
                               workerRuntime = workerRuntime.get(),
                               workerState = workerState]() {
    try {
      std::shared_ptr<const jsi::Buffer> buffer;
      if (source.needsResolve) {
        assert(provider && "URL input requires a provider");
        std::string error;
        buffer = provider->resolveScript(source.url, error);
        if (!buffer || buffer->size() == 0) {
          throw jsi::JSError(
              *workerRuntime,
              error.empty() ? std::string("Failed to load worker script")
                            : error);
        }
      } else {
        buffer = source.eagerBuffer;
      }

      if (provider) {
        provider->initWorkerRuntime(*workerRuntime);
      }

      workerRuntime->evaluateJavaScript(buffer, "");
    } catch (const jsi::JSError &scriptError) {
      postError(*workerRuntime, scriptError.value(), workerState);
    } catch (const jsi::JSINativeException &) {
      ::hermes::hermesLog(
          "HermesWorker",
          "Encountered JSINativeException while running Worker script.");
      setTerminationState(workerState, *workerRuntime, false);
      return;
    }

    std::unique_lock<std::mutex> lock(workerState->stateMutex);
    while (!workerState->terminated) {
      workerState->toWorkerCondition.wait(lock, [workerState] {
        return workerState->terminated || !workerState->toWorkerQueue.empty();
      });
      if (workerState->terminated) {
        break;
      }
      Message message = std::move(workerState->toWorkerQueue.front());
      workerState->toWorkerQueue.pop_front();
      lock.unlock();

      auto workerGlobal = workerRuntime->global();
      jsi::Value onMessage = getHandler(
          *workerRuntime,
          workerGlobal,
          jsi::PropNameID::forAscii(*workerRuntime, "onmessage"));
      if (LLVM_LIKELY(!onMessage.isUndefined())) {
        try {
          auto onMessageFunc =
              onMessage.asObject(*workerRuntime).asFunction(*workerRuntime);
          processMessageWithHandler(
              *workerRuntime, std::move(message), onMessageFunc);
        } catch (const jsi::JSError &error) {
          postError(*workerRuntime, error.value(), workerState);
        }
      }
      lock.lock();
    }
  });
}
```

- [ ] **Step 4: Rewrite `initializeWorker` for classification**

Replace the body of `initializeWorker` with the version below. Buffer extraction
uses the existing `checkBufferAttached`/`copyBufferBytes`/`isDataView`/
`dataViewBuffer`/... helpers already in the file. (Object coercion and `data:`
land in later tasks; the `else` still throws for now.)

```cpp
jsi::Value initializeWorker(
    jsi::Runtime &rt,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  // Called only by 11-Worker.js: (self, script, inline, allowData).
  assert(count == 4);
  auto self = args[0].asObject(rt);
  const jsi::Value &input = args[1];
  bool inlineFlag = args[2].getBool();
  (void)args[3]; // allowData: consumed in a later task.

  IWorkerSetup *provider = getWorkerSetup(rt);

  if (input.isString()) {
    std::string str = input.asString(rt).utf8(rt);
    WorkerScriptSource source;
    if (provider && !inlineFlag) {
      source.url = std::move(str);
      source.needsResolve = true;
    } else {
      source.eagerBuffer =
          std::make_shared<jsi::StringBuffer>(std::move(str));
    }
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }

  if (input.isObject()) {
    jsi::Object obj = input.asObject(rt);
    std::string bytes;
    if (obj.isArrayBuffer(rt)) {
      jsi::ArrayBuffer ab = obj.getArrayBuffer(rt);
      checkBufferAttached(rt, ab);
      size_t size = ab.size(rt);
      bytes = copyBufferBytes(rt, std::move(ab), 0, size);
    } else if (obj.isTypedArray(rt)) {
      jsi::TypedArray ta = obj.getTypedArray(rt);
      jsi::ArrayBuffer ab = ta.buffer(rt);
      checkBufferAttached(rt, ab);
      bytes = copyBufferBytes(
          rt, std::move(ab), ta.byteOffset(rt), ta.byteLength(rt));
    } else if (isDataView(rt, obj)) {
      jsi::ArrayBuffer ab = dataViewBuffer(rt, obj);
      checkBufferAttached(rt, ab);
      bytes = copyBufferBytes(
          rt,
          std::move(ab),
          dataViewByteOffset(rt, obj),
          dataViewByteLength(rt, obj));
    } else {
      throwTypeError(
          rt,
          "Worker script must be a string, ArrayBuffer, TypedArray, or DataView");
    }
    WorkerScriptSource source;
    source.eagerBuffer = std::make_shared<jsi::StringBuffer>(std::move(bytes));
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }

  throwTypeError(
      rt,
      "Worker script must be a string, ArrayBuffer, TypedArray, or DataView");
}
```

Then update `installWorker` to create `initWorker` with arity 4:

```cpp
  jsi::Function initWorker = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "initWorker"), 4, initializeWorker);
```

- [ ] **Step 5: Write the failing tests (URL path, init hook, custom buffer, failure, nested)**

Add to `unittests/API/APITest.cpp`, inside the
`#if HERMES_ENABLE_CORE_EXTENSIONS` block near the other `HermesWorkerTest`
cases. Register the provider on `rt` before constructing the worker.

```cpp
TEST_P(HermesWorkerTest, WorkerFromUrlSource) {
  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  provider.onResolve = [](const std::string &url, std::string &) {
    EXPECT_EQ(url, "worker://main");
    return bufferFromString("__workerRan('source-ok');");
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  auto worker = eval("var w = new Worker('worker://main'); w;").asObject(*rt);

  std::string tag;
  ASSERT_TRUE(signal->wait(tag, 5000));
  EXPECT_EQ(tag, "source-ok");
  EXPECT_TRUE(provider.initCalled.load());

  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerFromUrlBytecode) {
  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("__workerRan('bc-ok');", bytecode));

  auto destroyed = std::make_shared<std::atomic<bool>>(false);
  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  provider.onResolve = [bytecode, destroyed](
                           const std::string &, std::string &) {
    return std::shared_ptr<const jsi::Buffer>(
        new TrackedBuffer(bytecode, destroyed));
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  auto worker = eval("var w = new Worker('worker://bc'); w;").asObject(*rt);

  std::string tag;
  ASSERT_TRUE(signal->wait(tag, 5000));
  EXPECT_EQ(tag, "bc-ok");
  // Bytecode is referenced, not copied: the buffer is still alive while the
  // worker runs.
  EXPECT_FALSE(destroyed->load());

  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerFromUrlResolverFailure) {
  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  provider.onResolve = [](const std::string &, std::string &error) {
    error = "not found";
    return std::shared_ptr<const jsi::Buffer>(nullptr);
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  auto worker = eval("var w = new Worker('worker://missing'); w;").asObject(*rt);

  // The script must NOT run: __workerRan is never called.
  std::string tag;
  EXPECT_FALSE(signal->wait(tag, 500));
  EXPECT_EQ(provider.lastUrl, "worker://missing");

  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerInlineOptionForcesSource) {
  TestWorkerSetup provider;
  provider.onResolve = [](const std::string &, std::string &error) {
    ADD_FAILURE() << "resolver must not be called for inline source";
    error = "unexpected";
    return std::shared_ptr<const jsi::Buffer>(nullptr);
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  // With {inline:true} the string is source even though a provider is set.
  auto worker =
      eval("var w = new Worker('var x = 1;', {inline: true}); w;")
          .asObject(*rt);
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
  EXPECT_TRUE(provider.lastUrl.empty());
}
```

- [ ] **Step 6: Build and run — verify fail then pass**

Run: `cmake --build cmake-build-asan --target APITests`
(This rebuilds the extensions bytecode from the edited `11-Worker.js`.)

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest*'`
Expected: all existing worker tests + the four new ones PASS. (Existing
string/buffer tests must still pass — they now exercise the eager-buffer path.)

- [ ] **Step 7: `arc f` and commit**

```bash
arc f
sl commit --reason "resolve worker URL input on worker thread via provider - sl help commit" \
  -m "$(printf '%s' '[SH]: Load Worker scripts from a URL via the provider

Classify a Worker string argument as a URL when a provider is
registered (unless {inline:true}); resolve it to a jsi::Buffer on the
worker thread, run the init hook, then evaluate. Propagate the provider
to nested worker runtimes.')"
```

---

### Task 3: Worker RuntimeConfig hook

Have `startWorker` call `configureWorkerRuntime` so the integrator controls the
worker `RuntimeConfig`.

**Files:**
- Modify: `API/hermes/extensions/Worker.cpp` (`startWorker`)
- Test: `unittests/API/APITest.cpp`

**Interfaces:**
- Consumes: `IWorkerSetup::configureWorkerRuntime` (Task 1),
  `startWorker` (Task 2).

- [ ] **Step 1: Write the failing test**

```cpp
TEST_P(HermesWorkerTest, WorkerConfigHookInvoked) {
  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  provider.onResolve = [](const std::string &, std::string &) {
    return bufferFromString("__workerRan('ok');");
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  auto worker = eval("var w = new Worker('worker://cfg'); w;").asObject(*rt);
  // configureWorkerRuntime runs synchronously on the constructor thread, so it
  // must already be recorded once construction returns.
  EXPECT_TRUE(provider.configureCalled.load());

  std::string tag;
  ASSERT_TRUE(signal->wait(tag, 5000));
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}
```

- [ ] **Step 2: Run it to confirm it fails**

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*WorkerConfigHookInvoked*'`
Expected: FAIL (`configureCalled` is false — the hook is not called yet).

- [ ] **Step 3: Call the config hook in `startWorker`**

In `Worker.cpp`, change the runtime creation lines in `startWorker` from:

```cpp
  auto *api = jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  // Default config for now; the config hook is added in a later task.
  auto workerRuntime = api->makeHermesRuntime(::hermes::vm::RuntimeConfig());
```

to:

```cpp
  auto *api = jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  // Seed a default config; let the integrator adjust it in place.
  ::hermes::vm::RuntimeConfig workerConfig;
  if (provider) {
    provider->configureWorkerRuntime(workerConfig);
  }
  auto workerRuntime = api->makeHermesRuntime(workerConfig);
```

- [ ] **Step 4: Build and run — verify pass**

Run: `cmake --build cmake-build-asan --target APITests`
Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest*'`
Expected: PASS (including `WorkerConfigHookInvoked`).

- [ ] **Step 5: `arc f` and commit**

```bash
arc f
sl commit --reason "let integrator configure worker RuntimeConfig - sl help commit" \
  -m "$(printf '%s' '[SH]: Call configureWorkerRuntime for worker runtimes

Seed a default RuntimeConfig and let the provider adjust it in place
before creating the worker runtime.')"
```

---

### Task 4: Object `ToString` coercion + primitive `TypeError`

Coerce a non-buffer *object* argument (e.g. an RN `URL`) via `ToString`, then
classify the result as a string. Keep `TypeError` for non-string, non-object
primitives.

**Files:**
- Modify: `API/hermes/extensions/Worker.cpp` (`initializeWorker`)
- Test: `unittests/API/APITest.cpp`, `test/hermes/worker/worker-url-object-coercion.js`

**Interfaces:**
- Consumes: `initializeWorker` classification (Task 2).

- [ ] **Step 1: Write the failing APITest**

```cpp
TEST_P(HermesWorkerTest, WorkerFromUrlObjectCoercion) {
  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  provider.onResolve = [](const std::string &url, std::string &) {
    EXPECT_EQ(url, "worker://obj");
    return bufferFromString("__workerRan('obj-ok');");
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  // A URL-like object stringifies to its href and is treated as a URL.
  auto worker =
      eval("var u = { toString() { return 'worker://obj'; } };"
           "var w = new Worker(u); w;")
          .asObject(*rt);

  std::string tag;
  ASSERT_TRUE(signal->wait(tag, 5000));
  EXPECT_EQ(tag, "obj-ok");
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);

  // Non-string, non-object primitives still throw.
  EXPECT_THROW(eval("new Worker(123);"), JSError);
  EXPECT_THROW(eval("new Worker(true);"), JSError);
}
```

- [ ] **Step 2: Run it to confirm it fails**

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*WorkerFromUrlObjectCoercion*'`
Expected: FAIL (`new Worker(u)` currently throws `TypeError` for a plain object).

- [ ] **Step 3: Add object coercion to `initializeWorker`**

In `Worker.cpp`, at the end of the `if (input.isObject())` block, replace the
final `else { throwTypeError(...); }` (the one inside the object block) so that a
non-buffer object is coerced instead of rejected. Change:

```cpp
    } else {
      throwTypeError(
          rt,
          "Worker script must be a string, ArrayBuffer, TypedArray, or DataView");
    }
    WorkerScriptSource source;
    source.eagerBuffer = std::make_shared<jsi::StringBuffer>(std::move(bytes));
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }
```

to:

```cpp
    } else {
      // Non-buffer object: coerce to string via ToString (invokes toString /
      // Symbol.toPrimitive), matching the web's USVString coercion, so an RN
      // URL is used as its href. Reclassify the result as a string.
      std::string str = input.toString(rt).utf8(rt);
      WorkerScriptSource source;
      if (provider && !inlineFlag) {
        source.url = std::move(str);
        source.needsResolve = true;
      } else {
        source.eagerBuffer =
            std::make_shared<jsi::StringBuffer>(std::move(str));
      }
      startWorker(rt, std::move(self), std::move(source), provider);
      return jsi::Value::undefined();
    }
    WorkerScriptSource source;
    source.eagerBuffer = std::make_shared<jsi::StringBuffer>(std::move(bytes));
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }
```

Note: `jsi::Value::toString(jsi::Runtime&)` performs JS `ToString` and returns a
`jsi::String`. The trailing `throwTypeError` after the object block (for
non-object primitives) stays as-is.

- [ ] **Step 4: Write the LIT coercion test (no provider needed)**

Create `test/hermes/worker/worker-url-object-coercion.js`:

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

// With no provider registered, a non-buffer object is coerced with ToString and
// (no provider) treated as source. Here the object's string form is valid JS.
var codeObject = {
  toString() {
    return "postMessage('coerced-ran');";
  },
};

var worker = new Worker(codeObject);
worker.onmessage = function (msg) {
  print(msg);
  worker.terminate();
};

// CHECK: coerced-ran
```

- [ ] **Step 5: Build and run both suites — verify pass**

Run: `cmake --build cmake-build-asan --target APITests`
Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest*'`
Expected: PASS.

Run: `LIT_OPTS="-j1" LIT_FILTER="worker/worker-url-object-coercion" cmake --build cmake-build-asan --target check-hermes`
Expected: PASS.

- [ ] **Step 6: `arc f` and commit**

```bash
arc f
sl commit --reason "coerce non-buffer object Worker arg via ToString - sl help commit" \
  -m "$(printf '%s' '[SH]: Coerce non-buffer object Worker arguments to string

Match the web USVString coercion: a non-buffer object argument (e.g. an
RN URL) is stringified via ToString and classified as a string.
Non-string, non-object primitives still throw TypeError.')"
```

---

### Task 5: `data:` URL decoding (`allowData`)

Add opt-in native `data:` decoding. When `{allowData:true}` and the (coerced)
string has the `data:` scheme, decode it into eager bytes on the constructor
thread, taking precedence over the resolver and source.

**Files:**
- Modify: `API/hermes/extensions/Worker.cpp`
- Test: `unittests/API/APITest.cpp`, `test/hermes/worker/worker-data-url.js`

**Interfaces:**
- Produces (internal to `Worker.cpp`):
  - `bool decodeDataUrl(const std::string &url, std::string &out);` — returns
    true and fills `out` for a well-formed `data:` URL; false if `url` is not a
    `data:` URL; throws `TypeError` (via `throwTypeError`) for a malformed one.

- [ ] **Step 1: Write the failing APITest**

```cpp
TEST_P(HermesWorkerTest, WorkerFromDataUrl) {
  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  // Resolver must NOT be used for a data: URL with allowData.
  provider.onResolve = [](const std::string &, std::string &error) {
    ADD_FAILURE() << "resolver must not be called for allowData data: URL";
    error = "unexpected";
    return std::shared_ptr<const jsi::Buffer>(nullptr);
  };
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  // Percent-encoded (non-base64) data: URL, source.
  auto worker = eval(
                    "var w = new Worker("
                    "'data:text/javascript,__workerRan(%22data-ok%22)%3B',"
                    "{allowData: true}); w;")
                    .asObject(*rt);
  std::string tag;
  ASSERT_TRUE(signal->wait(tag, 5000));
  EXPECT_EQ(tag, "data-ok");
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
  EXPECT_TRUE(provider.lastUrl.empty());
}

TEST_P(HermesWorkerTest, WorkerDataUrlRequiresAllowData) {
  TestWorkerSetup provider;
  provider.onResolve = [](const std::string &url, std::string &) {
    // Without allowData, a data: URL is just a URL handed to the resolver.
    EXPECT_EQ(url.rfind("data:", 0), 0u);
    return bufferFromString("__workerRan('via-resolver');");
  };
  provider.signal = std::make_shared<WorkerTestSignal>();
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  auto worker =
      eval("var w = new Worker('data:text/javascript,1'); w;").asObject(*rt);
  std::string tag;
  ASSERT_TRUE(provider.signal->wait(tag, 5000));
  EXPECT_EQ(tag, "via-resolver");
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerDataUrlBase64Bytecode) {
  // A base64 data: URL carrying bytecode still runs (magic-number path).
  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("__workerRan('data-bc');", bytecode));
  // base64-encode the bytecode using JS btoa is unavailable; build the URL in
  // C++ using the same encoder the test links against.
  std::string b64 = ::hermes::base64Encode(bytecode); // see note below
  std::string url = "data:application/octet-stream;base64," + b64;

  auto signal = std::make_shared<WorkerTestSignal>();
  TestWorkerSetup provider;
  provider.signal = signal;
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      &provider);

  auto g = rt->global();
  g.setProperty(*rt, "__dataUrl", String::createFromUtf8(*rt, url));
  auto worker =
      eval("var w = new Worker(__dataUrl, {allowData:true}); w;").asObject(*rt);
  std::string tag;
  ASSERT_TRUE(signal->wait(tag, 5000));
  EXPECT_EQ(tag, "data-bc");
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerMalformedDataUrlThrows) {
  castInterface<ISetWorkerSetup>(rt.get())->setWorkerSetup(
      nullptr);
  // Missing comma.
  EXPECT_THROW(
      eval("new Worker('data:text/javascript', {allowData:true});"), JSError);
  // Bad base64.
  EXPECT_THROW(
      eval("new Worker('data:;base64,@@@@', {allowData:true});"), JSError);
}
```

Note: if `::hermes::base64Encode` does not exist, inline a tiny base64 encoder in
the test (encoding is trivial and only needed by this test), or construct the
base64 with a fixed precomputed string for a known-tiny script. Do not add a
production base64 *encoder*; production only needs the decoder (Step 3).

- [ ] **Step 2: Run to confirm failure**

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*WorkerFromDataUrl*:*DataUrl*'`
Expected: FAIL (`data:` not decoded; `allowData` ignored).

- [ ] **Step 3: Implement the `data:` decoder and wire it in**

In `Worker.cpp`'s anonymous namespace, add a self-contained decoder (llvh has no
base64 helper):

```cpp
/// Decode one base64 character, or -1 if invalid (whitespace is skipped by the
/// caller). Standard alphabet only.
int base64Value(char c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (c == '+')
    return 62;
  if (c == '/')
    return 63;
  return -1;
}

/// Decode base64 \p in into \p out. Returns false on invalid input.
bool base64Decode(llvh::StringRef in, std::string &out) {
  out.clear();
  int buf = 0, bits = 0, pads = 0;
  for (char c : in) {
    if (c == '\n' || c == '\r' || c == ' ' || c == '\t')
      continue;
    if (c == '=') {
      ++pads;
      continue;
    }
    if (pads)
      return false; // data after padding
    int v = base64Value(c);
    if (v < 0)
      return false;
    buf = (buf << 6) | v;
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      out.push_back(static_cast<char>((buf >> bits) & 0xFF));
    }
  }
  return true;
}

/// Percent-decode \p in into \p out. Returns false on a malformed escape.
bool percentDecode(llvh::StringRef in, std::string &out) {
  out.clear();
  auto hex = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
  };
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 2 >= in.size())
        return false;
      int hi = hex(in[i + 1]), lo = hex(in[i + 2]);
      if (hi < 0 || lo < 0)
        return false;
      out.push_back(static_cast<char>((hi << 4) | lo));
      i += 2;
    } else {
      out.push_back(in[i]);
    }
  }
  return true;
}

/// If \p url is a data: URL, decode its payload into \p out and return true.
/// Return false if \p url is not a data: URL. Throw a TypeError (via
/// throwTypeError) for a malformed data: URL. Format:
/// data:[<mediatype>][;base64],<payload>
bool decodeDataUrl(jsi::Runtime &rt, const std::string &url, std::string &out) {
  llvh::StringRef ref(url);
  if (!ref.startswith("data:"))
    return false;
  ref = ref.drop_front(5); // after "data:"
  size_t comma = ref.find(',');
  if (comma == llvh::StringRef::npos) {
    throwTypeError(rt, "Malformed data: URL (missing comma)");
  }
  llvh::StringRef meta = ref.take_front(comma);
  llvh::StringRef payload = ref.drop_front(comma + 1);
  bool isBase64 = meta.endswith(";base64");
  bool ok = isBase64 ? base64Decode(payload, out) : percentDecode(payload, out);
  if (!ok) {
    throwTypeError(rt, "Malformed data: URL payload");
  }
  return true;
}
```

Add `#include "llvh/ADT/StringRef.h"` near the other includes if not already
transitively available.

Now wire it into `initializeWorker`. The classification for a string (used in
both the `input.isString()` branch and the object-coercion branch) must first
try `data:` when `allowData` is set. To keep it DRY, add a helper that builds a
`WorkerScriptSource` from a classified string and call it from both places:

```cpp
/// Build a WorkerScriptSource from a string argument \p str per the option
/// flags and whether a \p provider is present.
WorkerScriptSource sourceFromString(
    jsi::Runtime &rt,
    std::string str,
    bool inlineFlag,
    bool allowData,
    IWorkerSetup *provider) {
  WorkerScriptSource source;
  std::string decoded;
  if (!inlineFlag && allowData && decodeDataUrl(rt, str, decoded)) {
    if (decoded.empty()) {
      throwTypeError(rt, "Cannot create Worker from empty data: URL");
    }
    source.eagerBuffer =
        std::make_shared<jsi::StringBuffer>(std::move(decoded));
  } else if (provider && !inlineFlag) {
    source.url = std::move(str);
    source.needsResolve = true;
  } else {
    source.eagerBuffer = std::make_shared<jsi::StringBuffer>(std::move(str));
  }
  return source;
}
```

Replace the inline string-classification logic in both the `input.isString()`
branch and the object-coercion branch with calls to `sourceFromString(...)`.
For example the string branch becomes:

```cpp
  if (input.isString()) {
    WorkerScriptSource source = sourceFromString(
        rt, input.asString(rt).utf8(rt), inlineFlag, allowData, provider);
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }
```

and the object-coercion branch:

```cpp
      std::string str = input.toString(rt).utf8(rt);
      WorkerScriptSource source =
          sourceFromString(rt, std::move(str), inlineFlag, allowData, provider);
      startWorker(rt, std::move(self), std::move(source), provider);
      return jsi::Value::undefined();
```

Finally, replace `(void)args[3];` at the top of `initializeWorker` with:

```cpp
  bool allowData = args[3].getBool();
```

- [ ] **Step 4: Write the LIT data: tests (no provider needed)**

Create `test/hermes/worker/worker-data-url.js`:

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

// {allowData:true} decodes a data: URL natively (no provider needed).
var w1 = new Worker("data:text/javascript,postMessage('data-src')", {
  allowData: true,
});
w1.onmessage = function (msg) {
  print("src: " + msg);
  w1.terminate();
};

// CHECK: src: data-src
```

- [ ] **Step 5: Build and run both suites — verify pass**

Run: `cmake --build cmake-build-asan --target APITests`
Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest*'`
Expected: PASS (all worker tests, including the four new `data:` tests).

Run: `LIT_OPTS="-j1" LIT_FILTER="worker/worker-data-url" cmake --build cmake-build-asan --target check-hermes`
Expected: PASS.

- [ ] **Step 6: `arc f` and commit**

```bash
arc f
sl commit --reason "add opt-in native data: URL decoding for Worker - sl help commit" \
  -m "$(printf '%s' '[SH]: Add opt-in data: URL decoding to Worker ctor

With {allowData:true}, natively decode a data: URL (percent or base64)
into worker script bytes on the constructor thread, taking precedence
over the resolver and source. Malformed data: URLs throw TypeError.')"
```

---

### Task 6: Full-suite verification

Confirm nothing regressed across the whole worker suite and the changed
components.

**Files:** none (verification only).

- [ ] **Step 1: Run the entire worker LIT directory**

Run: `LIT_OPTS="-j1" LIT_FILTER="worker/" cmake --build cmake-build-asan --target check-hermes`
Expected: all `test/hermes/worker/*.js` PASS (existing + `worker-data-url.js` +
`worker-url-object-coercion.js`).

- [ ] **Step 2: Run all worker + serialization + runtime API unit tests**

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*Worker*:*WorkerSetup*'`
Expected: PASS.

- [ ] **Step 3: Confirm `arc f` is clean on all touched files**

Run: `arc f`
Expected: no diffs remain (all files already formatted).

- [ ] **Step 4: Sanity-check the non-extensions build path**

The interfaces and runtime plumbing (Task 1) are outside
`HERMES_ENABLE_CORE_EXTENSIONS` / `JSI_UNSTABLE`, while `Worker.cpp` is gated.
Confirm the tree still builds with the default configuration:

Run: `cmake --build cmake-build-asan --target hermes`
Expected: build succeeds.

---

## Self-Review

**Spec coverage:**
- URL input + integrator resolver → Task 2 (`resolveScript`, worker-thread
  resolve). ✓
- `jsi::Buffer` / zero-copy / mmap → Task 1 (signature), Task 2 (referenced,
  `TrackedBuffer` destructor assertion). ✓
- Init hook (`initWorkerRuntime`) → Task 1 (decl), Task 2 (call + test). ✓
- Config hook (`configureWorkerRuntime`) → Task 3. ✓
- `ICast`-based ABI-stable interface + catch-all delegation + no getter →
  Task 1. ✓
- Nested-worker provider propagation → Task 2 (`startWorker` re-attach). ✓
- String-vs-URL classification + `{inline}` → Task 2. ✓
- Object `ToString` coercion (RN `URL`) → Task 4. ✓
- Non-string/non-object primitive `TypeError` → Task 4. ✓
- `data:` + `{allowData}` (percent/base64, precedence, malformed→TypeError,
  base64 bytecode) → Task 5. ✓
- Error routing to `onerror` via existing `postError` path (resolver failure,
  init throw) → Task 2 (`throw jsi::JSError` reuses the existing catch). ✓
- Testing split (APITest for provider paths, LIT for provider-independent) →
  Tasks 2-5, Task 6. ✓
- Buffer paths unchanged → Task 2 keeps `checkBufferAttached`/`copyBufferBytes`
  logic; existing buffer tests must still pass (Step 6 checks). ✓

**Placeholder scan:** The only intentional blanks are the two `/* GENERATE a
fresh uuid */` markers in Task 1 (real UUIDs must be generated at implementation
time) and the `::hermes::base64Encode` note in Task 5 (test-only encoder; guidance
given). No production placeholders.

**Type consistency:** `WorkerScriptSource` (fields `eagerBuffer`, `url`,
`needsResolve`) is used identically in `startWorker`, `startWorkerThread`,
`initializeWorker`, and `sourceFromString`. `getWorkerSetup` returns
`IWorkerSetup*` throughout. `resolveScript` signature
(`std::shared_ptr<const jsi::Buffer>(const std::string&, std::string&)`) matches
between the interface (Task 1) and every test `onResolve` lambda (Tasks 2-5).
`decodeDataUrl(rt, url, out)` and `sourceFromString(rt, str, inlineFlag,
allowData, provider)` are defined once (Task 5) and called consistently.
