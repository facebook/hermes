/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode -fstrip-function-names -O %s | %FileCheck --match-full-lines %s

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
  helper();
}

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}helper{{.*}}
function helper() {
  var s = "abc";
  var x = 1;
  var y;
  var z = false;
  for (var i = 0; i < 1000000; ++i) {
    x = s.length;
    y = s[i % 3];
    z = s.substring(0, 1);
  }
}
entryPoint();
print("Done");
})();
