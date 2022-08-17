/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -commonjs -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

import type Foo from 'foo';

import typeof Foo, {bar} from 'foo';

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Foo",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 13
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 16
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 16
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 27
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "type",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 28
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Foo",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 15
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 18
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 15
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 18
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 21
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 21
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 21
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 24
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 31
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 36
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "typeof",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 37
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
