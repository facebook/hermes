/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL:     "body": [

import type {x} from 'foo';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportSpecifier",
// CHECK-NEXT:           "imported": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "local": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "importKind": "value"
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [],
// CHECK-NEXT:       "importKind": "type"
// CHECK-NEXT:     }
