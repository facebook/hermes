// RUN: %hermes -hermes-parser -dump-ir %s     -O
// RUN: %hermes -hermes-parser -dump-ir %s

// Make sure that we are not crashing on this one:
function assertEquals(x, y) { }
assertEquals(true, true ? true : false);


