// Creates an empty object.

function foo() { return {}; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f();
  }
}

bar();
