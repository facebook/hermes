/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:    "type": "Program",
// CHECK-NEXT:    "body": [

declare namespace NS { import type T from "TM"; }
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "DeclareNamespace",
// CHECK-NEXT:        "id": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "NS"
// CHECK-NEXT:        },
// CHECK-NEXT:        "body": {
// CHECK-NEXT:          "type": "BlockStatement",
// CHECK-NEXT:          "body": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ImportDeclaration",
// CHECK-NEXT:              "specifiers": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "ImportDefaultSpecifier",
// CHECK-NEXT:                  "local": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "T"
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "source": {
// CHECK-NEXT:                "type": "StringLiteral",
// CHECK-NEXT:                "value": "TM"
// CHECK-NEXT:              },
// CHECK-NEXT:              "assertions": [],
// CHECK-NEXT:              "importKind": "type"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        }
// CHECK-NEXT:      },

// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "DeclareNamespace",
// CHECK-NEXT:        "id": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "both_type_and_value"
// CHECK-NEXT:        },
// CHECK-NEXT:        "body": {
// CHECK-NEXT:          "type": "BlockStatement",
// CHECK-NEXT:          "body": [
declare namespace both_type_and_value {
  declare export const bar1: number;
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "DeclareExportDeclaration",
// CHECK-NEXT:              "declaration": {
// CHECK-NEXT:                "type": "DeclareVariable",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "bar1",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "TypeAnnotation",
// CHECK-NEXT:                    "typeAnnotation": {
// CHECK-NEXT:                      "type": "NumberTypeAnnotation"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "kind": "const"
// CHECK-NEXT:              },
// CHECK-NEXT:              "specifiers": [],
// CHECK-NEXT:              "source": null,
// CHECK-NEXT:              "default": false
// CHECK-NEXT:            },
  declare const bar2: boolean;
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "DeclareVariable",
// CHECK-NEXT:              "id": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "bar2",
// CHECK-NEXT:                "typeAnnotation": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "BooleanTypeAnnotation"
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "kind": "const"
// CHECK-NEXT:            },
  declare export type Baz = string;
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "DeclareExportDeclaration",
// CHECK-NEXT:              "declaration": {
// CHECK-NEXT:                "type": "TypeAlias",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Baz"
// CHECK-NEXT:                },
// CHECK-NEXT:                "typeParameters": null,
// CHECK-NEXT:                "right": {
// CHECK-NEXT:                  "type": "StringTypeAnnotation"
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "specifiers": [],
// CHECK-NEXT:              "source": null,
// CHECK-NEXT:              "default": false
// CHECK-NEXT:            },
  import React from 'react';
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ImportDeclaration",
// CHECK-NEXT:              "specifiers": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "ImportDefaultSpecifier",
// CHECK-NEXT:                  "local": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "React"
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "source": {
// CHECK-NEXT:                "type": "StringLiteral",
// CHECK-NEXT:                "value": "react"
// CHECK-NEXT:              },
// CHECK-NEXT:              "assertions": [],
// CHECK-NEXT:              "importKind": "value"
// CHECK-NEXT:            }
}
// CHECK-NEXT:          ]
// CHECK-NEXT:        }
// CHECK-NEXT:      }
// CHECK-NEXT:    ]
// CHECK-NEXT:  }
