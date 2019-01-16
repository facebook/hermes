// Run a loop with AddN
function foo(y) {
  y = +y;
  for (var i = 0; i < 20000000; i++) {
    y++;
  }
  return y;
}

foo(55);
