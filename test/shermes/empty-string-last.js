/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --exec %s | %FileCheck --match-full-lines %s

"use strict";

function sink(x) {}

print('test');
// CHECK-LABEL: test
sink('a');
// Make empty string the final entry in the string storage.
sink('');
