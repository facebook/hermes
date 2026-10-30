# Hermes JS Engine
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/facebook/hermes/blob/HEAD/LICENSE)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/facebook/hermes/blob/HEAD/CONTRIBUTING.md)
<img src="./doc/img/logo.svg" alt="Hermes logo - large H with wings" align="right" width="20%"/>

Hermes is a JavaScript engine built for fast start-up and low memory use.
Its whole design centers on compiling JavaScript ahead of time: apps ship
compact bytecode, or optionally native code, and the engine does as little
work as possible when they launch. Hermes was created for
[React Native](https://reactnative.dev/), where it is the default engine.
It also runs standalone or can be embedded as a library in other programs.

## Ahead-of-time first

Most engines parse and compile JavaScript on the user's device, every time the
app starts. Hermes moves that work to build time. The compiler parses,
analyzes and optimizes the entire program once, and emits bytecode that the
engine can memory-map and start executing immediately. Start-up cost, memory
and binary size all benefit, and the optimizer can see the whole program
instead of one function at a time.

In practice this means:

* **Ship bytecode.** Compile at build time and load the result. This is how
  React Native apps use Hermes, and it is the path that gets all the
  optimizations.
* **Source still runs.** The engine includes the compiler, so `.js` files can
  be run directly during development. It is not the intended production path.
* **`eval()` and `new Function()` are supported for small things.** They run
  the compiler at runtime. Code passed to `eval` sees the global scope only;
  giving it access to the enclosing function's locals would force the
  compiler to pessimize every function, so that is deliberately left out.

## Ways to run code

Hermes can execute JavaScript in four ways, and they can be mixed freely
within one runtime.

* **From source.** The engine can also be given JavaScript source, which it
  compiles to bytecode before running. By default, large sources are
  compiled lazily, one function at a time when it is first called. Good for
  development and for code generated at runtime.
* **From bytecode.** The production path. Code is compiled ahead of time,
  the bytecode file is memory-mapped, and the interpreter runs it. This is
  how React Native apps run.
* **JIT.** An optional baseline JIT compiles frequently executed functions to
  machine code while the app runs. It is deliberately simple; the heavy
  optimization already happened at build time, so the JIT stays small and
  predictable.
* **Native code.** The `shermes` tool compiles JavaScript
  ahead of time all the way to native code, with no interpreter involved.

An app can ship some code as native, some as bytecode, and generate some at
runtime, and the JIT will pick up hot functions from the latter two.

## Static typing (in development)

Hermes is gaining the ability to use type annotations to compile JavaScript
to much faster code. This is under active development and not ready for use.
Both Flow and TypeScript syntax are being supported, to different degrees.
Flow is the more static-typing-friendly of the two, so TypeScript is
currently converted to Flow by an AST pass contributed by Amazon. That is an
implementation detail and may change; TypeScript support is a priority.

Neither the Flow checker nor the TypeScript compiler guarantees that the
annotations are true at runtime. Both accept this program, for example:

```js
const a: number[] = [];
const n: number = a[0]; // the type says number, the value is undefined
```

A typed compiler has to strengthen the language semantics until the types
are actually true. In this example, typed Hermes range-checks array indexing
and throws on out-of-bounds access, so `a[0]` can never produce a value that
contradicts its type. Where a value of unknown type flows into a typed one,
for example from `any` or from untyped code, the compiler inserts a checked
cast, which throws at runtime if the value is not of the expected type. With
those guarantees in place, the generated code can rely on the types.

TypeScript is less friendly to static compilation than Flow because its typing
is structural, and it is unsound in more places. This program type-checks in
TypeScript and fails at runtime; Flow rejects it because object properties are
invariant:

```ts
class Animal {}
class Dog extends Animal { bark() {} }
class Cat extends Animal {}

const d: { pet: Dog } = { pet: new Dog() };
const a: { pet: Animal } = d; // TypeScript accepts, Flow rejects
a.pet = new Cat();
d.pet.bark();                  // d.pet is a Cat: "bark is not a function"
```

In TypeScript a value of type `Dog` need not be an instance of `Dog` at all;
any object with the same shape qualifies. A compiler can therefore not derive
an object layout from a class, which is exactly what makes typed code fast.

See [doc/TypedLanguage.md](doc/TypedLanguage.md) for the current state.

Independently from this, TypeScript can already be run in untyped, unsound
mode: the type annotations are stripped and the result is treated as plain
JavaScript. See [doc/typescript-stripping.md](doc/typescript-stripping.md).

## Native compilation

Hermes can also compile JavaScript ahead of time to native code, using the
`shermes` tool. It works both on ordinary untyped JavaScript and on typed
JavaScript, where the latter produces much better native code.

## About the name "Static Hermes"

"Static Hermes" was the code name for an umbrella project, developed on the
`static_h` branch, to add static typing and optional native compilation to
Hermes. The branch then became the main development line and gained the JIT,
modern ECMAScript features, and a large amount of unrelated work. It is not a
separate engine; it is Hermes.

Hermes releases had always been numbered 0.x. When the `static_h` line was
judged ready, its releases moved to a new scheme built from the branch date and
bytecode version (for example 260318099.0.0), and React Native calls it
**Hermes V1** to distinguish it from the legacy line on the `main` branch.
See [doc/Branches.md](doc/Branches.md).

## Try it in a minute

You need a C++17 compiler, CMake, Ninja and Python. Clang is strongly
recommended; gcc is supported but produces noticeably worse code. On Ubuntu:

```shell
apt install clang build-essential cmake git ninja-build python3 tzdata libreadline-dev
```

Clone and build the two command line tools, `hermes` (compile to bytecode, run
bytecode or source) and `shermes` (compile to native):

```shell
git clone https://github.com/facebook/hermes.git
cmake -S hermes -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build --target hermes shermes
```

Run a script from source, then the way apps do it, from bytecode:

```shell
echo "print('Hello from Hermes');" > hello.js
build/bin/hermes hello.js
build/bin/hermes -emit-binary -out hello.hbc hello.js
build/bin/hermes hello.hbc
```

Compile the same script to a native executable:

```shell
build/bin/shermes -O -o hello hello.js
./hello
```

Everything else, including macOS, Windows, release builds and running the
test suites, is in [doc/BuildingAndRunning.md](doc/BuildingAndRunning.md).

## Integration

Integration documentation is thin today and will grow. These are the starting
points:

* **Embedding through JSI.** JSI is the stable C++ API for hosting the engine
  and exchanging values and functions with JavaScript. Start with
  [hermes-jsi-demos](https://github.com/tmikov/hermes-jsi-demos), a set of
  small standalone CMake projects, from hello world to a working event loop.
  The API itself is [API/jsi/jsi/jsi.h](API/jsi/jsi/jsi.h) and the Hermes
  entry point is [API/hermes/hermes.h](API/hermes/hermes.h).
* **Node-API.** Native addons written against the Node-API ABI run on Hermes
  without modification. See [API/napi/README.md](API/napi/README.md).
* **React Native.** Hermes is the default engine. See
  [doc/ReactNative.md](doc/ReactNative.md).
* **Running in a browser.** Hermes itself can be compiled to Wasm with
  Emscripten, so it runs in the browser and in other Wasm hosts. JavaScript
  compiled with `shermes` can be built to Wasm the same way. See
  [doc/Emscripten.md](doc/Emscripten.md).

## Learn more

* [Design](doc/Design.md), [VM](doc/VM.md), [IR](doc/IR.md),
  [Optimizer](doc/Optimizer.md), [Hades GC](doc/Hades.md)
* [Language features](doc/Features.md) and
  [known spec incompatibilities](doc/SpecIncompat.md)
* [Lazy and eval compilation](doc/LazyEvalCompilation.md),
  [Strings](doc/Strings.md), [RegExp](doc/RegExp.md), [Intl](doc/IntlAPIs.md)
* [Performance profiling](doc/PerfProfiling.md) and
  [memory profilers](doc/MemoryProfilers.md)
* [Cross compilation](doc/CrossCompilation.md)
* The [Hermes blog](doc/blog/README.md): compilation and runtime modes, memory
  modes, JSON performance, JSI additions and release notes.

## Contributing

Contributions are welcome. The preferred way to add functionality is a
[JSI extension](API/hermes/extensions/README.md): extensions are written
against the stable JSI API rather than engine internals, so they are safer to
write and do not break when the engine changes. Read
[CONTRIBUTING.md](CONTRIBUTING.md) for the development process and how to
propose changes.

This project follows the Meta [Code of Conduct](CODE_OF_CONDUCT.md).
Hermes is [MIT licensed](LICENSE).
