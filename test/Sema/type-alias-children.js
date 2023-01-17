/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -parse-flow -dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that children of type alias AST node are not resolved as variables.
type A = B;

// Auto-generated content below. Please do not modify manually.

//CHECK:      SemContext
//CHECK-NEXT: Func loose
//CHECK-NEXT:     Scope %s.1
//CHECK-EMPTY:
//CHECK-NEXT: Program
//CHECK-NEXT:     TypeAlias
//CHECK-NEXT:         Id 'A'
//CHECK-NEXT:         GenericTypeAnnotation
//CHECK-NEXT:             Id 'B'
