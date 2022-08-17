/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

//CHECK: {
//CHECK-NEXT:    "type": "Program",
//CHECK-NEXT:    "body": [

new x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "x"
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new new x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new new new x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "NewExpression",
//CHECK-NEXT:               "callee": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "x"
//CHECK-NEXT:               },
//CHECK-NEXT:               "arguments": []
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new x();
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "x"
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new x()();
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "CallExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new new x();
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new new new x()();
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "NewExpression",
//CHECK-NEXT:               "callee": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "x"
//CHECK-NEXT:               },
//CHECK-NEXT:               "arguments": []
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new new x[10];
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "MemberExpression",
//CHECK-NEXT:               "object": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "x"
//CHECK-NEXT:               },
//CHECK-NEXT:               "property": {
//CHECK-NEXT:                 "type": "NumericLiteral",
//CHECK-NEXT:                 "value": 10,
//CHECK-NEXT:                 "raw": "10"
//CHECK-NEXT:               },
//CHECK-NEXT:               "computed": true
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new x()[10]();
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "CallExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "MemberExpression",
//CHECK-NEXT:             "object": {
//CHECK-NEXT:               "type": "NewExpression",
//CHECK-NEXT:               "callee": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "x"
//CHECK-NEXT:               },
//CHECK-NEXT:               "arguments": []
//CHECK-NEXT:             },
//CHECK-NEXT:             "property": {
//CHECK-NEXT:               "type": "NumericLiteral",
//CHECK-NEXT:               "value": 10,
//CHECK-NEXT:               "raw": "10"
//CHECK-NEXT:             },
//CHECK-NEXT:             "computed": true
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       },

new new x[10];
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "NewExpression",
//CHECK-NEXT:           "callee": {
//CHECK-NEXT:             "type": "NewExpression",
//CHECK-NEXT:             "callee": {
//CHECK-NEXT:               "type": "MemberExpression",
//CHECK-NEXT:               "object": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "x"
//CHECK-NEXT:               },
//CHECK-NEXT:               "property": {
//CHECK-NEXT:                 "type": "NumericLiteral",
//CHECK-NEXT:                 "value": 10,
//CHECK-NEXT:                 "raw": "10"
//CHECK-NEXT:               },
//CHECK-NEXT:               "computed": true
//CHECK-NEXT:             },
//CHECK-NEXT:             "arguments": []
//CHECK-NEXT:           },
//CHECK-NEXT:           "arguments": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       }

//CHECK-NEXT:    ]
//CHECK-NEXT:  }
