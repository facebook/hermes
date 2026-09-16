/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

'use strict';

// A field common to every arm of an object-literal union (at the same slot in
// each arm) can be read off the union. A union value is built by constructing a
// named arm via an object literal and upcasting it to the union.

// 'tag' is at slot 0 and 'id' is at slot 1 in both arms.
type Circle = {tag: 'circle', id: number, radius: number};
type Square = {tag: 'square', id: number, side: number};

function describe(s: Circle | Square): void {
  // s.tag reads the common discriminant (slot 0); s.id the common field (slot
  // 1). The single load is correct no matter which arm s actually is.
  print(s.tag, s.id);
}

let c: Circle = {tag: 'circle', id: 1, radius: 10};
let sq: Square = {tag: 'square', id: 2, side: 20};

// Upcast each named arm to the union at the call.
describe(c);
// CHECK: circle 1
describe(sq);
// CHECK-NEXT: square 2

// The shared field sits at a *different* slot in each arm ('tag' is slot 1 in
// Left, slot 0 in Right). This must lower to a by-name load, which reads the
// correct value regardless of which arm is passed.
type Left = {other: number, tag: 'left'};
type Right = {tag: 'right', other: number};

function tagOf(v: Left | Right): void {
  print(v.tag);
}

let l: Left = {other: 1, tag: 'left'};
let r: Right = {tag: 'right', other: 2};

tagOf(l);
// CHECK-NEXT: left
tagOf(r);
// CHECK-NEXT: right
