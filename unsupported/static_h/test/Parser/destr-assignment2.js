/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

({a : x = 10, get b() {}} = x)
//CHECK: {{.*}}destr-assignment2.js:10:15: error: invalid destructuring target
//CHECK-NEXT: ({a : x = 10, get b() {}} = x)
//CHECK-NEXT:               ^~~~~
