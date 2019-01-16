// During the function call, creates an empty array, and then set two named integer indexes/values.

function foo() {
  var a = [];
  a[0] = 0;
  a[1] = 1;
  return a;
}

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f();
  }
}

bar();
