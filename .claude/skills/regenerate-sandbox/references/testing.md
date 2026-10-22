# Testing the sandbox

- **The API tests run on the sandbox.** It is one of the runtimes in
  `unittests/API/APITestFactory.cpp`, so the parameterized tests
  (`HermesRuntimeTest` in `unittests/API/APITest.cpp`, and `JSITest` in the
  JSI test library) run on it. Tests that need something the sandbox does not
  support return early for it, with
  `dynamic_cast<HermesSandboxRuntime *>(rt.get())`, since the googletest in
  the CMake build has no `GTEST_SKIP`. Suites for Hermes-only interfaces, such
  as `ISerialization`, use `hermesRuntimeGenerators()`, which leaves the
  sandbox out.
- **Test both generated variants.** A debug build of the host compiles the
  dbg files and a release build the opt files, and most configurations only
  build one of them.
- **Test with UBSan.** The generated code calls functions through pointers
  that take the instance as `void *`, while the functions take a typed
  pointer, which Clang's function type check reports. The CMake build passes
  `-fno-sanitize=function` for the generated files
  (`API/hermes_sandbox/CMakeLists.txt`); other builds that enable UBSan need
  the same.
- **Check the new behavior.** After a regeneration for a VM change, check from
  JS in the sandbox that the change arrived, for example new built-ins, or
  that bytecode of the new version loads and of the old version is rejected.
- **Iterate with the CMake build.** Running its `APITests` binary directly is
  the fastest loop; a sandbox test takes a few milliseconds in an optimized
  build.
