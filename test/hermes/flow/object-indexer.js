/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

// Basic read and write through an indexer.
{
  let d: {[string]: number} = {a: 1, b: 2};
  print(d.a, d['b']);
  // CHECK: 1 2
  d.a = 10;
  d['c'] = 3;
  print(d.a, d.b, d.c);
  // CHECK-NEXT: 10 2 3
}

// `delete` removes a key. A subsequent read of the deleted key throws via the
// narrowing checked cast (a missing key reads as `undefined`).
{
  let d: {[string]: number} = {a: 1, b: 2};
  delete d.a;
  print(d.b);
  // CHECK-NEXT: 2
  try {
    print(d.a);
    print('no throw');
  } catch (e) {
    print(e.name);
    // CHECK-NEXT: TypeError
  }
  // Computed delete works too.
  delete d['b'];
  let {...rest} = d;
  let count = 0;
  for (let _ in rest) ++count;
  print(count);
  // CHECK-NEXT: 0
}

// Destructuring resolves every property through the indexer.
let obj: {[string]: number} = {a: 1, b: 2, c: 3};

// Named property + rest: 'a' resolves through the indexer, 'rest' is an exact
// object with the same indexer.
{
  let {a, ...rest} = obj;
  print(a);
  // CHECK-NEXT: 1
  let r: {[string]: number} = rest;
  print(r.b, r.c);
  // CHECK-NEXT: 2 3
}

// Renamed property + rest.
{
  let {a: aa, ...rest} = obj;
  print(aa);
  // CHECK-NEXT: 1
  let r: {[string]: number} = rest;
  print(r.b, r.c);
  // CHECK-NEXT: 2 3
}

// Rest only: copies every key.
{
  let {...rest} = obj;
  let r: {[string]: number} = rest;
  print(r.a, r.b, r.c);
  // CHECK-NEXT: 1 2 3
}

// Default value: a missing key selects the initializer instead of throwing.
{
  let {z = 9, ...rest} = obj;
  print(z);
  // CHECK-NEXT: 9
  let r: {[string]: number} = rest;
  print(r.a, r.b, r.c);
  // CHECK-NEXT: 1 2 3
}

// Reading a missing destructured key (no initializer) throws via the narrowing
// checked cast.
{
  try {
    let {missing} = obj;
    print('no throw');
  } catch (e) {
    print(e.name);
    // CHECK-NEXT: TypeError
  }
}
