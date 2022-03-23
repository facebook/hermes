/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

function foo(a, b?, c: string, d?: string) {}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 10
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 13
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 14
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 15
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "b",
// CHECK-NEXT:           "optional": true,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 17
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 19
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "StringTypeAnnotation",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 30
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 22
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 30
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 30
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "d",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "StringTypeAnnotation",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 36
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 42
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 34
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 42
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "optional": true,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 32
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 42
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [],
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 44
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 46
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 46
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 46
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT: }
