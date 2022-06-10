---
id: language-features
title: Language Features
---

Hermes plans to target ECMAScript 2015 (ES6), with some carefully considered exceptions.

### Supported

- Symbols (including most well-known Symbols)
- Iteration (with `[Symbol.iterator]`)
- `for..of` loops
- Array spread
- Object rest/spread
- Shorthand property and computed property on object literals
- Destructuring assignment (with array and object "rest" properties)
- Template string literals
- Generators (`function*` and `yield`)
- TypedArrays
- Arrow functions
- Optional chaining and nullish coalescing (`?.` and `??`)
- Reflection (`Reflect` and `Proxy`) starting from [v0.7.0](https://github.com/facebook/hermes/releases/tag/v0.7.0)
- [`Intl`, or Internationalization APIs](IntlAPIs.md)
  - currently Android only
- ES6 Promise (with incompatibilities documented below)
- All ES6 JS library functions
  - Set/Map
  - WeakSet/WeakMap
  - ES6 String searching functions
  - ES6 Array searching functions

### In Progress

- WeakRef
- BigInt
- ES modules (`import` and `export`)
- `Intl` API glue has been added to enable community contribution of a complete, spec-compliant implementation on multiple platforms
- Async function (`async` and `await`).
- `Symbol.prototype.description` (it's not fully spec-conformant yet. `Symbol().description` should be `undefined` but it's currently `''`).

### Planned
- RegExp match indices and named capture groups
- Expanded Intl functionality (e.g., DisplayNames, ListFormat, PluralRules, RelativeTimeFormat, and Locale)
- `let` and `const` (block scoped variables, with support for the temporal dead zone)
- Classes and method definitions

### Excluded From Support

- Realms
- `with` statements
- Local mode `eval()` (use and introduce local variables)
- `Symbol.species` and its interactions with JS library functions
- use of `constructor` property when creating new Arrays in Array.prototype methods
- `Symbol.unscopables` (Hermes does not support `with`)
- Other features added to ECMAScript after ES6 not listed under "Supported"

### Miscellaneous Incompatibilities

- `Function.prototype.toString` cannot show source because Hermes executes from bytecode
- `arguments` changes in non-strict mode will not sync with named parameters
- `Promise` is implemented by pre-compiling [the JS polyfill from RN](https://github.com/facebook/react-native/blob/HEAD/Libraries/Promise.js) as the [internal bytecode](https://github.com/facebook/hermes/blob/HEAD/lib/InternalBytecode/Promise.js) to preserve the current interoperation, hence its conformance to the spec is up to conformance of the polyfill.
  - In case you want to bring in your own Promise and opt-out Hermes', you can turn it off by passing `-Xes6-promise=0` in CLI or setting `withES6Promise(false)` in the runtime configs.
  - N.B. ES6 Promise does not include `Promise.allSettled` (ES2020) and `Promise.any` (Stage 4).
