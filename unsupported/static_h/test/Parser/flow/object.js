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

type T1 = {
  p1: number,
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
// CHECK-NEXT:               "name": "p1"
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

type T2 = {
  p1(): number,
  p2(t2: number): number,
  p3<T>(): number,
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
// CHECK-NEXT:               "name": "p1"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
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
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p2"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "FunctionTypeParam",
// CHECK-NEXT:                   "name": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "t2"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "NumberTypeAnnotation"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "optional": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
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
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TypeParameterDeclaration",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "TypeParameter",
// CHECK-NEXT:                     "name": "T",
// CHECK-NEXT:                     "bound": null,
// CHECK-NEXT:                     "variance": null,
// CHECK-NEXT:                     "default": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
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

type T3 = {
  [t1]: number,
  [p2: number]: number,
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
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeIndexer",
// CHECK-NEXT:             "id": null,
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "t1"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "variance": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeIndexer",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p2"
// CHECK-NEXT:             },
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "variance": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T4 = {
  [[p5]]: number,
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
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeInternalSlot",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p5"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "method": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T5 = {
  +p1: number,
  -p2: number,
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
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p1"
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
// CHECK-NEXT:               "kind": "plus"
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init"
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p2"
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
// CHECK-NEXT:               "kind": "minus"
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

type T6 = {
  (): number,
  (number): string,
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
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeCallProperty",
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeCallProperty",
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
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
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T7 = {
  ...p1,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T7"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeSpreadProperty",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "p1"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T8 = {||};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T8"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": true
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T9 = {|
  x: number
|};
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
// CHECK-NEXT:               "name": "x"
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
// CHECK-NEXT:         "exact": true
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T10 = {|
  x: number,
|};
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
// CHECK-NEXT:               "name": "x"
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
// CHECK-NEXT:         "exact": true
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T11 = {
  foo: (?string) => void,
};
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
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "FunctionTypeParam",
// CHECK-NEXT:                   "name": null,
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "NullableTypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "StringTypeAnnotation"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "optional": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "VoidTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
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

type T12 = {
  static: number,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T12"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "static"
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

type T13 = {
  static?: number,
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T13"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "static"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
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

type T14 = {
  proto: number,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T14"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "proto"
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
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
