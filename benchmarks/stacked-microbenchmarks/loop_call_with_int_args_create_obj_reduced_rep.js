// Call a function with two integer arguments, and creates an empty object in the function.
// Loop repetition count is reduced to speed things up for later benchmarks.

function foo(a, b) { return {}; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 1000000; i++) {
    f(i, i + 1);
  }
}

bar();
