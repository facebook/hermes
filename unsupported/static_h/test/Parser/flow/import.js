/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

import typeof x from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "typeof"
// CHECK-NEXT:       },

import type x from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "type"
// CHECK-NEXT:       },

import { type type } from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "type"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import { type type as type } from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "type"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import { type T as U } from 'foo';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ImportDeclaration",
// CHECK-NEXT:       "specifiers": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ImportSpecifier",
// CHECK-NEXT:           "imported": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "local": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "U"
// CHECK-NEXT:           },
// CHECK-NEXT:           "importKind": "type"
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "assertions": [],
// CHECK-NEXT:       "importKind": "value"
// CHECK-NEXT:     },

import type from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       }
