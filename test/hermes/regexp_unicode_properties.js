/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: regexp_unicode_properties

print('RegExp Unicode Properties');
// CHECK-LABEL: RegExp Unicode Properties

function hexCodePointAt(s, idx) {
  return s.codePointAt(idx).toString(16).toUpperCase();
}

try { new RegExp("\\p{PropertyThatDoesNotExist}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\p{PropertyThatDoesNotExist}]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("\\p{Script=}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\p{Script=}]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

print(/\p{Lu}\p{Ll}/ui.exec("aB"));
// CHECK-NEXT: aB

print(/\p{Lowercase_Letter}/u.exec("a"));
// CHECK-NEXT: a

try { new RegExp("\\p{lowercase_letter}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

print(/\p{Decimal_Number}/u.exec("1"));
// CHECK-NEXT: 1
print(/[\p{Decimal_Number}]+/u.exec("123"));
// CHECK-NEXT: 123

// Not a Unicode regex, so `\p` is just a literal.
print(/\p{Nd}/.exec("p{Nd}"));
// CHECK-NEXT: p{Nd}
print(/\P{Nd}/.exec("P{Nd}"));
// CHECK-NEXT: P{Nd}

// Compound categories
print(/\p{N}/u.exec("1"));
// CHECK-NEXT: 1
print(/\p{N}/u.exec("a"));
// CHECK-NEXT: null
print(/\p{L}/u.exec("a"));
// CHECK-NEXT: a
print(/\p{L}/u.exec("1"));
// CHECK-NEXT: null

// U+FFFF is unassigned (as of Unicode 15.1.0).
print(hexCodePointAt(/\p{Unassigned}/u.exec("\u{FFFF}")[0]));
// CHECK-NEXT: FFFF
print(/\p{Assigned}/u.exec("\u{FFFF}"));
// CHECK-NEXT: null
// Script=Zzzz behaves like Unassigned / Cn.
print(hexCodePointAt(/\p{Script=Zzzz}/u.exec("\u{FFFF}")[0]));
// CHECK-NEXT: FFFF

// U+FFFD is the replacement character, which is assigned.
print(hexCodePointAt(/\P{Unassigned}/u.exec("\u{FFFD}")[0]));
// CHECK-NEXT: FFFD
print(/\P{Assigned}/u.exec("\u{FFFD}"));
// CHECK-NEXT: null
// Script=Zzzz behaves like Unassigned / Cn.
print(/\p{Script=Zzzz}/u.exec("\u{FFFD}"));
// CHECK-NEXT: null

print(/\p{Script=Latin}+/u.exec("Ave"));
// CHECK-NEXT: Ave

print(/\p{Script_Extensions=Latin}+/u.exec("Ave"));
// CHECK-NEXT: Ave

// U+202F only exists in script extensions for Latin.
print(/\p{Script=Latin}/u.exec("\u{202F}"));
// CHECK-NEXT: null
print(hexCodePointAt(/\p{Script_Extensions=Latin}/u.exec("\u{202F}")[0]));
// CHECK-NEXT: 202F

// U+30A0 only exists in script extensions for Hira / Kana.
print(/\p{Script=Hira}/u.exec("\u{30A0}"));
// CHECK-NEXT: null
print(/\p{Script=Kana}/u.exec("\u{30A0}"));
// CHECK-NEXT: null
print(hexCodePointAt(/\p{Script_Extensions=Hira}/u.exec("\u{30A0}")[0]));
// CHECK-NEXT: 30A0
print(hexCodePointAt(/\p{Script_Extensions=Kana}/u.exec("\u{30A0}")[0]));
// CHECK-NEXT: 30A0

// CJK ideographs are not in the Latin script.
print(/\p{Script=Latin}+/u.exec("\u{4f60}\u{597d}"));
// CHECK-NEXT: null
print(/\p{Script_Extensions=Latin}+/u.exec("\u{4f60}\u{597d}"));
// CHECK-NEXT: null

try { new RegExp("\\P{}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\P{}]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("\\p{}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\p{}]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("\\P{Ll", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\P{Ll]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("\\P}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\P}]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("\\p}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\p}]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("\\p", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\p]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("\\P", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name
try { new RegExp("[\\P]", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid property name

try { new RegExp("{", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid quantifier bracket

try { new RegExp("}", "u"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid quantifier bracket

print((new RegExp("{", "")).source);
// CHECK-NEXT: {

print((new RegExp("}", "")).source);
// CHECK-NEXT: }

// Here the match has length 2, because this emoji must be encoded via
// a surrogate pair.
print(/\p{Emoji}/u.exec("\u{1F600}")[0].length);
// CHECK-NEXT: 2

// Hex or non-hex codepoints, i.e. all codepoints.
print(/[\p{Hex}\P{Hex}]/u.exec('\u{1D306}'));
// CHECK-NEXT: 𝌆

// Not uppercase letters.
print(/[\P{Lu}]+/u.exec('abc'));
// CHECK-NEXT: abc
print(/[\P{Lu}]+/u.exec('ABC'));
// CHECK-NEXT: null

// Not not uppercase letters.
print(/[^\P{Lu}]+/u.exec('ABC'));
// CHECK-NEXT: ABC
print(/[^\P{Lu}]+/u.exec('abc'));
// CHECK-NEXT: null

print(/\p{Lu}+/ui.exec("aBc"));
// CHECK-NEXT: aBc
print(/[\p{Lu}]+/ui.exec("aBc"));
// CHECK-NEXT: aBc
