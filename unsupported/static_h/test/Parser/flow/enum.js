/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

enum E {
  A,
  B,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumStringBody",
// CHECK-NEXT:         "members": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumDefaultedMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumDefaultedMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "explicitType": false,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E of symbol {
  A,
  B,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumSymbolBody",
// CHECK-NEXT:         "members": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumDefaultedMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumDefaultedMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E {
  A = 1,
  B = 2,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumNumberBody",
// CHECK-NEXT:         "members": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumNumberMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             },
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumNumberMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             },
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 2,
// CHECK-NEXT:               "raw": "2"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "explicitType": false,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E {
  A = true,
  B = false,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumBooleanBody",
// CHECK-NEXT:         "members": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumBooleanMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             },
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "BooleanLiteral",
// CHECK-NEXT:               "value": true
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumBooleanMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             },
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "BooleanLiteral",
// CHECK-NEXT:               "value": false
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "explicitType": false,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E {
  A = 'a',
  B = 'b',
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumStringBody",
// CHECK-NEXT:         "members": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumStringMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             },
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "a"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "EnumStringMember",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             },
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "b"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "explicitType": false,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E of number {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumNumberBody",
// CHECK-NEXT:         "members": [],
// CHECK-NEXT:         "explicitType": true,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E of string {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumStringBody",
// CHECK-NEXT:         "members": [],
// CHECK-NEXT:         "explicitType": true,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

enum E of boolean {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "EnumDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EnumBooleanBody",
// CHECK-NEXT:         "members": [],
// CHECK-NEXT:         "explicitType": true,
// CHECK-NEXT:         "hasUnknownMembers": false
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
