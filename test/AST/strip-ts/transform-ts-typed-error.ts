/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc --transform-ts --typed --dump-ast %s 2>&1) | %FileCheck %s --match-full-lines

// CHECK: error: --transform-ts is incompatible with typed mode
let x: number = 42;
