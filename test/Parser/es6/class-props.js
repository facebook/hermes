/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

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

  a;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             },

  b = 3;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "b"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 3,
// CHECK-NEXT:                 "raw": "3"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             },

  static c = 4;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "c"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 4,
// CHECK-NEXT:                 "raw": "4"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": true,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             },

  [d];
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "d"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": true,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             },

  get;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "get"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": null,
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             },

  set = 4;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "set"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 4,
// CHECK-NEXT:                 "raw": "4"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             },

  async = 5;
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ClassProperty",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "async"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 5,
// CHECK-NEXT:                 "raw": "5"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false,
// CHECK-NEXT:               "declare": false
// CHECK-NEXT:             }

});
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

class C {
  get
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
// CHECK-NEXT:               "name": "get"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C {
  set
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
// CHECK-NEXT:               "name": "set"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C {
  async
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
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C {
  async
  foo
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
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class C {
  get
  x() {}
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
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "get",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
