// Run a loop with reduced repetition count.

function foo() {
  // This loop needs to be inside a function to avoid the overhead of loading "i" from global scope.
  for (var i = 0; i < 200000; i++) {
  }
}

foo();
