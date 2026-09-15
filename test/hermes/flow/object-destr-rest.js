/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

let obj: {a: number, b: string, c: boolean} =
    {a: 1, b: 'x', c: true};

// Basic rest: assign to a variable declared with the rest's exact-object
// type, then read fields back via typed access.
{
  let {a, ...rest} = obj;
  print(a);
  // CHECK: 1
  let r: {b: string, c: boolean} = rest;
  print(r.b, r.c);
  // CHECK-NEXT: x true
}

// Renamed property + rest.
{
  let {a: aa, ...rest} = obj;
  print(aa);
  // CHECK-NEXT: 1
  let r: {b: string, c: boolean} = rest;
  print(r.b, r.c);
  // CHECK-NEXT: x true
}

// All fields consumed: rest is an empty exact object.
{
  let {a, b, c, ...rest} = obj;
  print(a, b, c);
  // CHECK-NEXT: 1 x true
  let r: {} = rest;
  print(typeof r);
  // CHECK-NEXT: object
}

// Rest from 'any'.
{
  function f(x: any) {
    let {a, ...rest} = x;
    print(a, rest.b, rest.c);
  }
  f({a: 10, b: 'y', c: false});
  // CHECK-NEXT: 10 y false
}
