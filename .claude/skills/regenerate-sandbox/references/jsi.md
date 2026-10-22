# JSI methods in HermesSandboxRuntime.cpp

The JS in the sandbox is untrusted. The host runtime's JSI methods must not
look up JS globals, since JS can replace them, and must check the kind of
every value from the sandbox before using the unchecked `Value` accessors
(`getString`, `getObject`, `getBool`, `getNumber`, `getFunction`,
`getArray`), which only assert in debug builds.

## When JSI gains a method

The sandbox runtime inherits the default in `jsi.cpp`. Read it. If it looks
up a global, or uses an unchecked accessor on what JS returns, override it in
`HermesSandboxRuntime.cpp`. Helpers that are not `jsi::Runtime` methods, such
as `jsi::Value::toString`, cannot be overridden there; fix them in JSI, for
example by using `asString` instead of `getString`.

## How to implement a method

In order of preference:

1. **With the existing ABI entries.** `getValueAtIndex` shows the pattern:
   arguments and results live in sandbox memory in `StackAlloc`s, which must
   be destroyed in the reverse order of their creation, and `intoJSIValue` or
   `unwrap` turn an error result into a C++ exception.
2. **With a built-in captured at startup.** `captureIntrinsics()` runs at the
   end of the constructor, before any JS, and stores built-ins in
   `intrinsics_`. Call them through `getIntrinsic()`, which throws if one is
   missing. The destructor releases them before the VM.
3. **With a new ABI entry**, which needs a regeneration; see abi.md.

Then:

- Check results with `isString()` and friends before `getString()`, or use
  `asString()` and friends, which throw.
- Values that the host writes into sandbox memory, `SandboxValue` and
  `SandboxValueOrError`, must start fully zeroed (`sb::createValue`), so that
  their padding cannot carry host stack contents into the sandbox.
- Add tests to `unittests/API/APITest.cpp` as `TEST_P(HermesRuntimeTest, ...)`,
  so they run on all three runtimes. `JSIIgnoresReplacedBuiltins` replaces the
  built-ins the runtime relies on with functions that return numbers; extend
  it whenever the runtime relies on another one.
- Update the "JSI support" section of `API/hermes_sandbox/README.md`.
