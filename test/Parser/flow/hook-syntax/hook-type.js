/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type A = hook () => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = hook (number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
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
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = hook (foo: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
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
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = hook (foo?: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
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
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = hook (foo: number, bar: number,) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
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
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = hook (foo: number, bar: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
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
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = hook (number, ...number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
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

type A = hook (...foo: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
// CHECK-NEXT:         "params": [],
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
// CHECK-NEXT:     },

type A = hook <T: string = 42>(...foo: number) => number;

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
// CHECK-NEXT:         "params": [],
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
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "T",
// CHECK-NEXT:               "bound": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "StringTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": {
// CHECK-NEXT:                 "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:                 "value": 42,
// CHECK-NEXT:                 "raw": "42"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = ?hook (...foo: number) => number;

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NullableTypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "HookTypeAnnotation",
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": {
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
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type s = hook (hook (number) => string) => hook (number) => string;

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "s"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": null,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "HookTypeAnnotation",
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "FunctionTypeParam",
// CHECK-NEXT:                   "name": null,
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "NumberTypeAnnotation"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "optional": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "HookTypeAnnotation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "FunctionTypeParam",
// CHECK-NEXT:               "name": null,
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "optional": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": null,
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type w = hook <T>(T) => hook (T) => T;

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "w"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "HookTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": null,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "HookTypeAnnotation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "FunctionTypeParam",
// CHECK-NEXT:               "name": null,
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "optional": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "T"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": null,
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "T",
// CHECK-NEXT:               "bound": null,
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
