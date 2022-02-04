/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -fno-inline -dump-bytecode -fstrip-function-names -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -strict -fno-inline -fstrip-function-names -O %s | %FileCheck --match-full-lines %s --check-prefix=EXEC

//CHECK-LABEL:Global String Table:
//CHECK-NEXT: s0[ASCII, {{[0-9]+\.\.[0-9]+}}]: Done
//CHECK-NEXT: s1[ASCII, {{[0-9]+\.\.[0-9]+}}]: abc
//CHECK-NEXT: s2[ASCII, {{[0-9]+\.\.[0-9]+}}]: function-name-stripped
//CHECK-NEXT: i3[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-F]+}}: length
//CHECK-NEXT: i4[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-F]+}}: print
//CHECK-NEXT: i5[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-F]+}}: substring

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}global{{.*}}
(function() {

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}entryPoint{{.*}}
function entryPoint() {
  return helper();
}

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}helper{{.*}}
function helper() {
  // Dummy code.
  var s = "abc";
  var x = s.length;
  var y = s[1];
  var z = s.substring(0, 1);
  return 3;
}
var result = entryPoint();
print("Done", result);
})();

//EXEC: Done 3
