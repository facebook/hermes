/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

print('RegExp Unicode');
// CHECK: RegExp Unicode

try { new RegExp("\\u{FFFFFFFFF}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Escaped value too large

print(/abc/ui.exec("AbC"));
// CHECK-NEXT: AbC

print(/\u{48}/.exec("H"));
// CHECK-NEXT: null

print(/\u{48}/u.exec("H"));
// CHECK-NEXT: H

print(/\110/.exec("H"));
// CHECK-NEXT: H

try { new RegExp("\\110", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\u{}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\u{ABC", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\u{}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\u}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\u", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\uZZZZ", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid escape

try { new RegExp("\\u{FFFFFFFFF}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Escaped value too large

try { new RegExp("{", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid quantifier bracket

try { new RegExp("}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid quantifier bracket

print((new RegExp("{", "")).source);
// CHECK-NEXT: {

print((new RegExp("}", "")).source);
// CHECK-NEXT: }

print(/\u{1F600}/.exec("\u{1F600}"));
// CHECK-NEXT: null

// Here the match has length 2, because this emoji must be encoded via
// a surrogate pair.
print(/\u{1F600}/u.exec("\u{1F600}")[0].length);
// CHECK-NEXT: 2

// Here we expect to match because the surrogate pair looks like two characters to a non-unicode regexp.
print(!! /^..$/.exec("\u{1F600}"));
// CHECK-NEXT: true

// Here we expect to not match because the surrogate pair should look like one character to a non-unicode regexp.
print(!! /^..$/u.exec("\u{1F600}"));
// CHECK-NEXT: false
print(!! /^.$/u.exec("\u{1F600}"));
// CHECK-NEXT: true

// Ensure that we don't match newlines with the dot.
print(/.*/u.exec("abc\ndef"), "DONE");
// CHECK-NEXT: abc DONE
print(/.*/u.exec("\u0101bc\ndef")[0].length);
// CHECK-NEXT: 3

// Test advanceStringIndex.
// We should not match a low surrogate in a Unicode regexp.
print(!! /\uDE42/u.exec("\uD83D\uDE42ZZZ"));
// CHECK-NEXT: false
// We should match an unpaired surrogate.
print(!! /\uDC00/u.exec("\uDC00"));
// CHECK-NEXT: true
// Test the case insensitive variant.
print(!! /\uDC00/iu.exec("\uDC00"));
// CHECK-NEXT: true
// We should match the low surrogate when Unicode is off.
print(!! /\uDE42/.exec("\uD83D\uDE42ZZZ"));
// CHECK-NEXT: true

// Unicode regexps do not allow lastIndex to end on a low surrogate pair.
var ure = /(?:)/gu;
ure.exec("\ud800\udc00");
print(ure.lastIndex);
// CHECK-NEXT: 0
ure.lastIndex = 1;
ure.exec("\ud800\udc00");
print(ure.lastIndex);
// CHECK-NEXT: 0

// Non-Unicode regexps don't care.
var nure = /(?:)/g;
nure.exec("\ud800\udc00");
print(nure.lastIndex);
// CHECK-NEXT: 0
nure.lastIndex = 1;
nure.exec("\ud800\udc00");
print(nure.lastIndex);
// CHECK-NEXT: 1

ure = /(?:)/gu;
ure.lastIndex = 1;
print("\ud800\udc00\ud800\udc00".replace(ure, "!").length);
// CHECK-NEXT: 7
