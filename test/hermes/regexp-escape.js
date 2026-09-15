/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s
"use strict";

// ES2025 RegExp.escape ( S )

print('properties');
// CHECK-LABEL: properties
print(typeof RegExp.escape);
// CHECK-NEXT: function
print(RegExp.escape.length);
// CHECK-NEXT: 1
print(RegExp.escape.name);
// CHECK-NEXT: escape
var desc = Object.getOwnPropertyDescriptor(RegExp, 'escape');
print(desc.writable, desc.enumerable, desc.configurable);
// CHECK-NEXT: true false true

print('syntax-characters');
// CHECK-LABEL: syntax-characters
// Syntax characters and the '/' delimiter are escaped with a leading backslash.
print(RegExp.escape('.*+?^${}()|[]\\/'));
// CHECK-NEXT: \.\*\+\?\^\$\{\}\(\)\|\[\]\\\/

print('leading-character');
// CHECK-LABEL: leading-character
// A leading ASCII letter or decimal digit is escaped as \xNN.
print(RegExp.escape('foo'));
// CHECK-NEXT: \x66oo
print(RegExp.escape('1abc'));
// CHECK-NEXT: \x31abc
print(RegExp.escape('Zoo'));
// CHECK-NEXT: \x5aoo
// A non-alphanumeric leading character is not \xNN-escaped.
print(RegExp.escape('(foo)'));
// CHECK-NEXT: \(foo\)
// The underscore is never escaped.
print(RegExp.escape('_hello'));
// CHECK-NEXT: _hello

print('other-punctuators');
// CHECK-LABEL: other-punctuators
// These cannot be backslash-escaped, so they use \xNN.
print(RegExp.escape('a,-=<>#&!%:;@~\'`"'));
// CHECK-NEXT: \x61\x2c\x2d\x3d\x3c\x3e\x23\x26\x21\x25\x3a\x3b\x40\x7e\x27\x60\x22

print('control-and-space');
// CHECK-LABEL: control-and-space
// Control escapes from Table 64.
print(RegExp.escape('a\t\n\v\f\r'));
// CHECK-NEXT: \x61\t\n\v\f\r
// The space character is escaped as \x20.
print(RegExp.escape('a b'));
// CHECK-NEXT: \x61\x20b

print('whitespace-unicode');
// CHECK-LABEL: whitespace-unicode
// NBSP (<=0xFF) uses \xNN; other whitespace/line terminators use \uNNNN.
print(RegExp.escape('a\u00A0\u2028\uFEFF'));
// CHECK-NEXT: \x61\xa0\u2028\ufeff

print('surrogates');
// CHECK-LABEL: surrogates
// A lone surrogate is escaped as \uNNNN (lowercase).
print(RegExp.escape('\uD800'));
// CHECK-NEXT: \ud800
// A valid surrogate pair (astral code point) is passed through unchanged.
print(RegExp.escape('a\u{1F600}'));
// CHECK-NEXT: \x61😀

print('non-ascii-passthrough');
// CHECK-LABEL: non-ascii-passthrough
// Non-whitespace non-ASCII characters are not escaped; the leading 'r' is.
print(RegExp.escape('résumé'));
// CHECK-NEXT: \x72ésumé

print('empty');
// CHECK-LABEL: empty
print(JSON.stringify(RegExp.escape('')));
// CHECK-NEXT: ""

print('round-trip');
// CHECK-LABEL: round-trip
// The escaped string matches the original literally.
var s = 'a.b*c(foo) 1+2';
print(new RegExp(RegExp.escape(s)).test(s));
// CHECK-NEXT: true

print('non-string-throws');
// CHECK-LABEL: non-string-throws
function check(arg) {
  try {
    RegExp.escape(arg);
    print('no throw');
  } catch (e) {
    print(e.name);
  }
}
check(123);
// CHECK-NEXT: TypeError
check({});
// CHECK-NEXT: TypeError
check(null);
// CHECK-NEXT: TypeError
check(undefined);
// CHECK-NEXT: TypeError
check(Symbol('x'));
// CHECK-NEXT: TypeError
