/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type A = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = string;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = any;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "AnyTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NullLiteralTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "VoidTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = mixed;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "MixedTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = boolean;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "BooleanTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = bool;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "BooleanTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = symbol;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "SymbolTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = B;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = B.C;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "QualifiedTypeIdentifier",
// CHECK-NEXT:           "qualification": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "C"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = 1;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:         "value": 1,
// CHECK-NEXT:         "raw": "1"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = 1n;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "BigIntLiteralTypeAnnotation",
// CHECK-NEXT:         "raw": "1n"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = -1;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:         "value": -1,
// CHECK-NEXT:         "raw": "-1"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = 'foo';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:         "value": "foo",
// CHECK-NEXT:         "raw": "'foo'"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = "foo";
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:         "value": "foo",
// CHECK-NEXT:         "raw": "\"foo\""
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = '\'foo';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:         "value": "'foo",
// CHECK-NEXT:         "raw": "'\\'foo'"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = '"foo';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:         "value": "\"foo",
// CHECK-NEXT:         "raw": "'\"foo'"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = "\"foo";
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:         "value": "\"foo",
// CHECK-NEXT:         "raw": "\"\\\"foo\""
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
