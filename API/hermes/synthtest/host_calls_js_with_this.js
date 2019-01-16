'use strict';

(function(global) {
  // Native code creates a function foo.
  // foo calls f with o as this.
  // f calls the member function bar on its this argument
  // That call sets a to 2.
  var a = 1;
  global.f = function() {
    this.bar();
  };
  var o = {
    bar: function() {
      a = 2;
    }
  };
  global.foo(o);
  if (a !== 2) {
    throw new Error();
  }
})(this);
