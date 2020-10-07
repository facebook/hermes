/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -dump-source-location -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

const {[a]: b} = c;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "c",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 18
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 19
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               389,
// CHECK-NEXT:               390
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "ObjectPattern",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "a",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 9
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 10
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "range": [
// CHECK-NEXT:                     380,
// CHECK-NEXT:                     381
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "b",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 13
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 14
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "range": [
// CHECK-NEXT:                     384,
// CHECK-NEXT:                     385
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": true,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": false,
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 8
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 14
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "range": [
// CHECK-NEXT:                   379,
// CHECK-NEXT:                   385
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 7
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 15
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               378,
// CHECK-NEXT:               386
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 19
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             378,
// CHECK-NEXT:             390
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 20
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         372,
// CHECK-NEXT:         391
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 20
// CHECK-NEXT:     }
// CHECK-NEXT:   },
// CHECK-NEXT:   "range": [
// CHECK-NEXT:     372,
// CHECK-NEXT:     391
// CHECK-NEXT:   ]
// CHECK-NEXT: }
