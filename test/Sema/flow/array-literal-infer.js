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
// CHECK-NEXT:  %homeObject: %class.12
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.3 = class_constructor(%class.2)
// CHECK-NEXT:%class.4 = class(Derived extends %class.2 {
// CHECK-NEXT:  %constructor: %function.6
// CHECK-NEXT:  %homeObject: %class.13
// CHECK-NEXT:})
// CHECK-NEXT:%class_constructor.5 = class_constructor(%class.4)
// CHECK-NEXT:%function.6 = function(this: %class.4): void
// CHECK-NEXT:%array.7 = array(%union.14)
// CHECK-NEXT:%array.8 = array(%class.2)
// CHECK-NEXT:%array.9 = array(number)
// CHECK-NEXT:%array.10 = array(%union.15)
// CHECK-NEXT:%object.11 = object({
// CHECK-NEXT:})
// CHECK-NEXT:%class.12 = class( {
// CHECK-NEXT:})
// CHECK-NEXT:%class.13 = class( extends %class.12 {
// CHECK-NEXT:})
// CHECK-NEXT:%union.14 = union(void | string | number)
// CHECK-NEXT:%union.15 = union(string | number)

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
// CHECK-NEXT:        Decl %d.23 'Hermes' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.24 'Infinity' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.25 'NaN' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.26 'globalThis' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.27 'undefined' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.28 'AggregateError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.29 'EvalError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.30 'RangeError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.31 'ReferenceError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.32 'SyntaxError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.33 'TypeError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.34 'URIError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.35 'ArrayBuffer' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.36 'DataView' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.37 'TextEncoder' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.38 'Uint8Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.39 'Int8Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.40 'Uint8ClampedArray' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.41 'Uint16Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.42 'Int16Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.43 'Uint32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.44 'Int32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.45 'Float32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.46 'Float64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.47 'BigUint64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.48 'BigInt64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.49 'print' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.50 'eval' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.51 'parseInt' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.52 'parseFloat' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.53 'isNaN' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.54 'isFinite' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.55 'escape' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.56 'unescape' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.57 'decodeURI' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.58 'decodeURIComponent' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.59 'encodeURI' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.60 'encodeURIComponent' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.61 'gc' UndeclaredGlobalProperty
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.62 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.63 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.64 'unionArr' Var : %array.7
// CHECK-NEXT:                Decl %d.65 'annotatedArr' Var : %array.7
// CHECK-NEXT:                Decl %d.66 'Base' Class : %class_constructor.3
// CHECK-NEXT:                Decl %d.67 'Derived' Class : %class_constructor.5
// CHECK-NEXT:                Decl %d.68 'baseArr' Var : %array.8
// CHECK-NEXT:                Decl %d.69 'numArr' Var : %array.9
// CHECK-NEXT:                Decl %d.70 'strArr' Var : %array.10
// CHECK-NEXT:                Decl %d.71 'arguments' Var Arguments
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.6
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.7
// CHECK-NEXT:                    Decl %d.72 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.62 'exports']
// CHECK-NEXT:                BlockStatement
// CHECK-NEXT:                    ExpressionStatement
// CHECK-NEXT:                        CallExpression : any
// CHECK-NEXT:                            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                                BlockStatement
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.7
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                Id 'undefined' [D:E:%d.27 'undefined'] : void
// CHECK-NEXT:                                            Id 'unionArr' [D:E:%d.64 'unionArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.7
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                            Id 'annotatedArr' [D:E:%d.65 'annotatedArr']
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        AssignmentExpression : %array.7
// CHECK-NEXT:                                            Id 'annotatedArr' [D:E:%d.65 'annotatedArr'] : %array.7
// CHECK-NEXT:                                            Id 'unionArr' [D:E:%d.64 'unionArr'] : %array.7
// CHECK-NEXT:                                    ClassDeclaration Scope %s.4
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.66 'Base']
// CHECK-NEXT:                                        ClassBody
// CHECK-NEXT:                                    ClassDeclaration Scope %s.5
// CHECK-NEXT:                                        Id 'Derived' [D:E:%d.67 'Derived']
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.66 'Base'] : %class_constructor.3
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
// CHECK-NEXT:                                            ArrayExpression : %array.8
// CHECK-NEXT:                                                NewExpression : %class.2
// CHECK-NEXT:                                                    Id 'Base' [D:E:%d.66 'Base'] : %class_constructor.3
// CHECK-NEXT:                                                NewExpression : %class.4
// CHECK-NEXT:                                                    Id 'Derived' [D:E:%d.67 'Derived'] : %class_constructor.5
// CHECK-NEXT:                                            Id 'baseArr' [D:E:%d.68 'baseArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.9
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                            Id 'numArr' [D:E:%d.69 'numArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.10
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                SpreadElement
// CHECK-NEXT:                                                    Id 'numArr' [D:E:%d.69 'numArr'] : %array.9
// CHECK-NEXT:                                            Id 'strArr' [D:E:%d.70 'strArr']
// CHECK-NEXT:            ObjectExpression : %object.11
