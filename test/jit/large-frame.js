/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "function f(x){ var a0=x++;" "var a"{1..4200}"=x++;" "return a0" "+a"{1..4200} ";} print(f(7));" | %hermes - -O -Xjit=force -Xjit-crash-on-error | %FileCheck --match-full-lines %s
// REQUIRES: jit

// The above echo generates a function with 4201 simultaneously live locals:
//   function f(x){ var a0=x++; ... var a4200=x++; return a0 + ... + a4200; }
//
// A frame slot sits at (index + FirstLocal) * 8 from xFrame, so past about
// 4090 registers the offset no longer fits the scaled immediate of LDR/STR.
// _loadFrame and _storeFrame used to emit it unconditionally, so a function
// this size failed to JIT. x++ keeps each initializer distinct; using the
// same value lets the optimizer collapse them and the frame never grows.

// CHECK: 8851507
