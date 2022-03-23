/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%juno %s 2>&1 || true) | %FileCheck %s --match-full-lines

function foo() { new.target }
new.target

//CHECK: {{.*}}new_target.js:11:1: error: 'new.target' outside of a function
//CHECK-NEXT: 1 error(s), 0 warning(s)
