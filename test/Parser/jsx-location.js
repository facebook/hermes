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

<a>{b}</a>;
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
// CHECK-NEXT:                 "line": 161,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 161,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               5882,
// CHECK-NEXT:               5883
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 161,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 161,
// CHECK-NEXT:               "column": 4
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             5881,
// CHECK-NEXT:             5884
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
// CHECK-NEXT:                   "line": 161,
// CHECK-NEXT:                   "column": 5
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 161,
// CHECK-NEXT:                   "column": 6
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 5885,
// CHECK-NEXT:                 5886
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 161,
// CHECK-NEXT:                 "column": 4
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 161,
// CHECK-NEXT:                 "column": 7
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               5884,
// CHECK-NEXT:               5887
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
// CHECK-NEXT:                 "line": 161,
// CHECK-NEXT:                 "column": 9
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 161,
// CHECK-NEXT:                 "column": 10
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               5889,
// CHECK-NEXT:               5890
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 161,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 161,
// CHECK-NEXT:               "column": 11
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             5887,
// CHECK-NEXT:             5891
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 161,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 161,
// CHECK-NEXT:             "column": 11
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           5881,
// CHECK-NEXT:           5891
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 161,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 161,
// CHECK-NEXT:           "column": 12
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         5881,
// CHECK-NEXT:         5892
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

<a {...b} />;
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
// CHECK-NEXT:                 "line": 307,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 307,
// CHECK-NEXT:                 "column": 3
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               11098,
// CHECK-NEXT:               11099
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
// CHECK-NEXT:                     "line": 307,
// CHECK-NEXT:                     "column": 8
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 307,
// CHECK-NEXT:                     "column": 9
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "range": [
// CHECK-NEXT:                   11104,
// CHECK-NEXT:                   11105
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 307,
// CHECK-NEXT:                   "column": 4
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 307,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 11100,
// CHECK-NEXT:                 11106
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "selfClosing": true,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 307,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 307,
// CHECK-NEXT:               "column": 13
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             11097,
// CHECK-NEXT:             11109
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [],
// CHECK-NEXT:         "closingElement": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 307,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 307,
// CHECK-NEXT:             "column": 13
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           11097,
// CHECK-NEXT:           11109
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 307,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 307,
// CHECK-NEXT:           "column": 14
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         11097,
// CHECK-NEXT:         11110
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

<div>{}</div>;
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
// CHECK-NEXT:                 "line": 419,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 419,
// CHECK-NEXT:                 "column": 5
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               15156,
// CHECK-NEXT:               15159
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 419,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 419,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             15155,
// CHECK-NEXT:             15160
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "JSXExpressionContainer",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "JSXEmptyExpression",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 419,
// CHECK-NEXT:                   "column": 7
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 419,
// CHECK-NEXT:                   "column": 7
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 15161,
// CHECK-NEXT:                 15161
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 419,
// CHECK-NEXT:                 "column": 6
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 419,
// CHECK-NEXT:                 "column": 8
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               15160,
// CHECK-NEXT:               15162
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
// CHECK-NEXT:                 "line": 419,
// CHECK-NEXT:                 "column": 10
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 419,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               15164,
// CHECK-NEXT:               15167
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 419,
// CHECK-NEXT:               "column": 8
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 419,
// CHECK-NEXT:               "column": 14
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             15162,
// CHECK-NEXT:             15168
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 419,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 419,
// CHECK-NEXT:             "column": 14
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           15155,
// CHECK-NEXT:           15168
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 419,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 419,
// CHECK-NEXT:           "column": 15
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         15155,
// CHECK-NEXT:         15169
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

<div>{   }</div>;
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
// CHECK-NEXT:                 "line": 564,
// CHECK-NEXT:                 "column": 2
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 564,
// CHECK-NEXT:                 "column": 5
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               20362,
// CHECK-NEXT:               20365
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 564,
// CHECK-NEXT:               "column": 1
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 564,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             20361,
// CHECK-NEXT:             20366
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "JSXExpressionContainer",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "JSXEmptyExpression",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 564,
// CHECK-NEXT:                   "column": 7
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 564,
// CHECK-NEXT:                   "column": 10
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 20367,
// CHECK-NEXT:                 20370
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 564,
// CHECK-NEXT:                 "column": 6
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 564,
// CHECK-NEXT:                 "column": 11
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               20366,
// CHECK-NEXT:               20371
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
// CHECK-NEXT:                 "line": 564,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 564,
// CHECK-NEXT:                 "column": 16
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               20373,
// CHECK-NEXT:               20376
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 564,
// CHECK-NEXT:               "column": 11
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 564,
// CHECK-NEXT:               "column": 17
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             20371,
// CHECK-NEXT:             20377
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 564,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 564,
// CHECK-NEXT:             "column": 17
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           20361,
// CHECK-NEXT:           20377
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 564,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 564,
// CHECK-NEXT:           "column": 18
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         20361,
// CHECK-NEXT:         20378
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 564,
// CHECK-NEXT:       "column": 18
// CHECK-NEXT:     }
// CHECK-NEXT:   },
// CHECK-NEXT:   "range": [
// CHECK-NEXT:     383,
// CHECK-NEXT:     20378
// CHECK-NEXT:   ]
// CHECK-NEXT: }
