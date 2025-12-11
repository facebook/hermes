/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-sema %s | %FileCheckOrRegen %s --match-full-lines

(function() {
// The resulting type is a union of its elements.
var unionArr = [1, "", undefined];

// Assigning to an annotated variable lets us use its type.
var annotatedArr: (string | number | void)[] = [1, ""];

// The two arrays have the same type.
annotatedArr = unionArr

class Base {}
class Derived extends Base {
    constructor(){
        super();
    }
}

var baseArr: Base[] = [new Base(), new Derived()];

var numArr: number[] = [1, 2, 3];
var strArr: (number | string)[] = ["a", "b", "c", ...numArr];
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:%untyped_function.1 = untyped_function()
// CHECK-NEXT:%class.2 = class(Base {
// CHECK-NEXT:  %homeObject: %class.3
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.4 = class_constructor(%class.2)
// CHECK-NEXT:%class.5 = class(Derived extends %class.2 {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.7
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.8 = class_constructor(%class.5)
// CHECK-NEXT:%class.3 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%function.6 = function(this: %class.5): void
// CHECK-NEXT:%class.7 = class( extends %class.3 {
// CHECK-NEXT:})
// CHECK-NEXT:%union.9 = union(void | string | number)
// CHECK-NEXT:%array.10 = array(%union.9)
// CHECK-NEXT:%array.11 = array(%class.2)
// CHECK-NEXT:%array.12 = array(number)
// CHECK-NEXT:%union.13 = union(string | number)
// CHECK-NEXT:%array.14 = array(%union.13)
// CHECK-NEXT:%object.15 = object({
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
// CHECK-NEXT:                Decl %d.63 'unionArr' Var : %array.10
// CHECK-NEXT:                Decl %d.64 'annotatedArr' Var : %array.10
// CHECK-NEXT:                Decl %d.65 'Base' Class : %class_constructor.4
// CHECK-NEXT:                Decl %d.66 'Derived' Class : %class_constructor.8
// CHECK-NEXT:                Decl %d.67 'baseArr' Var : %array.11
// CHECK-NEXT:                Decl %d.68 'numArr' Var : %array.12
// CHECK-NEXT:                Decl %d.69 'strArr' Var : %array.14
// CHECK-NEXT:                Decl %d.70 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:                    Decl %d.71 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.61 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : any
// CHECK-NEXT:                            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.10
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                Id 'undefined' [D:E:%d.26 'undefined'] : void
// CHECK-NEXT:                                            Id 'unionArr' [D:E:%d.63 'unionArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.10
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                            Id 'annotatedArr' [D:E:%d.64 'annotatedArr']
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        AssignmentExpression : %array.10
// CHECK-NEXT:                                            Id 'annotatedArr' [D:E:%d.64 'annotatedArr'] : %array.10
// CHECK-NEXT:                                            Id 'unionArr' [D:E:%d.63 'unionArr'] : %array.10
// CHECK-NEXT:                                    ClassDeclaration
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.65 'Base']
// CHECK-NEXT:                                        ClassBody
// CHECK-NEXT:                                    ClassDeclaration
// CHECK-NEXT:                                        Id 'Derived' [D:E:%d.66 'Derived']
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.65 'Base'] : %class_constructor.4
// CHECK-NEXT:                                        ClassBody
// CHECK-NEXT:                                            MethodDefinition : %function.6
// CHECK-NEXT:                                                Id 'constructor'
// CHECK-NEXT:                                                FunctionExpression : %function.6
// CHECK-NEXT:                                                    BlockStatement
// CHECK-NEXT:                                                        ExpressionStatement
// CHECK-NEXT:                                                            CallExpression
// CHECK-NEXT:                                                                Super
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.11
// CHECK-NEXT:                                                NewExpression : %class.2
// CHECK-NEXT:                                                    Id 'Base' [D:E:%d.65 'Base'] : %class_constructor.4
// CHECK-NEXT:                                                NewExpression : %class.5
// CHECK-NEXT:                                                    Id 'Derived' [D:E:%d.66 'Derived'] : %class_constructor.8
// CHECK-NEXT:                                            Id 'baseArr' [D:E:%d.67 'baseArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.12
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                            Id 'numArr' [D:E:%d.68 'numArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.14
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                SpreadElement
// CHECK-NEXT:                                                    Id 'numArr' [D:E:%d.68 'numArr'] : %array.12
// CHECK-NEXT:                                            Id 'strArr' [D:E:%d.69 'strArr']
// CHECK-NEXT:            ObjectExpression : %object.15
