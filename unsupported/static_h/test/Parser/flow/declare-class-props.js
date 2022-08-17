/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -parse-flow -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

(class {
  declare x;
  declare static y;
});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "superClass": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": true
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "y"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": true,
// CHECK-NEXT:               "declare": true
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
