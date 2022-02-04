/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

class A {
  #p;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class A {
  #p
  #q;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "q"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class A {
  #p
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "p"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class A {
  get #p() {}
  set #p(v) {}
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "PrivateName",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "p"
// CHECK-NEXT:               }
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
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "PrivateName",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "p"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "v"
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "set",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     },

class A {
  #x;
  foo() {
    this.#x;
    this.this.#x;
    this.foo().#x;
    this.foo().#x();
  }
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
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
// CHECK-NEXT:                       "type": "MemberExpression",
// CHECK-NEXT:                       "object": {
// CHECK-NEXT:                         "type": "ThisExpression"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "PrivateName",
// CHECK-NEXT:                         "id": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "x"
// CHECK-NEXT:                         }
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": false
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "MemberExpression",
// CHECK-NEXT:                       "object": {
// CHECK-NEXT:                         "type": "MemberExpression",
// CHECK-NEXT:                         "object": {
// CHECK-NEXT:                           "type": "ThisExpression"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "property": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "this"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "computed": false
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "PrivateName",
// CHECK-NEXT:                         "id": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "x"
// CHECK-NEXT:                         }
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": false
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "MemberExpression",
// CHECK-NEXT:                       "object": {
// CHECK-NEXT:                         "type": "CallExpression",
// CHECK-NEXT:                         "callee": {
// CHECK-NEXT:                           "type": "MemberExpression",
// CHECK-NEXT:                           "object": {
// CHECK-NEXT:                             "type": "ThisExpression"
// CHECK-NEXT:                           },
// CHECK-NEXT:                           "property": {
// CHECK-NEXT:                             "type": "Identifier",
// CHECK-NEXT:                             "name": "foo"
// CHECK-NEXT:                           },
// CHECK-NEXT:                           "computed": false
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "arguments": []
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "PrivateName",
// CHECK-NEXT:                         "id": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "x"
// CHECK-NEXT:                         }
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": false
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "CallExpression",
// CHECK-NEXT:                       "callee": {
// CHECK-NEXT:                         "type": "MemberExpression",
// CHECK-NEXT:                         "object": {
// CHECK-NEXT:                           "type": "CallExpression",
// CHECK-NEXT:                           "callee": {
// CHECK-NEXT:                             "type": "MemberExpression",
// CHECK-NEXT:                             "object": {
// CHECK-NEXT:                               "type": "ThisExpression"
// CHECK-NEXT:                             },
// CHECK-NEXT:                             "property": {
// CHECK-NEXT:                               "type": "Identifier",
// CHECK-NEXT:                               "name": "foo"
// CHECK-NEXT:                             },
// CHECK-NEXT:                             "computed": false
// CHECK-NEXT:                           },
// CHECK-NEXT:                           "arguments": []
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "property": {
// CHECK-NEXT:                           "type": "PrivateName",
// CHECK-NEXT:                           "id": {
// CHECK-NEXT:                             "type": "Identifier",
// CHECK-NEXT:                             "name": "x"
// CHECK-NEXT:                           }
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "computed": false
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "arguments": []
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
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

class A {
  #x;
  foo() {
    this?.#x;
  }
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": null,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
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
// CHECK-NEXT:                       "type": "OptionalMemberExpression",
// CHECK-NEXT:                       "object": {
// CHECK-NEXT:                         "type": "ThisExpression"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "PrivateName",
// CHECK-NEXT:                         "id": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "x"
// CHECK-NEXT:                         }
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": false,
// CHECK-NEXT:                       "optional": true
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
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
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
