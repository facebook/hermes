/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1 ) | %FileCheck %s

const x = 1;
x = 2;
//CHECK: error: invalid assignment left-hand side
