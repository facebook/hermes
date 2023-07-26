/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s -dump-ast 2>&1 ) | %FileCheck %s
let x: number extends string number extends string ? string : string;
// CHECK: {{.*}}:9:6: error: ';' expected
