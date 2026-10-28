/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

// Object.keys/values/entries on an `any` argument may receive objects the
// typed fast path can't scan; they take the generic path and match untyped JS.

'use strict';
function show(label: string, x: any): void {
  const k = Object.keys(x);
  const v = Object.values(x);
  const e = Object.entries(x);
  let es = '';
  for (let i = 0; i < e.length; ++i) es += e[i][0] + '=' + (e[i][1] as any) + ';';
  print(label, k.join(), v.join(), es);
}
show('proxy', new globalThis.Proxy({a: 1, b: 2}, {}));
const g: any = JSON.parse('{"x":1}');
globalThis.Object.defineProperty(g, 'z', {get: function() { return 9; }, enumerable: true});
show('accessor', g);
show('array', JSON.parse('[5,6]'));
show('string', new globalThis.String('ab'));
show('u8', new globalThis.Uint8Array(2));
show('plain', JSON.parse('{"2":3,"q":4}'));

// CHECK: proxy a,b 1,2 a=1;b=2;
// CHECK-NEXT: accessor x,z 1,9 x=1;z=9;
// CHECK-NEXT: array 0,1 5,6 0=5;1=6;
// CHECK-NEXT: string 0,1 a,b 0=a;1=b;
// CHECK-NEXT: u8 0,1 0,0 0=0;1=0;
// CHECK-NEXT: plain 2,q 3,4 2=3;q=4;
