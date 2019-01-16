'use strict';

(function(global) {
  // Native code creates a function foo.
  // JS source code creates a global object o.
  // The first time foo is called, it sets a and b on o to be "hello" and true respectively.
  // The second time it's called, it changes a to "bar", and adds a new property c which is set to null.
  global.o = {};
  global.foo();
  if (global.o.a !== "hello") {
    throw new Error("o.a !== \"hello\"");
  }
  if (global.o.b !== true) {
    throw new Error("o.b !== true");
  }
  global.foo();
  if (global.o.a !== "bar") {
    throw new Error("o.a !== \"bar\"");
  }
  if (global.o.b !== true) {
    throw new Error("o.b !== true after second mutation");
  }
  if (global.o.c !== null) {
    throw new Error("o.c !== null");
  }
})(this);
