/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

// This is an example of a JSON file from json.org:
// http://json.org/example.html
var json = {
    "glossary": {
        "title": "example glossary",
    "GlossDiv": {
            "title": "S",
      "GlossList": {
                "GlossEntry": {
                    "ID": "SGML",
          "SortAs": "SGML",
          "GlossTerm": "Standard Generalized Markup Language",
          "Acronym": "SGML",
          "Abbrev": "ISO 8879:1986",
          "GlossDef": {
                        "para": "A meta-markup language, used to create markup languages such as DocBook.",
            "GlossSeeAlso": ["GML", "XML"]
                    },
          "GlossSee": "markup"
                }
            }
        }
    }
};

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [json]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst "example glossary" : string, %4 : object, "title" : string, true : boolean
// CHECK-NEXT:  %6 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst "S" : string, %6 : object, "title" : string, true : boolean
// CHECK-NEXT:  %8 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %9 = AllocObjectInst 7 : number, empty
// CHECK-NEXT:  %10 = StoreNewOwnPropertyInst "SGML" : string, %9 : object, "ID" : string, true : boolean
// CHECK-NEXT:  %11 = StoreNewOwnPropertyInst "SGML" : string, %9 : object, "SortAs" : string, true : boolean
// CHECK-NEXT:  %12 = StoreNewOwnPropertyInst "Standard Generalized Markup Language" : string, %9 : object, "GlossTerm" : string, true : boolean
// CHECK-NEXT:  %13 = StoreNewOwnPropertyInst "SGML" : string, %9 : object, "Acronym" : string, true : boolean
// CHECK-NEXT:  %14 = StoreNewOwnPropertyInst "ISO 8879:1986" : string, %9 : object, "Abbrev" : string, true : boolean
// CHECK-NEXT:  %15 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %16 = StoreNewOwnPropertyInst "A meta-markup language, used to create markup languages such as DocBook." : string, %15 : object, "para" : string, true : boolean
// CHECK-NEXT:  %17 = AllocArrayInst 2 : number, "GML" : string, "XML" : string
// CHECK-NEXT:  %18 = StoreNewOwnPropertyInst %17 : object, %15 : object, "GlossSeeAlso" : string, true : boolean
// CHECK-NEXT:  %19 = StoreNewOwnPropertyInst %15 : object, %9 : object, "GlossDef" : string, true : boolean
// CHECK-NEXT:  %20 = StoreNewOwnPropertyInst "markup" : string, %9 : object, "GlossSee" : string, true : boolean
// CHECK-NEXT:  %21 = StoreNewOwnPropertyInst %9 : object, %8 : object, "GlossEntry" : string, true : boolean
// CHECK-NEXT:  %22 = StoreNewOwnPropertyInst %8 : object, %6 : object, "GlossList" : string, true : boolean
// CHECK-NEXT:  %23 = StoreNewOwnPropertyInst %6 : object, %4 : object, "GlossDiv" : string, true : boolean
// CHECK-NEXT:  %24 = StoreNewOwnPropertyInst %4 : object, %3 : object, "glossary" : string, true : boolean
// CHECK-NEXT:  %25 = StorePropertyInst %3 : object, globalObject : object, "json" : string
// CHECK-NEXT:  %26 = LoadStackInst %1
// CHECK-NEXT:  %27 = ReturnInst %26
// CHECK-NEXT:function_end
