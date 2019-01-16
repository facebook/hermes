'use strict';

(function(global) {
  // Native code creates a foo function on the global object.
  // foo takes an object, and calls f with it.
  // f then sets that object's x property to 5, and returns it.
  // foo returns the object it got from f.
  global.f = function(a) {
    if (a.x !== 2) {
      throw new Error();
    }
    a.x = 5;
    return a;
  };
  var a = {x: 2};
  var y = global.foo(a);
  if (a !== y || y.x !== 5) {
    throw new Error();
  }
})(this);
