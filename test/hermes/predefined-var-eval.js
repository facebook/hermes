/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Wno-direct-eval -O %s | %FileCheck --match-full-lines %s
// We need to distinguish global scope eval from anonymous function eval.
// This test needs to treat the var statement as local scope to an anonymous
// function. T173289597
// XFAIL: true


// In this eval, the `var Math` declaration should be hoisted,
// causing Math to be undefined prior to the conditional check,
// since eval doesn't give the compiler any knowledge about
// what's in global scope.

eval('"use strict";' +
     'if (typeof Math === "undefined") var Math = 10;' +
     'print(Math);')
// CHECK: 10
