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

class C {
  a<T>() {}
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 7
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 8
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 15,
// CHECK-NEXT:                   "column": 3
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 15,
// CHECK-NEXT:                   "column": 4
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [],
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 15,
// CHECK-NEXT:                     "column": 10
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 15,
// CHECK-NEXT:                     "column": 12
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TypeParameterDeclaration",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "TypeParameter",
// CHECK-NEXT:                     "name": "T",
// CHECK-NEXT:                     "bound": null,
// CHECK-NEXT:                     "variance": null,
// CHECK-NEXT:                     "default": null,
// CHECK-NEXT:                     "loc": {
// CHECK-NEXT:                       "start": {
// CHECK-NEXT:                         "line": 15,
// CHECK-NEXT:                         "column": 5
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "end": {
// CHECK-NEXT:                         "line": 15,
// CHECK-NEXT:                         "column": 6
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 15,
// CHECK-NEXT:                     "column": 4
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 15,
// CHECK-NEXT:                     "column": 7
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false,
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 15,
// CHECK-NEXT:                   "column": 4
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 15,
// CHECK-NEXT:                   "column": 12
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 15,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 15,
// CHECK-NEXT:                 "column": 12
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }

// CHECK-NEXT:         ],
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 9
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 2
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 2
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
// CHECK-NEXT:       "column": 2
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT: }
