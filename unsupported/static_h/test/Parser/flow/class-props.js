/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -parse-flow -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

(class Foo {
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "superClass": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": [

  +a: number;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false,
// CHECK-NEXT:               "variance": {
// CHECK-NEXT:                 "type": "Variance",
// CHECK-NEXT:                 "kind": "plus"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },

  -y: number;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "y"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false,
// CHECK-NEXT:               "variance": {
// CHECK-NEXT:                 "type": "Variance",
// CHECK-NEXT:                 "kind": "minus"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },

  b: number;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "b"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false,
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },

  get: number;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "get"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false,
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },

  set: number;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "set"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false,
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },

  get foo() {};
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "foo"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "get",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },

  set foo(x) {};
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "foo"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "set",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             }

});
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

class C {
  async: number
}
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
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "async"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C {
  async
  : number
}
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
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "async"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false,
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C {
  declare +foo: string;
}
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
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": true,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "plus"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
