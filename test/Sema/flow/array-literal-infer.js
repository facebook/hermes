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
// CHECK-NEXT:        Decl %d.8 'JSON' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.9 'Map' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.10 'Math' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.11 'Number' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.12 'Object' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.13 'Proxy' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.14 'Reflect' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.15 'RegExp' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.16 'Set' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.17 'String' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.18 'Symbol' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.19 'WeakMap' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.20 'WeakSet' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.21 '$SHBuiltin' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.22 'Infinity' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.23 'NaN' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.24 'globalThis' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.25 'undefined' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.26 'AggregateError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.27 'EvalError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.28 'RangeError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.29 'ReferenceError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.30 'SyntaxError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.31 'TypeError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.32 'URIError' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.33 'ArrayBuffer' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.34 'DataView' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.35 'Int8Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.36 'Int16Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.37 'Int32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.38 'Uint8Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.39 'Uint8ClampedArray' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.40 'Uint16Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.41 'Uint32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.42 'Float32Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.43 'Float64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.44 'BigInt64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.45 'BigUint64Array' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.46 'print' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.47 'eval' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.48 'parseInt' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.49 'parseFloat' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.50 'isNaN' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.51 'isFinite' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.52 'escape' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.53 'unescape' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.54 'decodeURI' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.55 'decodeURIComponent' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.56 'encodeURI' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.57 'encodeURIComponent' UndeclaredGlobalProperty
// CHECK-NEXT:        Decl %d.58 'gc' UndeclaredGlobalProperty
// CHECK-NEXT:    Func strict
// CHECK-NEXT:        Scope %s.2
// CHECK-NEXT:            Decl %d.59 'exports' Parameter : any
// CHECK-NEXT:            Decl %d.60 'arguments' Var Arguments
// CHECK-NEXT:        Func strict
// CHECK-NEXT:            Scope %s.3
// CHECK-NEXT:                Decl %d.61 'unionArr' Var : %array.10
// CHECK-NEXT:                Decl %d.62 'annotatedArr' Var : %array.10
// CHECK-NEXT:                Decl %d.63 'Base' Class : %class_constructor.4
// CHECK-NEXT:                Decl %d.64 'Derived' Class : %class_constructor.8
// CHECK-NEXT:                Decl %d.65 'baseArr' Var : %array.11
// CHECK-NEXT:                Decl %d.66 'numArr' Var : %array.12
// CHECK-NEXT:                Decl %d.67 'strArr' Var : %array.14
// CHECK-NEXT:                Decl %d.68 'arguments' Var Arguments
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.4
// CHECK-NEXT:            Func strict
// CHECK-NEXT:                Scope %s.5
// CHECK-NEXT:                    Decl %d.69 'arguments' Var Arguments

// CHECK:Program Scope %s.1
// CHECK-NEXT:    ExpressionStatement
// CHECK-NEXT:        CallExpression : any
// CHECK-NEXT:            FunctionExpression : %untyped_function.1
// CHECK-NEXT:                Id 'exports' [D:E:%d.59 'exports']
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
// CHECK-NEXT:                                                Id 'undefined' [D:E:%d.25 'undefined'] : void
// CHECK-NEXT:                                            Id 'unionArr' [D:E:%d.61 'unionArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.10
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                            Id 'annotatedArr' [D:E:%d.62 'annotatedArr']
// CHECK-NEXT:                                    ExpressionStatement
// CHECK-NEXT:                                        AssignmentExpression : %array.10
// CHECK-NEXT:                                            Id 'annotatedArr' [D:E:%d.62 'annotatedArr'] : %array.10
// CHECK-NEXT:                                            Id 'unionArr' [D:E:%d.61 'unionArr'] : %array.10
// CHECK-NEXT:                                    ClassDeclaration
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.63 'Base']
// CHECK-NEXT:                                        ClassBody
// CHECK-NEXT:                                    ClassDeclaration
// CHECK-NEXT:                                        Id 'Derived' [D:E:%d.64 'Derived']
// CHECK-NEXT:                                        Id 'Base' [D:E:%d.63 'Base'] : %class_constructor.4
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
// CHECK-NEXT:                                                    Id 'Base' [D:E:%d.63 'Base'] : %class_constructor.4
// CHECK-NEXT:                                                NewExpression : %class.5
// CHECK-NEXT:                                                    Id 'Derived' [D:E:%d.64 'Derived'] : %class_constructor.8
// CHECK-NEXT:                                            Id 'baseArr' [D:E:%d.65 'baseArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.12
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                                NumericLiteral : number
// CHECK-NEXT:                                            Id 'numArr' [D:E:%d.66 'numArr']
// CHECK-NEXT:                                    VariableDeclaration
// CHECK-NEXT:                                        VariableDeclarator
// CHECK-NEXT:                                            ArrayExpression : %array.14
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                StringLiteral : string
// CHECK-NEXT:                                                SpreadElement
// CHECK-NEXT:                                                    Id 'numArr' [D:E:%d.66 'numArr'] : %array.12
// CHECK-NEXT:                                            Id 'strArr' [D:E:%d.67 'strArr']
// CHECK-NEXT:            ObjectExpression : %object.15
