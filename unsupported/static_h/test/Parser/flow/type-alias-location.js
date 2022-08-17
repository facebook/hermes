/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -commonjs -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

type T = number

type T = number;

declare opaque type T

declare opaque type T<U>

declare opaque type T: string

export type T = string;

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
// CHECK-NEXT:           }
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
// CHECK-NEXT:           }
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
// CHECK-NEXT:         }
// CHECK-NEXT:       },

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "TypeAlias",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 6
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 7
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 10
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 16,
// CHECK-NEXT:               "column": 16
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 17
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareOpaqueType",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 18,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 18,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "impltype": null,
// CHECK-NEXT:         "supertype": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 18,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 18,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareOpaqueType",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             }
// CHECK-NEXT:           }
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
// CHECK-NEXT:                   "line": 20,
// CHECK-NEXT:                   "column": 23
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 20,
// CHECK-NEXT:                   "column": 24
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 20,
// CHECK-NEXT:               "column": 25
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "impltype": null,
// CHECK-NEXT:         "supertype": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 20,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 20,
// CHECK-NEXT:             "column": 25
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareOpaqueType",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "T",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 22
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "impltype": null,
// CHECK-NEXT:         "supertype": {
// CHECK-NEXT:           "type": "StringTypeAnnotation",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 24
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 30
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 22,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 22,
// CHECK-NEXT:             "column": 30
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },

// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "TypeAlias",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 24,
// CHECK-NEXT:                 "column": 13
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 24,
// CHECK-NEXT:                 "column": 14
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "StringTypeAnnotation",
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 24,
// CHECK-NEXT:                 "column": 17
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 24,
// CHECK-NEXT:                 "column": 23
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 24,
// CHECK-NEXT:               "column": 8
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 24,
// CHECK-NEXT:               "column": 24
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "type",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 24,
// CHECK-NEXT:             "column": 1
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 24,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
