'use strict';

(function(global) {
  // Native code creates a function foo.
  // It takes in an object, sets a to "hello" and b to false, and then returns it.
  // It should return the same object it takes in.
  var o = {};
  var x = global.foo(o);
  if (x !== o) {
    throw new Error("Didn't return the same object it took in");
  }
  if (o.a !== "hello") {
    throw new Error("o.a !== \"hello\"");
  }
  if (o.b !== false) {
    throw new Error("o.b !== false");
  }
})(this);
