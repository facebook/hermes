/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1 ) | %FileCheck %s

"use strict";
0x

// CHECK: No digits after 0x

0b2
// CHECK: invalid integer literal

0b2n
// CHECK: invalid numeric literal

0bn
// CHECK: invalid numeric literal
