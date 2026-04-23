/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

// readonly as variance on object type property
type T1 = {
  readonly foo: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
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

// readonly as a property name (not variance) when followed by colon
type T2 = {
  readonly: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T2"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "readonly"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
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

// readonly as a property name when followed by question mark
type T3 = {
  readonly?: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T3"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "readonly"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": true,
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

// readonly on string key
type T4 = {
  readonly "foo": string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T4"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
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

// readonly on numeric key
type T5 = {
  readonly 0: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T5"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 0,
// CHECK-NEXT:               "raw": "0"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
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

// readonly on indexer
type T6 = {
  readonly [string]: mixed,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T6"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeIndexer",
// CHECK-NEXT:             "id": null,
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "MixedTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// readonly on tuple labeled element
type T7 = [readonly label: string];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T7"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "TupleTypeAnnotation",
// CHECK-NEXT:         "elementTypes": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TupleTypeLabeledElement",
// CHECK-NEXT:             "label": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "label"
// CHECK-NEXT:             },
// CHECK-NEXT:             "elementType": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "inexact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// readonly on class property
class C {
  readonly x: number;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// readonly with reserved-word property names
// `+with: string` parses; `readonly with: string` should too.
type T8 = {
  readonly with: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T8"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "with"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
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

type T9 = {
  readonly enum: number,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T9"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "enum"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
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

type T10 = {
  readonly default?: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T10"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "default"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": true,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
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

type T11 = {
  readonly new: string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T11"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "new"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "readonly"
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK:        ]
// CHECK-NEXT: }
