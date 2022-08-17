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

type A = {
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeLiteral",
// CHECK-NEXT:         "members": [

  p1: number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p1"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           },

  p2,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p2"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": null,
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           },

  p3(a1: number),
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSMethodSignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a1",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "returnType": null,
// CHECK-NEXT:             "computed": false
// CHECK-NEXT:           },

  (a1: number): number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSCallSignatureDeclaration",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a1",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "returnType": {
// CHECK-NEXT:               "type": "TSNumberKeyword"
// CHECK-NEXT:             }
// CHECK-NEXT:           },

  [p4],
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p4"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": null,
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "computed": true,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           },

  [p5]: number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p5"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "computed": true,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           },

  [p6](a1: number),
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSMethodSignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p6"
// CHECK-NEXT:             },
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a1",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "returnType": null,
// CHECK-NEXT:             "computed": true
// CHECK-NEXT:           },

  [p7](a1: number): number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSMethodSignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p7"
// CHECK-NEXT:             },
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a1",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "returnType": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": true
// CHECK-NEXT:           },

  p8?: number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p8"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": true,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           },

  [p9: number]: number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSIndexSignature",
// CHECK-NEXT:             "parameters": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "p9",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },

  [p10: number, p11: number]: number,
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSIndexSignature",
// CHECK-NEXT:             "parameters": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "p10",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "p11",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }

};

// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
