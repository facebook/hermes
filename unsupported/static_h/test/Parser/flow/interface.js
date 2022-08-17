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

interface T {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T<U> {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "U",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T<U> extends S {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "U",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "extends": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "InterfaceExtends",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "S"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T extends S<U> {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "InterfaceExtends",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "S"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "U"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T<U> extends S1, S2 {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "U",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "extends": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "InterfaceExtends",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "S1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "InterfaceExtends",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "S2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare interface T {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareInterface",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T {
  static(): void
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "static"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "VoidTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": true,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "kind": "init"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T {
  proto(): void
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "proto"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "VoidTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": true,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "kind": "init"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface T {
  @@iterator: number,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "InterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "@@iterator"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "kind": "init"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = interface {};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "InterfaceTypeAnnotation",
// CHECK-NEXT:         "extends": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ObjectTypeAnnotation",
// CHECK-NEXT:           "properties": [],
// CHECK-NEXT:           "indexers": [],
// CHECK-NEXT:           "callProperties": [],
// CHECK-NEXT:           "internalSlots": [],
// CHECK-NEXT:           "inexact": false,
// CHECK-NEXT:           "exact": false
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

let x: interface {x: number} = {x: 3};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ObjectExpression",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 3,
// CHECK-NEXT:                   "raw": "3"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": false
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "InterfaceTypeAnnotation",
// CHECK-NEXT:                 "extends": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "ObjectTypeAnnotation",
// CHECK-NEXT:                   "properties": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "ObjectTypeProperty",
// CHECK-NEXT:                       "key": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "x"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "value": {
// CHECK-NEXT:                         "type": "NumberTypeAnnotation"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "method": false,
// CHECK-NEXT:                       "optional": false,
// CHECK-NEXT:                       "static": false,
// CHECK-NEXT:                       "proto": false,
// CHECK-NEXT:                       "variance": null,
// CHECK-NEXT:                       "kind": "init"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ],
// CHECK-NEXT:                   "indexers": [],
// CHECK-NEXT:                   "callProperties": [],
// CHECK-NEXT:                   "internalSlots": [],
// CHECK-NEXT:                   "inexact": false,
// CHECK-NEXT:                   "exact": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
