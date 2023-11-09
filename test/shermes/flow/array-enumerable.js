/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

var x: string[] = ['a', 'b'];

for (var i in x)
  print(i, x[i]);
// CHECK: 0 a
// CHECK-NEXT: 1 b

print('done')
// CHECK-NEXT: done
