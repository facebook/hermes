/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -dump-source-location -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

class C {
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 7
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 8
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           378,
// CHECK-NEXT:           379
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [

  static a = 1;
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 40,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 40,
// CHECK-NEXT:                   "column": 11
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 1202,
// CHECK-NEXT:                 1203
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 40,
// CHECK-NEXT:                   "column": 14
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 40,
// CHECK-NEXT:                   "column": 15
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 1206,
// CHECK-NEXT:                 1207
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": true,
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 40,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 40,
// CHECK-NEXT:                 "column": 16
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               1195,
// CHECK-NEXT:               1208
// CHECK-NEXT:             ]
// CHECK-NEXT:           },

  static b() {}
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 98,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 98,
// CHECK-NEXT:                   "column": 11
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 3385,
// CHECK-NEXT:                 3386
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [],
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 98,
// CHECK-NEXT:                     "column": 14
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 98,
// CHECK-NEXT:                     "column": 16
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "range": [
// CHECK-NEXT:                   3389,
// CHECK-NEXT:                   3391
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false,
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 98,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 98,
// CHECK-NEXT:                   "column": 16
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 3385,
// CHECK-NEXT:                 3391
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": true,
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 98,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 98,
// CHECK-NEXT:                 "column": 16
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               3378,
// CHECK-NEXT:               3391
// CHECK-NEXT:             ]
// CHECK-NEXT:           }

}
// CHECK-NEXT:         ],
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 9
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 177,
// CHECK-NEXT:             "column": 2
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           380,
// CHECK-NEXT:           6451
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 177,
// CHECK-NEXT:           "column": 2
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         372,
// CHECK-NEXT:         6451
// CHECK-NEXT:       ]
// CHECK-NEXT:     }
// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 177,
// CHECK-NEXT:       "column": 2
// CHECK-NEXT:     }
// CHECK-NEXT:   },
// CHECK-NEXT:   "range": [
// CHECK-NEXT:     372,
// CHECK-NEXT:     6451
// CHECK-NEXT:   ]
// CHECK-NEXT: }
