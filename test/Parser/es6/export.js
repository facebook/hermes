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

export * from 'foo.js';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportAllDeclaration",
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo.js"
// CHECK-NEXT:         },
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export * as bar from 'foo.js';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExportNamedDeclaration",
// CHECK-NEXT:       "declaration": null,
// CHECK-NEXT:       "specifiers": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ExportNamespaceSpecifier",
// CHECK-NEXT:           "exported": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "foo.js"
// CHECK-NEXT:       },
// CHECK-NEXT:       "exportKind": "value"
// CHECK-NEXT:     },

export default function myFun() { return 3; }
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportDefaultDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "FunctionDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "myFun"
// CHECK-NEXT:           },
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "ReturnStatement",
// CHECK-NEXT:                 "argument": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 3,
// CHECK-NEXT:                   "raw": "3"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         }
// CHECK-NEXT:       },

export var abc = 3;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "VariableDeclaration",
// CHECK-NEXT:           "kind": "var",
// CHECK-NEXT:           "declarations": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "VariableDeclarator",
// CHECK-NEXT:               "init": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 3,
// CHECK-NEXT:                 "raw": "3"
// CHECK-NEXT:               },
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "abc"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export function funDecl() {}
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "FunctionDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "funDecl"
// CHECK-NEXT:           },
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export let letValue = 123;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "VariableDeclaration",
// CHECK-NEXT:           "kind": "let",
// CHECK-NEXT:           "declarations": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "VariableDeclarator",
// CHECK-NEXT:               "init": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 123,
// CHECK-NEXT:                 "raw": "123"
// CHECK-NEXT:               },
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "letValue"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export const constValue = 321;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "VariableDeclaration",
// CHECK-NEXT:           "kind": "const",
// CHECK-NEXT:           "declarations": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "VariableDeclarator",
// CHECK-NEXT:               "init": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 321,
// CHECK-NEXT:                 "raw": "321"
// CHECK-NEXT:               },
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "constValue"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export { };
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export { x };
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export { y , };
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export { a as b , c , last };
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "c"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "c"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "last"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "last"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       }

// CHECK-NEXT:     ]
// CHECK-NEXT:   },
