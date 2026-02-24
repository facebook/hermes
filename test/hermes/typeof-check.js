/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-inline -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -fno-inline -exec %s | %FileCheck --match-full-lines %s

// Make sure that the JmpTypeOfIs instructions work properly.

function t(x) {
  if (typeof x === 'undefined') print('undefined');
  if (typeof x === 'string')    print('string');
  if (typeof x === 'symbol')    print('symbol');
  if (typeof x === 'boolean')   print('boolean');
  if (typeof x === 'number')    print('number');
  if (typeof x === 'bigint')    print('bigint');
  if (typeof x === 'function')  print('function');
  if (typeof x === 'object')    print('object');
}

print("typeof checks");
// CHECK-LABEL: typeof checks

t(undefined);
// CHECK-NEXT: undefined
t('abc');
// CHECK-NEXT: string
t(Symbol());
// CHECK-NEXT: symbol
t(true);
// CHECK-NEXT: boolean
t(123);
// CHECK-NEXT: number
t(123n);
// CHECK-NEXT: bigint
t(function() {});
// CHECK-NEXT: function
t(null);
// CHECK-NEXT: object
t({});
// CHECK-NEXT: object
t(/a/);
// CHECK-NEXT: object

function nt(x) {
  if (typeof x !== 'undefined') print('NOT undefined');
  if (typeof x !== 'string')    print('NOT string');
  if (typeof x !== 'symbol')    print('NOT symbol');
  if (typeof x !== 'boolean')   print('NOT boolean');
  if (typeof x !== 'number')    print('NOT number');
  if (typeof x !== 'bigint')    print('NOT bigint');
  if (typeof x !== 'function')  print('NOT function');
  if (typeof x !== 'object')    print('NOT object');
}

print("typeof NOT checks");
// CHECK-LABEL: typeof NOT checks

nt(undefined);
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
// CHECK-NEXT: NOT object
nt('abc');
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
// CHECK-NEXT: NOT object
nt(Symbol());
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
// CHECK-NEXT: NOT object
nt(true);
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
// CHECK-NEXT: NOT object
nt(123);
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
// CHECK-NEXT: NOT object
nt(123n);
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT function
// CHECK-NEXT: NOT object
nt(function() {});
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT object
nt(null);
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
nt({});
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function
nt(/a/);
// CHECK-NEXT: NOT undefined
// CHECK-NEXT: NOT string
// CHECK-NEXT: NOT symbol
// CHECK-NEXT: NOT boolean
// CHECK-NEXT: NOT number
// CHECK-NEXT: NOT bigint
// CHECK-NEXT: NOT function

// Make sure that the bool-returning TypeOfIs instructions work properly.

function t2(x) {
  return [
   typeof x === 'undefined',
   typeof x === 'string',
   typeof x === 'symbol',
   typeof x === 'boolean',
   typeof x === 'number',
   typeof x === 'bigint',
   typeof x === 'function',
   typeof x === 'object',
  ];
}

print("typeof checks");
// CHECK-LABEL: typeof checks

print(t2(undefined));
// CHECK-NEXT: true,false,false,false,false,false,false,false
print(t2('abc'));
// CHECK-NEXT: false,true,false,false,false,false,false,false
print(t2(Symbol()));
// CHECK-NEXT: false,false,true,false,false,false,false,false
print(t2(true));
// CHECK-NEXT: false,false,false,true,false,false,false,false
print(t2(123));
// CHECK-NEXT: false,false,false,false,true,false,false,false
print(t2(123n));
// CHECK-NEXT: false,false,false,false,false,true,false,false
print(t2(function() {}));
// CHECK-NEXT: false,false,false,false,false,false,true,false
print(t2(null));
// CHECK-NEXT: false,false,false,false,false,false,false,true
print(t2({}));
// CHECK-NEXT: false,false,false,false,false,false,false,true
print(t2(/a/));
// CHECK-NEXT: false,false,false,false,false,false,false,true

function nt2(x) {
  return [
   typeof x !== 'undefined',
   typeof x !== 'string',
   typeof x !== 'symbol',
   typeof x !== 'boolean',
   typeof x !== 'number',
   typeof x !== 'bigint',
   typeof x !== 'function',
   typeof x !== 'object',
  ];
}

print("typeof NOT checks");
// CHECK-LABEL: typeof NOT checks

print(nt2(undefined));
// CHECK-NEXT: false,true,true,true,true,true,true,true
print(nt2('abc'));
// CHECK-NEXT: true,false,true,true,true,true,true,true
print(nt2(Symbol()));
// CHECK-NEXT: true,true,false,true,true,true,true,true
print(nt2(false));
// CHECK-NEXT: true,true,true,false,true,true,true,true
print(nt2(123));
// CHECK-NEXT: true,true,true,true,false,true,true,true
print(nt2(123n));
// CHECK-NEXT: true,true,true,true,true,false,true,true
print(nt2(function() {}));
// CHECK-NEXT: true,true,true,true,true,true,false,true
print(nt2(null));
// CHECK-NEXT: true,true,true,true,true,true,true,false
print(nt2({}));
// CHECK-NEXT: true,true,true,true,true,true,true,false
print(nt2(/a/));
// CHECK-NEXT: true,true,true,true,true,true,true,false
