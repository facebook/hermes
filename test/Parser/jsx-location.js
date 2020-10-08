/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-jsx -dump-ast -dump-source-location -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

<a b={c} />;

<a>{b}</a>;

<a {...b} />;

<div>{}</div>;

<div>{   }</div>;

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "a",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 14,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               384,
// CHECK-NEXT:               385
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "JSXAttribute",
// CHECK-NEXT:               "name": {
// CHECK-NEXT:                 "type": "JSXIdentifier",
// CHECK-NEXT:                 "name": "b",
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 4
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 5
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "range": [
// CHECK-NEXT:                   386,
// CHECK-NEXT:                   387
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "JSXExpressionContainer",
// CHECK-NEXT:                 "expression": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "c",
// CHECK-NEXT:                   "loc": {
// CHECK-NEXT:                     "start": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 7
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "end": {
// CHECK-NEXT:                       "line": 14,
// CHECK-NEXT:                       "column": 8
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "range": [
// CHECK-NEXT:                     389,
// CHECK-NEXT:                     390
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 6
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 14,
// CHECK-NEXT:                     "column": 9
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "range": [
// CHECK-NEXT:                   388,
// CHECK-NEXT:                   391
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 4
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 14,
// CHECK-NEXT:                   "column": 9
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 386,
// CHECK-NEXT:                 391
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "selfClosing": true,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 12
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             383,
// CHECK-NEXT:             394
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [],
// CHECK-NEXT:         "closingElement": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 12
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           383,
// CHECK-NEXT:           394
// CHECK-NEXT:         ]
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
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         383,
// CHECK-NEXT:         395
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "a",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               398,
// CHECK-NEXT:               399
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 4
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             397,
// CHECK-NEXT:             400
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "JSXExpressionContainer",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 5
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 16,
// CHECK-NEXT:                   "column": 6
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 401,
// CHECK-NEXT:                 402
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 4
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 7
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               400,
// CHECK-NEXT:               403
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "closingElement": {
// CHECK-NEXT:           "type": "JSXClosingElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "a",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 9
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 16,
// CHECK-NEXT:                 "column": 10
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               405,
// CHECK-NEXT:               406
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 11
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             403,
// CHECK-NEXT:             407
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 11
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           397,
// CHECK-NEXT:           407
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 12
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         397,
// CHECK-NEXT:         408
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "a",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 18,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 18,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               411,
// CHECK-NEXT:               412
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "JSXSpreadAttribute",
// CHECK-NEXT:               "argument": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "b",
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 18,
// CHECK-NEXT:                     "column": 8
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 18,
// CHECK-NEXT:                     "column": 9
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "range": [
// CHECK-NEXT:                   417,
// CHECK-NEXT:                   418
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 18,
// CHECK-NEXT:                   "column": 4
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 18,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 413,
// CHECK-NEXT:                 419
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "selfClosing": true,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 18,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 18,
// CHECK-NEXT:               "column": 13
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             410,
// CHECK-NEXT:             422
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [],
// CHECK-NEXT:         "closingElement": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 18,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 18,
// CHECK-NEXT:             "column": 13
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           410,
// CHECK-NEXT:           422
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 18,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 18,
// CHECK-NEXT:           "column": 14
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         410,
// CHECK-NEXT:         423
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "div",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 20,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 20,
// CHECK-NEXT:                 "column": 5
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               426,
// CHECK-NEXT:               429
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             425,
// CHECK-NEXT:             430
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "JSXExpressionContainer",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "JSXEmptyExpression",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 20,
// CHECK-NEXT:                   "column": 7
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 20,
// CHECK-NEXT:                   "column": 7
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 431,
// CHECK-NEXT:                 431
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 20,
// CHECK-NEXT:                 "column": 6
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 20,
// CHECK-NEXT:                 "column": 8
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               430,
// CHECK-NEXT:               432
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "closingElement": {
// CHECK-NEXT:           "type": "JSXClosingElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "div",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 20,
// CHECK-NEXT:                 "column": 10
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 20,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               434,
// CHECK-NEXT:               437
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 8
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 14
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             432,
// CHECK-NEXT:             438
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 20,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 20,
// CHECK-NEXT:             "column": 14
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           425,
// CHECK-NEXT:           438
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 20,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 20,
// CHECK-NEXT:           "column": 15
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         425,
// CHECK-NEXT:         439
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "div",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 22,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 22,
// CHECK-NEXT:                 "column": 5
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               442,
// CHECK-NEXT:               445
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             441,
// CHECK-NEXT:             446
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "JSXExpressionContainer",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "JSXEmptyExpression",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 22,
// CHECK-NEXT:                   "column": 7
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 22,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 447,
// CHECK-NEXT:                 450
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 22,
// CHECK-NEXT:                 "column": 6
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 22,
// CHECK-NEXT:                 "column": 11
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               446,
// CHECK-NEXT:               451
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "closingElement": {
// CHECK-NEXT:           "type": "JSXClosingElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "div",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 22,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 22,
// CHECK-NEXT:                 "column": 16
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               453,
// CHECK-NEXT:               456
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 11
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 17
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             451,
// CHECK-NEXT:             457
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 22,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 22,
// CHECK-NEXT:             "column": 17
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           441,
// CHECK-NEXT:           457
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 22,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 22,
// CHECK-NEXT:           "column": 18
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         441,
// CHECK-NEXT:         458
// CHECK-NEXT:       ]

// CHECK-NEXT:     }
// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 22,
// CHECK-NEXT:       "column": 18
// CHECK-NEXT:     }
// CHECK-NEXT:   },
// CHECK-NEXT:   "range": [
// CHECK-NEXT:     383,
// CHECK-NEXT:     458
// CHECK-NEXT:   ]
// CHECK-NEXT: }
