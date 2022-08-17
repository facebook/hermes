/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

a?.b;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a"
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "b"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a?.b.c?.d.e
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "OptionalMemberExpression",
// CHECK-NEXT:             "object": {
// CHECK-NEXT:               "type": "OptionalMemberExpression",
// CHECK-NEXT:               "object": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a"
// CHECK-NEXT:               },
// CHECK-NEXT:               "property": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "b"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "optional": true
// CHECK-NEXT:             },
// CHECK-NEXT:             "property": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "c"
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "d"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "e"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a ?. (b);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalCallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a"
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "b"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a ?. [b];
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a"
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "b"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": true,
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a ?. (b) . c;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalCallExpression",
// CHECK-NEXT:           "callee": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a"
// CHECK-NEXT:           },
// CHECK-NEXT:           "arguments": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(a?.b.c).d;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "MemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "OptionalMemberExpression",
// CHECK-NEXT:             "object": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "property": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "c"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "d"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(a?.[b].c).d;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "MemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "OptionalMemberExpression",
// CHECK-NEXT:             "object": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "property": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": true,
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "c"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "d"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a?.(b).c;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalCallExpression",
// CHECK-NEXT:           "callee": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a"
// CHECK-NEXT:           },
// CHECK-NEXT:           "arguments": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(a?.b().c);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalCallExpression",
// CHECK-NEXT:           "callee": {
// CHECK-NEXT:             "type": "OptionalMemberExpression",
// CHECK-NEXT:             "object": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "property": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           },
// CHECK-NEXT:           "arguments": [],
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(a?.b()).c;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "MemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalCallExpression",
// CHECK-NEXT:           "callee": {
// CHECK-NEXT:             "type": "OptionalMemberExpression",
// CHECK-NEXT:             "object": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "property": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           },
// CHECK-NEXT:           "arguments": [],
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

new a().b?.c;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "MemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "NewExpression",
// CHECK-NEXT:             "callee": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "arguments": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "b"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a()?.b();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalCallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "CallExpression",
// CHECK-NEXT:             "callee": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "arguments": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "b"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [],
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a()?.b.c;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "CallExpression",
// CHECK-NEXT:             "callee": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "arguments": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "b"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "c"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a()?.b;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalMemberExpression",
// CHECK-NEXT:         "object": {
// CHECK-NEXT:           "type": "CallExpression",
// CHECK-NEXT:           "callee": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a"
// CHECK-NEXT:           },
// CHECK-NEXT:           "arguments": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "property": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "b"
// CHECK-NEXT:         },
// CHECK-NEXT:         "computed": false,
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

// Parenthesized expressions don't carry optional chains over.
(x?.())();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "CallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "OptionalCallExpression",
// CHECK-NEXT:           "callee": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "arguments": [],
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

// Ensure we don't do optional chaining when it should be a conditional.
x ? .3 : .4;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 0.4,
// CHECK-NEXT:           "raw": ".4"
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 0.3,
// CHECK-NEXT:           "raw": ".3"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
