/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-transformed-ast %s | %FileCheck --match-full-lines %s

// Test lowering of fastarray push

var x: number[];
x.push(10, 20);

// CHECK-LABEL:                     "name": "?fastArrayPush"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "computed": false
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "arguments": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "x"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 10,
// CHECK-NEXT:                      "raw": "10"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 20,
// CHECK-NEXT:                      "raw": "20"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "directive": null
