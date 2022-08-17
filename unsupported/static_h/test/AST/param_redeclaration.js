/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

function foo(p, p) {
//CHECK: {{.*}}param_redeclaration.js:10:17: error: cannot declare two parameters with the same name 'p'
//CHECK-NEXT: function foo(p, p) {
//CHECK-NEXT:                 ^

  "use strict";
  return p + p;
}

var bar = (a, b, a) => 1;
//CHECK: {{.*}}param_redeclaration.js:19:18: error: cannot declare two parameters with the same name 'a'
//CHECK-NEXT: var bar = (a, b, a) => 1;
//CHECK-NEXT:                  ^
