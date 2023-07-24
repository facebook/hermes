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

function foo(x: mixed): asserts x is number {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "MixedTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TypePredicate",
// CHECK-NEXT:           "parameterName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "asserts": true
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

function foo(x: mixed): asserts x {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "MixedTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TypePredicate",
// CHECK-NEXT:           "parameterName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": null,
// CHECK-NEXT:           "asserts": true
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

(x: mixed): asserts x is number => true;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "MixedTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BooleanLiteral",
// CHECK-NEXT:           "value": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypePredicate",
// CHECK-NEXT:             "parameterName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "asserts": true
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(x: mixed): asserts x => true;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "MixedTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BooleanLiteral",
// CHECK-NEXT:           "value": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypePredicate",
// CHECK-NEXT:             "parameterName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": null,
// CHECK-NEXT:             "asserts": true
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

class C { m(): asserts x is D { return x !== null } }
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
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "m"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "BinaryExpression",
// CHECK-NEXT:                       "left": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "x"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "right": {
// CHECK-NEXT:                         "type": "NullLiteral"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "operator": "!=="
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TypePredicate",
// CHECK-NEXT:                   "parameterName": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "GenericTypeAnnotation",
// CHECK-NEXT:                     "id": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "D"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeParameters": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "asserts": true
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C { m(): asserts x { return x !== null } }
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
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "m"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "BinaryExpression",
// CHECK-NEXT:                       "left": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "x"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "right": {
// CHECK-NEXT:                         "type": "NullLiteral"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "operator": "!=="
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TypePredicate",
// CHECK-NEXT:                   "parameterName": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeAnnotation": null,
// CHECK-NEXT:                   "asserts": true
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = (x: mixed) => asserts x is number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "MixedTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypePredicate",
// CHECK-NEXT:           "parameterName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "asserts": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = (x: mixed) => asserts x;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "FunctionTypeAnnotation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "FunctionTypeParam",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "MixedTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "this": null,
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypePredicate",
// CHECK-NEXT:           "parameterName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": null,
// CHECK-NEXT:           "asserts": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
