'use strict';

(function(global) {
  // Native code creates a function foo, that when called returns undefined.
  if (global.foo() !== undefined) {
    throw new Error();
  }
})(this);
