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
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
