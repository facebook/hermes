/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: serializer

var x = function 𞸆() { 'show source'; }
print(x.toString());
// CHECK: function 𞸆() { 'show source'; }
