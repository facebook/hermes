/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

function foo() {
  return import.meta;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ReturnStatement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "MetaProperty",
// CHECK-NEXT:               "meta": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "import"
// CHECK-NEXT:               },
// CHECK-NEXT:               "property": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "meta"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

function bar() {
  return import.meta.prop;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "bar"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ReturnStatement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "MemberExpression",
// CHECK-NEXT:               "object": {
// CHECK-NEXT:                 "type": "MetaProperty",
// CHECK-NEXT:                 "meta": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "import"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "property": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "meta"
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "property": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "prop"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
