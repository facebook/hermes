/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

// A boolean-literal-typed field without an initializer defaults to its value.
class C {
  t: true;
  f: false;
  constructor() {}
}
const o = new C();
print(o.t, o.f);
// CHECK: true false
