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

type T = A extends B ? C : D;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "A"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "C"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "D"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },
type T = number extends string ? boolean : null;

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "StringTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "BooleanTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "NullLiteralTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },
type T = string | number extends string ? boolean : null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "StringTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "BooleanTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "NullLiteralTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = | number extends | number ? | number : | number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = string | number extends string | number ? string | number : string | number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = number extends string ? number extends string ? 1 : 2 : number extends string ? 1 : 2;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "StringTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:           "checkType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "extendsType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "trueType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 1,
// CHECK-NEXT:             "raw": "1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "falseType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:           "checkType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "extendsType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "trueType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 1,
// CHECK-NEXT:             "raw": "1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "falseType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = (number extends string ? 1 : 2) extends (number extends string ? 1 : 2) ? boolean : null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:           "checkType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "extendsType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "trueType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 1,
// CHECK-NEXT:             "raw": "1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "falseType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:           "checkType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "extendsType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "trueType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 1,
// CHECK-NEXT:             "raw": "1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "falseType": {
// CHECK-NEXT:             "type": "NumberLiteralTypeAnnotation",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "BooleanTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "NullLiteralTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = number extends (string extends number ? string : number) ? string : number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:           "checkType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "extendsType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "trueType": {
// CHECK-NEXT:             "type": "StringTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "falseType": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "StringTypeAnnotation"
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type ArrayElement<T extends A> = T extends Array<infer E> ? E : empty;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "ArrayElement"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "bound": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "A"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null,
// CHECK-NEXT:             "usesExtendsBound": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:         "checkType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "extendsType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Array"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "InferTypeAnnotation",
// CHECK-NEXT:                 "typeParameter": {
// CHECK-NEXT:                   "type": "TypeParameter",
// CHECK-NEXT:                   "name": "E",
// CHECK-NEXT:                   "bound": null,
// CHECK-NEXT:                   "variance": null,
// CHECK-NEXT:                   "default": null,
// CHECK-NEXT:                   "usesExtendsBound": true
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "trueType": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "E"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "falseType": {
// CHECK-NEXT:           "type": "EmptyTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

let x : number extends infer T extends number ? string : number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": null,
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:                 "checkType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "extendsType": {
// CHECK-NEXT:                   "type": "InferTypeAnnotation",
// CHECK-NEXT:                   "typeParameter": {
// CHECK-NEXT:                     "type": "TypeParameter",
// CHECK-NEXT:                     "name": "T",
// CHECK-NEXT:                     "bound": {
// CHECK-NEXT:                       "type": "NumberTypeAnnotation"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "variance": null,
// CHECK-NEXT:                     "default": null,
// CHECK-NEXT:                     "usesExtendsBound": true
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "trueType": {
// CHECK-NEXT:                   "type": "StringTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "falseType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

let x : infer T extends number ? string : number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": null,
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:                 "checkType": {
// CHECK-NEXT:                   "type": "InferTypeAnnotation",
// CHECK-NEXT:                   "typeParameter": {
// CHECK-NEXT:                     "type": "TypeParameter",
// CHECK-NEXT:                     "name": "T",
// CHECK-NEXT:                     "bound": null,
// CHECK-NEXT:                     "variance": null,
// CHECK-NEXT:                     "default": null,
// CHECK-NEXT:                     "usesExtendsBound": true
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "extendsType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "trueType": {
// CHECK-NEXT:                   "type": "StringTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "falseType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

let x : number extends (infer T extends number ? string : number) ? string : number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": null,
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:                 "checkType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "extendsType": {
// CHECK-NEXT:                   "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:                   "checkType": {
// CHECK-NEXT:                     "type": "InferTypeAnnotation",
// CHECK-NEXT:                     "typeParameter": {
// CHECK-NEXT:                       "type": "TypeParameter",
// CHECK-NEXT:                       "name": "T",
// CHECK-NEXT:                       "bound": null,
// CHECK-NEXT:                       "variance": null,
// CHECK-NEXT:                       "default": null,
// CHECK-NEXT:                       "usesExtendsBound": true
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "extendsType": {
// CHECK-NEXT:                     "type": "NumberTypeAnnotation"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "trueType": {
// CHECK-NEXT:                     "type": "StringTypeAnnotation"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "falseType": {
// CHECK-NEXT:                     "type": "NumberTypeAnnotation"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "trueType": {
// CHECK-NEXT:                   "type": "StringTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "falseType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

let x: infer A extends (infer B extends infer C ? infer D : infer E) ? string : number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": null,
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:                 "checkType": {
// CHECK-NEXT:                   "type": "InferTypeAnnotation",
// CHECK-NEXT:                   "typeParameter": {
// CHECK-NEXT:                     "type": "TypeParameter",
// CHECK-NEXT:                     "name": "A",
// CHECK-NEXT:                     "bound": null,
// CHECK-NEXT:                     "variance": null,
// CHECK-NEXT:                     "default": null,
// CHECK-NEXT:                     "usesExtendsBound": true
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "extendsType": {
// CHECK-NEXT:                   "type": "ConditionalTypeAnnotation",
// CHECK-NEXT:                   "checkType": {
// CHECK-NEXT:                     "type": "InferTypeAnnotation",
// CHECK-NEXT:                     "typeParameter": {
// CHECK-NEXT:                       "type": "TypeParameter",
// CHECK-NEXT:                       "name": "B",
// CHECK-NEXT:                       "bound": null,
// CHECK-NEXT:                       "variance": null,
// CHECK-NEXT:                       "default": null,
// CHECK-NEXT:                       "usesExtendsBound": true
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "extendsType": {
// CHECK-NEXT:                     "type": "InferTypeAnnotation",
// CHECK-NEXT:                     "typeParameter": {
// CHECK-NEXT:                       "type": "TypeParameter",
// CHECK-NEXT:                       "name": "C",
// CHECK-NEXT:                       "bound": null,
// CHECK-NEXT:                       "variance": null,
// CHECK-NEXT:                       "default": null,
// CHECK-NEXT:                       "usesExtendsBound": true
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "trueType": {
// CHECK-NEXT:                     "type": "InferTypeAnnotation",
// CHECK-NEXT:                     "typeParameter": {
// CHECK-NEXT:                       "type": "TypeParameter",
// CHECK-NEXT:                       "name": "D",
// CHECK-NEXT:                       "bound": null,
// CHECK-NEXT:                       "variance": null,
// CHECK-NEXT:                       "default": null,
// CHECK-NEXT:                       "usesExtendsBound": true
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "falseType": {
// CHECK-NEXT:                     "type": "InferTypeAnnotation",
// CHECK-NEXT:                     "typeParameter": {
// CHECK-NEXT:                       "type": "TypeParameter",
// CHECK-NEXT:                       "name": "E",
// CHECK-NEXT:                       "bound": null,
// CHECK-NEXT:                       "variance": null,
// CHECK-NEXT:                       "default": null,
// CHECK-NEXT:                       "usesExtendsBound": true
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "trueType": {
// CHECK-NEXT:                   "type": "StringTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "falseType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
