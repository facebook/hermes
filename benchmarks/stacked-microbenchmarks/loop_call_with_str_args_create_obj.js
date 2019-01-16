// Call a function with two string arguments, and creates an empty object in the function.

function foo(a, b) { return {}; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 2000000; i++) {
    f('a', 'b');
  }
}

bar();
