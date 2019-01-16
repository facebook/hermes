// Read two computed string properties from an object.

function foo(a, b) {
  var obj = {};
  obj[a] = 0;
  obj[b] = 1;
  return obj;
}

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  for (var i = 0; i < 20000000; i++) {
    f('a', 'b');
  }
}

bar();
