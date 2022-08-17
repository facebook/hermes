/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

function *foo(yield) {}
// CHECK:{{.*}}:{{.*}}:15: error: Unexpected usage of 'yield' as an identifier
// CHECK-NEXT:function *foo(yield) {}
// CHECK-NEXT:              ^~~~~

function *bar() { var yield = 1; }
// CHECK:{{.*}}:{{.*}}:23: error: Unexpected usage of 'yield' as an identifier
// CHECK-NEXT:function *bar() { var yield = 1; }
// CHECK-NEXT:                      ^~~~~

a: function *labeled() {}
// CHECK: {{.*}}:20:4: error: Function declaration not allowed as body of labeled statement
// CHECK-NEXT: a: function *labeled() {}
// CHECK-NEXT:    ^

function *g() { void yield }
// CHECK:{{.*}}:25:22: error: Unexpected usage of 'yield' as an identifier reference
// CHECK-NEXT:function *g() { void yield }
// CHECK-NEXT:                     ^~~~~

function *h() {
  for (yield x in y;;) {}
}
// CHECK:{{.*}}:31:20: error: ')' expected after 'for(... in/of ...'
// CHECK-NEXT:for (yield x in y;;) {}
// CHECK-NEXT:    ~~~~~~~~~~~~~^
