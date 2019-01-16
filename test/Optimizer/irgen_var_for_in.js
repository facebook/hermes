// RUN: %hermes -O -dump-ir %s

// Make sure that we are not crashing on this one.

function simple_loop(obj) {
  foo(x);
  for (var x in obj) { }
}

