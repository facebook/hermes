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

print('done');
// CHECK-NEXT: done
