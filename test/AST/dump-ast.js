/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -dump-ast -pretty %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-PRETTY
// RUN: %hermes -dump-ast -dump-source-location=both -pretty %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-SOURCE-LOC
// RUN: %hermes -dump-ast -Xinclude-empty-ast-nodes -pretty-json %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-FULL

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

// CHECK: {"type":"Program","body":[{"type":"FunctionDeclaration","id":{"type":"Identifier","name":"foo"},"params":[],"body":{"type":"BlockStatement","body":[{"type":"ReturnStatement","argument":{"type":"CallExpression","callee":{"type":"MemberExpression","object":{"type":"Identifier","name":"Math"},"property":{"type":"Identifier","name":"random"},"computed":false},"arguments":[]}}]},"generator":false,"async":false},{"type":"SwitchStatement","discriminant":{"type":"CallExpression","callee":{"type":"Identifier","name":"foo"},"arguments":[]},"cases":[{"type":"SwitchCase","test":{"type":"NumericLiteral","value":3,"raw":"3"},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print"},"arguments":[{"type":"StringLiteral","value":"fizz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":{"type":"NumericLiteral","value":5,"raw":"5"},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print"},"arguments":[{"type":"StringLiteral","value":"buzz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":null,"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print"},"arguments":[{"type":"CallExpression","callee":{"type":"Identifier","name":"foo"},"arguments":[]}]},"directive":null}]}]}]}

// CHECK-PRETTY:   {
// CHECK-PRETTY:     "type": "Program",
// CHECK-PRETTY:     "body": [
// CHECK-PRETTY:       {
// CHECK-PRETTY:         "type": "FunctionDeclaration",
// CHECK-PRETTY:         "id": {
// CHECK-PRETTY:           "type": "Identifier",
// CHECK-PRETTY:           "name": "foo"
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
// CHECK-PRETTY:                     "name": "Math"
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "property": {
// CHECK-PRETTY:                     "type": "Identifier",
// CHECK-PRETTY:                     "name": "random"
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "computed": false
// CHECK-PRETTY:                 },
// CHECK-PRETTY:                 "arguments": []
// CHECK-PRETTY:               }
// CHECK-PRETTY:             }
// CHECK-PRETTY:           ]
// CHECK-PRETTY:         },
// CHECK-PRETTY:         "generator": false,
// CHECK-PRETTY:         "async": false
// CHECK-PRETTY:       },
// CHECK-PRETTY:       {
// CHECK-PRETTY:         "type": "SwitchStatement",
// CHECK-PRETTY:         "discriminant": {
// CHECK-PRETTY:           "type": "CallExpression",
// CHECK-PRETTY:           "callee": {
// CHECK-PRETTY:             "type": "Identifier",
// CHECK-PRETTY:             "name": "foo"
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
// CHECK-PRETTY:                     "name": "print"
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
// CHECK-PRETTY:                     "name": "print"
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
// CHECK-PRETTY:                     "name": "print"
// CHECK-PRETTY:                   },
// CHECK-PRETTY:                   "arguments": [
// CHECK-PRETTY:                     {
// CHECK-PRETTY:                       "type": "CallExpression",
// CHECK-PRETTY:                       "callee": {
// CHECK-PRETTY:                         "type": "Identifier",
// CHECK-PRETTY:                         "name": "foo"
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
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 13,
// CHECK-SOURCE-LOC:               "column": 10
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 13,
// CHECK-SOURCE-LOC:               "column": 13
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             636,
// CHECK-SOURCE-LOC:             639
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
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 14,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 14,
// CHECK-SOURCE-LOC:                         "column": 14
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       653,
// CHECK-SOURCE-LOC:                       657
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "property": {
// CHECK-SOURCE-LOC:                     "type": "Identifier",
// CHECK-SOURCE-LOC:                     "name": "random",
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 14,
// CHECK-SOURCE-LOC:                         "column": 15
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 14,
// CHECK-SOURCE-LOC:                         "column": 21
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       658,
// CHECK-SOURCE-LOC:                       664
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "computed": false,
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 14,
// CHECK-SOURCE-LOC:                       "column": 10
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 14,
// CHECK-SOURCE-LOC:                       "column": 21
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     653,
// CHECK-SOURCE-LOC:                     664
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "arguments": [],
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 14,
// CHECK-SOURCE-LOC:                     "column": 10
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 14,
// CHECK-SOURCE-LOC:                     "column": 23
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   653,
// CHECK-SOURCE-LOC:                   666
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "loc": {
// CHECK-SOURCE-LOC:                 "start": {
// CHECK-SOURCE-LOC:                   "line": 14,
// CHECK-SOURCE-LOC:                   "column": 3
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 14,
// CHECK-SOURCE-LOC:                   "column": 24
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 646,
// CHECK-SOURCE-LOC:                 667
// CHECK-SOURCE-LOC:               ]
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           ],
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 13,
// CHECK-SOURCE-LOC:               "column": 16
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 15,
// CHECK-SOURCE-LOC:               "column": 2
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             642,
// CHECK-SOURCE-LOC:             669
// CHECK-SOURCE-LOC:           ]
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "generator": false,
// CHECK-SOURCE-LOC:         "async": false,
// CHECK-SOURCE-LOC:         "loc": {
// CHECK-SOURCE-LOC:           "start": {
// CHECK-SOURCE-LOC:             "line": 13,
// CHECK-SOURCE-LOC:             "column": 1
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "end": {
// CHECK-SOURCE-LOC:             "line": 15,
// CHECK-SOURCE-LOC:             "column": 2
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "range": [
// CHECK-SOURCE-LOC:           627,
// CHECK-SOURCE-LOC:           669
// CHECK-SOURCE-LOC:         ]
// CHECK-SOURCE-LOC:       },
// CHECK-SOURCE-LOC:       {
// CHECK-SOURCE-LOC:         "type": "SwitchStatement",
// CHECK-SOURCE-LOC:         "discriminant": {
// CHECK-SOURCE-LOC:           "type": "CallExpression",
// CHECK-SOURCE-LOC:           "callee": {
// CHECK-SOURCE-LOC:             "type": "Identifier",
// CHECK-SOURCE-LOC:             "name": "foo",
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 17,
// CHECK-SOURCE-LOC:                 "column": 9
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 17,
// CHECK-SOURCE-LOC:                 "column": 12
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               679,
// CHECK-SOURCE-LOC:               682
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "arguments": [],
// CHECK-SOURCE-LOC:           "loc": {
// CHECK-SOURCE-LOC:             "start": {
// CHECK-SOURCE-LOC:               "line": 17,
// CHECK-SOURCE-LOC:               "column": 9
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "end": {
// CHECK-SOURCE-LOC:               "line": 17,
// CHECK-SOURCE-LOC:               "column": 14
// CHECK-SOURCE-LOC:             }
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "range": [
// CHECK-SOURCE-LOC:             679,
// CHECK-SOURCE-LOC:             684
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
// CHECK-SOURCE-LOC:                   "line": 18,
// CHECK-SOURCE-LOC:                   "column": 8
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 18,
// CHECK-SOURCE-LOC:                   "column": 9
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 695,
// CHECK-SOURCE-LOC:                 696
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
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 19,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 19,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       702,
// CHECK-SOURCE-LOC:                       707
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "StringLiteral",
// CHECK-SOURCE-LOC:                       "value": "fizz",
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 19,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 19,
// CHECK-SOURCE-LOC:                           "column": 17
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         708,
// CHECK-SOURCE-LOC:                         714
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 19,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 19,
// CHECK-SOURCE-LOC:                       "column": 18
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     702,
// CHECK-SOURCE-LOC:                     715
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 19,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 19,
// CHECK-SOURCE-LOC:                     "column": 19
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   702,
// CHECK-SOURCE-LOC:                   716
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "BreakStatement",
// CHECK-SOURCE-LOC:                 "label": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 20,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 20,
// CHECK-SOURCE-LOC:                     "column": 11
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   721,
// CHECK-SOURCE-LOC:                   727
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 18,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 20,
// CHECK-SOURCE-LOC:                 "column": 11
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               690,
// CHECK-SOURCE-LOC:               727
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
// CHECK-SOURCE-LOC:                   "line": 21,
// CHECK-SOURCE-LOC:                   "column": 8
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "end": {
// CHECK-SOURCE-LOC:                   "line": 21,
// CHECK-SOURCE-LOC:                   "column": 9
// CHECK-SOURCE-LOC:                 }
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "range": [
// CHECK-SOURCE-LOC:                 735,
// CHECK-SOURCE-LOC:                 736
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
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 22,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 22,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       742,
// CHECK-SOURCE-LOC:                       747
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "StringLiteral",
// CHECK-SOURCE-LOC:                       "value": "buzz",
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 22,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 22,
// CHECK-SOURCE-LOC:                           "column": 17
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         748,
// CHECK-SOURCE-LOC:                         754
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 22,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 22,
// CHECK-SOURCE-LOC:                       "column": 18
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     742,
// CHECK-SOURCE-LOC:                     755
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 22,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 22,
// CHECK-SOURCE-LOC:                     "column": 19
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   742,
// CHECK-SOURCE-LOC:                   756
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               {
// CHECK-SOURCE-LOC:                 "type": "BreakStatement",
// CHECK-SOURCE-LOC:                 "label": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 23,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 23,
// CHECK-SOURCE-LOC:                     "column": 11
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   761,
// CHECK-SOURCE-LOC:                   767
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 21,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 23,
// CHECK-SOURCE-LOC:                 "column": 11
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               730,
// CHECK-SOURCE-LOC:               767
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
// CHECK-SOURCE-LOC:                     "loc": {
// CHECK-SOURCE-LOC:                       "start": {
// CHECK-SOURCE-LOC:                         "line": 25,
// CHECK-SOURCE-LOC:                         "column": 5
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "end": {
// CHECK-SOURCE-LOC:                         "line": 25,
// CHECK-SOURCE-LOC:                         "column": 10
// CHECK-SOURCE-LOC:                       }
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "range": [
// CHECK-SOURCE-LOC:                       783,
// CHECK-SOURCE-LOC:                       788
// CHECK-SOURCE-LOC:                     ]
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "arguments": [
// CHECK-SOURCE-LOC:                     {
// CHECK-SOURCE-LOC:                       "type": "CallExpression",
// CHECK-SOURCE-LOC:                       "callee": {
// CHECK-SOURCE-LOC:                         "type": "Identifier",
// CHECK-SOURCE-LOC:                         "name": "foo",
// CHECK-SOURCE-LOC:                         "loc": {
// CHECK-SOURCE-LOC:                           "start": {
// CHECK-SOURCE-LOC:                             "line": 25,
// CHECK-SOURCE-LOC:                             "column": 11
// CHECK-SOURCE-LOC:                           },
// CHECK-SOURCE-LOC:                           "end": {
// CHECK-SOURCE-LOC:                             "line": 25,
// CHECK-SOURCE-LOC:                             "column": 14
// CHECK-SOURCE-LOC:                           }
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "range": [
// CHECK-SOURCE-LOC:                           789,
// CHECK-SOURCE-LOC:                           792
// CHECK-SOURCE-LOC:                         ]
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "arguments": [],
// CHECK-SOURCE-LOC:                       "loc": {
// CHECK-SOURCE-LOC:                         "start": {
// CHECK-SOURCE-LOC:                           "line": 25,
// CHECK-SOURCE-LOC:                           "column": 11
// CHECK-SOURCE-LOC:                         },
// CHECK-SOURCE-LOC:                         "end": {
// CHECK-SOURCE-LOC:                           "line": 25,
// CHECK-SOURCE-LOC:                           "column": 16
// CHECK-SOURCE-LOC:                         }
// CHECK-SOURCE-LOC:                       },
// CHECK-SOURCE-LOC:                       "range": [
// CHECK-SOURCE-LOC:                         789,
// CHECK-SOURCE-LOC:                         794
// CHECK-SOURCE-LOC:                       ]
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   ],
// CHECK-SOURCE-LOC:                   "loc": {
// CHECK-SOURCE-LOC:                     "start": {
// CHECK-SOURCE-LOC:                       "line": 25,
// CHECK-SOURCE-LOC:                       "column": 5
// CHECK-SOURCE-LOC:                     },
// CHECK-SOURCE-LOC:                     "end": {
// CHECK-SOURCE-LOC:                       "line": 25,
// CHECK-SOURCE-LOC:                       "column": 17
// CHECK-SOURCE-LOC:                     }
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "range": [
// CHECK-SOURCE-LOC:                     783,
// CHECK-SOURCE-LOC:                     795
// CHECK-SOURCE-LOC:                   ]
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "directive": null,
// CHECK-SOURCE-LOC:                 "loc": {
// CHECK-SOURCE-LOC:                   "start": {
// CHECK-SOURCE-LOC:                     "line": 25,
// CHECK-SOURCE-LOC:                     "column": 5
// CHECK-SOURCE-LOC:                   },
// CHECK-SOURCE-LOC:                   "end": {
// CHECK-SOURCE-LOC:                     "line": 25,
// CHECK-SOURCE-LOC:                     "column": 18
// CHECK-SOURCE-LOC:                   }
// CHECK-SOURCE-LOC:                 },
// CHECK-SOURCE-LOC:                 "range": [
// CHECK-SOURCE-LOC:                   783,
// CHECK-SOURCE-LOC:                   796
// CHECK-SOURCE-LOC:                 ]
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             ],
// CHECK-SOURCE-LOC:             "loc": {
// CHECK-SOURCE-LOC:               "start": {
// CHECK-SOURCE-LOC:                 "line": 24,
// CHECK-SOURCE-LOC:                 "column": 3
// CHECK-SOURCE-LOC:               },
// CHECK-SOURCE-LOC:               "end": {
// CHECK-SOURCE-LOC:                 "line": 25,
// CHECK-SOURCE-LOC:                 "column": 18
// CHECK-SOURCE-LOC:               }
// CHECK-SOURCE-LOC:             },
// CHECK-SOURCE-LOC:             "range": [
// CHECK-SOURCE-LOC:               770,
// CHECK-SOURCE-LOC:               796
// CHECK-SOURCE-LOC:             ]
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         ],
// CHECK-SOURCE-LOC:         "loc": {
// CHECK-SOURCE-LOC:           "start": {
// CHECK-SOURCE-LOC:             "line": 17,
// CHECK-SOURCE-LOC:             "column": 1
// CHECK-SOURCE-LOC:           },
// CHECK-SOURCE-LOC:           "end": {
// CHECK-SOURCE-LOC:             "line": 26,
// CHECK-SOURCE-LOC:             "column": 2
// CHECK-SOURCE-LOC:           }
// CHECK-SOURCE-LOC:         },
// CHECK-SOURCE-LOC:         "range": [
// CHECK-SOURCE-LOC:           671,
// CHECK-SOURCE-LOC:           798
// CHECK-SOURCE-LOC:         ]
// CHECK-SOURCE-LOC:       }
// CHECK-SOURCE-LOC:     ],
// CHECK-SOURCE-LOC:     "loc": {
// CHECK-SOURCE-LOC:       "start": {
// CHECK-SOURCE-LOC:         "line": 13,
// CHECK-SOURCE-LOC:         "column": 1
// CHECK-SOURCE-LOC:       },
// CHECK-SOURCE-LOC:       "end": {
// CHECK-SOURCE-LOC:         "line": 26,
// CHECK-SOURCE-LOC:         "column": 2
// CHECK-SOURCE-LOC:       }
// CHECK-SOURCE-LOC:     },
// CHECK-SOURCE-LOC:     "range": [
// CHECK-SOURCE-LOC:       627,
// CHECK-SOURCE-LOC:       798
// CHECK-SOURCE-LOC:     ]
// CHECK-SOURCE-LOC:   }

// CHECK-FULL: {
// CHECK-FULL:   "type": "Program",
// CHECK-FULL:   "body": [
// CHECK-FULL:     {
// CHECK-FULL:       "type": "FunctionDeclaration",
// CHECK-FULL:       "id": {
// CHECK-FULL:         "type": "Identifier",
// CHECK-FULL:         "name": "foo",
// CHECK-FULL:         "typeAnnotation": null,
// CHECK-FULL:         "optional": false
// CHECK-FULL:       },
// CHECK-FULL:       "params": [],
// CHECK-FULL:       "body": {
// CHECK-FULL:         "type": "BlockStatement",
// CHECK-FULL:         "body": [
// CHECK-FULL:           {
// CHECK-FULL:             "type": "ReturnStatement",
// CHECK-FULL:             "argument": {
// CHECK-FULL:               "type": "CallExpression",
// CHECK-FULL:               "callee": {
// CHECK-FULL:                 "type": "MemberExpression",
// CHECK-FULL:                 "object": {
// CHECK-FULL:                   "type": "Identifier",
// CHECK-FULL:                   "name": "Math",
// CHECK-FULL:                   "typeAnnotation": null,
// CHECK-FULL:                   "optional": false
// CHECK-FULL:                 },
// CHECK-FULL:                 "property": {
// CHECK-FULL:                   "type": "Identifier",
// CHECK-FULL:                   "name": "random",
// CHECK-FULL:                   "typeAnnotation": null,
// CHECK-FULL:                   "optional": false
// CHECK-FULL:                 },
// CHECK-FULL:                 "computed": false
// CHECK-FULL:               },
// CHECK-FULL:               "typeArguments": null,
// CHECK-FULL:               "arguments": []
// CHECK-FULL:             }
// CHECK-FULL:           }
// CHECK-FULL:         ]
// CHECK-FULL:       },
// CHECK-FULL:       "typeParameters": null,
// CHECK-FULL:       "returnType": null,
// CHECK-FULL:       "generator": false,
// CHECK-FULL:       "async": false
// CHECK-FULL:     },
// CHECK-FULL:     {
// CHECK-FULL:       "type": "SwitchStatement",
// CHECK-FULL:       "discriminant": {
// CHECK-FULL:         "type": "CallExpression",
// CHECK-FULL:         "callee": {
// CHECK-FULL:           "type": "Identifier",
// CHECK-FULL:           "name": "foo",
// CHECK-FULL:           "typeAnnotation": null,
// CHECK-FULL:           "optional": false
// CHECK-FULL:         },
// CHECK-FULL:         "typeArguments": null,
// CHECK-FULL:         "arguments": []
// CHECK-FULL:       },
// CHECK-FULL:       "cases": [
// CHECK-FULL:         {
// CHECK-FULL:           "type": "SwitchCase",
// CHECK-FULL:           "test": {
// CHECK-FULL:             "type": "NumericLiteral",
// CHECK-FULL:             "value": 3,
// CHECK-FULL:             "raw": "3"
// CHECK-FULL:           },
// CHECK-FULL:           "consequent": [
// CHECK-FULL:             {
// CHECK-FULL:               "type": "ExpressionStatement",
// CHECK-FULL:               "expression": {
// CHECK-FULL:                 "type": "CallExpression",
// CHECK-FULL:                 "callee": {
// CHECK-FULL:                   "type": "Identifier",
// CHECK-FULL:                   "name": "print",
// CHECK-FULL:                   "typeAnnotation": null,
// CHECK-FULL:                   "optional": false
// CHECK-FULL:                 },
// CHECK-FULL:                 "typeArguments": null,
// CHECK-FULL:                 "arguments": [
// CHECK-FULL:                   {
// CHECK-FULL:                     "type": "StringLiteral",
// CHECK-FULL:                     "value": "fizz"
// CHECK-FULL:                   }
// CHECK-FULL:                 ]
// CHECK-FULL:               },
// CHECK-FULL:               "directive": null
// CHECK-FULL:             },
// CHECK-FULL:             {
// CHECK-FULL:               "type": "BreakStatement",
// CHECK-FULL:               "label": null
// CHECK-FULL:             }
// CHECK-FULL:           ]
// CHECK-FULL:         },
// CHECK-FULL:         {
// CHECK-FULL:           "type": "SwitchCase",
// CHECK-FULL:           "test": {
// CHECK-FULL:             "type": "NumericLiteral",
// CHECK-FULL:             "value": 5,
// CHECK-FULL:             "raw": "5"
// CHECK-FULL:           },
// CHECK-FULL:           "consequent": [
// CHECK-FULL:             {
// CHECK-FULL:               "type": "ExpressionStatement",
// CHECK-FULL:               "expression": {
// CHECK-FULL:                 "type": "CallExpression",
// CHECK-FULL:                 "callee": {
// CHECK-FULL:                   "type": "Identifier",
// CHECK-FULL:                   "name": "print",
// CHECK-FULL:                   "typeAnnotation": null,
// CHECK-FULL:                   "optional": false
// CHECK-FULL:                 },
// CHECK-FULL:                 "typeArguments": null,
// CHECK-FULL:                 "arguments": [
// CHECK-FULL:                   {
// CHECK-FULL:                     "type": "StringLiteral",
// CHECK-FULL:                     "value": "buzz"
// CHECK-FULL:                   }
// CHECK-FULL:                 ]
// CHECK-FULL:               },
// CHECK-FULL:               "directive": null
// CHECK-FULL:             },
// CHECK-FULL:             {
// CHECK-FULL:               "type": "BreakStatement",
// CHECK-FULL:               "label": null
// CHECK-FULL:             }
// CHECK-FULL:           ]
// CHECK-FULL:         },
// CHECK-FULL:         {
// CHECK-FULL:           "type": "SwitchCase",
// CHECK-FULL:           "test": null,
// CHECK-FULL:           "consequent": [
// CHECK-FULL:             {
// CHECK-FULL:               "type": "ExpressionStatement",
// CHECK-FULL:               "expression": {
// CHECK-FULL:                 "type": "CallExpression",
// CHECK-FULL:                 "callee": {
// CHECK-FULL:                   "type": "Identifier",
// CHECK-FULL:                   "name": "print",
// CHECK-FULL:                   "typeAnnotation": null,
// CHECK-FULL:                   "optional": false
// CHECK-FULL:                 },
// CHECK-FULL:                 "typeArguments": null,
// CHECK-FULL:                 "arguments": [
// CHECK-FULL:                   {
// CHECK-FULL:                     "type": "CallExpression",
// CHECK-FULL:                     "callee": {
// CHECK-FULL:                       "type": "Identifier",
// CHECK-FULL:                       "name": "foo",
// CHECK-FULL:                       "typeAnnotation": null,
// CHECK-FULL:                       "optional": false
// CHECK-FULL:                     },
// CHECK-FULL:                     "typeArguments": null,
// CHECK-FULL:                     "arguments": []
// CHECK-FULL:                   }
// CHECK-FULL:                 ]
// CHECK-FULL:               },
// CHECK-FULL:               "directive": null
// CHECK-FULL:             }
// CHECK-FULL:           ]
// CHECK-FULL:         }
// CHECK-FULL:       ]
// CHECK-FULL:     }
// CHECK-FULL:   ]
// CHECK-FULL: }
