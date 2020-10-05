/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -dump-source-location -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type T = number
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 6
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 7
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           389,
// CHECK-NEXT:           390
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 10
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 16
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           393,
// CHECK-NEXT:           399
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 16
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         384,
// CHECK-NEXT:         399
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

type T = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 69,
// CHECK-NEXT:             "column": 6
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 69,
// CHECK-NEXT:             "column": 7
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           2127,
// CHECK-NEXT:           2128
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 69,
// CHECK-NEXT:             "column": 10
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 69,
// CHECK-NEXT:             "column": 16
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           2131,
// CHECK-NEXT:           2137
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 69,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 69,
// CHECK-NEXT:           "column": 17
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         2122,
// CHECK-NEXT:         2138
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

declare opaque type T
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareOpaqueType",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 124,
// CHECK-NEXT:             "column": 21
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 124,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           3887,
// CHECK-NEXT:           3888
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "impltype": null,
// CHECK-NEXT:       "supertype": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 124,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 124,
// CHECK-NEXT:           "column": 22
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         3867,
// CHECK-NEXT:         3888
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

declare opaque type T<U>
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareOpaqueType",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 164,
// CHECK-NEXT:             "column": 21
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 164,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           5158,
// CHECK-NEXT:           5159
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "U",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null,
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 164,
// CHECK-NEXT:                 "column": 23
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 164,
// CHECK-NEXT:                 "column": 24
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "range": [
// CHECK-NEXT:               5160,
// CHECK-NEXT:               5161
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 164,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 164,
// CHECK-NEXT:             "column": 25
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           5159,
// CHECK-NEXT:           5162
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "impltype": null,
// CHECK-NEXT:       "supertype": null,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 164,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 164,
// CHECK-NEXT:           "column": 25
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         5138,
// CHECK-NEXT:         5162
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

declare opaque type T: string
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareOpaqueType",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 243,
// CHECK-NEXT:             "column": 21
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 243,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           7831,
// CHECK-NEXT:           7832
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "impltype": null,
// CHECK-NEXT:       "supertype": {
// CHECK-NEXT:         "type": "StringTypeAnnotation",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 243,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 243,
// CHECK-NEXT:             "column": 30
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "range": [
// CHECK-NEXT:           7834,
// CHECK-NEXT:           7840
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 243,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 243,
// CHECK-NEXT:           "column": 30
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "range": [
// CHECK-NEXT:         7811,
// CHECK-NEXT:         7840
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 243,
// CHECK-NEXT:       "column": 30
// CHECK-NEXT:     }
// CHECK-NEXT:   },
// CHECK-NEXT:   "range": [
// CHECK-NEXT:     384,
// CHECK-NEXT:     7840
// CHECK-NEXT:   ]
// CHECK-NEXT: }
