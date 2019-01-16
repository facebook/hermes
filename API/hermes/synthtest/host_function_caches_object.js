'use strict';

(function(global) {
  // Native code creates a function foo, which returns an object with one
  // property, a.
  // foo is called a second time, and is expected to return the same object, and
  // also set its a property to be true.
  var o = global.foo();
  if (!("a" in o) || o.a !== undefined) {
    throw new Error("o.a !== undefined");
  }
  var p = global.foo();
  if (o !== p) {
    throw new Error("Didn't cache the object");
  }
  if (o.a !== true || p.a !== true) {
    throw new Error("o.a !== true");
  }
})(this);
