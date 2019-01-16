'use strict';

(function(global) {
  // Native code creates a function foo.
  // foo takes in an argument and returns it immediately.
  // It should have the same identity as the passed-in object.
  var o = {};
  if (global.foo(o) !== o) {
    throw new Error();
  }
})(this);
