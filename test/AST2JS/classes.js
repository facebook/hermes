/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc --dump-js %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHKJS
// RUN: %hermesc --dump-ast %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHKAST

class A extends class {

} {
    constructor() {
        if (a) {
            super();
        }
    }

    #uninitializedPrivateProperty;
    #initializedPrivateProperty1 = 10;
    #initializedPrivateProperty2 = async function () {}
    #initializedPrivateProperty3 = function *() {}
    #initializedPrivateProperty4 = async *function () {}

    #uninitializedPublicProperty;
    #initializedPublicProperty1 = 10;
    #initializedPublicProperty2 = async function () {}
    #initializedPublicProperty3 = function *() {}
    #initializedPublicProperty4 = async *function () {}

    #privateFunction() {}

    get #privateGet() {}
    set #privateSet(v) {}
    async #privateAsyncFunc() {}
    *#privateAsyncGen() {}
    async *#privateAsyncGen() {}

    static get #privateStaticGet() {}
    static set #privateStaticSet(v) {}
    static async #privateStaticAsyncFunc() {}
    static *#privateStaticAsyncGen() {}
    static async *#privateStaticAsyncGen() {}

    publicFunction() {}

    get publicGet() {}
    set publicSet(v) {}
    async publicAsyncFunc() {}
    *publicAsyncGen() {}
    async *publicAsyncGen() {}

    static get publicStaticGet() {}
    static set publicStaticSet(v) {}
    static async publicStaticAsyncFunc() {}
    static *publicStaticAsyncGen() {}
    static async *publicStaticAsyncGen() {}

    [computedPublicFunction]() {}

    get [computedPublicGet]() {}
    set [computedPublicSet](v) {}
    async [computedPublicAsyncFunc]() {}
    *[computedPublicAsyncGen]() {}
    async *[computedPublicAsyncGen]() {}

    static get [computedPublicStaticGet]() {}
    static set [computedPublicStaticSet](v) {}
    static async [computedPublicStaticAsyncFunc]() {}
    static *[computedPublicStaticAsyncGen]() {}
    static async *[computedPublicStaticAsyncGen]() {}

    "stringPublicFunction"() {}

    get "stringPublicGet"() {}
    set "stringPublicSet"(v) {}
    async "stringPublicAsyncFunc"() {}
    *"stringPublicAsyncGen"() {}
    async *"stringPublicAsyncGen"() {}

    static get "stringPublicStaticGet"() {}
    static set "stringPublicStaticSet"(v) {}
    static async "stringPublicStaticAsyncFunc"() {}
    static *"stringPublicStaticAsyncGen"() {}
    static async *"stringPublicStaticAsyncGen"() {}
}

// Auto-generated content below. Please do not modify manually.

// CHKJS:class A extends class {} {
// CHKJS-NEXT:  constructor(){
// CHKJS-NEXT:    if (a) {
// CHKJS-NEXT:      super();
// CHKJS-NEXT:    }
// CHKJS-NEXT:  }
// CHKJS-NEXT:  #uninitializedPrivateProperty;
// CHKJS-NEXT:  #initializedPrivateProperty1 = 10;
// CHKJS-NEXT:  #initializedPrivateProperty2 = async function(){};
// CHKJS-NEXT:  #initializedPrivateProperty3 = function*(){};
// CHKJS-NEXT:  #initializedPrivateProperty4 = async * function(){};
// CHKJS-NEXT:  #uninitializedPublicProperty;
// CHKJS-NEXT:  #initializedPublicProperty1 = 10;
// CHKJS-NEXT:  #initializedPublicProperty2 = async function(){};
// CHKJS-NEXT:  #initializedPublicProperty3 = function*(){};
// CHKJS-NEXT:  #initializedPublicProperty4 = async * function(){};
// CHKJS-NEXT:  #privateFunction(){}
// CHKJS-NEXT:  get #privateGet(){}
// CHKJS-NEXT:  set #privateSet(v){}
// CHKJS-NEXT:  async #privateAsyncFunc(){}
// CHKJS-NEXT:  *#privateAsyncGen(){}
// CHKJS-NEXT:  async *#privateAsyncGen(){}
// CHKJS-NEXT:  static get #privateStaticGet(){}
// CHKJS-NEXT:  static set #privateStaticSet(v){}
// CHKJS-NEXT:  static async #privateStaticAsyncFunc(){}
// CHKJS-NEXT:  static *#privateStaticAsyncGen(){}
// CHKJS-NEXT:  static async *#privateStaticAsyncGen(){}
// CHKJS-NEXT:  publicFunction(){}
// CHKJS-NEXT:  get publicGet(){}
// CHKJS-NEXT:  set publicSet(v){}
// CHKJS-NEXT:  async publicAsyncFunc(){}
// CHKJS-NEXT:  *publicAsyncGen(){}
// CHKJS-NEXT:  async *publicAsyncGen(){}
// CHKJS-NEXT:  static get publicStaticGet(){}
// CHKJS-NEXT:  static set publicStaticSet(v){}
// CHKJS-NEXT:  static async publicStaticAsyncFunc(){}
// CHKJS-NEXT:  static *publicStaticAsyncGen(){}
// CHKJS-NEXT:  static async *publicStaticAsyncGen(){}
// CHKJS-NEXT:  [computedPublicFunction](){}
// CHKJS-NEXT:  get [computedPublicGet](){}
// CHKJS-NEXT:  set [computedPublicSet](v){}
// CHKJS-NEXT:  async [computedPublicAsyncFunc](){}
// CHKJS-NEXT:  *[computedPublicAsyncGen](){}
// CHKJS-NEXT:  async *[computedPublicAsyncGen](){}
// CHKJS-NEXT:  static get [computedPublicStaticGet](){}
// CHKJS-NEXT:  static set [computedPublicStaticSet](v){}
// CHKJS-NEXT:  static async [computedPublicStaticAsyncFunc](){}
// CHKJS-NEXT:  static *[computedPublicStaticAsyncGen](){}
// CHKJS-NEXT:  static async *[computedPublicStaticAsyncGen](){}
// CHKJS-NEXT:  "stringPublicFunction"(){}
// CHKJS-NEXT:  get "stringPublicGet"(){}
// CHKJS-NEXT:  set "stringPublicSet"(v){}
// CHKJS-NEXT:  async "stringPublicAsyncFunc"(){}
// CHKJS-NEXT:  *"stringPublicAsyncGen"(){}
// CHKJS-NEXT:  async *"stringPublicAsyncGen"(){}
// CHKJS-NEXT:  static get "stringPublicStaticGet"(){}
// CHKJS-NEXT:  static set "stringPublicStaticSet"(v){}
// CHKJS-NEXT:  static async "stringPublicStaticAsyncFunc"(){}
// CHKJS-NEXT:  static *"stringPublicStaticAsyncGen"(){}
// CHKJS-NEXT:  static async *"stringPublicStaticAsyncGen"(){}
// CHKJS-NEXT:}

// CHKAST:{
// CHKAST-NEXT:  "type": "Program",
// CHKAST-NEXT:  "body": [
// CHKAST-NEXT:    {
// CHKAST-NEXT:      "type": "ClassDeclaration",
// CHKAST-NEXT:      "id": {
// CHKAST-NEXT:        "type": "Identifier",
// CHKAST-NEXT:        "name": "A"
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "superClass": {
// CHKAST-NEXT:        "type": "ClassExpression",
// CHKAST-NEXT:        "id": null,
// CHKAST-NEXT:        "superClass": null,
// CHKAST-NEXT:        "body": {
// CHKAST-NEXT:          "type": "ClassBody",
// CHKAST-NEXT:          "body": []
// CHKAST-NEXT:        }
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "body": {
// CHKAST-NEXT:        "type": "ClassBody",
// CHKAST-NEXT:        "body": [
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "constructor"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": [
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "IfStatement",
// CHKAST-NEXT:                    "test": {
// CHKAST-NEXT:                      "type": "Identifier",
// CHKAST-NEXT:                      "name": "a"
// CHKAST-NEXT:                    },
// CHKAST-NEXT:                    "consequent": {
// CHKAST-NEXT:                      "type": "BlockStatement",
// CHKAST-NEXT:                      "body": [
// CHKAST-NEXT:                        {
// CHKAST-NEXT:                          "type": "ExpressionStatement",
// CHKAST-NEXT:                          "expression": {
// CHKAST-NEXT:                            "type": "CallExpression",
// CHKAST-NEXT:                            "callee": {
// CHKAST-NEXT:                              "type": "Super"
// CHKAST-NEXT:                            },
// CHKAST-NEXT:                            "arguments": []
// CHKAST-NEXT:                          },
// CHKAST-NEXT:                          "directive": null
// CHKAST-NEXT:                        }
// CHKAST-NEXT:                      ]
// CHKAST-NEXT:                    },
// CHKAST-NEXT:                    "alternate": null
// CHKAST-NEXT:                  }
// CHKAST-NEXT:                ]
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "constructor",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "uninitializedPrivateProperty"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": null,
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPrivateProperty1"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "NumericLiteral",
// CHKAST-NEXT:              "value": 10,
// CHKAST-NEXT:              "raw": "10"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPrivateProperty2"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPrivateProperty3"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPrivateProperty4"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "BinaryExpression",
// CHKAST-NEXT:              "left": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "async"
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "right": {
// CHKAST-NEXT:                "type": "FunctionExpression",
// CHKAST-NEXT:                "id": null,
// CHKAST-NEXT:                "params": [],
// CHKAST-NEXT:                "body": {
// CHKAST-NEXT:                  "type": "BlockStatement",
// CHKAST-NEXT:                  "body": []
// CHKAST-NEXT:                },
// CHKAST-NEXT:                "generator": false,
// CHKAST-NEXT:                "async": false
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "operator": "*"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "uninitializedPublicProperty"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": null,
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPublicProperty1"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "NumericLiteral",
// CHKAST-NEXT:              "value": 10,
// CHKAST-NEXT:              "raw": "10"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPublicProperty2"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPublicProperty3"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ClassPrivateProperty",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "initializedPublicProperty4"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "BinaryExpression",
// CHKAST-NEXT:              "left": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "async"
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "right": {
// CHKAST-NEXT:                "type": "FunctionExpression",
// CHKAST-NEXT:                "id": null,
// CHKAST-NEXT:                "params": [],
// CHKAST-NEXT:                "body": {
// CHKAST-NEXT:                  "type": "BlockStatement",
// CHKAST-NEXT:                  "body": []
// CHKAST-NEXT:                },
// CHKAST-NEXT:                "generator": false,
// CHKAST-NEXT:                "async": false
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "operator": "*"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "static": false,
// CHKAST-NEXT:            "declare": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateFunction"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateGet"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateSet"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateAsyncFunc"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateAsyncGen"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateAsyncGen"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateStaticGet"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateStaticSet"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateStaticAsyncFunc"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateStaticAsyncGen"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "PrivateName",
// CHKAST-NEXT:              "id": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "privateStaticAsyncGen"
// CHKAST-NEXT:              }
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicFunction"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicGet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicSet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicAsyncFunc"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicStaticGet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicStaticSet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicStaticAsyncFunc"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicStaticAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "publicStaticAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicFunction"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicGet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicSet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicAsyncFunc"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicStaticGet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicStaticSet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicStaticAsyncFunc"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicStaticAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "Identifier",
// CHKAST-NEXT:              "name": "computedPublicStaticAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": true,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicFunction"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicGet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicSet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicAsyncFunc"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": false
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicStaticGet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "get",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicStaticSet"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "v"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "set",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicStaticAsyncFunc"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": false,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicStaticAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "MethodDefinition",
// CHKAST-NEXT:            "key": {
// CHKAST-NEXT:              "type": "StringLiteral",
// CHKAST-NEXT:              "value": "stringPublicStaticAsyncGen"
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "value": {
// CHKAST-NEXT:              "type": "FunctionExpression",
// CHKAST-NEXT:              "id": null,
// CHKAST-NEXT:              "params": [],
// CHKAST-NEXT:              "body": {
// CHKAST-NEXT:                "type": "BlockStatement",
// CHKAST-NEXT:                "body": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "generator": true,
// CHKAST-NEXT:              "async": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "kind": "method",
// CHKAST-NEXT:            "computed": false,
// CHKAST-NEXT:            "static": true
// CHKAST-NEXT:          }
// CHKAST-NEXT:        ]
// CHKAST-NEXT:      }
// CHKAST-NEXT:    }
// CHKAST-NEXT:  ]
// CHKAST-NEXT:}
