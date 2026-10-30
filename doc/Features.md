---
id: language-features
title: Language Features
---

Hermes is a JavaScript engine optimized for fast start-up. It executes JavaScript using a combination of ahead-of-time (AOT) compilation to bytecode and runtime interpretation.

Hermes aims to support the **latest ECMAScript specification**, prioritizing language features over new library functions. However, some features are intentionally excluded or de-prioritized if they conflict with Hermes' design goals of performance, low memory use, and efficient AOT compilation.

### Supported ECMAScript Features

Hermes provides broad support for ECMAScript features, including:

*   **Full support for ES2015 (ES6) Language Features:**
    *   `let` and `const` declarations (Temporal Dead Zone checks and per-iteration bindings are supported but off by default, see "Known Deviations")
    *   Arrow Functions (`=>`)
    *   Classes (Basic syntax: `class`, `extends`, `static` methods, `super`, `constructor`)
    *   Enhanced Object Literals (shorthand properties, computed names, methods)
    *   Template Literals (tagged and untagged)
    *   Destructuring Assignment (arrays and objects, including rest `...`)
    *   Default, Rest, and Spread (`...`) operators
    *   Iterators and Generators (`Symbol.iterator`, `for..of`, `function*`)
    *   Symbols (primitive type, `Symbol()`, well-known symbols)
    *   _And all other ES2015 language constructs (unless listed below)._

*   **Key Post-ES2015 Language Features:**
    *   `async` / `await` (ES2017)
    *   Object Rest/Spread Properties (ES2018)
    *   RegExp Named Capture Groups (ES2018)
    *   Optional Chaining (`?.`) (ES2020)
    *   Nullish Coalescing (`??`) (ES2020)
    *   `BigInt` (ES2020)
    *   **Class Fields & Static Blocks (ES2022):**
        *   Public and Private Instance Fields (property declarations and initializers, e.g., `myField = 1;`, `#privateField = 2;`)
        *   Public and Private Static Fields (e.g., `static staticField = 3;`, `static #privateStatic = 4;`)
        *   Static Initialization Blocks (`static {}`)
        *   Private Methods, Private Static Methods and Private Accessors
        *   Private Brand Checks (`#x in obj`)
    *   RegExp Match Indices (`/d` flag) (ES2022)
    *   Logical Assignment Operators (`??=`, `||=`, `&&=`) (ES2021)
    *   `for await...of` and `Symbol.asyncIterator` (ES2018)
    *   Async Generators (ES2018), behind the `-Xasync-generators` compiler flag. Off by default; without it the compiler reports "async generators are unsupported".

*   **Key Library Features:**
    *   ES2015 standard built-ins (`Promise`, `Set`, `Map`, `WeakSet`, `WeakMap`, TypedArrays, `Reflect`, `Proxy`, updated methods on `Array`, `String`, `Object`, etc.)
    *   **Extended `Promise` API** (see "Known Deviations" for the limits of `Symbol.species` and microtask timing):
        *   `Promise.prototype.finally` (ES2018)
        *   `Promise.allSettled` (ES2020)
        *   `Promise.any` (ES2021)
        *   `Promise.withResolvers` (ES2024)
        *   `Promise.try` (ES2025)
    *   `Symbol.prototype.description` (ES2019)
    *   `WeakRef` and `FinalizationRegistry` (ES2021). `WeakRef`, `WeakMap` and `WeakSet` also accept symbols as targets (ES2023).
    *   `Object.hasOwn` (ES2022), `Array.prototype.at` / `findLast` (ES2022), `Array.prototype.toSorted` / `toReversed` / `with` / `toSpliced` (ES2023)
    *   `String.prototype.isWellFormed` / `toWellFormed` (ES2024)
    *   Iterator Helpers (ES2025): `Iterator` with `map`, `filter`, `take`, `drop`, `flatMap`, `reduce`, `toArray`, `forEach`, `some`, `every`, `find`, plus `Iterator.from` and `Iterator.concat`
    *   Array grouping (ES2024): `Object.groupBy` and `Map.groupBy`
    *   `Set` methods (ES2025): `union`, `intersection`, `difference`, `symmetricDifference`, `isSubsetOf`, `isSupersetOf`, `isDisjointFrom`
    *   `RegExp.escape` (ES2025), `Math.sumPrecise` (ES2026), `Error.isError` (ES2026)
    *   `ArrayBuffer.prototype.detached` (ES2024), `Float16Array` (ES2025), `Uint8Array.fromBase64` (ES2026), `Map.prototype.getOrInsert` / `WeakMap.prototype.getOrInsert` (ES2026)
    *   `TextEncoder` and `TextDecoder` (WHATWG, not ECMAScript)
    *   _Note: Support for the latest standard library features may lag behind language feature support._

### Planned Features

Features Hermes intends to support in the future. Active development or implementation hasn't started or completed yet.

*   **`with` Statements:** Support is planned. See "Intentionally Excluded" for the current state.
*   **ES Modules (`import`/`export`):** A module system is planned. See "Intentionally Excluded" for the current state.
*   **Other Standard Library Features:** Newer library additions are considered but may be lower priority. Currently absent: RegExp `/v` (`unicodeSets`) flag, `ArrayBuffer.prototype.transfer` and resizable buffers, `SharedArrayBuffer` and `Atomics`, `Array.fromAsync`, explicit resource management (`using`, `Symbol.dispose`, `Symbol.asyncDispose`), `Temporal`.

### Intentionally Excluded / De-prioritized Features

These features are not supported, often due to incompatibility with Hermes' AOT compilation strategy, performance concerns, limited utility, or complexity costs.

*   **Local `eval()`:** `eval()` **cannot** access or modify local variables in the surrounding lexical scope (neither in strict nor non-strict mode). This restriction is fundamental to Hermes' AOT compilation and optimization strategy; supporting local `eval` would severely degrade performance across the engine. Use of `eval` is strongly discouraged. Every direct call to `eval()` produces the compiler warning "Direct call to eval(), but lexical scope is not supported". The precise behavior:
    *   Reading a local from inside `eval` throws `ReferenceError`, in both strict and loose mode.
    *   Writing a local from inside `eval` throws `ReferenceError` in strict mode. In **loose mode it fails silently**: the local is left unchanged and a global of that name is created or overwritten instead.
    *   `eval` sees the global scope, but only `var` and function declarations and properties of the global object. Top-level `let`, `const` and `class` declarations of the running script are **not** visible to it.
*   **`with` Statements:** Not supported yet. When source is compiled ahead of time this is a **compile-time error** ("with statement is not supported") which fails the build, not a runtime exception. When the VM compiles source lazily (`-lazy`), it surfaces as a thrown `SyntaxError` the first time the enclosing function is called, and the same happens when it is reached through `eval()` or `new Function()`. `with` hinders performance and optimization and is disallowed in strict mode. This is a current limitation rather than a permanent exclusion: support is planned.
*   **ES Modules (`import`/`export`):** Hermes **does not** currently provide a runtime ES module loader. The parser accepts the full module syntax and builds the corresponding AST, but semantic analysis rejects it ("'import' statement requires module mode"), and dynamic `import()` is not supported. An earlier implementation was removed because the React Native ecosystem relies on bundlers such as Metro, which provide their own module systems. An ES module system is planned.
*   **`Symbol.species`:** Not supported. The well-known symbol itself does not exist (`Symbol.species` is `undefined`), and no built-in exposes an `@@species` accessor. Hermes does not commit to supporting the pattern, due to its complexity and performance overhead for an AOT-focused engine. Note that the consequences reach beyond `@@species` overrides: subclass propagation is missing outright, so `class Sub extends Array {}` followed by `new Sub().map(f)` yields a plain `Array` rather than a `Sub`. The same applies to TypedArray methods such as `subarray`. `Promise` is the exception, because its polyfill dispatches on `this.constructor`; see "Known Deviations".
*   **Non-Strict `arguments` Object Behavior:** See "Known Deviations" for details on how Hermes deviates from the spec regarding parameter syncing, assignment, and `var` shadowing for the `arguments` object in non-strict mode. These deviations simplify implementation and improve performance.
*   **[`Intl` API](IntlAPIs.md):** Not supported. A partial implementation exists for Android, which calls into Java platform APIs, and for Apple platforms, which use Foundation. It is not built by default (`HERMES_ENABLE_INTL` is `OFF`), so in a default build `Intl` is `undefined`. Even where it is enabled, coverage is incomplete: `RelativeTimeFormat`, `PluralRules` and `formatRange` are missing or partial, and using them can throw. The underlying platform APIs are inadequate, there is no public ICU4C on mobile, and bundling ICU4C would cost tens of megabytes. Meta does not use `Intl` internally and cannot invest in it, so the implementation is expected to move to `extensions/contrib` or a separate community-maintained repository. **Use a polyfill such as FormatJS instead**, accepting its start-up cost. See [discussion #1211](https://github.com/facebook/hermes/discussions/1211).
*   **`Symbol.unscopables`:** Not supported. `Symbol.unscopables` and `Array.prototype[Symbol.unscopables]` are both `undefined`. Its main purpose is tied to `with`, which is also unsupported, but the absence is observable to any code that reflects on the built-in prototypes.

### Known Deviations & Implementation Details

Specific behaviors where Hermes differs from the ECMAScript specification or has notable implementation characteristics. These deviations are often deliberate choices prioritizing performance, simplicity, or compatibility with Hermes' AOT compilation model, especially concerning rarely used or complex legacy features. [SpecIncompat.md](SpecIncompat.md) discusses some of them in more detail.

*   **Temporal Dead Zone (TDZ) Checks:** Reading a `let` or `const` binding before its initializer has run should throw a `ReferenceError`. Hermes implements these checks, but they are **off by default**, because they impose a cost on every access to a lexical variable. By default such a read produces `undefined` instead of throwing. Pass `-Xenable-tdz` to `hermes` or `shermes` to enable them:

    ```javascript
    function f() {
      var get = function () { return v; };
      get();     // undefined by default; ReferenceError with -Xenable-tdz
      let v = 1;
    }
    ```

    With the flag on, a violation the compiler can prove statically becomes a compile-time error rather than a runtime throw. Class declarations follow the same rule: by default a class binding reads as `undefined` before its declaration, so `typeof C` yields `"undefined"` and `new C()` throws a `TypeError` (not a `ReferenceError`).

*   **Block Scoping of Captured Bindings:** A `let`, `const` or `class` binding declared in a block or a loop head should be created anew each time the block is entered or the loop iterates. By default Hermes creates it once per function call instead. Name resolution and shadowing are unaffected; the difference shows only when closures capture the binding, because closures created in different iterations then share one variable. Pass `-Xes6-block-scoping` to `hermes` or `shermes` to get spec behavior:

    ```javascript
    var fns = [];
    for (let i = 0; i < 3; i++) fns.push(() => i);
    fns.map(f => f());  // [3, 3, 3] by default; [0, 1, 2] with -Xes6-block-scoping
    ```

*   **`arguments` Object Behavior (Non-Strict Mode):** Hermes simplifies `arguments` object handling in non-strict ("loose") mode compared to the specification:
    *   **No Parameter Syncing:** Assigning to indices of the `arguments` object does **not** dynamically update the corresponding named parameters, and vice-versa (e.g., setting `arguments[0] = x` does not change the value of the first named parameter). This matches strict mode behavior. *(Motivation: High cost and complexity for a rare feature).*
    *   **Assignment Forbidden:** Direct assignment to the `arguments` identifier itself (e.g., `arguments = ...;`) is **disallowed**, unlike in spec-compliant loose mode. *(Motivation: Rare, complex, little practical benefit).*
    *   **`var arguments` Shadows:** Declaring `var arguments;` inside a function creates a new variable that **shadows** the arguments object (like `let arguments;`) rather than aliasing it. *(Motivation: Extremely rare, no known practical uses).*
    *   _These `arguments` deviations are considered very low priority to align with the spec due to the reasons mentioned._

*   **Function Declaration Hoisting (Non-Strict Mode Blocks):** Most of the scoped function promotion semantics are implemented, but some corner cases in non-strict mode differ from the spec: a function declared in an inner block can overwrite one declared in an outer block at the function scope level.

    ```javascript
    function g() {
        { function f() { return 1; } { function f() { return 2; } } }
        print(f());   // spec requires 1, Hermes prints 2
    }
    ```

    *(Motivation: Affects rare edge cases primarily in non-strict mode; other major engines behave the same way in quick testing. Low priority to fix).*

*   **`Function.prototype.toString()`:** Because functions are compiled ahead of time, this method does not normally return the original JavaScript source. It returns a synthesized declaration whose body is a placeholder and whose parameters are renamed:

    ```javascript
    function foo(a, b) {}       // "function foo(a0, a1) { [bytecode] }"
    async function bar() {}     // "async function bar() { [bytecode] }"
    Math.max                    // "function max() { [native code] }"
    ```

    The result is the same whether the function was compiled from source or loaded from precompiled bytecode. A function can opt out with a source visibility directive in its body: `'show source'` makes `toString()` return the real source text, while `'hide source'` and `'sensitive'` make it report `[native code]`.

*   **`Promise` Implementation:** Promises are implemented using an internally bundled JavaScript polyfill rather than as a native intrinsic. It is compiled to bytecode by default, or compiled natively when the engine is built with `HERMESVM_INTERNAL_JAVASCRIPT_NATIVE=ON`. The polyfill covers the full ES2025 surface (`then`, `catch`, `finally`, `all`, `allSettled`, `any`, `race`, `resolve`, `reject`, `try`, `withResolvers`, `Symbol.toStringTag`), but the following corners differ from the spec:
    *   **No `Symbol.species` dispatch.** Per spec, the prototype methods `Promise.prototype.then`, `Promise.prototype.catch`, and `Promise.prototype.finally` use `SpeciesConstructor(this, %Promise%)` to determine the constructor of the chained promise. The polyfill substitutes `this.constructor` (and takes the built-in `Promise` directly on the common-case `this.constructor === Promise` fast path), so a subclass that overrides `Symbol.species` will not see its override applied (consistent with the engine-wide "Symbol.species not supported" stance noted above). The static methods (`all`, `allSettled`, `any`, `race`, `resolve`, `reject`) use `this` via `NewPromiseCapability` per spec — `Symbol.species` is not involved there.
    *   **Extra microtask hop on subclass `.then`.** When `this.constructor !== Promise`, the polyfill bridges the user's reaction to the subclass capability through an intermediate core `Promise`, costing one additional microtask hop compared to spec's `PerformPromiseThen`, which attaches the reaction directly to the capability. The fast path (`this.constructor === Promise`) is unaffected.
    *   **`Promise.all` 1-microtask fast path (non-spec, default).** When an input is an already-fulfilled core `Promise`, the polyfill synchronously invokes the resolve element and collapses the spec-mandated `PerformPromiseAll` step 8.e microtask hop. This is a deliberate performance default; the spec-compliant `.then`-only dispatch is enabled under the `--test262` runtime flag.
    *   **Microtask timing in `await`.** Hermes' `async function` machinery lowers to a generator wrapped with the polyfill's `then`, and a few test262 tick-counting tests (e.g. `await-non-promise-thenable`, the `for-await-of/ticks-with-…-constructor-lookup` family) observe a different microtask sequence than spec because the polyfill does one fewer `.constructor` read on each await hop.
    *   See the [polyfill source](https://github.com/facebook/hermes/blob/static_h/lib/InternalJavaScript/01-Promise.js).
