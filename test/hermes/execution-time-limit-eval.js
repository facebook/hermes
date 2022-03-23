/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -w -O0 -emit-async-break-check -time-limit=300 %s 2>&1 ) | %FileCheck --match-full-lines %s

eval(`
function entryPoint() {
  helper();
}

function helper() {
  var i = 0;
  while (true) {
    ++i;
  }
}

entryPoint();
`);

//CHECK:Uncaught TimeoutError: Javascript execution has timed out.
//CHECK-NEXT:    at helper {{.*}}
//CHECK-NEXT:    at entryPoint {{.*}}
//CHECK-NEXT:    at eval {{.*}}
//CHECK-NEXT:    at global {{.*}}
