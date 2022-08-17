/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Wno-direct-eval -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Wno-direct-eval -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print('strict eval');
// CHECK-LABEL: strict eval

var x = 3;
eval('x = 4');
print(x);
// CHECK-NEXT: 4

function f() {
  print('called f');
}

eval('f()');
// CHECK-NEXT: called f

eval('print(123)');
// CHECK-NEXT: 123

try {
    eval('"use strict"; y = 4');
} catch (e) {
    print(e.name, e.message);
}
// CHECK-NEXT: ReferenceError {{.*}}
