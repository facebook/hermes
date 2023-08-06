/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

try {} catch (e) { let e; let a; let a; }

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}catch-block-error.js:10:24: error: Identifier 'e' is already declared
// CHECK-NEXT:try {} catch (e) { let e; let a; let a; }
// CHECK-NEXT:                       ^
// CHECK-NEXT:{{.*}}catch-block-error.js:10:15: note: previous declaration
// CHECK-NEXT:try {} catch (e) { let e; let a; let a; }
// CHECK-NEXT:              ^
// CHECK-NEXT:{{.*}}catch-block-error.js:10:38: error: Identifier 'a' is already declared
// CHECK-NEXT:try {} catch (e) { let e; let a; let a; }
// CHECK-NEXT:                                     ^
// CHECK-NEXT:{{.*}}catch-block-error.js:10:31: note: previous declaration
// CHECK-NEXT:try {} catch (e) { let e; let a; let a; }
// CHECK-NEXT:                              ^
// CHECK-NEXT:Emitted 2 errors. exiting.
