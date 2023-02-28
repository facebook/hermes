/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

(function() {

'use strict';

print('super');
// CHECK-LABEL: super

class C {
  constructor() {
    print('C run');
  }
}

class D extends C {
  constructor() {
    super();
    print('D run');
  }
}

var d = new D();
// CHECK-NEXT: C run
// CHECK-NEXT: D run
print(d instanceof C);
// CHECK-NEXT: true
print(d instanceof D);
// CHECK-NEXT: true

})();
