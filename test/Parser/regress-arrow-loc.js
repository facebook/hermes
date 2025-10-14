/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -dump-source-location=loc --pretty-json %s | %FileCheckOrRegen %s --match-full-lines

// Ensure the closing parenthesis is included in the range.
x => (y);

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "ArrowFunctionExpression",
// CHECK-NEXT:        "params": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "x",
// CHECK-NEXT:            "loc": {
// CHECK-NEXT:              "start": {
// CHECK-NEXT:                "line": 11,
// CHECK-NEXT:                "column": 1
// CHECK-NEXT:              },
// CHECK-NEXT:              "end": {
// CHECK-NEXT:                "line": 11,
// CHECK-NEXT:                "column": 2
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ],
// CHECK-NEXT:        "body": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "y",
// CHECK-NEXT:          "loc": {
// CHECK-NEXT:            "start": {
// CHECK-NEXT:              "line": 11,
// CHECK-NEXT:              "column": 7
// CHECK-NEXT:            },
// CHECK-NEXT:            "end": {
// CHECK-NEXT:              "line": 11,
// CHECK-NEXT:              "column": 8
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        },
// CHECK-NEXT:        "expression": true,
// CHECK-NEXT:        "async": false,
// CHECK-NEXT:        "loc": {
// CHECK-NEXT:          "start": {
// CHECK-NEXT:            "line": 11,
// CHECK-NEXT:            "column": 1
// CHECK-NEXT:          },
// CHECK-NEXT:          "end": {
// CHECK-NEXT:            "line": 11,
// CHECK-NEXT:            "column": 9
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null,
// CHECK-NEXT:      "loc": {
// CHECK-NEXT:        "start": {
// CHECK-NEXT:          "line": 11,
// CHECK-NEXT:          "column": 1
// CHECK-NEXT:        },
// CHECK-NEXT:        "end": {
// CHECK-NEXT:          "line": 11,
// CHECK-NEXT:          "column": 10
// CHECK-NEXT:        }
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ],
// CHECK-NEXT:  "loc": {
// CHECK-NEXT:    "start": {
// CHECK-NEXT:      "line": 11,
// CHECK-NEXT:      "column": 1
// CHECK-NEXT:    },
// CHECK-NEXT:    "end": {
// CHECK-NEXT:      "line": 11,
// CHECK-NEXT:      "column": 10
// CHECK-NEXT:    }
// CHECK-NEXT:  }
// CHECK-NEXT:}
