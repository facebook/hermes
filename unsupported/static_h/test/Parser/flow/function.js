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

type A = () => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = number => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": null,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": null,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (foo: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (foo?: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (foo: number, bar: number,) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (foo: number, bar: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (number, ...number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": null,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": {
// CHECK-NEXT:           "type": "FunctionTypeParam",
// CHECK-NEXT:           "name": null,
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (this & T) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": null,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "IntersectionTypeAnnotation",
// CHECK-NEXT:               "types": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "GenericTypeAnnotation",
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "this"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeParameters": null
// CHECK-NEXT:                 },
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "GenericTypeAnnotation",
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "T"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeParameters": null
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "VoidTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (...foo: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": {
// CHECK-NEXT:           "type": "FunctionTypeParam",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
