/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

import 'foo.js' assert {};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import 'foo.js' assert {type: 'json'};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "type"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "json"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import 'foo.js' assert {'type': 'json'};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "type"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "json"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import 'foo.js' assert {'type': 'json',};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "type"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "json"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import 'foo.js' assert {'type': 'json', 'foo': 'bar', };
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "type"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "json"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "bar"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import 'foo.js' assert {'type': 'json', 'foo': 'bar', };
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "type"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "json"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportAttribute",
// CHECK-NEXT:           "key": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "value": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "bar"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import('foo', 1);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ImportExpression",
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "attributes": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

import('foo', 1,);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ImportExpression",
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "attributes": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
