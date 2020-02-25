/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

// CHECK: {"type":"Program","body":[{"type":"FunctionDeclaration","id":{"type":"Identifier","name":"foo","typeAnnotation":null},"params":[],"body":{"type":"BlockStatement","body":[{"type":"ReturnStatement","argument":{"type":"CallExpression","callee":{"type":"MemberExpression","object":{"type":"Identifier","name":"Math","typeAnnotation":null},"property":{"type":"Identifier","name":"random","typeAnnotation":null},"computed":false},"arguments":[]}}]},"returnType":null,"generator":false,"async":false},{"type":"SwitchStatement","discriminant":{"type":"CallExpression","callee":{"type":"Identifier","name":"foo","typeAnnotation":null},"arguments":[]},"cases":[{"type":"SwitchCase","test":{"type":"NumericLiteral","value":3,"raw":"3"},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print","typeAnnotation":null},"arguments":[{"type":"StringLiteral","value":"fizz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":{"type":"NumericLiteral","value":5,"raw":"5"},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print","typeAnnotation":null},"arguments":[{"type":"StringLiteral","value":"buzz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":null,"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print","typeAnnotation":null},"arguments":[{"type":"CallExpression","callee":{"type":"Identifier","name":"foo","typeAnnotation":null},"arguments":[]}]},"directive":null}]}]}]}

// CHECK-PRETTY:   {
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
// CHECK-PRETTY:         "generator": false,
// CHECK-PRETTY:         "async": false
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
// CHECK-PRETTY:               "value": 3,
// CHECK-PRETTY:               "raw": "3"
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
// CHECK-PRETTY:               "value": 5,
// CHECK-PRETTY:               "raw": "5"
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

// CHECK-SOURCE-LOC:   {
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
// CHECK-SOURCE-LOC:               "line": 12,
// CHECK-SOURCE-LOC:               "column": 10
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 12,
// CHECK-SOURCE-LOC:               "column": 13
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             500,
// CHECK-SOURCE-LOC:             503
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
// CHECK-SOURCE-LOC:                         "line": 13,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 13,
// CHECK-SOURCE-LOC:                         "column": 14
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       517,
// CHECK-SOURCE-LOC:                       521
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "property": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "random",
// CHECK-SOURCE-LOC:                     "typeAnnotation": null,
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 13,
// CHECK-SOURCE-LOC:                         "column": 15
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 13,
// CHECK-SOURCE-LOC:                         "column": 21
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       522,
// CHECK-SOURCE-LOC:                       528
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "computed": false,
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 13,
// CHECK-SOURCE-LOC:                       "column": 10
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 13,
// CHECK-SOURCE-LOC:                       "column": 21
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     517,
// CHECK-SOURCE-LOC:                     528
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "arguments": [],
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 13,
// CHECK-SOURCE-LOC:                     "column": 10
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 13,
// CHECK-SOURCE-LOC:                     "column": 23
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   517,
// CHECK-SOURCE-LOC:                   530
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 13,
// CHECK-SOURCE-LOC:                   "column": 3
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 13,
// CHECK-SOURCE-LOC:                   "column": 24
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 510,
// CHECK-SOURCE-LOC:                 531
// CHECK-SOURCE-LOC:               ]
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           ],
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 12,
// CHECK-SOURCE-LOC:               "column": 16
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 14,
// CHECK-SOURCE-LOC:               "column": 2
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             506,
// CHECK-SOURCE-LOC:             533
// CHECK-SOURCE-LOC:           ]
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "returnType": null,
// CHECK-SOURCE-LOC:         "generator": false,
// CHECK-SOURCE-LOC:         "async": false,
// CHECK-SOURCE-LOC:         "loc": {
// CHECK-SOURCE-LOC:           "start": {
// CHECK-SOURCE-LOC:             "line": 12,
// CHECK-SOURCE-LOC:             "column": 1
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "end": {
// CHECK-SOURCE-LOC:             "line": 14,
// CHECK-SOURCE-LOC:             "column": 2
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "range": [
// CHECK-SOURCE-LOC:           491,
// CHECK-SOURCE-LOC:           533
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
// CHECK-SOURCE-LOC:                 "line": 16,
// CHECK-SOURCE-LOC:                 "column": 9
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 16,
// CHECK-SOURCE-LOC:                 "column": 12
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               543,
// CHECK-SOURCE-LOC:               546
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "arguments": [],
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 16,
// CHECK-SOURCE-LOC:               "column": 9
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 16,
// CHECK-SOURCE-LOC:               "column": 14
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             543,
// CHECK-SOURCE-LOC:             548
// CHECK-SOURCE-LOC:           ]
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "cases": [
// CHECK-SOURCE-LOC:           {
// CHECK-SOURCE-LOC:             "type": "SwitchCase",
// CHECK-SOURCE-LOC:             "test": {
// CHECK-SOURCE-LOC:               "type": "NumericLiteral",
// CHECK-SOURCE-LOC:               "value": 3,
// CHECK-SOURCE-LOC:               "raw": "3",
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 17,
// CHECK-SOURCE-LOC:                   "column": 8
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 17,
// CHECK-SOURCE-LOC:                   "column": 9
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 559,
// CHECK-SOURCE-LOC:                 560
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
// CHECK-SOURCE-LOC:                         "line": 18,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 18,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       566,
// CHECK-SOURCE-LOC:                       571
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "StringLiteral",
// CHECK-SOURCE-LOC:                       "value": "fizz",
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 18,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 18,
// CHECK-SOURCE-LOC:                           "column": 17
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         572,
// CHECK-SOURCE-LOC:                         578
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 18,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 18,
// CHECK-SOURCE-LOC:                       "column": 18
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     566,
// CHECK-SOURCE-LOC:                     579
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 18,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 18,
// CHECK-SOURCE-LOC:                     "column": 19
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   566,
// CHECK-SOURCE-LOC:                   580
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "BreakStatement",
// CHECK-SOURCE-LOC:                 "label": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 19,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 19,
// CHECK-SOURCE-LOC:                     "column": 11
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   585,
// CHECK-SOURCE-LOC:                   591
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 17,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 19,
// CHECK-SOURCE-LOC:                 "column": 11
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               554,
// CHECK-SOURCE-LOC:               591
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           {
// CHECK-SOURCE-LOC:             "type": "SwitchCase",
// CHECK-SOURCE-LOC:             "test": {
// CHECK-SOURCE-LOC:               "type": "NumericLiteral",
// CHECK-SOURCE-LOC:               "value": 5,
// CHECK-SOURCE-LOC:               "raw": "5",
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 20,
// CHECK-SOURCE-LOC:                   "column": 8
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 20,
// CHECK-SOURCE-LOC:                   "column": 9
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 599,
// CHECK-SOURCE-LOC:                 600
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
// CHECK-SOURCE-LOC:                         "line": 21,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 21,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       606,
// CHECK-SOURCE-LOC:                       611
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "StringLiteral",
// CHECK-SOURCE-LOC:                       "value": "buzz",
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 21,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 21,
// CHECK-SOURCE-LOC:                           "column": 17
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         612,
// CHECK-SOURCE-LOC:                         618
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 21,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 21,
// CHECK-SOURCE-LOC:                       "column": 18
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     606,
// CHECK-SOURCE-LOC:                     619
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 21,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 21,
// CHECK-SOURCE-LOC:                     "column": 19
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   606,
// CHECK-SOURCE-LOC:                   620
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "BreakStatement",
// CHECK-SOURCE-LOC:                 "label": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 22,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 22,
// CHECK-SOURCE-LOC:                     "column": 11
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   625,
// CHECK-SOURCE-LOC:                   631
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 20,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 22,
// CHECK-SOURCE-LOC:                 "column": 11
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               594,
// CHECK-SOURCE-LOC:               631
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
// CHECK-SOURCE-LOC:                         "line": 24,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 24,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       647,
// CHECK-SOURCE-LOC:                       652
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
// CHECK-SOURCE-LOC:                             "line": 24,
// CHECK-SOURCE-LOC:                             "column": 11
// CHECK-SOURCE-LOC:                           },
// CHECK-SOURCE-LOC:                           "end": {
// CHECK-SOURCE-LOC:                             "line": 24,
// CHECK-SOURCE-LOC:                             "column": 14
// CHECK-SOURCE-LOC:                           }
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "range": [
// CHECK-SOURCE-LOC:                           653,
// CHECK-SOURCE-LOC:                           656
// CHECK-SOURCE-LOC:                         ]
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "arguments": [],
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 24,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 24,
// CHECK-SOURCE-LOC:                           "column": 16
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         653,
// CHECK-SOURCE-LOC:                         658
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 24,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 24,
// CHECK-SOURCE-LOC:                       "column": 17
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     647,
// CHECK-SOURCE-LOC:                     659
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 24,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 24,
// CHECK-SOURCE-LOC:                     "column": 18
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   647,
// CHECK-SOURCE-LOC:                   660
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 23,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 24,
// CHECK-SOURCE-LOC:                 "column": 18
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               634,
// CHECK-SOURCE-LOC:               660
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         ],
// CHECK-SOURCE-LOC:         "loc": {
// CHECK-SOURCE-LOC:           "start": {
// CHECK-SOURCE-LOC:             "line": 16,
// CHECK-SOURCE-LOC:             "column": 1
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "end": {
// CHECK-SOURCE-LOC:             "line": 25,
// CHECK-SOURCE-LOC:             "column": 2
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "range": [
// CHECK-SOURCE-LOC:           535,
// CHECK-SOURCE-LOC:           662
// CHECK-SOURCE-LOC:         ]
// CHECK-SOURCE-LOC:       }
// CHECK-SOURCE-LOC:     ],
// CHECK-SOURCE-LOC:     "loc": {
// CHECK-SOURCE-LOC:       "start": {
// CHECK-SOURCE-LOC:         "line": 12,
// CHECK-SOURCE-LOC:         "column": 1
// CHECK-SOURCE-LOC:       },
// CHECK-SOURCE-LOC:       "end": {
// CHECK-SOURCE-LOC:         "line": 25,
// CHECK-SOURCE-LOC:         "column": 2
// CHECK-SOURCE-LOC:       }
// CHECK-SOURCE-LOC:     },
// CHECK-SOURCE-LOC:     "range": [
// CHECK-SOURCE-LOC:       491,
// CHECK-SOURCE-LOC:       662
// CHECK-SOURCE-LOC:     ]
// CHECK-SOURCE-LOC:   }
