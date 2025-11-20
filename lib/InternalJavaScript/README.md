# Internal Bytecode

## Promise

The Promise Internal Bytecode is generated on the fly by running:

```sh
$ utils/gen-promise-internal-bc.sh
```

## Iterator helpers

The Iterator Helper polyfill is adapted from https://github.com/es-shims/iterator-helpers/ at commit da02ab656dce1c4f73355290a3bd9e4c25ccc3b. The required dependencies are taken and flattened in `05-IteratorHelperDependencies.js` and the actual Iterator implementation are followed after the file. The dependencies may be simplified from the original source. Assertions from the polyfill, which threw `TypeError`s, are removed.

Note the original polyfill's dependencies included many polyfills for features that are supported by Hermes. In those cases, they are may be removed and use Hermes implementation directly.
Examples (non-exhausive):
* `Error`s are supported so its polyfill implementation is removed.
* Checks for `Symbol`s support are removed since Hermes support `Symbol`s.
* Hermes implements `Function.prototype.bind`, so the polyfill for this is removed.

The implementation for `Iterator.prototype.constructor` and `Iterator.prototype [ %Symbol.toStringTag% ]` differs from the original polyfill in order to adhere to spec compliance. The changes uses the polyfill for the abstract operation [SetterThatIgnoresPrototypeProperties](https://github.com/es-shims/es-abstract/blob/ba4a616318c6a9854d17f771dcae718d90b8939e/2025/SetterThatIgnoresPrototypeProperties.js) to help with the implementation.
