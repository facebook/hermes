/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

print('RegExp Modifier Group Errors');
// CHECK-LABEL: RegExp Modifier Group Errors

// Duplicate flag.
try { new RegExp("(?ii:)"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// Flag in both positive and negative positions.
try { new RegExp("(?i-i:)"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// Double hyphen.
try { new RegExp("(?--i:)"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// No modifier characters.
try { new RegExp("(?-:)"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// Invalid modifier character.
try { new RegExp("(?x:)"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// Missing colon.
try { new RegExp("(?i)"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// Unterminated.
try { new RegExp("(?i-"); } catch (e) { print(e.message); }
// CHECK-NEXT: Invalid RegExp: Invalid flags

// Valid modifier groups (sanity check).
print(new RegExp("(?i:abc)").source);
// CHECK-NEXT: (?i:abc)

print(new RegExp("(?-i:abc)").source);
// CHECK-NEXT: (?-i:abc)

print(new RegExp("(?i-ms:abc)").source);
// CHECK-NEXT: (?i-ms:abc)

// Test that modifier groups affect matching behavior.
print('Modifier Group Matching');
// CHECK-LABEL: Modifier Group Matching

// (?i:...) enables case-insensitive matching inside the group.
print(/(?i:abc)/.test('ABC'));
// CHECK-NEXT: true
print(/(?i:abc)/.test('abc'));
// CHECK-NEXT: true
print(/(?i:abc)def/.test('ABCdef'));
// CHECK-NEXT: true
print(/(?i:abc)def/.test('ABCDEF'));
// CHECK-NEXT: false

// (?-i:...) disables case-insensitive matching inside the group.
print(/(?-i:abc)/i.test('ABC'));
// CHECK-NEXT: false
print(/(?-i:abc)/i.test('abc'));
// CHECK-NEXT: true
print(/(?-i:abc)def/i.test('abcDEF'));
// CHECK-NEXT: true
print(/(?-i:abc)def/i.test('ABCDEF'));
// CHECK-NEXT: false

// (?m:...) enables multiline matching inside the group.
print(/(?m:^abc)/.test('xyz\nabc'));
// CHECK-NEXT: true
print(/^abc/.test('xyz\nabc'));
// CHECK-NEXT: false
print(/(?m:abc$)/.test('abc\nxyz'));
// CHECK-NEXT: true
print(/abc$/.test('abc\nxyz'));
// CHECK-NEXT: false

// (?-m:...) disables multiline matching inside the group.
print(/(?-m:^abc)/m.test('xyz\nabc'));
// CHECK-NEXT: false
print(/(?-m:abc$)/m.test('abc\nxyz'));
// CHECK-NEXT: false

// (?s:...) enables dotAll matching inside the group.
print(/(?s:a.b)/.test('a\nb'));
// CHECK-NEXT: true
print(/a.b/.test('a\nb'));
// CHECK-NEXT: false

// (?-s:...) disables dotAll matching inside the group.
print(/(?-s:a.b)/s.test('a\nb'));
// CHECK-NEXT: false

// (?i:...) affects backreferences inside the group.
print(/(?i:(abc)\1)/.test('abcABC'));
// CHECK-NEXT: true
print(/(abc)(?i:\1)/.test('abcABC'));
// CHECK-NEXT: true
print(/(abc)\1/.test('abcABC'));
// CHECK-NEXT: false

// (?i:...) affects character classes.
print(/(?i:[a-z])/.test('A'));
// CHECK-NEXT: true
print(/[a-z]/.test('A'));
// CHECK-NEXT: false

print('done');
// CHECK-NEXT: done
