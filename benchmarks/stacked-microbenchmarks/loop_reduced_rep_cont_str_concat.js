// Continuous string concatenation on the same string.

function foo(a) {
  var o = '';
  for (var i = 0; i < 200000; i++) {
    o = o + a;
  }
  return o;
}

foo('a');
