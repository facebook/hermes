/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

//CHECK:      {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [

var {a} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": true
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a,} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": true
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a, b} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": true
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": true
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a: b, c: d, } = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "d"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a: b = 10} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "AssignmentPattern",
//CHECK-NEXT:                     "left": {
//CHECK-NEXT:                       "type": "Identifier",
//CHECK-NEXT:                       "name": "b"
//CHECK-NEXT:                     },
//CHECK-NEXT:                     "right": {
//CHECK-NEXT:                       "type": "NumericLiteral",
//CHECK-NEXT:                       "value": 10,
//CHECK-NEXT:                       "raw": "10"
//CHECK-NEXT:                     }
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a: [b]} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "ArrayPattern",
//CHECK-NEXT:                     "elements": [
//CHECK-NEXT:                       {
//CHECK-NEXT:                         "type": "Identifier",
//CHECK-NEXT:                         "name": "b"
//CHECK-NEXT:                       }
//CHECK-NEXT:                     ]
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a: [b] = [1]} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "AssignmentPattern",
//CHECK-NEXT:                     "left": {
//CHECK-NEXT:                       "type": "ArrayPattern",
//CHECK-NEXT:                       "elements": [
//CHECK-NEXT:                         {
//CHECK-NEXT:                           "type": "Identifier",
//CHECK-NEXT:                           "name": "b"
//CHECK-NEXT:                         }
//CHECK-NEXT:                       ]
//CHECK-NEXT:                     },
//CHECK-NEXT:                     "right": {
//CHECK-NEXT:                       "type": "ArrayExpression",
//CHECK-NEXT:                       "elements": [
//CHECK-NEXT:                         {
//CHECK-NEXT:                           "type": "NumericLiteral",
//CHECK-NEXT:                           "value": 1,
//CHECK-NEXT:                           "raw": "1"
//CHECK-NEXT:                         }
//CHECK-NEXT:                       ],
//CHECK-NEXT:                       "trailingComma": false
//CHECK-NEXT:                     }
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

var {a: [b = 2] = [1]} = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "AssignmentPattern",
//CHECK-NEXT:                     "left": {
//CHECK-NEXT:                       "type": "ArrayPattern",
//CHECK-NEXT:                       "elements": [
//CHECK-NEXT:                         {
//CHECK-NEXT:                           "type": "AssignmentPattern",
//CHECK-NEXT:                           "left": {
//CHECK-NEXT:                             "type": "Identifier",
//CHECK-NEXT:                             "name": "b"
//CHECK-NEXT:                           },
//CHECK-NEXT:                           "right": {
//CHECK-NEXT:                             "type": "NumericLiteral",
//CHECK-NEXT:                             "value": 2,
//CHECK-NEXT:                             "raw": "2"
//CHECK-NEXT:                           }
//CHECK-NEXT:                         }
//CHECK-NEXT:                       ]
//CHECK-NEXT:                     },
//CHECK-NEXT:                     "right": {
//CHECK-NEXT:                       "type": "ArrayExpression",
//CHECK-NEXT:                       "elements": [
//CHECK-NEXT:                         {
//CHECK-NEXT:                           "type": "NumericLiteral",
//CHECK-NEXT:                           "value": 1,
//CHECK-NEXT:                           "raw": "1"
//CHECK-NEXT:                         }
//CHECK-NEXT:                       ],
//CHECK-NEXT:                       "trailingComma": false
//CHECK-NEXT:                     }
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       }

//CHECK-NEXT:     ]
//CHECK-NEXT:   }
