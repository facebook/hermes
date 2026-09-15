# Worker constructor: accept ArrayBuffer / TypedArray / DataView input

Date: 2026-07-28
Status: Design approved, pending spec review

## Summary

Extend the Hermes `Worker` constructor (a JSI-based extension modeled on HTML
Web Workers) so it accepts binary input in addition to a source string. The
binary input may contain either precompiled Hermes bytecode or UTF-8 source;
`evaluateJavaScript` already sniffs the bytecode magic number and dispatches
accordingly, so no VM or serialization changes are needed.

Accepted input types:

- `string` — JavaScript source (existing behavior, unchanged).
- `ArrayBuffer` — raw bytes.
- Any `TypedArray` (`Uint8Array`, `Float64Array`, ...) — honors
  `byteOffset`/`byteLength`.
- `DataView` — honors `byteOffset`/`byteLength`.

## Motivation

Today `new Worker(x)` requires `x` to be a string. The native path converts it
via `jsi::String::utf8()` and wraps the result in a `jsi::StringBuffer` before
calling `evaluateJavaScript`. Because `utf8()` re-encodes the string's code
points, arbitrary binary (e.g. Hermes bytecode, whose magic number contains
bytes >= 0x80) cannot survive the round-trip. This blocks two use cases:

1. Running precompiled bytecode in a worker to skip parse/compile at startup.
2. Passing raw/binary source faithfully.

Accepting a buffer type carries the raw bytes intact. `evaluateJavaScript`'s
existing magic-number check (`isHermesBytecode`) then selects the bytecode or
source path automatically.

## Design principles

- **All type detection in native C++.** The JS shim forwards the raw argument
  unchanged; C++ validates. This is tamper-proof: user code cannot subvert
  detection by replacing `ArrayBuffer`, `ArrayBuffer.isView`, `DataView`, or
  `Symbol.hasInstance`, because native reads VM internal slots (for
  ArrayBuffer/TypedArray) and uses install-time-captured prototype getters (for
  DataView) rather than live globals.
- **Copy at construction.** The byte slice is copied into an owned buffer on
  the parent thread, then moved to the worker thread. This avoids all
  cross-thread and cross-runtime GC hazards. The source ArrayBuffer belongs to
  the parent runtime; the worker runs on its own thread with its own runtime,
  and bytecode is referenced (not copied) by `BCProviderFromBuffer` for the
  life of the worker runtime, so the bytes must be independently owned. A
  one-time startup copy is acceptable; bytecode still skips compilation. This
  mirrors how the existing string path already copies via `utf8()`.
- **No change to the shared `jsi.h` contract.** DataView introspection is added
  as contained C++ helpers in the Hermes extensions layer, not as new
  `jsi::Runtime` virtual methods. ArrayBuffer and TypedArray use the existing
  stable JSI methods.

## Detection and dispatch flow

`11-Worker.js` constructor (trivial, unchanged from today):

```js
constructor(script) {
  nativeInit(this, script); // forwards raw value; native does all validation
}
```

`initializeWorker(self, x)` in `Worker.cpp` becomes the dispatcher. It selects
a byte range from `x` and copies it into a `std::string`:

```
isString(x)                      -> x.utf8()                       (existing string path)
isArrayBuffer(x)                 -> bytes [0, size)
isTypedArray(x)                  -> buffer()/byteOffset()/byteLength()
isDataView(x)  [new C++ helper]  -> dataViewBuffer/Offset/Length()
else                             -> throwTypeError
```

The resulting `std::string` is passed to the extracted `startWorker` helper,
which performs the existing runtime/handler/native-state/event-loop setup and
starts the worker thread. Downstream is unchanged: `startWorkerThread` wraps the
`std::string` in a `jsi::StringBuffer` and calls `evaluateJavaScript`, which
sniffs the magic number and runs bytecode or compiles source.

`std::string` carries arbitrary binary safely, so the binary paths converge with
the source path at the `std::string` boundary.

## Components

### `API/hermes/extensions/Intrinsics.{h,cpp}`

`ExtensionIntrinsics` gains three captured DataView prototype getter functions,
grabbed in its constructor (which runs in `captureIntrinsics`, before any user
code) via the genuine `Object.getOwnPropertyDescriptor`:

```cpp
jsi::Function dataViewBufferGetter;
jsi::Function dataViewByteOffsetGetter;
jsi::Function dataViewByteLengthGetter;
```

Capture sketch (in the `ExtensionIntrinsics` constructor):

```cpp
auto getOwnDesc = rt.global()
    .getPropertyAsObject(rt, "Object")
    .getPropertyAsFunction(rt, "getOwnPropertyDescriptor");
auto dvProto = rt.global()
    .getPropertyAsObject(rt, "DataView")
    .getPropertyAsObject(rt, "prototype");
auto getterOf = [&](const char *name) {
  return getOwnDesc
      .call(rt, dvProto, jsi::String::createFromAscii(rt, name))
      .asObject(rt)
      .getPropertyAsFunction(rt, "get");
};
// store getterOf("buffer"), getterOf("byteOffset"), getterOf("byteLength")
```

New free helpers in the same layer, shaped to mirror the existing TypedArray
JSI methods:

```cpp
bool isDataView(jsi::Runtime &rt, const jsi::Object &obj);
jsi::ArrayBuffer dataViewBuffer(jsi::Runtime &rt, const jsi::Object &dv);
size_t dataViewByteOffset(jsi::Runtime &rt, const jsi::Object &dv);
size_t dataViewByteLength(jsi::Runtime &rt, const jsi::Object &dv);
```

- `isDataView` calls the captured `buffer` getter on `obj`; a thrown
  `jsi::JSError` means "not a DataView" (the getter's spec-mandated internal
  brand-check). This is immune to own-property shadowing and later global
  replacement. The `buffer` getter is used rather than `byteLength`/`byteOffset`
  because it does not reject a genuine but detached DataView (detachment is then
  reported as a detached-buffer error instead of misclassifying the object).
- The three accessors assume a genuine DataView (call after `isDataView`), read
  the captured getter, and convert the result. `dataViewBuffer` returns the
  underlying `jsi::ArrayBuffer`.

### `API/hermes/extensions/Worker.cpp`

1. Extract the current runtime creation / `postMessage`+`close` install /
   `WorkerNativeState` creation / event-loop registration / thread start into
   a shared helper: `startWorker(jsi::Runtime &rt, jsi::Object self,
   std::string script)`.
2. Rewrite `initializeWorker` as the dispatcher above. Each branch produces the
   raw-byte `std::string`; the view branches slice using
   `byteOffset`/`byteLength`. Then call `startWorker`.
3. `startWorkerThread` and all message-passing / lifecycle code unchanged.

### `API/hermes/extensions/11-Worker.js`

Constructor reverts to `nativeInit(this, script)` — one native helper, no type
logic in JS.

## Error handling and edge cases

- **Wrong type** (not string / ArrayBuffer / TypedArray / DataView) ->
  `TypeError` via the captured `TypeError` constructor.
- **Empty binary input** (0-byte ArrayBuffer / TypedArray / DataView) ->
  `TypeError`. A zero-byte binary buffer is meaningless (cannot be bytecode,
  and empty source via a binary container is pointless).
- **Empty string** (`new Worker("")`) -> unchanged existing behavior (runs an
  empty program). Not a breaking change.
- **Detached buffer** (ArrayBuffer detached, or a view over a detached buffer)
  -> `TypeError`, rather than silently running an empty program. Detection:
  `checkBufferAttached` calls `ArrayBuffer::detached(rt)` on the underlying
  buffer (for a view, its backing buffer is checked the same way) before any
  size/`byteOffset`/`byteLength` query, yielding a "detached buffer" TypeError
  directly.
- **Bounds**: view slices honor `byteOffset`/`byteLength`; ArrayBuffer uses the
  full `size`. Copy exactly the selected range.

## Testing

Lit tests under `test/hermes/worker/`:

- Construct from `ArrayBuffer` containing UTF-8 source; assert the worker runs
  and round-trips a message.
- Construct from a `Uint8Array` with a non-zero `byteOffset` (slice of a larger
  buffer); assert only the intended bytes execute.
- Construct from a `DataView` (with `byteOffset`); assert it runs.
- Construct from precompiled Hermes bytecode (produced via `shermes`/`hermesc`
  in the test) held in a `Uint8Array`/`ArrayBuffer`; assert it runs and skips
  compilation (magic-number path).

Negative tests:

- Non-buffer argument (number, object, null) -> `TypeError`.
- Empty ArrayBuffer / TypedArray / DataView -> `TypeError`.
- Detached ArrayBuffer -> `TypeError`.
- Tamper-resistance: after `ArrayBuffer = null`, `DataView = <fake>`,
  overriding `Symbol.hasInstance`, and shadowing an instance's own
  `byteOffset`/`byteLength`, construction from genuine buffers still succeeds
  and reads the correct bytes.

## Out of scope

- No changes to the shared `jsi.h` `Runtime` interface (DataView stays as
  contained extension-layer helpers).
- No `SharedArrayBuffer` support (consistent with the existing transfer path,
  which rejects it).
- No new zero-copy / transfer-of-input optimization; input is always copied at
  construction.
- No change to `postMessage` transfer semantics (this spec is about the
  constructor input only).
