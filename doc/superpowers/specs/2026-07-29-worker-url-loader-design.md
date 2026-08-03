# Worker constructor: URL input via an integrator worker setup

Date: 2026-07-29
Status: Design approved, pending spec review

## Summary

Extend the Hermes `Worker` constructor (a JSI-based extension modeled on HTML
Web Workers) so `new Worker(url)` can load a worker's script by URL. Loading is
delegated to an **integrator-provided interface** that maps a URL string to a
`jsi::Buffer` of script bytes (precompiled Hermes bytecode *or* source),
enabling efficient native loading without shipping the script through JS. Because
the resolver hands back a `jsi::Buffer` (not a copied `std::string`), the
integrator can memory-map a bytecode file and let the VM reference it directly,
zero-copy and lazily paged.

The same integrator interface also lets the embedder:

- supply the `RuntimeConfig` used to create each worker runtime, and
- run one-time JS setup (polyfills, globals) in a freshly created worker runtime
  before the worker script executes.

The interface is a `jsi::ICast`-based cast interface, so future capabilities can
be added as new UUID-keyed interfaces without an ABI break.

This builds directly on the existing string/`ArrayBuffer`/`TypedArray`/`DataView`
constructor support; the buffer paths are unchanged.

## Motivation

Today `new Worker(x)` accepts a source string or a binary buffer, but the bytes
must already be materialized in JS before construction. Real embedders usually
have workers as named resources (a bundle path, a packager URL, a bytecode file
on disk). Forcing the script through JS means:

1. reading the file/URL in JS and passing the bytes across the constructor, and
2. no path for the integrator to hand the engine precompiled bytecode it already
   has natively.

A URL-based constructor with a native resolver lets the integrator return bytes
(bytecode or source) directly on the worker thread, skipping the JS round-trip
and enabling bytecode workers that skip parse/compile at startup.

Separately, worker runtimes today are created with a hardcoded default
`RuntimeConfig` and receive only the standard auto-installed extensions. Embedders
that need custom heap settings or extra polyfills in workers have no hook. The
same integrator interface fills both gaps.

## Design principles

- **Integrator interface is `ICast`-based and extensible.** The embedder
  implements a single `IWorkerSetup` (itself a `jsi::ICast`) and
  registers it as an opaque `jsi::ICast *`. New capabilities are added later as
  *new* UUID-keyed interfaces on the same object — never by adding methods to an
  existing interface — so the ABI is stable. All methods on the current
  interface are pure virtual.
- **Load off the constructor's thread.** URL resolution runs synchronously on
  the freshly spawned worker thread, so `new Worker(url)` returns immediately and
  never blocks the parent thread on I/O. Load failures surface via the worker's
  `onerror`, exactly like a script that throws at top level.
- **Zero-copy loading via `jsi::Buffer`.** The resolver returns a
  `std::shared_ptr<const jsi::Buffer>` rather than a `std::string`, so the
  integrator controls the storage: an mmap-backed buffer (unmapped in its
  destructor), a pointer into a shared/static bytecode blob, or a
  `jsi::StringBuffer` if it already has a `std::string`. For bytecode,
  `evaluateJavaScript` → `BCProviderFromBuffer` references the buffer without
  copying for the life of the worker runtime; the `shared_ptr` keeps it alive.
- **Reuse the existing byte pipeline.** The buffer flows through the existing
  `evaluateJavaScript` path, whose magic-number sniff (`isHermesBytecode`)
  selects bytecode vs. source. No VM or serialization changes.
- **No new jsi-library dependencies.** The interfaces live in
  `xplat/jsi/jsi/hermes-interfaces.h` beside `IEventLoopControl` /
  `ISetEventLoopControl`. That header is deliberately dependency-light (it is
  vendored into React Native's jsi copy and forward-declares Hermes types rather
  than including their headers). The config hook therefore takes
  `hermes::vm::RuntimeConfig &` by reference, which needs only a forward
  declaration — no `hermes/Public/RuntimeConfig.h` include.
- **String-vs-URL is explicit and backward compatible.** A string argument is
  treated as a URL only when a provider is registered; otherwise it remains
  source (today's behavior). An `{ inline: true }` construction option forces a
  string to be treated as source even when a provider is registered, and
  `{ allowData: true }` opts in to native `data:` URL decoding. A non-buffer
  object argument is coerced with `ToString` (so an RN `URL` works via its
  `href`).
- **Hermes owns no URL/Blob semantics.** The engine ships neither `URL` nor
  `Blob` (nor `fetch`); those are host-provided. URL *meaning* — file paths,
  `blob:`, schemes — belongs to the integrator's resolver, not the engine. The
  only exception is opt-in `data:` decoding, which is self-contained and needs no
  host. See "Relationship to Blob / createObjectURL" below.

## Interfaces (`xplat/jsi/jsi/hermes-interfaces.h`)

Add a forward declaration alongside the existing ones:

```cpp
namespace hermes::vm { class RuntimeConfig; }
```

### `IWorkerSetup`

Integrator-implemented. All methods pure virtual.

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
  static constexpr jsi::UUID uuid{/* fresh uuid */};

  /// Resolve \p url to worker script bytes, returned as an immutable
  /// jsi::Buffer holding either precompiled Hermes bytecode or UTF-8 source
  /// (the caller sniffs the magic number). The buffer may be memory-mapped or
  /// otherwise externally owned; for bytecode it is referenced (not copied) for
  /// the life of the worker runtime, so it must remain valid until the returned
  /// shared_ptr is released. On failure returns nullptr and sets \p error to a
  /// human-readable message. Called on the worker thread; must NOT perform
  /// operations on the worker runtime.
  virtual std::shared_ptr<const jsi::Buffer> resolveScript(
      const std::string &url,
      std::string &error) = 0;

  /// Run one-time JS setup in a freshly created worker runtime \p rt, after the
  /// standard extensions are installed and before the worker script runs.
  /// Called on the worker thread. May throw jsi::JSError (routed to onerror).
  virtual void initWorkerRuntime(jsi::Runtime &rt) = 0;

  /// Adjust the RuntimeConfig used to create a worker runtime. \p config is
  /// seeded with the Hermes default; the integrator may rebuild it in place
  /// (e.g. `config = config.rebuild().withGCConfig(...).build();`). Called on
  /// the constructor thread.
  virtual void configureWorkerRuntime(::hermes::vm::RuntimeConfig &config) = 0;

 protected:
  ~IWorkerSetup() = default;
};
```

### `ISetWorkerSetup`

Setter on the runtime. Stores an opaque `jsi::ICast *`; no getter.

```cpp
/// Interface for registering an IWorkerSetup on a Runtime. The stored
/// pointer is opaque (jsi::ICast*) so additional provider capabilities can be
/// added later as new cast interfaces without changing this ABI.
struct JSI_EXPORT ISetWorkerSetup : public jsi::ICast {
  static constexpr jsi::UUID uuid{/* fresh uuid */};

  /// Register \p provider (may be nullptr to clear). The provider must outlive
  /// the runtime. The provider is reachable afterward via
  /// `jsi::castInterface<IWorkerSetup>(&runtime)`.
  virtual void setWorkerSetup(jsi::ICast *provider) = 0;

 protected:
  ~ISetWorkerSetup() = default;
};
```

Rationale for no getter: the runtime re-exposes the provider through its own
`castInterface` (see below), so both *using* the provider and *propagating* it to
nested workers go through `jsi::castInterface<IWorkerSetup>(&runtime)`,
which returns an `IWorkerSetup *` that is itself a `jsi::ICast *`.

## Runtime plumbing (`API/hermes/hermes.cpp`)

`HermesRuntimeImpl` (which already implements `ISetEventLoopControl`):

1. Add a `jsi::ICast *workerSetup_{nullptr};` member and implement
   `ISetWorkerSetup::setWorkerSetup` to store it.
2. In `castInterface`:
   - return `this` for `ISetWorkerSetup::uuid` (as it does for
     `ISetEventLoopControl::uuid`), and
   - **catch-all delegate**: after all of the runtime's own UUIDs fail to match,
     if `workerSetup_` is set, return
     `workerSetup_->castInterface(interfaceUUID)`.

The catch-all delegation is what keeps the ABI promise: a future
`IWorkerSetupV2` (new UUID) is reachable through the runtime with **zero**
changes here — the runtime never learns provider UUIDs. The runtime's own
interfaces are matched first, so nothing it implements can be shadowed.

## Worker changes (`API/hermes/extensions/Worker.cpp`)

### Provider lookup helper

```cpp
IWorkerSetup *getWorkerSetup(jsi::Runtime &rt) {
  return jsi::castInterface<IWorkerSetup>(&rt); // nullptr if none
}
```

### Constructor dispatch (`initializeWorker`)

`initializeWorker` gains two boolean flags from JS, `inline` and `allowData`
(see JS glue below). Classification of the script argument:

```
ArrayBuffer / TypedArray / DataView   -> inline bytes (unchanged)
non-buffer object                     -> ToString(x), then classify as a string
non-string primitive (number, ...)    -> TypeError
string x:
  inline                              -> source (x.utf8(); today's behavior)
  allowData && x is a data: URL       -> native-decode (see below)
  provider != nullptr                 -> URL (defer bytes to worker thread)
  otherwise                           -> source
```

Buffers are always inline bytes regardless of whether a provider is registered.

**Object coercion (`URL` objects).** Web `Worker` coerces its argument to a
`USVString`, so `new Worker(urlObject)` uses the object's `href`. To match, a
non-buffer *object* argument is coerced with `ToString` (invoking `toString` /
`Symbol.toPrimitive`) and the result is classified as a string. This makes an RN
`URL` instance Just Work. Buffer types are checked first so they are never
stringified. Non-string, non-object primitives (`number`, `boolean`, `null`,
...) still throw `TypeError` (stricter than the web, saner for an embedding).

**`data:` URLs (`allowData`).** `data:` support is opt-in via
`{ allowData: true }` in the second argument, because worker `data:` URLs are
commonly disallowed. When `allowData` is set and the (coerced) string has the
`data:` scheme, `Worker.cpp` decodes it natively — parse
`data:[<mediatype>][;base64],<payload>`, percent-decode or base64-decode the
payload into an owned `std::string`, wrap in a `jsi::StringBuffer` — taking
precedence over both the resolver and source interpretation, and working with
**no** provider registered. Because the decoded bytes flow through the normal
magic-number sniff, a `data:...;base64,<bytecode>` URL runs as bytecode too.
Without `allowData`, a `data:` string is not special: it falls through to the
provider (if any) or source. `inline` wins over `allowData` (explicit "source").

The worker "script" payload handed to the worker thread is uniformly a
`std::shared_ptr<const jsi::Buffer>`, produced from one of two sources:

- **eager bytes** — for string-as-source, native-decoded `data:` URLs, and all
  buffer cases, the selected bytes are copied/decoded into an owned `std::string`
  (still required: buffer bytes live in the *parent* runtime's heap and cannot be
  referenced cross-thread/cross-runtime) and wrapped in a `jsi::StringBuffer`, or
- **a pending URL** — the URL string plus a "needs resolve" marker; the buffer is
  produced on the worker thread by `resolveScript` (string-as-URL case). This is
  the path that can be zero-copy / memory-mapped.

### `startWorker` (constructor thread)

Extended from the current helper:

1. `provider = getWorkerSetup(parentRuntime);`
2. Build the worker `RuntimeConfig`: start from a default; if `provider`, call
   `provider->configureWorkerRuntime(config)`; then `makeHermesRuntime(config)`.
   `makeHermesRuntime` already auto-installs the standard extensions.
3. Re-attach the provider to the worker runtime for nested workers:
   `castInterface<ISetWorkerSetup>(workerRuntime)->setWorkerSetup(provider);`
   (`provider` is an `IWorkerSetup *`, i.e. a `jsi::ICast *`; casting it
   again on the child still reaches every interface the object implements).
4. Install `postMessage`/`close`, create `WorkerNativeState`, register the
   event-loop task-queue source (unchanged).
5. Start the worker thread, passing either the eager bytes or the pending URL.

### `startWorkerThread` (worker thread)

Before entering the event loop, and inside the existing `try/catch` that guards
top-level evaluation:

1. **Resolve** (only for a pending URL): call
   `buffer = provider->resolveScript(url, error)`. If it returns `nullptr`, or
   the buffer's `size()` is 0, construct a JS `Error`/`TypeError` (from `error`)
   and route it through the existing `postError` path (→ `onerror`), the same as
   a script that throws at top level. The worker is not force-terminated; it
   enters its (idle) event loop and is cleaned up on `terminate()`/GC. For the
   eager-bytes cases the buffer already exists.
2. **Init hook**: if `provider`, call `provider->initWorkerRuntime(*workerRuntime)`.
   A thrown `jsi::JSError` is caught and routed to `onerror` (same handling as a
   script that throws); `JSINativeException` terminates the worker (as today).
3. **Evaluate**: pass the `std::shared_ptr<const jsi::Buffer>` to
   `evaluateJavaScript`, which sniffs the magic number and runs bytecode
   (referenced zero-copy) or compiles source (unchanged).

Ordering is resolve → init → evaluate.

## JS glue (`API/hermes/extensions/11-Worker.js`)

```js
extensions.Worker = function(nativeInit, nativeTerminate, nativePostMessage) {
  class Worker {
    constructor(script, options) {
      nativeInit(
        this,
        script,
        !!(options && options.inline),
        !!(options && options.allowData));
    }
    terminate() { return nativeTerminate.call(this); }
    postMessage(...args) { return nativePostMessage.call(this, ...args); }
  }
  globalThis.Worker = Worker;
};
```

The raw `script` value is forwarded unchanged so native does all type detection
(tamper-proof); only the two option booleans are read in JS. `initWorker` is
created with arity 4 in `installWorker` (`Worker.cpp`). The `inline` (force
source) and `allowData` (permit native `data:` decoding) option names are
provisional.

## Error handling and edge cases

- **Resolver failure** (`resolveScript` returns `nullptr`) → JS `Error` with the
  provider's message, delivered via `onerror`, same as a top-level throw; the
  worker then idles in its event loop (not force-terminated).
- **Resolver returns an empty buffer** (`size() == 0`) → `TypeError` ("empty
  worker script") via `onerror`, same handling as above.
- **`initWorkerRuntime` throws `jsi::JSError`** → routed to `onerror`;
  `JSINativeException` → terminate (matches existing script-eval handling).
- **String is really a URL but no provider is registered** → treated as source;
  normally a JS syntax error surfaced via `onerror`. Documented, not
  special-cased.
- **`{ inline: true }` with a buffer argument** → ignored; buffers are always
  inline bytes.
- **`data:` URL without `allowData`** → not natively decoded; treated as a normal
  URL (resolver, if any) or source. The integrator's resolver may still choose to
  handle `data:`.
- **Malformed `data:` URL with `allowData`** (bad base64, missing comma) →
  `TypeError` from the constructor (decoding happens on the constructor thread,
  before the worker thread starts).
- **Non-buffer object argument** → coerced via `ToString`; a throwing
  `toString`/`Symbol.toPrimitive` propagates out of the constructor as usual.
- **Non-string, non-object primitive** (`number`, `boolean`, `null`,
  `undefined`, `symbol`) → `TypeError`.
- **Detached / empty buffer inputs** → `TypeError` from the constructor
  (unchanged from the existing buffer support).
- **Nested workers** — a `new Worker` created *inside* a worker uses the same
  provider, because it was re-attached to the worker runtime in `startWorker`.

## Testing

Native coverage (`unittests/API/APITest.cpp`) carries the URL path, because the
provider is a C++-only integrator interface and cannot be registered from the
`hermes` CLI. A test `IWorkerSetup` maps URLs to canned bytes:

- **URL → source**: resolver returns a `jsi::Buffer` of UTF-8 source; worker runs
  and round-trips a message.
- **URL → bytecode**: resolver returns a `jsi::Buffer` of precompiled Hermes
  bytecode; worker runs (magic-number path) and round-trips a message.
- **Custom buffer type**: resolver returns a non-`StringBuffer` `jsi::Buffer`
  subclass (e.g. one that owns a `malloc`/`mmap`-style region and frees/unmaps in
  its destructor); worker runs and the buffer's destructor is observed to run
  after the worker is torn down, confirming lifetime is tied to the worker
  runtime, not copied.
- **Init hook**: `initWorkerRuntime` installs a global that the worker script
  reads back and posts to the parent.
- **Config hook**: `configureWorkerRuntime` is invoked and its config is used
  (e.g. observe a setting the test toggles, or assert invocation).
- **Resolver failure** → worker's `onerror` fires with the provider's message.
- **`{ inline: true }`** forces a string to run as source even with a provider
  registered.
- **Nested worker** created inside a worker resolves through the same provider.
- **URL-object coercion**: a fake `URL`-like object (custom `toString`/`href`) is
  accepted and classified by its stringified value.

Lit tests under `test/hermes/worker/` cover everything that does **not** need a
provider — the existing string/buffer paths, plus the new provider-independent
behavior: `data:` decoding with `{ allowData: true }` (source and
`;base64,<bytecode>`), `data:` rejected/ignored without `allowData`, `{ inline:
true }` forcing source, object argument coercion via `toString`, and `TypeError`
for non-string/non-object primitives. Only the provider-driven URL/init/config
paths require the native `APITest` (the provider is a C++-only interface, not
registrable from the CLI).

## Relationship to Blob / `createObjectURL`

On the web, the portable, spec-compliant way to start a worker from in-memory
bytes is a Blob object URL:

```js
const url = URL.createObjectURL(new Blob([src], {type: "text/javascript"}));
new Worker(url);
```

This exists because the web `Worker` constructor accepts **only** a URL — a Blob
URL is the standard mechanism for exposing in-memory bytes behind a fetchable
URL. It is more portable than this extension's direct `ArrayBuffer` input, which
is Hermes-specific.

Hermes, however, provides **no `URL` and no `Blob`** (nor `createObjectURL` /
`fetch`); it is an ECMAScript engine, not a browser. So Blob URLs are not a
built-in alternative here — they would require a `Blob` type plus an object-URL
registry, which are host concerns.

The three input mechanisms are therefore layered by ownership, not competing:

- **`ArrayBuffer` / `TypedArray` / `DataView` (already shipped)** — the
  engine-local shortcut for "bytes already in JS." Most direct; Hermes-specific.
- **URL + resolver (this spec)** — "load these bytes natively/externally,"
  including mmap'd bytecode. The efficient native path. Hermes owns no URL
  meaning; the resolver does.
- **Blob URL (host-level, not in this spec)** — the spec-compliant web idiom.
  When web-source compatibility is a goal, the host implements `Blob` +
  `URL.createObjectURL` and backs `blob:` URLs with an `IWorkerSetup`
  that returns the registered bytes. It is composed *on top of* the resolver,
  not a replacement for it or for the `ArrayBuffer` path.

This relationship is also documented in the `IWorkerSetup` header
comment, so integrators discover the Blob integration point at the interface.

## Out of scope

- **Async URL resolution.** Resolution is synchronous on the worker thread; no
  promise/event-loop-pending-load mechanism. A future capability could add this
  as a new provider interface.
- **Per-worker config/init context.** `configureWorkerRuntime` and
  `initWorkerRuntime` receive no URL/identity. If per-worker variation is needed,
  a future `V2` interface can pass context.
- **`Blob` / `URL.createObjectURL` / `URL`.** Not provided by Hermes and not
  added here; they are host-level and, if desired, compose on top of the resolver
  (see "Relationship to Blob / createObjectURL"). Only opt-in `data:` decoding is
  built in.
- **`SharedArrayBuffer` / transfer changes.** Unchanged; this spec covers
  constructor input only.
- **Changes to the shared `jsi.h` `Runtime` interface.** The new interfaces live
  in `hermes-interfaces.h`, which already hosts the sibling event-loop
  interfaces.
- **Zero-copy of the eager (non-URL) paths.** String-as-source and buffer inputs
  still copy into an owned `std::string` (wrapped in `jsi::StringBuffer`), because
  they originate in the parent runtime's heap. Only the URL path is zero-copy;
  making the buffer inputs zero-copy would require cross-runtime buffer sharing,
  which is out of scope.
