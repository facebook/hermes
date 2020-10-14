/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-jsx -parse-flow -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

<T>(a) => {};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 5
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 6
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": [],
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 11
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 13
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "T",
// CHECK-NEXT:               "bound": null,
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": null,
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 2
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 3
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 4
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": false,
// CHECK-NEXT:         "async": false,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 4
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 13
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 4
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 14
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 14
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT: }
