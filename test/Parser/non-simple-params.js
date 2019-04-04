// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

//CHECK:      {
//CHECK-NEXT:   "type": "File",
//CHECK-NEXT:   "program": {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [

function foo(a = 10, {b = 20, c} = {c:30}, [d,e]) {}
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "FunctionDeclaration",
//CHECK-NEXT:         "id": {
//CHECK-NEXT:           "type": "Identifier",
//CHECK-NEXT:           "name": "foo",
//CHECK-NEXT:           "typeAnnotation": null
//CHECK-NEXT:         },
//CHECK-NEXT:         "params": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "AssignmentPattern",
//CHECK-NEXT:             "left": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "a",
//CHECK-NEXT:               "typeAnnotation": null
//CHECK-NEXT:             },
//CHECK-NEXT:             "right": {
//CHECK-NEXT:               "type": "NumericLiteral",
//CHECK-NEXT:               "value": 10
//CHECK-NEXT:             }
//CHECK-NEXT:           },
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "AssignmentPattern",
//CHECK-NEXT:             "left": {
//CHECK-NEXT:               "type": "ObjectPattern",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "AssignmentPattern",
//CHECK-NEXT:                     "left": {
//CHECK-NEXT:                       "type": "Identifier",
//CHECK-NEXT:                       "name": "b",
//CHECK-NEXT:                       "typeAnnotation": null
//CHECK-NEXT:                     },
//CHECK-NEXT:                     "right": {
//CHECK-NEXT:                       "type": "NumericLiteral",
//CHECK-NEXT:                       "value": 20
//CHECK-NEXT:                     }
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init"
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             },
//CHECK-NEXT:             "right": {
//CHECK-NEXT:               "type": "ObjectExpression",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 30
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init"
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           },
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "ArrayPattern",
//CHECK-NEXT:             "elements": [
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "d",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               },
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "e",
//CHECK-NEXT:                 "typeAnnotation": null
//CHECK-NEXT:               }
//CHECK-NEXT:             ]
//CHECK-NEXT:           }
//CHECK-NEXT:         ],
//CHECK-NEXT:         "body": {
//CHECK-NEXT:           "type": "BlockStatement",
//CHECK-NEXT:           "body": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "returnType": null
//CHECK-NEXT:       }

//CHECK-NEXT:     ]
//CHECK-NEXT:   }
//CHECK-NEXT: }
