// Call a function with two string arguments.

function foo(a, b) { return a; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f('a', 'b');
  }
}

bar();
