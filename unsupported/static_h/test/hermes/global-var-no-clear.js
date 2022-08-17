/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var globalEval = eval;

var q = true;
globalEval('var q;');
print(q);
// CHECK: true

var s = 'if (x === undefined) { var x; x = 0; }; x += 1;';
globalEval(s);
globalEval(s);
globalEval(s);
print(x);
// CHECK: 3
