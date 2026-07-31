# Worker ArrayBuffer/TypedArray/DataView Input Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let the Hermes `Worker` constructor accept `ArrayBuffer`, `TypedArray`, and `DataView` (carrying either precompiled bytecode or UTF-8 source) in addition to a source string.

**Architecture:** All type detection stays in native C++ (tamper-proof, reads VM internal slots for ArrayBuffer/TypedArray and install-time-captured `DataView.prototype` getters for DataView). The selected byte range is copied into a `std::string` on the parent thread and handed to the existing worker startup path; `evaluateJavaScript` already sniffs the bytecode magic number and dispatches source-vs-bytecode. The JS shim (`11-Worker.js`) is unchanged — it already forwards the raw argument.

**Tech Stack:** C++17 (no exceptions/RTTI in general VM code, but the extensions layer and `Worker.cpp` already use `try`/`catch`), JSI, Hermes extensions layer, GoogleTest (`APITests`), LLVM lit + FileCheck.

## Global Constraints

- Copyright header (Meta MIT, from `CLAUDE.md`) on every new file.
- C++ style: 80-column lines, 2-space indent, doc comment on every declaration.
- Feature is gated by `JSI_UNSTABLE` (Worker already is) and, for C++ tests, `HERMES_ENABLE_CORE_EXTENSIONS`.
- Run `arc f` after editing any `.cpp`/`.h` before committing.
- Every `sl` invocation passes `--reason "<intent> - sl help <cmd>"`. Commit messages use the `[SH]` format and hard-wrap the body at 72 columns.
- Detection order in the constructor: `isString` -> `isArrayBuffer` -> `isTypedArray` -> `isDataView` -> `TypeError`.
- Empty binary input (0 bytes) and detached buffers throw `TypeError`. Empty *string* keeps existing behavior (runs empty program).
- Copy at construction; never retain/transfer the parent's ArrayBuffer.

---

### Task 1: Native constructor accepts ArrayBuffer / TypedArray / DataView

**Files:**
- Modify: `API/hermes/extensions/Intrinsics.h` (add 3 captured getters + 4 helper decls)
- Modify: `API/hermes/extensions/Intrinsics.cpp` (capture getters + implement helpers)
- Modify: `API/hermes/extensions/Worker.cpp` (extract `startWorker`, add `copyBufferBytes`, rewrite `initializeWorker` as dispatcher)
- Test: `unittests/API/APITest.cpp` (new `TEST_P(HermesWorkerTest, ...)` cases)

**Interfaces:**
- Produces (in `namespace facebook::hermes`, declared in `Intrinsics.h`):
  - `bool isDataView(jsi::Runtime &rt, const jsi::Object &obj);`
  - `jsi::ArrayBuffer dataViewBuffer(jsi::Runtime &rt, const jsi::Object &dv);`
  - `size_t dataViewByteOffset(jsi::Runtime &rt, const jsi::Object &dv);`
  - `size_t dataViewByteLength(jsi::Runtime &rt, const jsi::Object &dv);`
- Consumes: existing `getIntrinsics(rt)`, `throwTypeError(rt, msg)`; JSI `Object::isArrayBuffer/getArrayBuffer/isTypedArray/getTypedArray`, `ArrayBuffer::size/data/detached`, `TypedArray::buffer/byteOffset/byteLength`, `Function::callWithThis`.

- [ ] **Step 1: Write the failing test**

Add these cases inside the existing `#if HERMES_ENABLE_CORE_EXTENSIONS` block in `unittests/API/APITest.cpp` (right after the `WebWorkerBasic` test, before the `#endif`):

```cpp
TEST_P(HermesWorkerTest, WorkerFromArrayBuffer) {
  // Source carried in an ArrayBuffer must construct a Worker (no throw).
  auto code = R"(
var bytes = new TextEncoder().encode("var x = 1;");
var worker = new Worker(bytes.buffer);
worker;
)";
  auto worker = eval(code).asObject(*rt);
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerFromTypedArrayWithOffset) {
  // A Uint8Array view with a non-zero byteOffset must slice correctly.
  auto code = R"(
var whole = new TextEncoder().encode("XXvar y = 2;");
var view = new Uint8Array(whole.buffer, 2); // skip the leading "XX"
var worker = new Worker(view);
worker;
)";
  auto worker = eval(code).asObject(*rt);
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerFromDataView) {
  auto code = R"(
var bytes = new TextEncoder().encode("var z = 3;");
var dv = new DataView(bytes.buffer);
var worker = new Worker(dv);
worker;
)";
  auto worker = eval(code).asObject(*rt);
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}

TEST_P(HermesWorkerTest, WorkerFromBinaryErrors) {
  // Non-buffer, non-string argument.
  EXPECT_THROW(eval("new Worker({});"), JSError);
  // Empty binary input.
  EXPECT_THROW(eval("new Worker(new ArrayBuffer(0));"), JSError);
  EXPECT_THROW(eval("new Worker(new Uint8Array(0));"), JSError);
  // Detached ArrayBuffer. Detach via HermesInternal, matching the existing
  // DetachedArrayBuffer / ArrayBufferDetached tests in this file.
  EXPECT_THROW(
      eval(R"(
var ab = new ArrayBuffer(8);
HermesInternal.detachArrayBuffer(ab);
new Worker(ab);
)"),
      JSError);
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build cmake-build-asan --target APITests && cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest.WorkerFrom*'`
Expected: FAIL — the ArrayBuffer/TypedArray/DataView cases throw `TypeError` today ("Must provide the source code as a String"), so the positive tests fail.

- [ ] **Step 3: Add the DataView getters and helper declarations to `Intrinsics.h`**

In `struct ExtensionIntrinsics`, after `jsi::Function uint8Array;`, add:

```cpp
  /// DataView.prototype accessor getters, captured before user code runs so
  /// DataView detection cannot be subverted by replacing globals.
  jsi::Function dataViewBufferGetter;
  jsi::Function dataViewByteOffsetGetter;
  jsi::Function dataViewByteLengthGetter;
```

After the `throwRangeError` declaration (before the closing `} // namespace hermes`), add:

```cpp
/// \return true iff \p obj is a genuine DataView. Brand-checks via the
/// captured DataView.prototype.buffer getter (immune to shadowing and global
/// replacement). The buffer getter is used rather than byteLength/byteOffset
/// because it does not reject a genuine but detached DataView.
bool isDataView(jsi::Runtime &rt, const jsi::Object &obj);

/// \return the underlying ArrayBuffer of the DataView \p dv.
/// \pre isDataView(rt, dv) is true.
jsi::ArrayBuffer dataViewBuffer(jsi::Runtime &rt, const jsi::Object &dv);

/// \return the byteOffset of the DataView \p dv.
/// \pre isDataView(rt, dv) is true.
size_t dataViewByteOffset(jsi::Runtime &rt, const jsi::Object &dv);

/// \return the byteLength of the DataView \p dv.
/// \pre isDataView(rt, dv) is true.
size_t dataViewByteLength(jsi::Runtime &rt, const jsi::Object &dv);
```

- [ ] **Step 4: Capture the getters and implement the helpers in `Intrinsics.cpp`**

Add a file-local helper in the anonymous namespace (after the `kIntrinsicsUUID` definition):

```cpp
/// Fetch the getter function of DataView.prototype.<name> using the genuine
/// Object.getOwnPropertyDescriptor. Called only during capture, before user
/// code runs.
jsi::Function captureDataViewGetter(jsi::Runtime &rt, const char *name) {
  auto getOwnDesc = rt.global()
                        .getPropertyAsObject(rt, "Object")
                        .getPropertyAsFunction(rt, "getOwnPropertyDescriptor");
  auto dvProto = rt.global()
                     .getPropertyAsObject(rt, "DataView")
                     .getPropertyAsObject(rt, "prototype");
  return getOwnDesc
      .call(rt, dvProto, jsi::String::createFromAscii(rt, name))
      .asObject(rt)
      .getPropertyAsFunction(rt, "get");
}
```

Extend the constructor initializer list:

```cpp
ExtensionIntrinsics::ExtensionIntrinsics(jsi::Runtime &rt)
    : typeError(rt.global().getPropertyAsFunction(rt, "TypeError")),
      rangeError(rt.global().getPropertyAsFunction(rt, "RangeError")),
      uint8Array(rt.global().getPropertyAsFunction(rt, "Uint8Array")),
      dataViewBufferGetter(captureDataViewGetter(rt, "buffer")),
      dataViewByteOffsetGetter(captureDataViewGetter(rt, "byteOffset")),
      dataViewByteLengthGetter(captureDataViewGetter(rt, "byteLength")) {}
```

Add the helper implementations before the closing `} // namespace hermes`:

```cpp
bool isDataView(jsi::Runtime &rt, const jsi::Object &obj) {
  const auto &intrinsics = getIntrinsics(rt);
  try {
    // The buffer getter throws a TypeError unless `obj` has a genuine
    // [[DataView]] internal slot. Unlike byteLength/byteOffset it does not
    // reject a genuine but detached DataView.
    intrinsics.dataViewBufferGetter.callWithThis(rt, obj);
    return true;
  } catch (const jsi::JSError &) {
    return false;
  }
}

jsi::ArrayBuffer dataViewBuffer(jsi::Runtime &rt, const jsi::Object &dv) {
  const auto &intrinsics = getIntrinsics(rt);
  return intrinsics.dataViewBufferGetter.callWithThis(rt, dv)
      .asObject(rt)
      .getArrayBuffer(rt);
}

size_t dataViewByteOffset(jsi::Runtime &rt, const jsi::Object &dv) {
  const auto &intrinsics = getIntrinsics(rt);
  return static_cast<size_t>(
      intrinsics.dataViewByteOffsetGetter.callWithThis(rt, dv).asNumber());
}

size_t dataViewByteLength(jsi::Runtime &rt, const jsi::Object &dv) {
  const auto &intrinsics = getIntrinsics(rt);
  return static_cast<size_t>(
      intrinsics.dataViewByteLengthGetter.callWithThis(rt, dv).asNumber());
}
```

- [ ] **Step 5: Extract `startWorker` and add `copyBufferBytes` in `Worker.cpp`**

Add these two functions in the anonymous namespace, immediately before `initializeWorker` (around line 526):

```cpp
/// Copy [\p offset, \p offset + \p length) of \p ab into a std::string.
/// Throws TypeError if the buffer is detached or the range is empty.
/// std::string carries arbitrary binary faithfully (no UTF-8 re-encoding).
std::string copyBufferBytes(
    jsi::Runtime &rt,
    jsi::ArrayBuffer ab,
    size_t offset,
    size_t length) {
  if (ab.detached(rt)) {
    throwTypeError(rt, "Cannot create Worker from a detached buffer");
  }
  if (length == 0) {
    throwTypeError(rt, "Cannot create Worker from empty binary input");
  }
  const uint8_t *data = ab.data(rt);
  return std::string(
      reinterpret_cast<const char *>(data + offset), length);
}

/// Create the Worker runtime/thread and attach state to \p self, running the
/// bytes in \p script (source or bytecode, decided by evaluateJavaScript).
/// Shared by all constructor input types.
void startWorker(jsi::Runtime &rt, jsi::Object self, std::string script) {
  auto *api = jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  auto workerRuntime = api->makeHermesRuntime(::hermes::vm::RuntimeConfig());
  auto workerState = std::make_shared<WorkerState>(rt, self);

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

  workerNativeState->startWorkerThread(std::move(script));
}
```

- [ ] **Step 6: Rewrite `initializeWorker` as the dispatcher**

Replace the entire body of `initializeWorker` (from `assert(count == 2);` through `return jsi::Value::undefined();`) with:

```cpp
  // This is only called by the Worker extension script in `11-Worker.js`, so we
  // can guarantee that the argument count is 2.
  assert(count == 2);

  // The self object is provided by `11-Worker.js`, so it is always an Object.
  auto self = args[0].asObject(rt);
  const jsi::Value &input = args[1];

  std::string script;
  if (input.isString()) {
    script = input.asString(rt).utf8(rt);
  } else if (input.isObject()) {
    jsi::Object obj = input.asObject(rt);
    if (obj.isArrayBuffer(rt)) {
      jsi::ArrayBuffer ab = obj.getArrayBuffer(rt);
      size_t size = ab.size(rt);
      script = copyBufferBytes(rt, std::move(ab), 0, size);
    } else if (obj.isTypedArray(rt)) {
      jsi::TypedArray ta = obj.getTypedArray(rt);
      size_t offset = ta.byteOffset(rt);
      size_t length = ta.byteLength(rt);
      jsi::ArrayBuffer ab = ta.buffer(rt);
      script = copyBufferBytes(rt, std::move(ab), offset, length);
    } else if (isDataView(rt, obj)) {
      size_t offset = dataViewByteOffset(rt, obj);
      size_t length = dataViewByteLength(rt, obj);
      jsi::ArrayBuffer ab = dataViewBuffer(rt, obj);
      script = copyBufferBytes(rt, std::move(ab), offset, length);
    } else {
      throwTypeError(
          rt,
          "Worker script must be a string, ArrayBuffer, TypedArray, or DataView");
    }
  } else {
    throwTypeError(
        rt,
        "Worker script must be a string, ArrayBuffer, TypedArray, or DataView");
  }

  startWorker(rt, std::move(self), std::move(script));
  return jsi::Value::undefined();
```

If `<string>` is not already transitively included, add `#include <string>` near the top of `Worker.cpp` (after the existing standard includes).

- [ ] **Step 7: Run the tests to verify they pass**

Run: `arc f && cmake --build cmake-build-asan --target APITests && cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest*'`
Expected: PASS — all `WorkerFrom*` cases and the existing `WebWorkerBasic` pass.

- [ ] **Step 8: Commit**

```bash
sl commit --reason "worker accepts ArrayBuffer/TypedArray/DataView input - sl help commit" --message "$(cat <<'EOF'
[SH]: Accept ArrayBuffer/TypedArray/DataView in Worker ctor

Summary:
Extend the Worker constructor to accept binary input (ArrayBuffer,
any TypedArray, or DataView) in addition to a source string. The
bytes may be precompiled bytecode or UTF-8 source; evaluateJavaScript
already sniffs the magic number and dispatches accordingly.

All type detection happens in native C++ and is tamper-proof:
ArrayBuffer/TypedArray use VM internal-slot checks, and DataView uses
DataView.prototype getters captured in captureIntrinsics before user
code runs. The selected byte range is copied into a std::string on
the parent thread, avoiding the UTF-8 re-encoding that previously
corrupted binary input. Empty binary input and detached buffers throw
TypeError.

Test Plan: cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest*'

Reviewers: Hermes

Subscribers:

Tasks:

Tags:
EOF
)"
```

---

### Task 2: LIT round-trip, byte fidelity, and tamper-resistance tests

**Files:**
- Create: `test/hermes/worker/worker-from-buffer.js`

**Interfaces:**
- Consumes: the constructor behavior from Task 1; the `hermes`/`shermes` CLIs (which provide an event loop that drains worker->parent messages).
- Produces: nothing consumed by later tasks.

Note: deliberately avoids `TextEncoder` (it is gated on `core_extensions`, is
untested under `shermes -exec`, and needs a UTF-8 locale). Bytes are built with
`Uint8Array.from(str, c => c.charCodeAt(0))` for ASCII source, and high-byte
fidelity is proven with an explicit byte array whose worker echoes a charCode
as ASCII output — keeping both RUN lines and ASCII-only FileCheck matching.

- [ ] **Step 1: Write the LIT test**

Create `test/hermes/worker/worker-from-buffer.js`:

```javascript
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec %s | %FileCheck %s --match-full-lines

// The Worker constructor accepts ArrayBuffer, TypedArray, and DataView carrying
// source, faithfully preserving bytes (including bytes >= 0x80).

// Build source bytes from ASCII text without TextEncoder (works under shermes).
function asciiBytes(str) {
  return Uint8Array.from(str, function (c) { return c.charCodeAt(0); });
}

var src = 'onmessage = function() { postMessage("ran"); };';
var bytes = asciiBytes(src);

// 1) ArrayBuffer
var w1 = new Worker(bytes.buffer);
w1.onmessage = function (msg) { print("ab: " + msg); w1.terminate(); };
w1.postMessage("go");

// 2) Uint8Array with non-zero byteOffset (prepend 2 junk bytes, then view).
var padded = new Uint8Array(bytes.length + 2);
padded.set(bytes, 2);
var view = new Uint8Array(padded.buffer, 2, bytes.length);
var w2 = new Worker(view);
w2.onmessage = function (msg) { print("ta: " + msg); w2.terminate(); };
w2.postMessage("go");

// 3) DataView
var w3 = new Worker(new DataView(bytes.buffer));
w3.onmessage = function (msg) { print("dv: " + msg); w3.terminate(); };
w3.postMessage("go");

// 4) Byte fidelity: the source buffer contains a byte >= 0x80 (the two-byte
// UTF-8 encoding of U+00E9 é). The worker echoes its charCode; if the copy
// corrupted high bytes the decoded char would not be 233.
var hi = [];
var prefix = 'onmessage = function() { postMessage("code=" + "';
for (var i = 0; i < prefix.length; i++) hi.push(prefix.charCodeAt(i));
hi.push(0xc3, 0xa9); // UTF-8 for U+00E9 é
var suffix = '".charCodeAt(0)); };';
for (var j = 0; j < suffix.length; j++) hi.push(suffix.charCodeAt(j));
var w4 = new Worker(new Uint8Array(hi).buffer);
w4.onmessage = function (msg) { print("fidelity: " + msg); w4.terminate(); };
w4.postMessage("go");

// Error cases (all synchronous throws at construction).
function expectThrow(fn, label) {
  try { fn(); print(label + ": NO THROW"); }
  catch (e) { print(label + ": " + e.constructor.name); }
}
expectThrow(function () { new Worker(123); }, "number");
expectThrow(function () { new Worker({}); }, "object");
expectThrow(function () { new Worker(new ArrayBuffer(0)); }, "empty-ab");
expectThrow(function () { new Worker(new Uint8Array(0)); }, "empty-ta");

// Tamper-resistance: replacing globals must not break native detection.
ArrayBuffer.isView = function () { return false; };
DataView = null;
var w5 = new Worker(bytes.buffer);
w5.onmessage = function (msg) { print("tampered: " + msg); w5.terminate(); };
w5.postMessage("go");

// CHECK: number: TypeError
// CHECK-NEXT: object: TypeError
// CHECK-NEXT: empty-ab: TypeError
// CHECK-NEXT: empty-ta: TypeError
// CHECK-DAG: ab: ran
// CHECK-DAG: ta: ran
// CHECK-DAG: dv: ran
// CHECK-DAG: fidelity: code=233
// CHECK-DAG: tampered: ran
```

Note: the synchronous `expectThrow` output is emitted before any worker
message is delivered (messages are processed by the event loop after the
top-level script finishes), so the four error lines are ordered with
`CHECK`/`CHECK-NEXT`, while the five async worker results use `CHECK-DAG`
(order among them is not guaranteed).

- [ ] **Step 2: Build the CLIs**

Run: `cmake --build cmake-build-asan --target hermes shermes`
Expected: build succeeds.

- [ ] **Step 3: Run the LIT test and verify it passes**

Run: `LIT_OPTS="-j1" LIT_FILTER="worker/worker-from-buffer" cmake --build cmake-build-asan --target check-hermes`
Expected: PASS (1 test, 2 RUN lines).

- [ ] **Step 4: Commit**

```bash
sl commit --reason "add LIT tests for Worker binary input - sl help commit" --message "$(cat <<'EOF'
[SH]: Add LIT tests for Worker binary input

Summary:
Add worker-from-buffer.js covering construction from ArrayBuffer,
TypedArray (non-zero byteOffset), and DataView carrying UTF-8 source,
verifying multi-byte fidelity via a round-tripped accented/emoji
string. Also covers TypeError on non-buffer/empty input and confirms
detection still works after replacing ArrayBuffer.isView and DataView.

Test Plan: LIT_OPTS="-j1" LIT_FILTER="worker/worker-from-buffer" cmake --build cmake-build-asan --target check-hermes

Reviewers: Hermes

Subscribers:

Tasks:

Tags:
EOF
)"
```

---

### Task 3: Bytecode-input C++ test

**Files:**
- Modify: `unittests/API/APITest.cpp` (one new `TEST_P(HermesWorkerTest, ...)`)

**Interfaces:**
- Consumes: `hermes::compileJS` (already included via `<hermes/CompileJS.h>`), the constructor from Task 1, and the existing `api->isHermesBytecode` root API.
- Produces: nothing consumed by later tasks.

- [ ] **Step 1: Write the failing test**

Add inside the `#if HERMES_ENABLE_CORE_EXTENSIONS` block in `unittests/API/APITest.cpp`, after the Task 1 cases:

```cpp
TEST_P(HermesWorkerTest, WorkerFromBytecode) {
  // Compile a trivial script to Hermes bytecode.
  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("var x = 1;", bytecode));

  auto *api = castInterface<IHermesRootAPI>(makeHermesRootAPI());
  ASSERT_TRUE(api->isHermesBytecode(
      reinterpret_cast<const uint8_t *>(bytecode.data()), bytecode.size()));

  // Expose the bytecode to JS as a Uint8Array so `new Worker` receives the
  // exact bytes (bytecode magic contains bytes >= 0x80, which the old
  // string/utf8 path would have corrupted).
  auto u8ctor = rt->global().getPropertyAsFunction(*rt, "Uint8Array");
  auto arr =
      u8ctor.callAsConstructor(*rt, (double)bytecode.size()).asObject(*rt);
  for (size_t i = 0; i < bytecode.size(); ++i) {
    arr.setProperty(
        *rt,
        PropNameID::forUtf8(*rt, std::to_string(i)),
        (double)(uint8_t)bytecode[i]);
  }
  rt->global().setProperty(*rt, "__bc", arr);

  // Constructing from the bytecode bytes must succeed (worker thread starts
  // and evaluateJavaScript takes the bytecode path).
  auto worker = eval("var w = new Worker(__bc); w;").asObject(*rt);
  worker.getPropertyAsFunction(*rt, "terminate").callWithThis(*rt, worker);
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build cmake-build-asan --target APITests && cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest.WorkerFromBytecode'`
Expected: FAIL if run before Task 1 is present. If Task 1 is already committed, this test should already PASS (it exercises the same code path); in that case treat this task as adding explicit bytecode coverage and skip to Step 3.

- [ ] **Step 3: Confirm it passes**

Run: `cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest.WorkerFromBytecode'`
Expected: PASS.

Rationale: worker execution cannot be observed in the unit-test harness (no integrator event loop), so this test verifies that real bytecode is accepted by the constructor without corruption. End-to-end source execution/fidelity is covered by the LIT test in Task 2; bytecode *execution* itself is covered by existing `evaluateJavaScript` bytecode tests.

- [ ] **Step 4: Commit**

```bash
sl commit --reason "add bytecode-input Worker test - sl help commit" --message "$(cat <<'EOF'
[SH]: Add bytecode-input test for Worker ctor

Summary:
Add WorkerFromBytecode, which compiles a script to Hermes bytecode and
constructs a Worker from a Uint8Array of those exact bytes, asserting
the constructor accepts them. This guards the binary path against the
UTF-8 corruption that would previously have mangled bytecode's
high-byte magic number.

Test Plan: cmake-build-asan/unittests/API/APITests --gtest_filter='*HermesWorkerTest.WorkerFromBytecode'

Reviewers: Hermes

Subscribers:

Tasks:

Tags:
EOF
)"
```

---

## Self-Review

**Spec coverage:**
- Accept string / ArrayBuffer / TypedArray / DataView -> Task 1 dispatcher.
- Bytecode-or-source via magic sniff -> unchanged `evaluateJavaScript`; exercised in Task 3 (bytecode) and Task 2 (source).
- Native, tamper-proof detection; trivial JS shim -> Task 1 (`11-Worker.js` unchanged, verified by Task 2 tamper cases).
- DataView via captured getters in `captureIntrinsics` -> Task 1, Steps 3-4.
- Copy at construction -> `copyBufferBytes` in Task 1, Step 5.
- Honor byteOffset/byteLength -> Task 1 TypedArray/DataView branches; Task 2 offset case.
- Empty binary -> TypeError; empty string unchanged -> `copyBufferBytes` length check (Task 1); Task 2 empty cases.
- Detached buffer -> TypeError -> `copyBufferBytes` detached check (Task 1); Task 2/Task 1 detached cases.
- Wrong type -> TypeError -> Task 1 `else` branches; Task 1/Task 2 negative cases.
- No change to shared `jsi.h` -> confirmed; DataView helpers live in the extensions layer.
- Tests under `test/hermes/worker/` and `unittests/API` -> Tasks 2 and 1/3.

**Placeholder scan:** none — every step contains concrete code/commands.

**Type consistency:** helper names (`isDataView`, `dataViewBuffer`, `dataViewByteOffset`, `dataViewByteLength`), `copyBufferBytes(rt, ArrayBuffer, size_t, size_t)`, and `startWorker(rt, Object, std::string)` are used identically in Task 1 declarations and call sites. `startWorkerThread(std::string)` is the existing method, unchanged.

## Execution Handoff

Plan complete and saved to `doc/superpowers/plans/2026-07-28-worker-arraybuffer-input.md`.
