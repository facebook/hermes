// RUN: %hermes -O -dump-ir %s

// Make sure that we are not crashing on this one.

function x29() {
  foo({get 19() { var V }})
  V;
}

