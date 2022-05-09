/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s %s | %FileCheck %s --match-full-lines

var x = 5;

// CHECK-LABEL: Module: {{.*}}/module-var-decl.js
// CHECK: 1 declarations
// CHECK: Decl#0 'x' Var NotSpecial
