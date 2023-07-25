/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-jsx -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

<Component<string> />;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "Component"
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": true,
// CHECK-NEXT:           "typeArguments": {
// CHECK-NEXT:             "type": "TypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [],
// CHECK-NEXT:         "closingElement": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
