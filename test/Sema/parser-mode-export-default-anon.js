/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xcompile=false -dump-sema %s | %FileCheck %s --match-full-lines

// An anonymous 'export default function' is only rewritten to a
// FunctionExpression when compiling, so with -Xcompile=false the hoisted
// FunctionDeclaration still has a null id. The dumper used to cast it
// unconditionally and crash; it must print '*default*' instead.

export default function () {}

// CHECK:SemContext
// CHECK-NEXT:Func loose mayReachImplicitReturn
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        hoistedFunction *default*
