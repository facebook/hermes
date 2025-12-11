/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema %s | %FileCheckOrRegen %s --match-full-lines

// This should get reasonable flow types for
//   * The call to $SHBuiltin.moduleFactory:
//        type of args[1].
//
//   * The $SHBuiltin.export member expression:
//        (string, any) => void
//   * The call to $SHBuiltin.export:
//        void
//
//   * The $SHBuiltin.import member expression:
//        (number, string, any) => any
//   * The call to $SHBuiltin.import
//        type of args[2]

// (Note: the (x as any).length expressions are a workaround for a
// current deficiency in static hermes.)

(function() {
    $SHBuiltin.moduleFactory(0, function(global, require, s: string): number {
        return 7 + (s as any).length;
    });
    $SHBuiltin.moduleFactory(1, function(global, require): number {
        var n: number = 100;
        $SHBuiltin.export('n', n);
        return 0;
    });
    $SHBuiltin.moduleFactory(2, function(global, require): number {
        var n2: number = ($SHBuiltin.import(1, 'n', "abc") as any).length + 2;
        return 0;
    });
})

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%function.2 = function(global: any, require: any, s: string): number
// CHECK-NEXT:%function.3 = function(global: any, require: any): number
// CHECK-NEXT:%object.4 = object({
// CHECK-NEXT:})

// CHECK:SemContext
// CHECK-NEXT:Func strict
// CHECK-NEXT:    Scope %s.1
// CHECK-NEXT:        Decl %d.1 'Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.2 'BigInt' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.3 'Boolean' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.4 'Date' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.5 'Error' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.6 'Function' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.7 'HermesInternal' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.8 'HermesAsyncIteratorsInternal' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.9 'JSON' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.10 'Map' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.11 'Math' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.12 'Number' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.13 'Object' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.14 'Proxy' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.15 'Reflect' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.16 'RegExp' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.17 'Set' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.18 'String' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.19 'Symbol' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.20 'WeakMap' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.21 'WeakSet' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.22 '$SHBuiltin' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.23 'Infinity' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.24 'NaN' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.25 'globalThis' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.26 'undefined' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.27 'AggregateError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.28 'EvalError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.29 'RangeError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.30 'ReferenceError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.31 'SyntaxError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.32 'TypeError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.33 'URIError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.34 'ArrayBuffer' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.35 'DataView' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.36 'TextEncoder' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.37 'Uint8Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.38 'Int8Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.39 'Uint8ClampedArray' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.40 'Uint16Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.41 'Int16Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.42 'Uint32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.43 'Int32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.44 'Float32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.45 'Float64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.46 'BigUint64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.47 'BigInt64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.48 'print' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.49 'eval' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.50 'parseInt' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.51 'parseFloat' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.52 'isNaN' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.53 'isFinite' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.54 'escape' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.55 'unescape' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.56 'decodeURI' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.57 'decodeURIComponent' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.58 'encodeURI' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.59 'encodeURIComponent' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.60 'gc' UndeclaredGlobalProperty
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.61 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.62 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.63 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                    Decl %d.64 'global' Parameter : any
// CHECK-NEXT:                    Decl %d.65 'require' Parameter : any
// CHECK-NEXT:                    Decl %d.66 's' Parameter : string
// CHECK-NEXT:                    Decl %d.67 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:                    Decl %d.68 'global' Parameter : any
// CHECK-NEXT:                    Decl %d.69 'require' Parameter : any
// CHECK-NEXT:                    Decl %d.70 'n' Var : number
// CHECK-NEXT:                    Decl %d.71 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.6
// CHECK-NEXT:                    Decl %d.72 'global' Parameter : any
// CHECK-NEXT:                    Decl %d.73 'require' Parameter : any
// CHECK-NEXT:                    Decl %d.74 'n2' Var : number
// CHECK-NEXT:                    Decl %d.75 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.61 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        FunctionExpression : %untyped_function.1
// CHECK-NEXT:                            BlockStatement
// CHECK-NEXT:                                ExpressionStatement
// CHECK-NEXT:                                    CallExpression : %function.2
// CHECK-NEXT:                                        MemberExpression : any
// CHECK-NEXT:                                            SHBuiltin
// CHECK-NEXT:                                            Id 'moduleFactory'
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        FunctionExpression : %function.2
// CHECK-NEXT:                                            Id 'global' [D:E:%d.64 'global']
// CHECK-NEXT:                                            Id 'require' [D:E:%d.65 'require']
// CHECK-NEXT:                                            Id 's' [D:E:%d.66 's']
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    ImplicitCheckedCast : number
// CHECK-NEXT:                                                        BinaryExpression : any
// CHECK-NEXT:                                                            NumericLiteral : number
// CHECK-NEXT:                                                            BinOp +
// CHECK-NEXT:                                                            MemberExpression : any
// CHECK-NEXT:                                                                AsExpression : any
// CHECK-NEXT:                                                                    Id 's' [D:E:%d.66 's'] : string
// CHECK-NEXT:                                                                    AnyTypeAnnotation
// CHECK-NEXT:                                                                Id 'length'
// CHECK-NEXT:                                ExpressionStatement
// CHECK-NEXT:                                    CallExpression : %function.3
// CHECK-NEXT:                                        MemberExpression : any
// CHECK-NEXT:                                            SHBuiltin
// CHECK-NEXT:                                            Id 'moduleFactory'
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        FunctionExpression : %function.3
// CHECK-NEXT:                                            Id 'global' [D:E:%d.68 'global']
// CHECK-NEXT:                                            Id 'require' [D:E:%d.69 'require']
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                VariableDeclaration
// CHECK-NEXT:                                                    VariableDeclarator
// CHECK-NEXT:                                                        NumericLiteral : number
// CHECK-NEXT:                                                        Id 'n' [D:E:%d.70 'n']
// CHECK-NEXT:                                                ExpressionStatement
// CHECK-NEXT:                                                    CallExpression : void
// CHECK-NEXT:                                                        MemberExpression : any
// CHECK-NEXT:                                                            SHBuiltin
// CHECK-NEXT:                                                            Id 'export'
// CHECK-NEXT:                                                        StringLiteral : string
// CHECK-NEXT:                                                        Id 'n' [D:E:%d.70 'n'] : number
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:                                ExpressionStatement
// CHECK-NEXT:                                    CallExpression : %function.3
// CHECK-NEXT:                                        MemberExpression : any
// CHECK-NEXT:                                            SHBuiltin
// CHECK-NEXT:                                            Id 'moduleFactory'
// CHECK-NEXT:                                        NumericLiteral : number
// CHECK-NEXT:                                        FunctionExpression : %function.3
// CHECK-NEXT:                                            Id 'global' [D:E:%d.72 'global']
// CHECK-NEXT:                                            Id 'require' [D:E:%d.73 'require']
// CHECK-NEXT:                                            BlockStatement
// CHECK-NEXT:                                                VariableDeclaration
// CHECK-NEXT:                                                    VariableDeclarator
// CHECK-NEXT:                                                        ImplicitCheckedCast : number
// CHECK-NEXT:                                                            BinaryExpression : any
// CHECK-NEXT:                                                                MemberExpression : any
// CHECK-NEXT:                                                                    AsExpression : any
// CHECK-NEXT:                                                                        CallExpression : string
// CHECK-NEXT:                                                                            MemberExpression : any
// CHECK-NEXT:                                                                                SHBuiltin
// CHECK-NEXT:                                                                                Id 'import'
// CHECK-NEXT:                                                                            NumericLiteral : number
// CHECK-NEXT:                                                                            StringLiteral : string
// CHECK-NEXT:                                                                            StringLiteral : string
// CHECK-NEXT:                                                                        AnyTypeAnnotation
// CHECK-NEXT:                                                                    Id 'length'
// CHECK-NEXT:                                                                BinOp +
// CHECK-NEXT:                                                                NumericLiteral : number
// CHECK-NEXT:                                                        Id 'n2' [D:E:%d.74 'n2']
// CHECK-NEXT:                                                ReturnStatement
// CHECK-NEXT:                                                    NumericLiteral : number
// CHECK-NEXT:            ObjectExpression : %object.4
