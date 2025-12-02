/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

{
  record R {
    static: number,
    async: number,
    static async: number = 1,
  }
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "BlockStatement",
// CHECK-NEXT:       "body": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "RecordDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "R"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "implements": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "RecordDeclarationBody",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "RecordDeclarationProperty",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "static"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "defaultValue": null
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "RecordDeclarationProperty",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "defaultValue": null
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "RecordDeclarationStaticProperty",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 1,
// CHECK-NEXT:                   "raw": "1"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

{
  record R {
    static() {}
    async() {}
    static async() {}
  }
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "BlockStatement",
// CHECK-NEXT:       "body": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "RecordDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "R"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "implements": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "RecordDeclarationBody",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "static"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": false
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": false
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

{
  record R {
    static<T>() {}
    async<T>() {}
    static async<T>() {}
  }
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "BlockStatement",
// CHECK-NEXT:       "body": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "RecordDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "R"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "implements": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "RecordDeclarationBody",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "static"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeParameters": {
// CHECK-NEXT:                     "type": "TypeParameterDeclaration",
// CHECK-NEXT:                     "params": [
// CHECK-NEXT:                       {
// CHECK-NEXT:                         "type": "TypeParameter",
// CHECK-NEXT:                         "name": "T",
// CHECK-NEXT:                         "bound": null,
// CHECK-NEXT:                         "variance": null,
// CHECK-NEXT:                         "default": null
// CHECK-NEXT:                       }
// CHECK-NEXT:                     ]
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": false
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeParameters": {
// CHECK-NEXT:                     "type": "TypeParameterDeclaration",
// CHECK-NEXT:                     "params": [
// CHECK-NEXT:                       {
// CHECK-NEXT:                         "type": "TypeParameter",
// CHECK-NEXT:                         "name": "T",
// CHECK-NEXT:                         "bound": null,
// CHECK-NEXT:                         "variance": null,
// CHECK-NEXT:                         "default": null
// CHECK-NEXT:                       }
// CHECK-NEXT:                     ]
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": false
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeParameters": {
// CHECK-NEXT:                     "type": "TypeParameterDeclaration",
// CHECK-NEXT:                     "params": [
// CHECK-NEXT:                       {
// CHECK-NEXT:                         "type": "TypeParameter",
// CHECK-NEXT:                         "name": "T",
// CHECK-NEXT:                         "bound": null,
// CHECK-NEXT:                         "variance": null,
// CHECK-NEXT:                         "default": null
// CHECK-NEXT:                       }
// CHECK-NEXT:                     ]
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

{
  record R {
    async static() {}
  }
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "BlockStatement",
// CHECK-NEXT:       "body": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "RecordDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "R"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "implements": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "RecordDeclarationBody",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "static"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": true
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": false
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

{
  record R {
    async static<T>() {}
  }
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "BlockStatement",
// CHECK-NEXT:       "body": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "RecordDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "R"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "implements": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "RecordDeclarationBody",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "MethodDefinition",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "static"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeParameters": {
// CHECK-NEXT:                     "type": "TypeParameterDeclaration",
// CHECK-NEXT:                     "params": [
// CHECK-NEXT:                       {
// CHECK-NEXT:                         "type": "TypeParameter",
// CHECK-NEXT:                         "name": "T",
// CHECK-NEXT:                         "bound": null,
// CHECK-NEXT:                         "variance": null,
// CHECK-NEXT:                         "default": null
// CHECK-NEXT:                       }
// CHECK-NEXT:                     ]
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false,
// CHECK-NEXT:                   "async": true
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "method",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "static": false
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
