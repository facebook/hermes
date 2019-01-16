// Call a function with an object and string argument.

function foo(o, a) { return o; }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  var o = {a: 0, b: 1};
  for (var i = 0; i < 20000000; i++) {
    f(o, 'a');
  }
}

bar();
