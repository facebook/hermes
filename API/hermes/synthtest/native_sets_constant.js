'use strict';

(function(global) {
  // Native code should inject the number 2 into the property name foo on
  // the global object.
  if (global.foo !== 2) {
    throw new Error("Expecting 2, got " + global.foo);
  }
})(this);
