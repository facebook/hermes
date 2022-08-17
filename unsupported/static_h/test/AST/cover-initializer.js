/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ir %s 2>&1 ) | %FileCheck --match-full-lines %s

var tmp = {a: 10, b = 30};
//CHECK: {{.*}}cover-initializer.js:10:21: error: ':' expected in property initialization
//CHECK-NEXT: var tmp = {a: 10, b = 30};
//CHECK-NEXT:                     ^
