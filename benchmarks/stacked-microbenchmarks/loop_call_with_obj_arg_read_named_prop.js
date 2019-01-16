// Read a named string property from an object.

function foo(o) { return o.a; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  var o = {a: 0, b: 1};
  for (var i = 0; i < 20000000; i++) {
    f(o);
  }
}

bar();
