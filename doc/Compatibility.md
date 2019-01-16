# Hermes Compatibility and Conformance Document

This document summarizes how much we are conforming to the JavaScript standard and
why. JavaScript developers who use Hermes should be aware of these differences.

## Highlights

* Hermes currently does not support ES6 or above at compile time. Babel is required for ES6 support.
* Hermes is fully conformant for ES5 strict mode. Non-strict mode is conformant with a few known exceptions.
* Code that uses `eval` can compile with Hermes but will not function properly at runtime (in both strict and non-strict mode).
* Hermes does not support `with` at compile time.
* Since Hermes compiles JavaScript source into bytecode, source code is no longer available at runtime. Anything that tries to operate on source code will no longer work (e.g. function.toString()).
* Per-function strictness is not supported. The entire application is either in strict or non-strict mode.
* Unicode identifiers are not recognized by the compiler.

## Non-strict mode incompatibilities
* The way we handle the `arguments` object is only limited to strict mode. Changes to `arguments` object and changes to individual arguments by name will not sync with each other.
