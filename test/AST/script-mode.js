/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-transpiled-ast -typed -script %s | %FileCheck --match-full-lines -check-prefix SM %s
// RUN: %shermes -dump-transpiled-ast -typed %s | %FileCheck --match-full-lines -check-prefix MM %s

print("hi");

// Script mode should just contain the print.
// SM:{
// SM-NEXT:  "type": "Program",
// SM-NEXT:  "body": [
// SM-NEXT:    {
// SM-NEXT:      "type": "ExpressionStatement",
// SM-NEXT:      "expression": {
// SM-NEXT:        "type": "CallExpression",
// SM-NEXT:        "callee": {
// SM-NEXT:          "type": "Identifier",
// SM-NEXT:          "name": "print"
// SM-NEXT:        },
// SM-NEXT:        "arguments": [
// SM-NEXT:          {
// SM-NEXT:            "type": "StringLiteral",
// SM-NEXT:            "value": "hi"
// SM-NEXT:          }
// SM-NEXT:        ]
// SM-NEXT:      },
// SM-NEXT:      "directive": null
// SM-NEXT:    }
// SM-NEXT:  ]
// SM-NEXT:}

// Without -script, the program should be wrapped in a IIFE.
// MM:{
// MM-NEXT:  "type": "Program",
// MM-NEXT:  "body": [
// MM-NEXT:    {
// MM-NEXT:      "type": "ExpressionStatement",
// MM-NEXT:      "expression": {
// MM-NEXT:        "type": "CallExpression",
// MM-NEXT:        "callee": {
// MM-NEXT:          "type": "FunctionExpression",
// MM-NEXT:          "id": null,
// MM-NEXT:          "params": [
// MM-NEXT:            {
// MM-NEXT:              "type": "Identifier",
// MM-NEXT:              "name": "exports"
// MM-NEXT:            }
// MM-NEXT:          ],
// MM-NEXT:          "body": {
// MM-NEXT:            "type": "BlockStatement",
// MM-NEXT:            "body": [
// MM-NEXT:              {
// MM-NEXT:                "type": "ExpressionStatement",
// MM-NEXT:                "expression": {
// MM-NEXT:                  "type": "CallExpression",
// MM-NEXT:                  "callee": {
// MM-NEXT:                    "type": "Identifier",
// MM-NEXT:                    "name": "print"
// MM-NEXT:                  },
// MM-NEXT:                  "arguments": [
// MM-NEXT:                    {
// MM-NEXT:                      "type": "StringLiteral",
// MM-NEXT:                      "value": "hi"
// MM-NEXT:                    }
// MM-NEXT:                  ]
// MM-NEXT:                },
// MM-NEXT:                "directive": null
// MM-NEXT:              }
// MM-NEXT:            ]
// MM-NEXT:          },
// MM-NEXT:          "generator": false,
// MM-NEXT:          "async": false
// MM-NEXT:        },
// MM-NEXT:        "arguments": [
// MM-NEXT:          {
// MM-NEXT:            "type": "ObjectExpression",
// MM-NEXT:            "properties": []
// MM-NEXT:          }
// MM-NEXT:        ]
// MM-NEXT:      },
// MM-NEXT:      "directive": null
// MM-NEXT:    }
// MM-NEXT:  ]
// MM-NEXT:}
