// Run an empty loop.
function foo() {
  // This loop needs to be inside a function to avoid the overhead of loading "i" from global scope.
  for (var i = 0; i < 20000000; i++) {
  }
}

foo();
