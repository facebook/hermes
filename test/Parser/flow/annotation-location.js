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

(x: number);

function foo(x: number): string {}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TypeCastExpression",
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 2
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 3
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 5
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 11
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 3
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 11
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 2
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 11
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 13
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 10
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 13
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 17
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 23
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
// CHECK-NEXT:                 "column": 23
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 14
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 23
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [],
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 33
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 35
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "StringTypeAnnotation",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 26
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 32
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 32
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 35
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
// CHECK-NEXT:       "line": 16,
// CHECK-NEXT:       "column": 35
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT: }
