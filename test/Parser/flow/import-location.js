/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -commonjs -dump-ast -dump-source-location -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

import type Foo from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Foo",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 13
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 16
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 427,
// CHECK-NEXT:                 430
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 16
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               427,
// CHECK-NEXT:               430
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 27
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             436,
// CHECK-NEXT:             441
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "importKind": "type",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 28
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           415,
// CHECK-NEXT:           442
// CHECK-NEXT:         ]
// CHECK-NEXT:       },

import typeof Foo, {bar} from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Foo",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 89,
// CHECK-NEXT:                   "column": 15
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 89,
// CHECK-NEXT:                   "column": 18
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 3100,
// CHECK-NEXT:                 3103
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 89,
// CHECK-NEXT:                 "column": 15
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 89,
// CHECK-NEXT:                 "column": 18
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               3100,
// CHECK-NEXT:               3103
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 89,
// CHECK-NEXT:                   "column": 21
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 89,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 3106,
// CHECK-NEXT:                 3109
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 89,
// CHECK-NEXT:                   "column": 21
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 89,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 3106,
// CHECK-NEXT:                 3109
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 89,
// CHECK-NEXT:                 "column": 21
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 89,
// CHECK-NEXT:                 "column": 24
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               3106,
// CHECK-NEXT:               3109
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 89,
// CHECK-NEXT:               "column": 31
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 89,
// CHECK-NEXT:               "column": 36
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             3116,
// CHECK-NEXT:             3121
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "importKind": "typeof",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 89,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 89,
// CHECK-NEXT:             "column": 37
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           3086,
// CHECK-NEXT:           3122
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
