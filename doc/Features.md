---
id: language-features
title: Language Features
---

Hermes is a JavaScript engine optimized for fast start-up. It executes JavaScript using a combination of ahead-of-time (AOT) compilation to bytecode and runtime interpretation.

Hermes aims to support the **latest ECMAScript specification**, prioritizing language features over new library functions. However, some features are intentionally excluded or de-prioritized if they conflict with Hermes' design goals of performance, low memory use, and efficient AOT compilation.

### Supported ECMAScript Features

Hermes provides broad support for ECMAScript features, including:

*   **Full support for ES2015 (ES6) Language Features:**
    *   `let` and `const` declarations (including Temporal Dead Zone behavior)
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
    *   RegExp Match Indices (`/d` flag) (ES2022)

*   **Key Library Features:**
    *   ES2015 standard built-ins (`Promise`, `Set`, `Map`, `WeakSet`, `WeakMap`, TypedArrays, `Reflect`, `Proxy`, updated methods on `Array`, `String`, `Object`, etc.)
    *   `Promise.prototype.finally` (ES2018)
    *   `Symbol.prototype.description` (ES2019 - see "Known Deviations")
    *   `WeakRef` (ES2021 - see "Planned Features" regarding `FinalizationRegistry`)
    *   [`Intl` API](IntlAPIs.md) (Basic `DateTimeFormat`, `NumberFormat` support; see "Planned Features")
    *   _Note: Support for the latest standard library features may lag behind language feature support._

### Planned Features

Features Hermes intends to support in the future. Active development or implementation hasn't started or completed yet.

*   **`FinalizationRegistry`:** The complementary API to `WeakRef`.
*   **Expanded `Intl` Functionality:** Support for APIs such as `DisplayNames`, `ListFormat`, `PluralRules`, `RelativeTimeFormat`, `Locale`, etc.
*   **Other Standard Library Features:** Newer library additions (e.g., Iterator Helpers, Array Grouping methods) are considered but may be lower priority.

### Intentionally Excluded / De-prioritized Features

These features are not supported, often due to incompatibility with Hermes' AOT compilation strategy, performance concerns, limited utility, or complexity costs.

*   **Local `eval()`:** `eval()` **cannot** access or modify local variables in the surrounding lexical scope (neither in strict nor non-strict mode). It operates **only on the global scope**. This restriction is fundamental to Hermes' AOT compilation and optimization strategy; supporting local `eval` would severely degrade performance across the engine. Use of `eval` is strongly discouraged.
*   **`with` Statements:** Not supported (throws a `SyntaxError`). It hinders performance and optimization and is disallowed in strict mode.
*   **ES Modules (`import`/`export`):** Hermes **does not** currently provide a runtime module loader. While an implementation existed previously, it was removed as the React Native ecosystem relies heavily on bundlers (like Metro) which provide their own module systems. A future goal is to potentially parse module syntax to give the AOT compiler better visibility for cross-module optimizations, but *not* necessarily to replace bundler functionality at runtime.
*   **`Symbol.species`:** Generally **not** supported for built-in methods. While some specific methods might incidentally behave compatibly, Hermes does not guarantee or commit to supporting the `Symbol.species` pattern due to its complexity and performance overhead, particularly for an AOT-focused engine.
*   **Non-Strict `arguments` Object Behavior:** See "Known Deviations" for details on how Hermes deviates from the spec regarding parameter syncing, assignment, and `var` shadowing for the `arguments` object in non-strict mode. These deviations simplify implementation and improve performance.
*   **`Symbol.unscopables`:** Not relevant as `with` is not supported.

### Known Deviations & Implementation Details

Specific behaviors where Hermes differs from the ECMAScript specification or has notable implementation characteristics. These deviations are often deliberate choices prioritizing performance, simplicity, or compatibility with Hermes' AOT compilation model, especially concerning rarely used or complex legacy features.

*   **`arguments` Object Behavior (Non-Strict Mode):** Hermes simplifies `arguments` object handling in non-strict ("loose") mode compared to the specification:
    *   **No Parameter Syncing:** Assigning to indices of the `arguments` object does **not** dynamically update the corresponding named parameters, and vice-versa (e.g., setting `arguments[0] = x` does not change the value of the first named parameter). This matches strict mode behavior. *(Motivation: High cost and complexity for a rare feature).*
    *   **Assignment Forbidden:** Direct assignment to the `arguments` identifier itself (e.g., `arguments = ...;`) is **disallowed**, unlike in spec-compliant loose mode. *(Motivation: Rare, complex, little practical benefit).*
    *   **`var arguments` Shadows:** Declaring `var arguments;` inside a function creates a new variable that **shadows** the arguments object (like `let arguments;`) rather than aliasing it. *(Motivation: Extremely rare, no known practical uses).*
    *   _These `arguments` deviations are considered very low priority to align with the spec due to the reasons mentioned._

*   **Function Declaration Hoisting (Non-Strict Mode Blocks):** The exact semantics of how function declarations within nested blocks are hoisted ("promoted") in non-strict mode may differ from the spec in some complex corner cases. For example, a function declared in an inner block might incorrectly overwrite a function declared in an outer block at the function scope level. *(Motivation: Affects rare edge cases primarily in non-strict mode; other engines may exhibit similar behavior. Low priority to fix).*

*   **`Function.prototype.toString()`:** Due to AOT compilation to bytecode, this method does not return the original JavaScript source code. It typically returns a placeholder like `"[native code]"` or `"[bytecode]"`.

*   **`Promise` Implementation:** Promises are implemented using an internally bundled polyfill compiled to bytecode. While generally conformant, microtask timing or specific edge cases might differ slightly from other engines or the exact specification. See the [polyfill source](https://github.com/facebook/hermes/blob/HEAD/utils/promise/index.js).

*   **`Symbol.prototype.description` Conformance:** While `Symbol('desc').description` works as expected, `Symbol().description` currently returns `''` (empty string) instead of the spec-compliant `undefined`.

*   **`Math.sumPrecise` Iterator Protocol:** The polyfill implementation does not call `iteratorClose` when iteration is terminated early as noted in the polyfill source. Additionally, error handling for invalid iterator states may not fully conform to the specification's `TypeError` throwing requirements in all edge cases.
