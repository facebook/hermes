/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -dump-sema %s | %FileCheckOrRegen --match-full-lines %s

// Test that "arguments" in a paramater initializer expression is resolved
// correctly and doesn't clash with "let arguments" in the body of the function.

function foo(a=arguments[1]) {
    let arguments;
    print(a, arguments);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:SemContext
// CHECK-NEXT:Func loose
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'foo' GlobalProperty
// CHECK-NEXT:        Decl %d.2 'print' UndeclaredGlobalProperty
// CHECK-NEXT:        hoistedFunction foo
// CHECK-NEXT:    Func loose
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.3 'a' Parameter
// CHECK-NEXT:            Decl %d.4 'arguments' Var Arguments
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:            Scope %s.4
// CHECK-NEXT:                Decl %d.5 'arguments' Let

// CHECK:Program Scope %s.1
// CHECK-NEXT:    FunctionDeclaration
// CHECK-NEXT:        Id 'foo' [D:E:%d.1 'foo']
// CHECK-NEXT:        AssignmentPattern
// CHECK-NEXT:            Id 'a' [D:E:%d.3 'a']
// CHECK-NEXT:            MemberExpression
// CHECK-NEXT:                Id 'arguments' [D:E:%d.4 'arguments']
// CHECK-NEXT:                NumericLiteral
// CHECK-NEXT:        BlockStatement
// CHECK-NEXT:            VariableDeclaration
// CHECK-NEXT:                VariableDeclarator
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.5 'arguments']
// CHECK-NEXT:            ExpressionStatement
// CHECK-NEXT:                CallExpression
// CHECK-NEXT:                    Id 'print' [D:E:%d.2 'print']
// CHECK-NEXT:                    Id 'a' [D:E:%d.3 'a']
// CHECK-NEXT:                    Id 'arguments' [D:E:%d.5 'arguments']
