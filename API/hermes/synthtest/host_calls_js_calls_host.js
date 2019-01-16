'use strict';

(function(global) {
  // Native code create the function foo which takes an object.
  // foo calls f with that object as an argument.
  // f calls the native function bar with a new object which has the same
  // properties of the passed in object.
  // bar returns the object immediately.
  // That object is returned from f, which is returned from foo back to JS.
  global.f = function(a) {
    var o = {};
    o.x = a.x;
    return global.bar(o);
  };
  var a = {x: 2};
  var y = global.foo(a);
  if (a === y || y.x !== 2) {
    throw new Error();
  }
})(this);
