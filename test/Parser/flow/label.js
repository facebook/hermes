/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines
// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

label:
for (;;) {
  break label;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "LabeledStatement",
// CHECK-NEXT:       "label": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "label"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ForStatement",
// CHECK-NEXT:         "init": null,
// CHECK-NEXT:         "test": null,
// CHECK-NEXT:         "update": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "BreakStatement",
// CHECK-NEXT:               "label": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "label"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
