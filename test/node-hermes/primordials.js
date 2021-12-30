/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

print('Primordials');
// CHECK-LABEL: Primordials

var _primordials = primordials,
  MathMin = _primordials.MathMin;

print(MathMin(0, 1, 2));
// CHECK-NEXT: 0
