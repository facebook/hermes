// RUN: %hermes -lazy -non-strict -Wno-direct-eval %s | %FileCheck --match-full-lines %s

// `eval` used to clean up Module/Context needed for lazy compilation.
// Ensure that we can eval a new module, return an uncompiled closure,
// gc everything else, and still run the closure.
// Use global.eval to modify the global scope.
this.eval("function getLazy() { " +
    "    return function() { " +
    "      return \"works\"; } }")

function test() {
  var local = getLazy();
  getLazy = undefined;
  gc();
  print(local());
}

// CHECK-LABEL: main
print("main");
// CHECK-NEXT: works
test();
