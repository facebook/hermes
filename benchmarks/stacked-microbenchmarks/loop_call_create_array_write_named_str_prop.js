// Creats an array, and sets string properties to it.

function foo() {
  var arr = [];
  arr.a = 0;
  arr.b = 1;
  return arr;
}

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f();
  }
}

bar();
