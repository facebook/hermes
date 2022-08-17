/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

//CHECK: {
//CHECK-NEXT:    "type": "Program",
//CHECK-NEXT:    "body": [

do; while (1) 2

//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "DoWhileStatement",
//CHECK-NEXT:      "body": {
//CHECK-NEXT:        "type": "EmptyStatement"
//CHECK-NEXT:      },
//CHECK-NEXT:      "test": {
//CHECK-NEXT:        "type": "NumericLiteral",
//CHECK-NEXT:        "value": 1,
//CHECK-NEXT:        "raw": "1"
//CHECK-NEXT:      }
//CHECK-NEXT:    },
//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "ExpressionStatement",
//CHECK-NEXT:      "expression": {
//CHECK-NEXT:        "type": "NumericLiteral",
//CHECK-NEXT:        "value": 2,
//CHECK-NEXT:        "raw": "2"
//CHECK-NEXT:      },
//CHECK-NEXT:      "directive": null
//CHECK-NEXT:    }

//CHECK-NEXT:  ]
//CHECK-NEXT:}
