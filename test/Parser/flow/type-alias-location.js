/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -commonjs -dump-ast -dump-source-location -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

type T = number
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "TypeAlias",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             420,
// CHECK-NEXT:             421
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 10
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 14,
// CHECK-NEXT:               "column": 16
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             424,
// CHECK-NEXT:             430
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 16
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           415,
// CHECK-NEXT:           430
// CHECK-NEXT:         ]
// CHECK-NEXT:       },

type T = number;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "TypeAlias",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 69,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 69,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             2264,
// CHECK-NEXT:             2265
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 69,
// CHECK-NEXT:               "column": 10
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 69,
// CHECK-NEXT:               "column": 16
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             2268,
// CHECK-NEXT:             2274
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 69,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 69,
// CHECK-NEXT:             "column": 17
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           2259,
// CHECK-NEXT:           2275
// CHECK-NEXT:         ]
// CHECK-NEXT:       },

declare opaque type T
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareOpaqueType",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 124,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 124,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             4130,
// CHECK-NEXT:             4131
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "impltype": null,
// CHECK-NEXT:         "supertype": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 124,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 124,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           4110,
// CHECK-NEXT:           4131
// CHECK-NEXT:         ]
// CHECK-NEXT:       },

declare opaque type T<U>
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareOpaqueType",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 164,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 164,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             5477,
// CHECK-NEXT:             5478
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "U",
// CHECK-NEXT:               "bound": null,
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": null,
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 164,
// CHECK-NEXT:                   "column": 23
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 164,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "range": [
// CHECK-NEXT:                 5479,
// CHECK-NEXT:                 5480
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 164,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 164,
// CHECK-NEXT:               "column": 25
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             5478,
// CHECK-NEXT:             5481
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "impltype": null,
// CHECK-NEXT:         "supertype": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 164,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 164,
// CHECK-NEXT:             "column": 25
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           5457,
// CHECK-NEXT:           5481
// CHECK-NEXT:         ]
// CHECK-NEXT:       },

declare opaque type T: string
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareOpaqueType",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 243,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 243,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             8304,
// CHECK-NEXT:             8305
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "impltype": null,
// CHECK-NEXT:         "supertype": {
// CHECK-NEXT:           "type": "StringTypeAnnotation",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 243,
// CHECK-NEXT:               "column": 24
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 243,
// CHECK-NEXT:               "column": 30
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             8307,
// CHECK-NEXT:             8313
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 243,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 243,
// CHECK-NEXT:             "column": 30
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           8284,
// CHECK-NEXT:           8313
// CHECK-NEXT:         ]
// CHECK-NEXT:       },

export type T = string;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "TypeAlias",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 299,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 299,
// CHECK-NEXT:                 "column": 14
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               10221,
// CHECK-NEXT:               10222
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "StringTypeAnnotation",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 299,
// CHECK-NEXT:                 "column": 17
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 299,
// CHECK-NEXT:                 "column": 23
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               10225,
// CHECK-NEXT:               10231
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 299,
// CHECK-NEXT:               "column": 8
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 299,
// CHECK-NEXT:               "column": 24
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "range": [
// CHECK-NEXT:             10216,
// CHECK-NEXT:             10232
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "type",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 299,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 299,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           10209,
// CHECK-NEXT:           10232
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
