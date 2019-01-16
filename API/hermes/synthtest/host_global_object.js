'use strict';

(function(global) {
  // Native code creates functions foo and baz.
  // foo calls bar on the global object, which sets quux on the global object, and calls baz.
  // baz checks that quux is an object with property b: true, and then returns.
  // foo then creates an object with a property a which is set to null, and
  // caches it.
  // The second time foo is called, it returns the cached object.
  global.bar = function() {
    global.quux = {b: true};
    global.baz();
  };
  global.foo();
  if (global.foo().a !== null) {
    throw new Error();
  }
})(this);
