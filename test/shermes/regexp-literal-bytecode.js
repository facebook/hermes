/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -emit-c -o - %s | %FileCheck --check-prefix=CHECK-C %s
// RUN: %shermes -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O -exec %s | %FileCheck --match-full-lines %s

// Regexp literals are compiled together with the unit. Check that the
// generated C passes the precompiled bytecode to the runtime, that identical
// literals share it, and that the resulting objects behave like regexps
// compiled at runtime.

'use strict';

function intrinsic(name) {
  'noinline';
  return /^%?[^%]*%?$/.exec(name);
}

function sameLiteral(name) {
  'noinline';
  return /^%?[^%]*%?$/.test(name);
}

function otherFlags(name) {
  'noinline';
  return /^%?[^%]*%?$/i.test(name);
}

function fresh() {
  'noinline';
  return /a/g;
}

// CHECK-C: static const uint8_t s_regexp_bytecode[];
// CHECK-C-LABEL: static SHLegacyValue _1_intrinsic(SHRuntime *shr) {
// CHECK-C: _sh_ljs_create_regexp(shr, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], s_regexp_bytecode + [[OFF:[0-9]+]], [[LEN:[0-9]+]]);
// CHECK-C-LABEL: static SHLegacyValue _2_sameLiteral(SHRuntime *shr) {
// CHECK-C: _sh_ljs_create_regexp(shr, {{.*}}, s_regexp_bytecode + [[OFF]], [[LEN]]);
// CHECK-C-LABEL: static SHLegacyValue _3_otherFlags(SHRuntime *shr) {
// CHECK-C-NOT: s_regexp_bytecode + [[OFF]],
// CHECK-C: _sh_ljs_create_regexp(shr, {{.*}}, s_regexp_bytecode + {{[0-9]+}}, {{[0-9]+}});
// CHECK-C: static const uint8_t s_regexp_bytecode[{{[0-9]+}}] = {

print('regexp literal bytecode');
// CHECK-LABEL: regexp literal bytecode

print(intrinsic('%Reflect.apply%')[0], intrinsic('a%b%c'));
// CHECK-NEXT: %Reflect.apply% null
print(sameLiteral('%Array%'), sameLiteral('A%B%C'));
// CHECK-NEXT: true false
print(otherFlags('%ARRAY%'), otherFlags('A%B%C'));
// CHECK-NEXT: true false

// Every evaluation creates a new object.
var r1 = fresh();
var r2 = fresh();
r1.lastIndex = 5;
print(r1 !== r2, r2.lastIndex, r1.source === r2.source);
// CHECK-NEXT: true 0 true

// The flags come from the precompiled bytecode.
var all = /a.c/dgimsuy;
print(all.flags, all.hasIndices, all.global, all.ignoreCase);
// CHECK-NEXT: dgimsuy true true true
print(all.multiline, all.dotAll, all.unicode, all.sticky);
// CHECK-NEXT: true true true true
var none = /a.c/;
print(none.flags === '', none.global, none.ignoreCase, none.sticky);
// CHECK-NEXT: true false false false
print(/a.c/is.test('A\nC'), /a.c/i.test('A\nC'), /a.c/s.test('A\nC'));
// CHECK-NEXT: true false false
print(/^b/m.test('a\nb'), /^b/.test('a\nb'));
// CHECK-NEXT: true false
var sticky = /b/y;
sticky.lastIndex = 1;
print(sticky.test('ab'), /b/y.test('ab'));
// CHECK-NEXT: true false
print(/b(c)/d.exec('abc').indices[1]);
// CHECK-NEXT: 2,3
print('aXbXc'.replace(/X/g, '-'), 'aXbXc'.replace(/X/, '-'));
// CHECK-NEXT: a-b-c a-bXc
print(/^.$/u.test('\u{1F601}'), /^.$/.test('\u{1F601}'));
// CHECK-NEXT: true false

// Named groups.
var date = /(?<year>\d{4})-(?<month>\d{2})/.exec('on 2026-10');
print(date.groups.year, date.groups.month, date.index);
// CHECK-NEXT: 2026 10 3
print('2026-10'.replace(/(?<y>\d+)-(?<m>\d+)/, '$<m>/$<y>'));
// CHECK-NEXT: 10/2026
print(/(?<c>.)\k<c>/.exec('xyyz').groups.c);
// CHECK-NEXT: y
print(/(\d)/.exec('a1').groups);
// CHECK-NEXT: undefined
var copied = new RegExp(/(?<d>\d)/);
var reflagged = new RegExp(/(?<d>\d)/, 'g');
print(copied.exec('a1').groups.d, reflagged.exec('a1').groups.d);
// CHECK-NEXT: 1 1

// Patterns with non-ASCII characters.
print(/é+/.exec('aééb')[0], /é/i.test('É'));
// CHECK-NEXT: éé true
print(/😁+/u.exec('a😁😁')[0].length, /😁/.test('😁'));
// CHECK-NEXT: 4 true

print(String(/a\/b/g), /a\/b/g.source);
// CHECK-NEXT: /a\/b/g a\/b
