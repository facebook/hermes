// RUN: %hermes -O -dump-ir %s

// Make sure that we are not crashing on this one.

while(
  { get my_getter ( ) {
  f
  function f   () { }
  }
  })
;

