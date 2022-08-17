/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck --match-full-lines %s --check-prefix=LOC
// RUN: %hermes -dump-ast -dump-source-location=range -pretty-json %s | %FileCheck --match-full-lines %s --check-prefix=RANGE

1;

// LOC-LABEL: {
// LOC-NEXT:   "type": "Program",
// LOC-NEXT:   "body": [
// LOC-NEXT:     {
// LOC-NEXT:       "type": "ExpressionStatement",
// LOC-NEXT:       "expression": {
// LOC-NEXT:         "type": "NumericLiteral",
// LOC-NEXT:         "value": 1,
// LOC-NEXT:         "raw": "1",
// LOC-NEXT:         "loc": {
// LOC-NEXT:           "start": {
// LOC-NEXT:             "line": 11,
// LOC-NEXT:             "column": 1
// LOC-NEXT:           },
// LOC-NEXT:           "end": {
// LOC-NEXT:             "line": 11,
// LOC-NEXT:             "column": 2
// LOC-NEXT:           }
// LOC-NEXT:         }
// LOC-NEXT:       },
// LOC-NEXT:       "directive": null,
// LOC-NEXT:       "loc": {
// LOC-NEXT:         "start": {
// LOC-NEXT:           "line": 11,
// LOC-NEXT:           "column": 1
// LOC-NEXT:         },
// LOC-NEXT:         "end": {
// LOC-NEXT:           "line": 11,
// LOC-NEXT:           "column": 3
// LOC-NEXT:         }
// LOC-NEXT:       }
// LOC-NEXT:     }
// LOC-NEXT:   ],
// LOC-NEXT:   "loc": {
// LOC-NEXT:     "start": {
// LOC-NEXT:       "line": 11,
// LOC-NEXT:       "column": 1
// LOC-NEXT:     },
// LOC-NEXT:     "end": {
// LOC-NEXT:       "line": 11,
// LOC-NEXT:       "column": 3
// LOC-NEXT:     }
// LOC-NEXT:   }
// LOC-NEXT: }

// RANGE-LABEL: {
// RANGE-NEXT:   "type": "Program",
// RANGE-NEXT:   "body": [
// RANGE-NEXT:     {
// RANGE-NEXT:       "type": "ExpressionStatement",
// RANGE-NEXT:       "expression": {
// RANGE-NEXT:         "type": "NumericLiteral",
// RANGE-NEXT:         "value": 1,
// RANGE-NEXT:         "raw": "1",
// RANGE-NEXT:         "range": [
// RANGE-NEXT:           441,
// RANGE-NEXT:           442
// RANGE-NEXT:         ]
// RANGE-NEXT:       },
// RANGE-NEXT:       "directive": null,
// RANGE-NEXT:       "range": [
// RANGE-NEXT:         441,
// RANGE-NEXT:         443
// RANGE-NEXT:       ]
// RANGE-NEXT:     }
// RANGE-NEXT:   ],
// RANGE-NEXT:   "range": [
// RANGE-NEXT:     441,
// RANGE-NEXT:     443
// RANGE-NEXT:   ]
// RANGE-NEXT: }
