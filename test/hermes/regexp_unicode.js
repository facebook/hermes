// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

print('RegExp Unicode');
// CHECK: RegExp Unicode

try { new RegExp("\\u{FFFFFFFFF}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Escaped value too large

print(/abc/ui.exec("AbC"));
// CHECK-NEXT: AbC

print(/\u{48}/.exec("H"));
// CHECK-NEXT: null

print(/\u{48}/u.exec("H"));
// CHECK-NEXT: H

print(/\110/.exec("H"));
// CHECK-NEXT: H

try { new RegExp("\\110", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\u{}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\u{ABC", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\u{}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\u}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\u", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\uZZZZ", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid escape

try { new RegExp("\\u{FFFFFFFFF}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Escaped value too large

try { new RegExp("{", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid quantifier bracket

try { new RegExp("}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp pattern: Invalid quantifier bracket

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

// Test advanceStringIndex.
// We should not match a low surrogate in a Unicode regexp.
print(!! /\uDE42/u.exec("\uD83D\uDE42ZZZ"));
// CHECK-NEXT: false
print(!! /\uDE42/.exec("\uD83D\uDE42ZZZ"));
// CHECK-NEXT: true
