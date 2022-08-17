/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s 2>&1 | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -O0 %s 2>&1 | %FileCheck --match-full-lines %s

// We want to make sure the `/` doesn't get lexed as a regexp.
print(isNaN(function(){} / 3));
// CHECK: true
