/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ! %hermes %s | %hermesc --dump-ir - %s 2>&1 | %FileCheck --match-whole-lines %s

// CHECK: <stdin>:1:8: error: Too many nested expressions/statements/declarations

let s = "0";
for(let i = 0; i < 1024; ++i)
    s += "+0";
print(s);
