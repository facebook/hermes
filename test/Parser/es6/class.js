// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc -dump-ast -pretty-json %s 2>/dev/null ) | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

(class Foo {
  constructor() {}
  bar() { return 1; }
  ['baz'](x, y) {}
  static sFun() { return 10; }
  get myGetter() { return 1; }
  *gen() {}
});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Foo",
// CHECK-NEXT:           "typeAnnotation": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "superClass": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "constructor",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "constructor",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "ReturnStatement",
// CHECK-NEXT:                       "argument": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 1
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "StringLiteral",
// CHECK-NEXT:                 "value": "baz"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x",
// CHECK-NEXT:                     "typeAnnotation": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "y",
// CHECK-NEXT:                     "typeAnnotation": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": true,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "sFun",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "ReturnStatement",
// CHECK-NEXT:                       "argument": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 10
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": true
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "myGetter",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "ReturnStatement",
// CHECK-NEXT:                       "argument": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 1
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "get",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "gen",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": true
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

class UseSuperProps {
  constructor() {
    super();
  }

  foo() {
    return super.x.y;
  }

  bar() {
    return super['x'];
  }
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "UseSuperProps",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "constructor",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "CallExpression",
// CHECK-NEXT:                       "callee": {
// CHECK-NEXT:                         "type": "Super"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "arguments": []
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "constructor",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo",
// CHECK-NEXT:               "typeAnnotation": null
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
// CHECK-NEXT:                       "type": "MemberExpression",
// CHECK-NEXT:                       "object": {
// CHECK-NEXT:                         "type": "MemberExpression",
// CHECK-NEXT:                         "object": {
// CHECK-NEXT:                           "type": "Super"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "property": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "x",
// CHECK-NEXT:                           "typeAnnotation": null
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "computed": false
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "y",
// CHECK-NEXT:                         "typeAnnotation": null
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": false
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar",
// CHECK-NEXT:               "typeAnnotation": null
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
// CHECK-NEXT:                       "type": "MemberExpression",
// CHECK-NEXT:                       "object": {
// CHECK-NEXT:                         "type": "Super"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "StringLiteral",
// CHECK-NEXT:                         "value": "x"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": true
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

(class ConsProperty {
  get ['constructor']() {}
  set ['constructor'](val) {}
});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "ConsProperty",
// CHECK-NEXT:           "typeAnnotation": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "superClass": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "StringLiteral",
// CHECK-NEXT:                 "value": "constructor"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "get",
// CHECK-NEXT:               "computed": true,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "StringLiteral",
// CHECK-NEXT:                 "value": "constructor"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "val",
// CHECK-NEXT:                     "typeAnnotation": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "set",
// CHECK-NEXT:               "computed": true,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

class SemiClass {
  ;
  foo() {}
  ;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "SemiClass",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class DeclClass {
  foo() {}
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "DeclClass",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
