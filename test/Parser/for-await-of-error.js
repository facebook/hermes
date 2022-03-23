/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

(async () => {

for await (let x in y) {}
// CHECK: {{.*}}:12:5: error: unexpected 'await' in for..in loop
// CHECK-NEXT: for await (let x in y) {}
// CHECK-NEXT:     ^~~~~

for await (;;) {}
// CHECK: {{.*}}:17:5: error: unexpected 'await' in for loop without 'of'
// CHECK-NEXT: for await (;;) {}
// CHECK-NEXT:     ^~~~~

})
