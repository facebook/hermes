/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -commonjs -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

import 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import * as Foo from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportNamespaceSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Foo"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import { x as y } from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import { x } from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import { abc, xyz as def, ghi , } from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "abc"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "abc"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value"
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "xyz"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "def"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value"
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "ghi"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "ghi"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import { catch as valid } from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportSpecifier",
// CHECK-NEXT:             "imported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "catch"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "valid"
// CHECK-NEXT:             },
// CHECK-NEXT:             "importKind": "value"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import defaultFoo from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "defaultFoo"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       },

import defaultBar, * as Bar from 'bar.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ImportDeclaration",
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDefaultSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "defaultBar"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportNamespaceSpecifier",
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Bar"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "bar.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "assertions": [],
// CHECK-NEXT:         "importKind": "value"
// CHECK-NEXT:       }

// CHECK-NEXT:     ]
// CHECK-NEXT:   },
// CHECK-NEXT:   "generator": false,
// CHECK-NEXT:   "async": false
// CHECK-NEXT: }
