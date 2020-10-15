# Language Features in Hermes

Hermes plans to target ECMAScript 2015 (ES6), with some carefully considered exceptions.

## Supported

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
- All ES6 JS library functions
  - Set/Map
  - WeakSet/WeakMap
  - ES6 String searching functions
  - ES6 Array searching functions

## In Progress

- `let` and `const` (block scoped variables, with support for the temporal dead zone)
- Classes and method definitions
- ES modules (`import` and `export`)
- `Intl` API glue for Android has been added, in order to enable community contribution of a complete, spec-compliant implementation.
- Promise is opt-in (via `-Xes6-promise` flag or `withES6Promise` runtime config).
- Async function (`async` and `await`).

## Excluded From Support

- Realms
- `with` statements
- Local mode `eval()` (use and introduce local variables)
- `Symbol.species` and its interactions with JS library functions
- use of `constructor` property when creating new Arrays in Array.prototype methods
- `Symbol.unscopables` (Hermes does not support `with`)
- Other features added to ECMAScript after ES6 not listed under "Supported"

## Miscellaneous Incompatibilities

- `Function.prototype.toString` cannot show source because Hermes executes from bytecode
- `arguments` changes in non-strict mode will not sync with named parameters
