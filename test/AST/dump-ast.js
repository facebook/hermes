// RUN: %hermes -dump-ast %s | %FileCheck --match-full-lines %s
// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-PRETTY
// RUN: %hermes -dump-ast -dump-source-location -pretty-json %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-SOURCE-LOC

function foo() {
  return Math.random();
}

switch (foo()) {
  case 3:
    print('fizz');
    break;
  case 5:
    print('buzz');
    break;
  default:
    print(foo());
}

// CHECK: {"type":"File","program":{"type":"Program","body":[{"type":"FunctionDeclaration","id":{"type":"Identifier","name":"foo","typeAnnotation":null},"params":[],"body":{"type":"BlockStatement","body":[{"type":"ReturnStatement","argument":{"type":"CallExpression","callee":{"type":"MemberExpression","object":{"type":"Identifier","name":"Math","typeAnnotation":null},"property":{"type":"Identifier","name":"random","typeAnnotation":null},"computed":false},"arguments":[]}}]},"returnType":null,"generator":false},{"type":"SwitchStatement","discriminant":{"type":"CallExpression","callee":{"type":"Identifier","name":"foo","typeAnnotation":null},"arguments":[]},"cases":[{"type":"SwitchCase","test":{"type":"NumericLiteral","value":3},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print","typeAnnotation":null},"arguments":[{"type":"StringLiteral","value":"fizz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":{"type":"NumericLiteral","value":5},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print","typeAnnotation":null},"arguments":[{"type":"StringLiteral","value":"buzz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":null,"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print","typeAnnotation":null},"arguments":[{"type":"CallExpression","callee":{"type":"Identifier","name":"foo","typeAnnotation":null},"arguments":[]}]},"directive":null}]}]}]}}

// CHECK-PRETTY: {
// CHECK-PRETTY:   "type": "File",
// CHECK-PRETTY:   "program": {
// CHECK-PRETTY:     "type": "Program",
// CHECK-PRETTY:     "body": [
// CHECK-PRETTY:       {
// CHECK-PRETTY:         "type": "FunctionDeclaration",
// CHECK-PRETTY:         "id": {
// CHECK-PRETTY:           "type": "Identifier",
// CHECK-PRETTY:           "name": "foo",
// CHECK-PRETTY:           "typeAnnotation": null
// CHECK-PRETTY:         },
// CHECK-PRETTY:         "params": [],
// CHECK-PRETTY:         "body": {
// CHECK-PRETTY:           "type": "BlockStatement",
// CHECK-PRETTY:           "body": [
// CHECK-PRETTY:             {
// CHECK-PRETTY:               "type": "ReturnStatement",
// CHECK-PRETTY:               "argument": {
// CHECK-PRETTY:                 "type": "CallExpression",
// CHECK-PRETTY:                 "callee": {
// CHECK-PRETTY:                   "type": "MemberExpression",
// CHECK-PRETTY:                   "object": {
// CHECK-PRETTY:                     "type": "Identifier",
// CHECK-PRETTY:                     "name": "Math",
// CHECK-PRETTY:                     "typeAnnotation": null
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "property": {
// CHECK-PRETTY:                     "type": "Identifier",
// CHECK-PRETTY:                     "name": "random",
// CHECK-PRETTY:                     "typeAnnotation": null
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "computed": false
// CHECK-PRETTY:                 },
// CHECK-PRETTY:                 "arguments": []
// CHECK-PRETTY:               }
// CHECK-PRETTY:             }
// CHECK-PRETTY:           ]
// CHECK-PRETTY:         },
// CHECK-PRETTY:         "returnType": null,
// CHECK-PRETTY:         "generator": false
// CHECK-PRETTY:       },
// CHECK-PRETTY:       {
// CHECK-PRETTY:         "type": "SwitchStatement",
// CHECK-PRETTY:         "discriminant": {
// CHECK-PRETTY:           "type": "CallExpression",
// CHECK-PRETTY:           "callee": {
// CHECK-PRETTY:             "type": "Identifier",
// CHECK-PRETTY:             "name": "foo",
// CHECK-PRETTY:             "typeAnnotation": null
// CHECK-PRETTY:           },
// CHECK-PRETTY:           "arguments": []
// CHECK-PRETTY:         },
// CHECK-PRETTY:         "cases": [
// CHECK-PRETTY:           {
// CHECK-PRETTY:             "type": "SwitchCase",
// CHECK-PRETTY:             "test": {
// CHECK-PRETTY:               "type": "NumericLiteral",
// CHECK-PRETTY:               "value": 3
// CHECK-PRETTY:             },
// CHECK-PRETTY:             "consequent": [
// CHECK-PRETTY:               {
// CHECK-PRETTY:                 "type": "ExpressionStatement",
// CHECK-PRETTY:                 "expression": {
// CHECK-PRETTY:                   "type": "CallExpression",
// CHECK-PRETTY:                   "callee": {
// CHECK-PRETTY:                     "type": "Identifier",
// CHECK-PRETTY:                     "name": "print",
// CHECK-PRETTY:                     "typeAnnotation": null
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "arguments": [
// CHECK-PRETTY:                     {
// CHECK-PRETTY:                       "type": "StringLiteral",
// CHECK-PRETTY:                       "value": "fizz"
// CHECK-PRETTY:                     }
// CHECK-PRETTY:                   ]
// CHECK-PRETTY:                 },
// CHECK-PRETTY:                 "directive": null
// CHECK-PRETTY:               },
// CHECK-PRETTY:               {
// CHECK-PRETTY:                 "type": "BreakStatement",
// CHECK-PRETTY:                 "label": null
// CHECK-PRETTY:               }
// CHECK-PRETTY:             ]
// CHECK-PRETTY:           },
// CHECK-PRETTY:           {
// CHECK-PRETTY:             "type": "SwitchCase",
// CHECK-PRETTY:             "test": {
// CHECK-PRETTY:               "type": "NumericLiteral",
// CHECK-PRETTY:               "value": 5
// CHECK-PRETTY:             },
// CHECK-PRETTY:             "consequent": [
// CHECK-PRETTY:               {
// CHECK-PRETTY:                 "type": "ExpressionStatement",
// CHECK-PRETTY:                 "expression": {
// CHECK-PRETTY:                   "type": "CallExpression",
// CHECK-PRETTY:                   "callee": {
// CHECK-PRETTY:                     "type": "Identifier",
// CHECK-PRETTY:                     "name": "print",
// CHECK-PRETTY:                     "typeAnnotation": null
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "arguments": [
// CHECK-PRETTY:                     {
// CHECK-PRETTY:                       "type": "StringLiteral",
// CHECK-PRETTY:                       "value": "buzz"
// CHECK-PRETTY:                     }
// CHECK-PRETTY:                   ]
// CHECK-PRETTY:                 },
// CHECK-PRETTY:                 "directive": null
// CHECK-PRETTY:               },
// CHECK-PRETTY:               {
// CHECK-PRETTY:                 "type": "BreakStatement",
// CHECK-PRETTY:                 "label": null
// CHECK-PRETTY:               }
// CHECK-PRETTY:             ]
// CHECK-PRETTY:           },
// CHECK-PRETTY:           {
// CHECK-PRETTY:             "type": "SwitchCase",
// CHECK-PRETTY:             "test": null,
// CHECK-PRETTY:             "consequent": [
// CHECK-PRETTY:               {
// CHECK-PRETTY:                 "type": "ExpressionStatement",
// CHECK-PRETTY:                 "expression": {
// CHECK-PRETTY:                   "type": "CallExpression",
// CHECK-PRETTY:                   "callee": {
// CHECK-PRETTY:                     "type": "Identifier",
// CHECK-PRETTY:                     "name": "print",
// CHECK-PRETTY:                     "typeAnnotation": null
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "arguments": [
// CHECK-PRETTY:                     {
// CHECK-PRETTY:                       "type": "CallExpression",
// CHECK-PRETTY:                       "callee": {
// CHECK-PRETTY:                         "type": "Identifier",
// CHECK-PRETTY:                         "name": "foo",
// CHECK-PRETTY:                         "typeAnnotation": null
// CHECK-PRETTY:                       },
// CHECK-PRETTY:                       "arguments": []
// CHECK-PRETTY:                     }
// CHECK-PRETTY:                   ]
// CHECK-PRETTY:                 },
// CHECK-PRETTY:                 "directive": null
// CHECK-PRETTY:               }
// CHECK-PRETTY:             ]
// CHECK-PRETTY:           }
// CHECK-PRETTY:         ]
// CHECK-PRETTY:       }
// CHECK-PRETTY:     ]
// CHECK-PRETTY:   }
// CHECK-PRETTY: }

// CHECK-SOURCE-LOC: {
// CHECK-SOURCE-LOC:   "type": "File",
// CHECK-SOURCE-LOC:   "program": {
// CHECK-SOURCE-LOC:     "type": "Program",
// CHECK-SOURCE-LOC:     "body": [
// CHECK-SOURCE-LOC:       {
// CHECK-SOURCE-LOC:         "type": "FunctionDeclaration",
// CHECK-SOURCE-LOC:         "id": {
// CHECK-SOURCE-LOC:           "type": "Identifier",
// CHECK-SOURCE-LOC:           "name": "foo",
// CHECK-SOURCE-LOC:           "typeAnnotation": null,
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 5,
// CHECK-SOURCE-LOC:               "column": 10
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 5,
// CHECK-SOURCE-LOC:               "column": 13
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             310,
// CHECK-SOURCE-LOC:             313
// CHECK-SOURCE-LOC:           ]
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "params": [],
// CHECK-SOURCE-LOC:         "body": {
// CHECK-SOURCE-LOC:           "type": "BlockStatement",
// CHECK-SOURCE-LOC:           "body": [
// CHECK-SOURCE-LOC:             {
// CHECK-SOURCE-LOC:               "type": "ReturnStatement",
// CHECK-SOURCE-LOC:               "argument": {
// CHECK-SOURCE-LOC:                 "type": "CallExpression",
// CHECK-SOURCE-LOC:                 "callee": {
// CHECK-SOURCE-LOC:                   "type": "MemberExpression",
// CHECK-SOURCE-LOC:                   "object": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "Math",
// CHECK-SOURCE-LOC:                     "typeAnnotation": null,
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 6,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 6,
// CHECK-SOURCE-LOC:                         "column": 14
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       327,
// CHECK-SOURCE-LOC:                       331
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "property": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "random",
// CHECK-SOURCE-LOC:                     "typeAnnotation": null,
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 6,
// CHECK-SOURCE-LOC:                         "column": 15
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 6,
// CHECK-SOURCE-LOC:                         "column": 21
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       332,
// CHECK-SOURCE-LOC:                       338
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "computed": false,
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 6,
// CHECK-SOURCE-LOC:                       "column": 10
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 6,
// CHECK-SOURCE-LOC:                       "column": 21
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     327,
// CHECK-SOURCE-LOC:                     338
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "arguments": [],
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 6,
// CHECK-SOURCE-LOC:                     "column": 10
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 6,
// CHECK-SOURCE-LOC:                     "column": 23
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   327,
// CHECK-SOURCE-LOC:                   340
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 6,
// CHECK-SOURCE-LOC:                   "column": 3
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 6,
// CHECK-SOURCE-LOC:                   "column": 24
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 320,
// CHECK-SOURCE-LOC:                 341
// CHECK-SOURCE-LOC:               ]
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           ],
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 5,
// CHECK-SOURCE-LOC:               "column": 16
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 7,
// CHECK-SOURCE-LOC:               "column": 2
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             316,
// CHECK-SOURCE-LOC:             343
// CHECK-SOURCE-LOC:           ]
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "returnType": null,
// CHECK-SOURCE-LOC:         "generator": false,
// CHECK-SOURCE-LOC:         "loc": {
// CHECK-SOURCE-LOC:           "start": {
// CHECK-SOURCE-LOC:             "line": 5,
// CHECK-SOURCE-LOC:             "column": 1
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "end": {
// CHECK-SOURCE-LOC:             "line": 7,
// CHECK-SOURCE-LOC:             "column": 2
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "range": [
// CHECK-SOURCE-LOC:           301,
// CHECK-SOURCE-LOC:           343
// CHECK-SOURCE-LOC:         ]
// CHECK-SOURCE-LOC:       },
// CHECK-SOURCE-LOC:       {
// CHECK-SOURCE-LOC:         "type": "SwitchStatement",
// CHECK-SOURCE-LOC:         "discriminant": {
// CHECK-SOURCE-LOC:           "type": "CallExpression",
// CHECK-SOURCE-LOC:           "callee": {
// CHECK-SOURCE-LOC:             "type": "Identifier",
// CHECK-SOURCE-LOC:             "name": "foo",
// CHECK-SOURCE-LOC:             "typeAnnotation": null,
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 9,
// CHECK-SOURCE-LOC:                 "column": 9
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 9,
// CHECK-SOURCE-LOC:                 "column": 12
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               353,
// CHECK-SOURCE-LOC:               356
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "arguments": [],
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 9,
// CHECK-SOURCE-LOC:               "column": 9
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 9,
// CHECK-SOURCE-LOC:               "column": 14
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             353,
// CHECK-SOURCE-LOC:             358
// CHECK-SOURCE-LOC:           ]
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "cases": [
// CHECK-SOURCE-LOC:           {
// CHECK-SOURCE-LOC:             "type": "SwitchCase",
// CHECK-SOURCE-LOC:             "test": {
// CHECK-SOURCE-LOC:               "type": "NumericLiteral",
// CHECK-SOURCE-LOC:               "value": 3,
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 10,
// CHECK-SOURCE-LOC:                   "column": 8
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 10,
// CHECK-SOURCE-LOC:                   "column": 9
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 369,
// CHECK-SOURCE-LOC:                 370
// CHECK-SOURCE-LOC:               ]
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "consequent": [
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "ExpressionStatement",
// CHECK-SOURCE-LOC:                 "expression": {
// CHECK-SOURCE-LOC:                   "type": "CallExpression",
// CHECK-SOURCE-LOC:                   "callee": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "print",
// CHECK-SOURCE-LOC:                     "typeAnnotation": null,
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 11,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 11,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       376,
// CHECK-SOURCE-LOC:                       381
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "StringLiteral",
// CHECK-SOURCE-LOC:                       "value": "fizz",
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 11,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 11,
// CHECK-SOURCE-LOC:                           "column": 17
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         382,
// CHECK-SOURCE-LOC:                         388
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 11,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 11,
// CHECK-SOURCE-LOC:                       "column": 18
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     376,
// CHECK-SOURCE-LOC:                     389
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 11,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 11,
// CHECK-SOURCE-LOC:                     "column": 19
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   376,
// CHECK-SOURCE-LOC:                   390
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "BreakStatement",
// CHECK-SOURCE-LOC:                 "label": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 12,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 12,
// CHECK-SOURCE-LOC:                     "column": 11
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   395,
// CHECK-SOURCE-LOC:                   401
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 10,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 12,
// CHECK-SOURCE-LOC:                 "column": 11
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               364,
// CHECK-SOURCE-LOC:               401
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           {
// CHECK-SOURCE-LOC:             "type": "SwitchCase",
// CHECK-SOURCE-LOC:             "test": {
// CHECK-SOURCE-LOC:               "type": "NumericLiteral",
// CHECK-SOURCE-LOC:               "value": 5,
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 13,
// CHECK-SOURCE-LOC:                   "column": 8
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 13,
// CHECK-SOURCE-LOC:                   "column": 9
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 409,
// CHECK-SOURCE-LOC:                 410
// CHECK-SOURCE-LOC:               ]
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "consequent": [
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "ExpressionStatement",
// CHECK-SOURCE-LOC:                 "expression": {
// CHECK-SOURCE-LOC:                   "type": "CallExpression",
// CHECK-SOURCE-LOC:                   "callee": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "print",
// CHECK-SOURCE-LOC:                     "typeAnnotation": null,
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 14,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 14,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       416,
// CHECK-SOURCE-LOC:                       421
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "StringLiteral",
// CHECK-SOURCE-LOC:                       "value": "buzz",
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 14,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 14,
// CHECK-SOURCE-LOC:                           "column": 17
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         422,
// CHECK-SOURCE-LOC:                         428
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 14,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 14,
// CHECK-SOURCE-LOC:                       "column": 18
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     416,
// CHECK-SOURCE-LOC:                     429
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 14,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 14,
// CHECK-SOURCE-LOC:                     "column": 19
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   416,
// CHECK-SOURCE-LOC:                   430
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "BreakStatement",
// CHECK-SOURCE-LOC:                 "label": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 15,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 15,
// CHECK-SOURCE-LOC:                     "column": 11
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   435,
// CHECK-SOURCE-LOC:                   441
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 13,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 15,
// CHECK-SOURCE-LOC:                 "column": 11
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               404,
// CHECK-SOURCE-LOC:               441
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           {
// CHECK-SOURCE-LOC:             "type": "SwitchCase",
// CHECK-SOURCE-LOC:             "test": null,
// CHECK-SOURCE-LOC:             "consequent": [
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "ExpressionStatement",
// CHECK-SOURCE-LOC:                 "expression": {
// CHECK-SOURCE-LOC:                   "type": "CallExpression",
// CHECK-SOURCE-LOC:                   "callee": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "print",
// CHECK-SOURCE-LOC:                     "typeAnnotation": null,
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 17,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 17,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       457,
// CHECK-SOURCE-LOC:                       462
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "CallExpression",
// CHECK-SOURCE-LOC:                       "callee": {
// CHECK-SOURCE-LOC:                         "type": "Identifier",
// CHECK-SOURCE-LOC:                         "name": "foo",
// CHECK-SOURCE-LOC:                         "typeAnnotation": null,
// CHECK-SOURCE-LOC:                         "loc": {
// CHECK-SOURCE-LOC:                           "start": {
// CHECK-SOURCE-LOC:                             "line": 17,
// CHECK-SOURCE-LOC:                             "column": 11
// CHECK-SOURCE-LOC:                           },
// CHECK-SOURCE-LOC:                           "end": {
// CHECK-SOURCE-LOC:                             "line": 17,
// CHECK-SOURCE-LOC:                             "column": 14
// CHECK-SOURCE-LOC:                           }
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "range": [
// CHECK-SOURCE-LOC:                           463,
// CHECK-SOURCE-LOC:                           466
// CHECK-SOURCE-LOC:                         ]
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "arguments": [],
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 17,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 17,
// CHECK-SOURCE-LOC:                           "column": 16
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         463,
// CHECK-SOURCE-LOC:                         468
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 17,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 17,
// CHECK-SOURCE-LOC:                       "column": 17
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     457,
// CHECK-SOURCE-LOC:                     469
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 17,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 17,
// CHECK-SOURCE-LOC:                     "column": 18
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   457,
// CHECK-SOURCE-LOC:                   470
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 16,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 17,
// CHECK-SOURCE-LOC:                 "column": 18
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               444,
// CHECK-SOURCE-LOC:               470
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         ],
// CHECK-SOURCE-LOC:         "loc": {
// CHECK-SOURCE-LOC:           "start": {
// CHECK-SOURCE-LOC:             "line": 9,
// CHECK-SOURCE-LOC:             "column": 1
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "end": {
// CHECK-SOURCE-LOC:             "line": 18,
// CHECK-SOURCE-LOC:             "column": 2
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "range": [
// CHECK-SOURCE-LOC:           345,
// CHECK-SOURCE-LOC:           472
// CHECK-SOURCE-LOC:         ]
// CHECK-SOURCE-LOC:       }
// CHECK-SOURCE-LOC:     ],
// CHECK-SOURCE-LOC:     "loc": {
// CHECK-SOURCE-LOC:       "start": {
// CHECK-SOURCE-LOC:         "line": 5,
// CHECK-SOURCE-LOC:         "column": 1
// CHECK-SOURCE-LOC:       },
// CHECK-SOURCE-LOC:       "end": {
// CHECK-SOURCE-LOC:         "line": 785,
// CHECK-SOURCE-LOC:         "column": 1
// CHECK-SOURCE-LOC:       }
// CHECK-SOURCE-LOC:     },
// CHECK-SOURCE-LOC:     "range": [
// CHECK-SOURCE-LOC:       301,
// CHECK-SOURCE-LOC:       37099
// CHECK-SOURCE-LOC:     ]
// CHECK-SOURCE-LOC:   },
// CHECK-SOURCE-LOC:   "loc": {
// CHECK-SOURCE-LOC:     "start": {
// CHECK-SOURCE-LOC:       "line": 5,
// CHECK-SOURCE-LOC:       "column": 1
// CHECK-SOURCE-LOC:     },
// CHECK-SOURCE-LOC:     "end": {
// CHECK-SOURCE-LOC:       "line": 785,
// CHECK-SOURCE-LOC:       "column": 1
// CHECK-SOURCE-LOC:     }
// CHECK-SOURCE-LOC:   },
// CHECK-SOURCE-LOC:   "range": [
// CHECK-SOURCE-LOC:     301,
// CHECK-SOURCE-LOC:     37099
// CHECK-SOURCE-LOC:   ]
// CHECK-SOURCE-LOC: }
