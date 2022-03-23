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

type A = Obj?.['a'];
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "TypeAlias",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "A"
// CHECK-NEXT:      },
// CHECK-NEXT:      "typeParameters": null,
// CHECK-NEXT:      "right": {
// CHECK-NEXT:        "type": "OptionalIndexedAccessType",
// CHECK-NEXT:        "objectType": {
// CHECK-NEXT:          "type": "GenericTypeAnnotation",
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "Obj"
// CHECK-NEXT:          },
// CHECK-NEXT:          "typeParameters": null
// CHECK-NEXT:        },
// CHECK-NEXT:        "indexType": {
// CHECK-NEXT:          "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:          "value": "a",
// CHECK-NEXT:          "raw": "'a'"
// CHECK-NEXT:        },
// CHECK-NEXT:        "optional": true
// CHECK-NEXT:      }
// CHECK-NEXT:    },

type B = Obj['a']?.['b'];
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "TypeAlias",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "B"
// CHECK-NEXT:      },
// CHECK-NEXT:      "typeParameters": null,
// CHECK-NEXT:      "right": {
// CHECK-NEXT:        "type": "OptionalIndexedAccessType",
// CHECK-NEXT:        "objectType": {
// CHECK-NEXT:          "type": "IndexedAccessType",
// CHECK-NEXT:          "objectType": {
// CHECK-NEXT:            "type": "GenericTypeAnnotation",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "Obj"
// CHECK-NEXT:            },
// CHECK-NEXT:            "typeParameters": null
// CHECK-NEXT:          },
// CHECK-NEXT:          "indexType": {
// CHECK-NEXT:            "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:            "value": "a",
// CHECK-NEXT:            "raw": "'a'"
// CHECK-NEXT:          }
// CHECK-NEXT:        },
// CHECK-NEXT:        "indexType": {
// CHECK-NEXT:          "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:          "value": "b",
// CHECK-NEXT:          "raw": "'b'"
// CHECK-NEXT:        },
// CHECK-NEXT:        "optional": true
// CHECK-NEXT:      }
// CHECK-NEXT:    },

type C = Obj?.['a']['b'];
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "TypeAlias",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "C"
// CHECK-NEXT:      },
// CHECK-NEXT:      "typeParameters": null,
// CHECK-NEXT:      "right": {
// CHECK-NEXT:        "type": "OptionalIndexedAccessType",
// CHECK-NEXT:        "objectType": {
// CHECK-NEXT:          "type": "OptionalIndexedAccessType",
// CHECK-NEXT:          "objectType": {
// CHECK-NEXT:            "type": "GenericTypeAnnotation",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "Obj"
// CHECK-NEXT:            },
// CHECK-NEXT:            "typeParameters": null
// CHECK-NEXT:          },
// CHECK-NEXT:          "indexType": {
// CHECK-NEXT:            "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:            "value": "a",
// CHECK-NEXT:            "raw": "'a'"
// CHECK-NEXT:          },
// CHECK-NEXT:          "optional": true
// CHECK-NEXT:        },
// CHECK-NEXT:        "indexType": {
// CHECK-NEXT:          "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:          "value": "b",
// CHECK-NEXT:          "raw": "'b'"
// CHECK-NEXT:        },
// CHECK-NEXT:        "optional": false
// CHECK-NEXT:      }
// CHECK-NEXT:    },

type D = Obj?.['a']?.['b'];
// CHECK-NEXT:   {
// CHECK-NEXT:      "type": "TypeAlias",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "D"
// CHECK-NEXT:      },
// CHECK-NEXT:      "typeParameters": null,
// CHECK-NEXT:      "right": {
// CHECK-NEXT:        "type": "OptionalIndexedAccessType",
// CHECK-NEXT:        "objectType": {
// CHECK-NEXT:          "type": "OptionalIndexedAccessType",
// CHECK-NEXT:          "objectType": {
// CHECK-NEXT:            "type": "GenericTypeAnnotation",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "Obj"
// CHECK-NEXT:            },
// CHECK-NEXT:            "typeParameters": null
// CHECK-NEXT:          },
// CHECK-NEXT:          "indexType": {
// CHECK-NEXT:            "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:            "value": "a",
// CHECK-NEXT:            "raw": "'a'"
// CHECK-NEXT:          },
// CHECK-NEXT:          "optional": true
// CHECK-NEXT:        },
// CHECK-NEXT:        "indexType": {
// CHECK-NEXT:          "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:          "value": "b",
// CHECK-NEXT:          "raw": "'b'"
// CHECK-NEXT:        },
// CHECK-NEXT:        "optional": true
// CHECK-NEXT:      }
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
