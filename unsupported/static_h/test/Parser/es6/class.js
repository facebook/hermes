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
  constructor() {}
  bar() { return 1; }
  ['baz'](x, y) {}
  static sFun() { return 10; }
  get myGetter() { return 1; }
  *gen() {}
  static() {}
  rest(a, ...b) {}
});
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
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "constructor"
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
// CHECK-NEXT:               "kind": "constructor",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar"
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
// CHECK-NEXT:                         "value": 1,
// CHECK-NEXT:                         "raw": "1"
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
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
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "y"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": true,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "sFun"
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
// CHECK-NEXT:                         "value": 10,
// CHECK-NEXT:                         "raw": "10"
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": true
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "myGetter"
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
// CHECK-NEXT:                         "value": 1,
// CHECK-NEXT:                         "raw": "1"
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "get",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "gen"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": true,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "static"
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
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "rest"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "a"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "RestElement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "b"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
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
// CHECK-NEXT:         "name": "UseSuperProps"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "constructor"
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
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "constructor",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
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
// CHECK-NEXT:                           "name": "x"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "computed": false
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "property": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "y"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "computed": false
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar"
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

(class Gens {
  *foo() {
    yield 1;
  }
  *bar() {
    yield* 2;
  }
});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Gens"
// CHECK-NEXT:         },
// CHECK-NEXT:         "superClass": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": [
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
// CHECK-NEXT:                   "body": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "ExpressionStatement",
// CHECK-NEXT:                       "expression": {
// CHECK-NEXT:                         "type": "YieldExpression",
// CHECK-NEXT:                         "argument": {
// CHECK-NEXT:                           "type": "NumericLiteral",
// CHECK-NEXT:                           "value": 1,
// CHECK-NEXT:                           "raw": "1"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "delegate": false
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "directive": null
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": true,
// CHECK-NEXT:                 "async": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "FunctionExpression",
// CHECK-NEXT:                 "id": null,
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "body": {
// CHECK-NEXT:                   "type": "BlockStatement",
// CHECK-NEXT:                   "body": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "ExpressionStatement",
// CHECK-NEXT:                       "expression": {
// CHECK-NEXT:                         "type": "YieldExpression",
// CHECK-NEXT:                         "argument": {
// CHECK-NEXT:                           "type": "NumericLiteral",
// CHECK-NEXT:                           "value": 2,
// CHECK-NEXT:                           "raw": "2"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "delegate": true
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "directive": null
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "generator": true,
// CHECK-NEXT:                 "async": false
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

(class ConsProperty {
  get ['constructor']() {}
  set ['constructor'](val) {}
  static constructor() {}
  static constructor() {}
});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "ConsProperty"
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
// CHECK-NEXT:                 "generator": false,
// CHECK-NEXT:                 "async": false
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
// CHECK-NEXT:                     "name": "val"
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
// CHECK-NEXT:               "computed": true,
// CHECK-NEXT:               "static": false
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "constructor"
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
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": true
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "MethodDefinition",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "constructor"
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
// CHECK-NEXT:               "kind": "method",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "static": true
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
// CHECK-NEXT:         "name": "SemiClass"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
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
// CHECK-NEXT:                 "body": []
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

class DeclClass {
  foo() {}
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "DeclClass"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
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
// CHECK-NEXT:                 "body": []
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
