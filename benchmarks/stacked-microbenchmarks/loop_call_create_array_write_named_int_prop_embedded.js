// Creates an array and at the same time setting two indexes/values to it.
// We may perform different in this one comparing the the non-embedded version
// because we do special optimizations when creating literal arrays.

function foo() {
  return [0, 1];
}

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f();
  }
}

bar();
