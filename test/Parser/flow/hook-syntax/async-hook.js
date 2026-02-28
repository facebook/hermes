/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

async hook useFoo1() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "HookDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "useFoo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

export default async hook useFoo2() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExportDefaultDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "HookDeclaration",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "useFoo2"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       }
// CHECK-NEXT:     },

export async hook useFoo3() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExportNamedDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "HookDeclaration",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "useFoo3"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "exportKind": "value"
// CHECK-NEXT:     },

async hook useFoo4(foo: string): Promise<number> {
  return await Promise.resolve(42);
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "HookDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "useFoo4"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ReturnStatement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "AwaitExpression",
// CHECK-NEXT:               "argument": {
// CHECK-NEXT:                 "type": "CallExpression",
// CHECK-NEXT:                 "callee": {
// CHECK-NEXT:                   "type": "MemberExpression",
// CHECK-NEXT:                   "object": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "Promise"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "property": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "resolve"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "computed": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "arguments": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 42,
// CHECK-NEXT:                     "raw": "42"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Promise"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

// Ensure `async\nhook useFoo() {}` is not parsed as an async hook
// declaration because of the line terminator after `async`.
async
hook useFoo5() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "async"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "HookDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "useFoo5"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
