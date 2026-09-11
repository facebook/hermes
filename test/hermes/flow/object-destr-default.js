/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';
type Dict = {+[k: string]: mixed};

// Named-field object: a present key keeps its value; a missing key would use
// the default.
function named(o: {a: number, b: string}): void {
  const {a = 99, b = 'DEF'} = o;
  print('named', a, b);
}
named({a: 1, b: 'present'});
// CHECK: named 1 present

// Indexer object: missing key 'm' uses its default; present key 'p' uses its
// value.
function indexer(o: Dict): void {
  const {m = 'DEF_M', p = 'DEF_P'} = o;
  print('indexer', m, p);
}
indexer({p: 'has_p'});
// CHECK-NEXT: indexer DEF_M has_p

// Parameter destructuring with a default.
function param({n = 42}: Dict): void {
  print('param', n);
}
param({});
// CHECK-NEXT: param 42
param({n: 7});
// CHECK-NEXT: param 7
