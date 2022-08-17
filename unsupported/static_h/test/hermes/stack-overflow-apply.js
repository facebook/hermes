/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

function func() {
    print(arguments.length);
}

try {
    var a = []
    a[8*1024*1024] = 100;
    func.apply(null, a);
} catch(e) {
    print("caught:", e.name, e.message);
}
//CHECK: caught: RangeError {{.*}}

var x = [];
x.length = 4294967295;

for (var i = 0; i < 20; i++) {
    x[i] = i;
}
try {
  func.apply(99, x);
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK: caught: RangeError {{.*}}

try {
  v0 = [1.1];
  function v1() {
    v1(...v0);
  }
  v1();
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK: caught: RangeError {{.*}}
