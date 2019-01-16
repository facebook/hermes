// Call constructor.

function cee() {}

function foo(a) { return new a(); }

function bar() {
  // Put foo to f to avoid loading foo during the loop.
  var f = foo;
  var c = cee;
  for (var i = 0; i < 20000000; i++) {
    f(c);
  }
}

bar();
