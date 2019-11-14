# Language Features in Hermes

Hermes plans to target ECMAScript 2015 (ES6), with some carefully considered exceptions.

## Supported

- Symbols (including most well-known Symbols)
- Iteration (with `[Symbol.iterator]`)
- `for..of` loops
- Destructuring assignment (with array and object "rest" properties)
- Template string literals
- Generators (`function*` and `yield`)
- TypedArrays
- Arrow functions
- All ES6 JS library functions
  - Set/Map
  - WeakSet/WeakMap
  - ES6 String searching functions
  - ES6 Array searching functions

## In Progress

- `let` and `const` (block scoped variables, with support for the temporal dead zone)
- Classes and method definitions
- Computed property keys on object literals
- ES modules (`import` and `export`)

## Excluded From Support

- Reflection (`Reflect` and `Proxy`)
- Realms
- `with` statements
- Local mode `eval()` (use and introduce local variables)
- `/u` (Unicode) and `/y` (sticky) flags in `RegExp`
- `Intl` API
- `Symbol.species` and its interactions with JS library functions
- use of `constructor` property when creating new Arrays in Array.prototype methods
- `Symbol.unscopables` (Hermes does not support `with`)
- Other features added to ECMAScript after ES6 not listed under "Supported"

## Miscellaneous Incompatibilities

- `Function.prototype.toString` cannot show source because Hermes executes from bytecode
- `arguments` changes in non-strict mode will not sync with named parameters
