/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type A = (x: number, y: string) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "y",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSStringKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSVoidKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = new (x: number, y: string) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSConstructorType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "y",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSStringKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSVoidKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = <T>(x: number, y: string) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "y",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSStringKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSVoidKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TSTypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TSTypeParameter",
// CHECK-NEXT:               "name": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "constraint": null,
// CHECK-NEXT:               "default": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (x?: number) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSVoidKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (x: number, ...y: number) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RestElement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSTypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSNumberKeyword"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSVoidKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (...y: number[]) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RestElement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSTypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSArrayType",
// CHECK-NEXT:                   "elementType": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSVoidKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = ({x}: number) => number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ObjectPattern",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSNumberKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
