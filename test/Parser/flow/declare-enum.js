/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheckOrRegen %s --match-full-lines

declare enum A { B };

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "DeclareEnum",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "A"
// CHECK-NEXT:      },
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "EnumStringBody",
// CHECK-NEXT:        "members": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "EnumDefaultedMember",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "B"
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ],
// CHECK-NEXT:        "explicitType": false,
// CHECK-NEXT:        "hasUnknownMembers": false
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "EmptyStatement"
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
