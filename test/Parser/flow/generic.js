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

type A = B;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = B<{x: number}>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ObjectTypeAnnotation",
// CHECK-NEXT:               "properties": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "ObjectTypeProperty",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "NumberTypeAnnotation"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "optional": false,
// CHECK-NEXT:                   "static": false,
// CHECK-NEXT:                   "proto": false,
// CHECK-NEXT:                   "variance": null,
// CHECK-NEXT:                   "kind": "init"
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "indexers": [],
// CHECK-NEXT:               "callProperties": [],
// CHECK-NEXT:               "internalSlots": [],
// CHECK-NEXT:               "inexact": false,
// CHECK-NEXT:               "exact": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = B.C;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "QualifiedTypeIdentifier",
// CHECK-NEXT:           "qualification": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "C"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A<T> = B<C>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "C"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A<T> = B<C<D>>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "C"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TypeParameterInstantiation",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "GenericTypeAnnotation",
// CHECK-NEXT:                     "id": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "D"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeParameters": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A<T> = B<C<D<E>>>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "C"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TypeParameterInstantiation",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "GenericTypeAnnotation",
// CHECK-NEXT:                     "id": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "D"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeParameters": {
// CHECK-NEXT:                       "type": "TypeParameterInstantiation",
// CHECK-NEXT:                       "params": [
// CHECK-NEXT:                         {
// CHECK-NEXT:                           "type": "GenericTypeAnnotation",
// CHECK-NEXT:                           "id": {
// CHECK-NEXT:                             "type": "Identifier",
// CHECK-NEXT:                             "name": "E"
// CHECK-NEXT:                           },
// CHECK-NEXT:                           "typeParameters": null
// CHECK-NEXT:                         }
// CHECK-NEXT:                       ]
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = this & T;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "IntersectionTypeAnnotation",
// CHECK-NEXT:         "types": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "this"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "T"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
