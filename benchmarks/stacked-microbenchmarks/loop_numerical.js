// Run an loop with Add
function foo(y) {
  for (var i = 0; i < 20000000; i++) {
    y += 1;
  }
  return y;
}

foo(55);
