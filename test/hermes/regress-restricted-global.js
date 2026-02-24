/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes --exec %s | %FileCheck --match-full-lines %s

{
  const NaN = 1;
  const Infinity = 1;
  const undefined = 1;
  print([
    NaN === 1 ? 'PASS NaN' : 'FAIL NaN',
    Infinity === 1 ? 'PASS Infinity' : 'FAIL Infinity',
    undefined === 1 ? 'PASS undefined' : 'FAIL undefined',
  ].join(', '));
}
// CHECK: PASS NaN, PASS Infinity, PASS undefined
