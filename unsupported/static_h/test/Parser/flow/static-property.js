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

declare class C {
  static: string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
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

declare class C {
  +static: string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
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
// CHECK-NEXT:               "type": "StringTypeAnnotation"
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
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare class C {
  static foo: string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
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
// CHECK-NEXT:             "static": true,
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

declare class C {
  static +foo: string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
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
// CHECK-NEXT:             "static": true,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "plus"
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

declare class C {
  static foo(): void
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
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
// CHECK-NEXT:             "static": true,
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

declare class C {
  static (): void
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
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
// CHECK-NEXT:                 "type": "VoidTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare class C {
  static [string]: string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
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
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": true,
// CHECK-NEXT:             "variance": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare class C {
  static [[foo]]: string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectTypeInternalSlot",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": true,
// CHECK-NEXT:             "method": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare class C {
  static get foo(): string
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [],
// CHECK-NEXT:       "body": {
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
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "static": true,
// CHECK-NEXT:             "proto": false,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "kind": "get"
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
