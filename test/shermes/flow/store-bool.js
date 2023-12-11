/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

class C {
  x: boolean;
}

let c = new C();
c.x = true;
print(c.x);
// CHECK: true
