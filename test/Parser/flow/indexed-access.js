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

type A = Obj['a'];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "IndexedAccessType",
// CHECK-NEXT:         "objectType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Obj"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "indexType": {
// CHECK-NEXT:           "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:           "value": "a",
// CHECK-NEXT:           "raw": "'a'"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type B = Array<string>[number];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "B"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "IndexedAccessType",
// CHECK-NEXT:         "objectType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Array"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "indexType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type C = Obj['bar'][foo]['boz'];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "IndexedAccessType",
// CHECK-NEXT:         "objectType": {
// CHECK-NEXT:           "type": "IndexedAccessType",
// CHECK-NEXT:           "objectType": {
// CHECK-NEXT:             "type": "IndexedAccessType",
// CHECK-NEXT:             "objectType": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "Obj"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "indexType": {
// CHECK-NEXT:               "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:               "value": "bar",
// CHECK-NEXT:               "raw": "'bar'"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "indexType": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:            "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "indexType": {
// CHECK-NEXT:           "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:           "value": "boz",
// CHECK-NEXT:           "raw": "'boz'"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type D = (Obj['bar'])['baz'];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "D"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "IndexedAccessType",
// CHECK-NEXT:         "objectType": {
// CHECK-NEXT:           "type": "IndexedAccessType",
// CHECK-NEXT:           "objectType": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Obj"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "indexType": {
// CHECK-NEXT:             "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:             "value": "bar",
// CHECK-NEXT:             "raw": "'bar'"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "indexType": {
// CHECK-NEXT:           "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:           "value": "baz",
// CHECK-NEXT:           "raw": "'baz'"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type E = Obj['bar'][];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "E"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ArrayTypeAnnotation",
// CHECK-NEXT:         "elementType": {
// CHECK-NEXT:           "type": "IndexedAccessType",
// CHECK-NEXT:           "objectType": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Obj"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "indexType": {
// CHECK-NEXT:             "type": "StringLiteralTypeAnnotation",
// CHECK-NEXT:             "value": "bar",
// CHECK-NEXT:             "raw": "'bar'"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
