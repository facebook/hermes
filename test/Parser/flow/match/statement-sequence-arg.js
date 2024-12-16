/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-match -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

match (a, b) {}

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "MatchStatement",
// CHECK-NEXT:      "argument": {
// CHECK-NEXT:        "type": "SequenceExpression",
// CHECK-NEXT:        "expressions": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "a"
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "b"
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "cases": []
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
