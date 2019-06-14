// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s


({a, ...foo(10), b, ...d})
//CHECK:      {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "ObjectExpression",
//CHECK-NEXT:           "properties": [
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "Property",
//CHECK-NEXT:               "key": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "a",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               },
//CHECK-NEXT:               "value": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "a",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               },
//CHECK-NEXT:               "kind": "init",
//CHECK-NEXT:               "computed": false
//CHECK-NEXT:             },
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "SpreadElement",
//CHECK-NEXT:               "argument": {
//CHECK-NEXT:                 "type": "CallExpression",
//CHECK-NEXT:                 "callee": {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "foo",
//CHECK-NEXT:                   "typeAnnotation": null
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "arguments": [
//CHECK-NEXT:                   {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 10
//CHECK-NEXT:                   }
//CHECK-NEXT:                 ]
//CHECK-NEXT:               }
//CHECK-NEXT:             },
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "Property",
//CHECK-NEXT:               "key": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "b",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               },
//CHECK-NEXT:               "value": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "b",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               },
//CHECK-NEXT:               "kind": "init",
//CHECK-NEXT:               "computed": false
//CHECK-NEXT:             },
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "SpreadElement",
//CHECK-NEXT:               "argument": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "d",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               }
//CHECK-NEXT:             }
//CHECK-NEXT:           ]
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       }
//CHECK-NEXT:     ]
//CHECK-NEXT:   }
