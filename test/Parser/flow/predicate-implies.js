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

declare function foo(number): implies;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareFunction",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "FunctionTypeAnnotation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "FunctionTypeParam",
// CHECK-NEXT:                 "name": null,
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "optional": false
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "this": null,
// CHECK-NEXT:             "returnType": {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "implies"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "rest": null,
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "predicate": null
// CHECK-NEXT:     },

declare function foo(number): implies x is number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareFunction",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "FunctionTypeAnnotation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "FunctionTypeParam",
// CHECK-NEXT:                 "name": null,
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "optional": false
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "this": null,
// CHECK-NEXT:             "returnType": {
// CHECK-NEXT:               "type": "TypePredicate",
// CHECK-NEXT:               "parameterName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "implies"
// CHECK-NEXT:             },
// CHECK-NEXT:             "rest": null,
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "predicate": null
// CHECK-NEXT:     },

function foo(): implies {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "implies"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

function foo(): implies x is number {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
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
// CHECK-NEXT:           "kind": "implies"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

(function foo(): implies {});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "FunctionExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "implies"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "generator": false,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(function foo(): implies x is number {});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "FunctionExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
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
// CHECK-NEXT:             "kind": "implies"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "generator": false,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(x): implies => x !== null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NullLiteral"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "!=="
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "implies"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(x): implies x is number => x !== null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NullLiteral"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "!=="
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
// CHECK-NEXT:             "kind": "implies"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

async (x): implies => x !== null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NullLiteral"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "!=="
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "implies"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

async (x): implies x is number => x !== null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NullLiteral"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "!=="
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
// CHECK-NEXT:             "kind": "implies"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

type T = (x: mixed) => implies;

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
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "implies"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = (x: mixed) => implies | number;

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
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "implies"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = (x: mixed) => implies x is number;

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
// CHECK-NEXT:           "kind": "implies"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = (empty: mixed) => implies empty is number;

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
// CHECK-NEXT:               "name": "empty"
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
// CHECK-NEXT:             "name": "empty"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "kind": "implies"
// CHECK-NEXT:         },
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = () => implies this is number;
// CHECK-NEXT: {
// CHECK-NEXT:   "type": "TypeAlias",
// CHECK-NEXT:   "id": {
// CHECK-NEXT:     "type": "Identifier",
// CHECK-NEXT:     "name": "T"
// CHECK-NEXT:   },
// CHECK-NEXT:   "typeParameters": null,
// CHECK-NEXT:   "right": {
// CHECK-NEXT:     "type": "FunctionTypeAnnotation",
// CHECK-NEXT:     "params": [],
// CHECK-NEXT:     "this": null,
// CHECK-NEXT:     "returnType": {
// CHECK-NEXT:       "type": "TypePredicate",
// CHECK-NEXT:       "parameterName": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "this"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation"
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "implies"
// CHECK-NEXT:     },
// CHECK-NEXT:     "rest": null,
// CHECK-NEXT:     "typeParameters": null
// CHECK-NEXT:   }
// CHECK-NEXT: }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
