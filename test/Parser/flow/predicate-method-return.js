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

declare class C { m(cb: (x: mixed) => x is T): void }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "DeclareClass",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "C"
// CHECK-NEXT:   },
// CHECK-NEXT:   "typeParameters": null,
// CHECK-NEXT:   "extends": [],
// CHECK-NEXT:   "implements": [],
// CHECK-NEXT:   "mixins": [],
// CHECK-NEXT:   "body": {
// CHECK-NEXT:     "type": "ObjectTypeAnnotation",
// CHECK-NEXT:     "properties": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ObjectTypeProperty",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionTypeAnnotation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "FunctionTypeParam",
// CHECK-NEXT:               "name": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "cb"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "FunctionTypeAnnotation",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "FunctionTypeParam",
// CHECK-NEXT:                     "name": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "x"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "MixedTypeAnnotation"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "optional": false
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "this": null,
// CHECK-NEXT:                 "returnType": {
// CHECK-NEXT:                   "type": "TypePredicate",
// CHECK-NEXT:                   "parameterName": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "GenericTypeAnnotation",
// CHECK-NEXT:                     "id": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "T"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeParameters": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "kind": null
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "rest": null,
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "optional": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "this": null,
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "VoidTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": null,
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "method": true,
// CHECK-NEXT:         "optional": false,
// CHECK-NEXT:         "static": false,
// CHECK-NEXT:         "proto": false,
// CHECK-NEXT:         "variance": null,
// CHECK-NEXT:         "kind": "init"
// CHECK-NEXT:       }
// CHECK-NEXT:     ],
// CHECK-NEXT:     "indexers": [],
// CHECK-NEXT:     "callProperties": [],
// CHECK-NEXT:     "internalSlots": [],
// CHECK-NEXT:     "inexact": false,
// CHECK-NEXT:     "exact": false
// CHECK-NEXT:   }
// CHECK-NEXT: },

declare class C { m(): x is T }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "DeclareClass",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "C"
// CHECK-NEXT:   },
// CHECK-NEXT:   "typeParameters": null,
// CHECK-NEXT:   "extends": [],
// CHECK-NEXT:   "implements": [],
// CHECK-NEXT:   "mixins": [],
// CHECK-NEXT:   "body": {
// CHECK-NEXT:     "type": "ObjectTypeAnnotation",
// CHECK-NEXT:     "properties": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ObjectTypeProperty",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionTypeAnnotation",
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "this": null,
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "TypePredicate",
// CHECK-NEXT:             "parameterName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": null,
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "method": true,
// CHECK-NEXT:         "optional": false,
// CHECK-NEXT:         "static": false,
// CHECK-NEXT:         "proto": false,
// CHECK-NEXT:         "variance": null,
// CHECK-NEXT:         "kind": "init"
// CHECK-NEXT:       }
// CHECK-NEXT:     ],
// CHECK-NEXT:     "indexers": [],
// CHECK-NEXT:     "callProperties": [],
// CHECK-NEXT:     "internalSlots": [],
// CHECK-NEXT:     "inexact": false,
// CHECK-NEXT:     "exact": false
// CHECK-NEXT:   }
// CHECK-NEXT: },

interface I { m(): this is T }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "InterfaceDeclaration",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "I"
// CHECK-NEXT:   },
// CHECK-NEXT:   "typeParameters": null,
// CHECK-NEXT:   "extends": [],
// CHECK-NEXT:   "body": {
// CHECK-NEXT:     "type": "ObjectTypeAnnotation",
// CHECK-NEXT:     "properties": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ObjectTypeProperty",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionTypeAnnotation",
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "this": null,
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "TypePredicate",
// CHECK-NEXT:             "parameterName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "this"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": null,
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "method": true,
// CHECK-NEXT:         "optional": false,
// CHECK-NEXT:         "static": false,
// CHECK-NEXT:         "proto": false,
// CHECK-NEXT:         "variance": null,
// CHECK-NEXT:         "kind": "init"
// CHECK-NEXT:       }
// CHECK-NEXT:     ],
// CHECK-NEXT:     "indexers": [],
// CHECK-NEXT:     "callProperties": [],
// CHECK-NEXT:     "internalSlots": [],
// CHECK-NEXT:     "inexact": false,
// CHECK-NEXT:     "exact": false
// CHECK-NEXT:   }
// CHECK-NEXT: },

type O = { m(): this is T }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "TypeAlias",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "O"
// CHECK-NEXT:   },
// CHECK-NEXT:   "typeParameters": null,
// CHECK-NEXT:   "right": {
// CHECK-NEXT:     "type": "ObjectTypeAnnotation",
// CHECK-NEXT:     "properties": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ObjectTypeProperty",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionTypeAnnotation",
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "this": null,
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "TypePredicate",
// CHECK-NEXT:             "parameterName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "this"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "rest": null,
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "method": true,
// CHECK-NEXT:         "optional": false,
// CHECK-NEXT:         "static": false,
// CHECK-NEXT:         "proto": false,
// CHECK-NEXT:         "variance": null,
// CHECK-NEXT:         "kind": "init"
// CHECK-NEXT:       }
// CHECK-NEXT:     ],
// CHECK-NEXT:     "indexers": [],
// CHECK-NEXT:     "callProperties": [],
// CHECK-NEXT:     "internalSlots": [],
// CHECK-NEXT:     "inexact": false,
// CHECK-NEXT:     "exact": false
// CHECK-NEXT:   }
// CHECK-NEXT: },

class C { m(): this is T {} }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "ClassDeclaration",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "C"
// CHECK-NEXT:   },
// CHECK-NEXT:   "superClass": null,
// CHECK-NEXT:   "body": {
// CHECK-NEXT:     "type": "ClassBody",
// CHECK-NEXT:     "body": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "MethodDefinition",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypePredicate",
// CHECK-NEXT:               "parameterName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "this"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": null
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "kind": "method",
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "static": false
// CHECK-NEXT:       }
// CHECK-NEXT:     ]
// CHECK-NEXT:   }
// CHECK-NEXT: },

class C { m(): implies x is T {} }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "ClassDeclaration",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "C"
// CHECK-NEXT:   },
// CHECK-NEXT:   "superClass": null,
// CHECK-NEXT:   "body": {
// CHECK-NEXT:     "type": "ClassBody",
// CHECK-NEXT:     "body": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "MethodDefinition",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypePredicate",
// CHECK-NEXT:               "parameterName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "implies"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "kind": "method",
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "static": false
// CHECK-NEXT:       }
// CHECK-NEXT:     ]
// CHECK-NEXT:   }
// CHECK-NEXT: },

class C { m(): implies this is T {} }
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "ClassDeclaration",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "C"
// CHECK-NEXT:   },
// CHECK-NEXT:   "superClass": null,
// CHECK-NEXT:   "body": {
// CHECK-NEXT:     "type": "ClassBody",
// CHECK-NEXT:     "body": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "MethodDefinition",
// CHECK-NEXT:         "key": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "m"
// CHECK-NEXT:         },
// CHECK-NEXT:         "value": {
// CHECK-NEXT:           "type": "FunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "returnType": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypePredicate",
// CHECK-NEXT:               "parameterName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "this"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "implies"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "kind": "method",
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "static": false
// CHECK-NEXT:       }
// CHECK-NEXT:     ]
// CHECK-NEXT:   }
// CHECK-NEXT: }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
