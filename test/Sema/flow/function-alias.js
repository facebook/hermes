/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema -fno-std-globals %s | %FileCheckOrRegen %s --match-full-lines

type A = (number) => string;
type B = (this: number, number) => string;

// Auto-generated content below. Please do not modify manually.

// CHECK:function %t.1 = function (: number): string
// CHECK-NEXT:function %t.2 = function (this: number, : number): string

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1

// CHECK:Program Scope %s.1
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'A'
// CHECK-NEXT:        FunctionTypeAnnotation
// CHECK-NEXT:            FunctionTypeParam
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:            StringTypeAnnotation
// CHECK-NEXT:    TypeAlias
// CHECK-NEXT:        Id 'B'
// CHECK-NEXT:        FunctionTypeAnnotation
// CHECK-NEXT:            FunctionTypeParam
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:            FunctionTypeParam
// CHECK-NEXT:                NumberTypeAnnotation
// CHECK-NEXT:            StringTypeAnnotation
