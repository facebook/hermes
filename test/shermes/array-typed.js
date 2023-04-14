/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

(function () {

var x: number[] = [1, 2, 3];

print(x[0]);
// CHECK-LABEL: 1

x[1] = 42;

for(var i = 0, e = x.length; i < e ; i++)
  print(x[i]);

// CHECK-NEXT: 1
// CHECK-NEXT: 42
// CHECK-NEXT: 3

try { x[3] } catch (e) { print(e.message); }
try { x[1.2] } catch (e) { print(e.message); }
// CHECK-NEXT: array load index out of range
// CHECK-NEXT: array load index out of range
})();
