// Creates an object as well as setting two named string properties to it.

function foo() { return {a: 0, b: 1}; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f();
  }
}

bar();
