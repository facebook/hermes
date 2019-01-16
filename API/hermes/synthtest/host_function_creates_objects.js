'use strict';

(function(global) {
  // Native code creates a function foo.
  // foo returns an object with two properties, a: "hello" and b: null.
  // The second time it is called, it should return an object with a: null, and
  // b: "hello".
  var o = global.foo();
  if (o.a !== "hello") {
    throw new Error("o.a !== \"hello\"");
  }
  if (o.b !== null) {
    throw new Error("o.b !== null");
  }
  // The second returned object should swap the properties.
  var p = global.foo();
  if (o === p) {
    throw new Error("o and p should be different objects");
  }
  if (p.a !== null) {
    throw new Error("o.a !== null");
  }
  if (p.b !== "hello") {
    throw new Error("o.b !== \"hello\"");
  }
})(this);
