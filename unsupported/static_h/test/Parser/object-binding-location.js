/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

const {[a]: b} = c;

({a = b} = c);

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "c",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 18
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 19
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "ObjectPattern",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "a",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 9
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 10
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "b",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 13
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 14
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": true,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": false,
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 8
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 14
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 7
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 15
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 19
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 20
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "AssignmentExpression",
// CHECK-NEXT:         "operator": "=",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "ObjectPattern",
// CHECK-NEXT:           "properties": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Property",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a",
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 16,
// CHECK-NEXT:                     "column": 3
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 16,
// CHECK-NEXT:                     "column": 4
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "AssignmentPattern",
// CHECK-NEXT:                 "left": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "a",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 16,
// CHECK-NEXT:                       "column": 3
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 16,
// CHECK-NEXT:                       "column": 4
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "right": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "b",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 16,
// CHECK-NEXT:                       "column": 7
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 16,
// CHECK-NEXT:                       "column": 8
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 16,
// CHECK-NEXT:                     "column": 3
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 16,
// CHECK-NEXT:                     "column": 8
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "init",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "method": false,
// CHECK-NEXT:               "shorthand": true,
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 3
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 8
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 2
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 9
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 12
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 13
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 2
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 13
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 15
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
// CHECK-NEXT:       "column": 15
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT: }
