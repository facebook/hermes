// RUN: %hermes -Wno-direct-eval -O %s | %FileCheck --match-full-lines %s

// In this eval, the `var Math` declaration should be hoisted,
// causing Math to be undefined prior to the conditional check,
// since eval doesn't give the compiler any knowledge about
// what's in global scope.

eval('"use strict";' +
     'if (typeof Math === "undefined") var Math = 10;' +
     'print(Math);')
// CHECK: 10
