---
id: language-features
title: Language Features
---

Hermes plans to target ECMAScript 2015 (ES6), with some carefully considered exceptions.

### Supported

- All ES6 JS library functions
  - ES6 Array searching functions
  - ES6 String searching functions
  - Set/Map
  - WeakSet/WeakMap
- Array spread
- Arrow functions
- BigInt
- Destructuring assignment (with array and object "rest" properties)
- ES6 Promise (with incompatibilities documented below)
- `for..of` loops
- Generators (`function*` and `yield`)
- [`Intl`, or Internationalization APIs](IntlAPIs.md)
- Iteration (with `[Symbol.iterator]`)
- Object rest/spread
- Optional chaining and nullish coalescing (`?.` and `??`)
- Reflection (`Reflect` and `Proxy`) starting from [v0.7.0](https://github.com/facebook/hermes/releases/tag/v0.7.0)
- Shorthand property and computed property on object literals
- Symbols (including most well-known Symbols)
- Template string literals
- TypedArrays

### In Progress

- Async function (`async` and `await`).
- ES modules (`import` and `export`)
- `Intl` API glue has been added to enable community contribution of a complete, spec-compliant implementation on multiple platforms
- `Symbol.prototype.description` (it's not fully spec-conformant yet. `Symbol().description` should be `undefined` but it's currently `''`).
- WeakRef

### Planned
- Block scoped variables (`let` and `const`), with support for the temporal dead zone
- Classes and method definitions
- Expanded Intl functionality (e.g., DisplayNames, ListFormat, PluralRules, RelativeTimeFormat, and Locale)
- RegExp match indices and named capture groups

### Excluded From Support

- Local mode `eval()` (use and introduce local variables)
- Other features added to ECMAScript after ES6 not listed under "Supported"
- Realms
- `Symbol.species` and its interactions with JS library functions
- `Symbol.unscopables` (Hermes does not support `with`)
- use of `constructor` property when creating new Arrays in Array.prototype methods
- `with` statements

### Miscellaneous Incompatibilities

- `arguments` changes in non-strict mode will not sync with named parameters
- `Function.prototype.toString` cannot show source because Hermes executes from bytecode
- `Promise` is implemented by pre-compiling [the JS polyfill from RN](https://github.com/facebook/react-native/blob/HEAD/Libraries/Promise.js) as the [internal bytecode](https://github.com/facebook/hermes/blob/HEAD/lib/InternalBytecode/Promise.js) to preserve the current interoperation, hence its conformance to the spec is up to conformance of the polyfill.
  - In case you want to bring in your own Promise and opt-out Hermes', you can turn it off by passing `-Xes6-promise=0` in CLI or setting `withES6Promise(false)` in the runtime configs.
  - N.B. ES6 Promise does not include `Promise.allSettled` (ES2020) and `Promise.any` (Stage 4).
