/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s -dump-ast 2>&1 ) | %FileCheck %s
let x: number extends infer T extends number ? number : number ? number : number;
// CHECK: {{.*}}:9:6: error: ';' expected
