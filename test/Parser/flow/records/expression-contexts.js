/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

function test() {
  return R {a: 1, b: 2};
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "test"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ReturnStatement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "RecordExpression",
// CHECK-NEXT:               "recordConstructor": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "R"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeArguments": null,
// CHECK-NEXT:               "properties": {
// CHECK-NEXT:                 "type": "RecordExpressionProperties",
// CHECK-NEXT:                 "properties": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Property",
// CHECK-NEXT:                     "key": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "a"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "value": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "kind": "init",
// CHECK-NEXT:                     "computed": false,
// CHECK-NEXT:                     "method": false,
// CHECK-NEXT:                     "shorthand": false
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Property",
// CHECK-NEXT:                     "key": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "b"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "value": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 2,
// CHECK-NEXT:                       "raw": "2"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "kind": "init",
// CHECK-NEXT:                     "computed": false,
// CHECK-NEXT:                     "method": false,
// CHECK-NEXT:                     "shorthand": false
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

const x = foo(R {c: 3});

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "CallExpression",
// CHECK-NEXT:             "callee": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "arguments": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "RecordExpression",
// CHECK-NEXT:                 "recordConstructor": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "R"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeArguments": null,
// CHECK-NEXT:                 "properties": {
// CHECK-NEXT:                   "type": "RecordExpressionProperties",
// CHECK-NEXT:                   "properties": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "Property",
// CHECK-NEXT:                       "key": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "c"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "value": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 3,
// CHECK-NEXT:                         "raw": "3"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "kind": "init",
// CHECK-NEXT:                       "computed": false,
// CHECK-NEXT:                       "method": false,
// CHECK-NEXT:                       "shorthand": false
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

const y = [R {d: 4}, R {e: 5}];

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ArrayExpression",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "RecordExpression",
// CHECK-NEXT:                 "recordConstructor": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "R"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeArguments": null,
// CHECK-NEXT:                 "properties": {
// CHECK-NEXT:                   "type": "RecordExpressionProperties",
// CHECK-NEXT:                   "properties": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "Property",
// CHECK-NEXT:                       "key": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "d"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "value": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 4,
// CHECK-NEXT:                         "raw": "4"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "kind": "init",
// CHECK-NEXT:                       "computed": false,
// CHECK-NEXT:                       "method": false,
// CHECK-NEXT:                       "shorthand": false
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "RecordExpression",
// CHECK-NEXT:                 "recordConstructor": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "R"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeArguments": null,
// CHECK-NEXT:                 "properties": {
// CHECK-NEXT:                   "type": "RecordExpressionProperties",
// CHECK-NEXT:                   "properties": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "Property",
// CHECK-NEXT:                       "key": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "e"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "value": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 5,
// CHECK-NEXT:                         "raw": "5"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "kind": "init",
// CHECK-NEXT:                       "computed": false,
// CHECK-NEXT:                       "method": false,
// CHECK-NEXT:                       "shorthand": false
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "trailingComma": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "y"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

const z = {nested: R {f: 6}};

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ObjectExpression",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "nested"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "RecordExpression",
// CHECK-NEXT:                   "recordConstructor": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "R"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeArguments": null,
// CHECK-NEXT:                   "properties": {
// CHECK-NEXT:                     "type": "RecordExpressionProperties",
// CHECK-NEXT:                     "properties": [
// CHECK-NEXT:                       {
// CHECK-NEXT:                         "type": "Property",
// CHECK-NEXT:                         "key": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "f"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "value": {
// CHECK-NEXT:                           "type": "NumericLiteral",
// CHECK-NEXT:                           "value": 6,
// CHECK-NEXT:                           "raw": "6"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "kind": "init",
// CHECK-NEXT:                         "computed": false,
// CHECK-NEXT:                         "method": false,
// CHECK-NEXT:                         "shorthand": false
// CHECK-NEXT:                       }
// CHECK-NEXT:                     ]
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": false
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "z"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

class C extends not_parsed_as_a_record_expression {}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "not_parsed_as_a_record_expression"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
