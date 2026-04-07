/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

let func = () => {
  let x = a;
  let y = b;
  let z = c;
}

let a: number = Math.random();
let b = 10;
let c = Math.random();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'exports' Parameter : any
// CHECK-NEXT:        Decl %d.2 'func' Let : %untyped_function.1
// CHECK-NEXT:        Decl %d.3 'a' Let : number
// CHECK-NEXT:        Decl %d.4 'b' Let : number
// CHECK-NEXT:        Decl %d.5 'c' Let : any
// CHECK-NEXT:        Decl %d.6 'arguments' Var Arguments
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.7 'x' Let : number
// CHECK-NEXT:            Decl %d.8 'y' Let : number
// CHECK-NEXT:            Decl %d.9 'z' Let : any

// CHECK:FunctionExpression : %untyped_function.1
// CHECK-NEXT:    Id 'exports' [D:E:%d.1 'exports']
// CHECK-NEXT:    BlockStatement
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ArrowFunctionExpression : %untyped_function.1
// CHECK-NEXT:                    BlockStatement
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                Id 'a' [D:E:%d.3 'a'] : number
// CHECK-NEXT:                                Id 'x' [D:E:%d.7 'x']
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                Id 'b' [D:E:%d.4 'b'] : number
// CHECK-NEXT:                                Id 'y' [D:E:%d.8 'y']
// CHECK-NEXT:                        VariableDeclaration
// CHECK-NEXT:                            VariableDeclarator
// CHECK-NEXT:                                Id 'c' [D:E:%d.5 'c'] : any
// CHECK-NEXT:                                Id 'z' [D:E:%d.9 'z']
// CHECK-NEXT:                Id 'func' [D:E:%d.2 'func']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                ImplicitCheckedCast : number
// CHECK-NEXT:                    CallExpression
// CHECK-NEXT:                        MemberExpression : any
// CHECK-NEXT:                            Id 'Math' [D:E:%d.10 'Math'] : any
// CHECK-NEXT:                            Id 'random'
// CHECK-NEXT:                Id 'a' [D:E:%d.3 'a']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                NumericLiteral : number
// CHECK-NEXT:                Id 'b' [D:E:%d.4 'b']
// CHECK-NEXT:        VariableDeclaration
// CHECK-NEXT:            VariableDeclarator
// CHECK-NEXT:                CallExpression
// CHECK-NEXT:                    MemberExpression : any
// CHECK-NEXT:                        Id 'Math' [D:E:%d.10 'Math'] : any
// CHECK-NEXT:                        Id 'random'
// CHECK-NEXT:                Id 'c' [D:E:%d.5 'c']
