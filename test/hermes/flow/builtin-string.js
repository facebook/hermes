/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test the String method trampolines in TypedLib.

'use strict';

(function () {

const s: string = 'Hello World';
const empty: string = '';

print('charAt');
// CHECK-LABEL: charAt
print(s.charAt(0), s.charAt(10));
// CHECK-NEXT: H d

print('at');
// CHECK-LABEL: at
print(s.at(0), s.at(6));
// CHECK-NEXT: H W
print(s.at(-1), s.at(-11));
// CHECK-NEXT: d H
print(s.at(11), s.at(-12), empty.at(0));
// CHECK-NEXT: undefined undefined undefined

print('charCodeAt');
// CHECK-LABEL: charCodeAt
print(s.charCodeAt(0), s.charCodeAt(1));
// CHECK-NEXT: 72 101
print(s.charCodeAt(11));
// CHECK-NEXT: NaN

print('codePointAt');
// CHECK-LABEL: codePointAt
print(s.codePointAt(0), '\u{1F600}'.codePointAt(0));
// CHECK-NEXT: 72 128512
print(s.codePointAt(11));
// CHECK-NEXT: undefined

print('concat');
// CHECK-LABEL: concat
print(s.concat('!'), empty.concat('a'));
// CHECK-NEXT: Hello World! a

print('endsWith');
// CHECK-LABEL: endsWith
print(s.endsWith('World'), s.endsWith('Hello'));
// CHECK-NEXT: true false
print(s.endsWith('Hello', 5), empty.endsWith(''));
// CHECK-NEXT: true true

print('includes');
// CHECK-LABEL: includes
print(s.includes('lo W'), s.includes('xyz'));
// CHECK-NEXT: true false
print(s.includes('Hello', 1));
// CHECK-NEXT: false

print('indexOf');
// CHECK-LABEL: indexOf
print(s.indexOf('o'), s.indexOf('o', 5), s.indexOf('z'));
// CHECK-NEXT: 4 7 -1

print('lastIndexOf');
// CHECK-LABEL: lastIndexOf
print(s.lastIndexOf('o'), s.lastIndexOf('o', 5), s.lastIndexOf('z'));
// CHECK-NEXT: 7 4 -1

print('padStart/padEnd');
// CHECK-LABEL: padStart/padEnd
print('5'.padStart(3, '0'), '5'.padEnd(3, '0'));
// CHECK-NEXT: 005 500
print('|' + 'ab'.padStart(4) + '|', '|' + 'ab'.padEnd(4) + '|');
// CHECK-NEXT: |  ab| |ab  |
// Target length already met: unchanged.
print('abc'.padStart(2, '0'), 'abc'.padEnd(2, '0'));
// CHECK-NEXT: abc abc

print('repeat');
// CHECK-LABEL: repeat
print('ab'.repeat(3), '|' + 'ab'.repeat(0) + '|');
// CHECK-NEXT: ababab ||

print('slice');
// CHECK-LABEL: slice
print(s.slice(0, 5), s.slice(6));
// CHECK-NEXT: Hello World
print(s.slice(-5), s.slice(-5, -3));
// CHECK-NEXT: World Wo
print('|' + s.slice(5, 2) + '|', '|' + s.slice(100) + '|');
// CHECK-NEXT: || ||

print('split');
// CHECK-LABEL: split
const parts: any = 'a,b,c'.split(',');
print(parts.length, parts[0], parts[2]);
// CHECK-NEXT: 3 a c
const limited: any = 'a,b,c'.split(',', 2);
print(limited.length, limited[1]);
// CHECK-NEXT: 2 b
// No separator: the whole string in a single element.
const whole: any = 'abc'.split();
print(whole.length, whole[0]);
// CHECK-NEXT: 1 abc
const chars: any = 'abc'.split('');
print(chars.length, chars[0], chars[2]);
// CHECK-NEXT: 3 a c

print('startsWith');
// CHECK-LABEL: startsWith
print(s.startsWith('Hello'), s.startsWith('World'));
// CHECK-NEXT: true false
print(s.startsWith('World', 6));
// CHECK-NEXT: true

print('substring');
// CHECK-LABEL: substring
print(s.substring(6), s.substring(0, 5));
// CHECK-NEXT: World Hello
// Arguments are swapped when start > end, and negatives clamp to 0.
print(s.substring(5, 0), s.substring(-3, 5));
// CHECK-NEXT: Hello Hello

print('case');
// CHECK-LABEL: case
print(s.toUpperCase(), s.toLowerCase());
// CHECK-NEXT: HELLO WORLD hello world

print('trim');
// CHECK-LABEL: trim
print('|' + '  x  '.trim() + '|');
// CHECK-NEXT: |x|
print('|' + '  x  '.trimStart() + '|');
// CHECK-NEXT: |x  |
print('|' + '  x  '.trimEnd() + '|');
// CHECK-NEXT: |  x|

})();
