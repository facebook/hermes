/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [json]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:  %3 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:  %4 = StoreNewOwnPropertyInst "example glossary" : string, %3 : object, "title" : string, true : boolean
//CHECK-NEXT:  %5 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:  %6 = StoreNewOwnPropertyInst "S" : string, %5 : object, "title" : string, true : boolean
//CHECK-NEXT:  %7 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:  %8 = AllocObjectInst 7 : number, empty
//CHECK-NEXT:  %9 = StoreNewOwnPropertyInst "SGML" : string, %8 : object, "ID" : string, true : boolean
//CHECK-NEXT:  %10 = StoreNewOwnPropertyInst "SGML" : string, %8 : object, "SortAs" : string, true : boolean
//CHECK-NEXT:  %11 = StoreNewOwnPropertyInst "Standard Generalized Markup Language" : string, %8 : object, "GlossTerm" : string, true : boolean
//CHECK-NEXT:  %12 = StoreNewOwnPropertyInst "SGML" : string, %8 : object, "Acronym" : string, true : boolean
//CHECK-NEXT:  %13 = StoreNewOwnPropertyInst "ISO 8879:1986" : string, %8 : object, "Abbrev" : string, true : boolean
//CHECK-NEXT:  %14 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:  %15 = StoreNewOwnPropertyInst "A meta-markup language, used to create markup languages such as DocBook." : string, %14 : object, "para" : string, true : boolean
//CHECK-NEXT:  %16 = AllocArrayInst 2 : number, "GML" : string, "XML" : string
//CHECK-NEXT:  %17 = StoreNewOwnPropertyInst %16 : object, %14 : object, "GlossSeeAlso" : string, true : boolean
//CHECK-NEXT:  %18 = StoreNewOwnPropertyInst %14 : object, %8 : object, "GlossDef" : string, true : boolean
//CHECK-NEXT:  %19 = StoreNewOwnPropertyInst "markup" : string, %8 : object, "GlossSee" : string, true : boolean
//CHECK-NEXT:  %20 = StoreNewOwnPropertyInst %8 : object, %7 : object, "GlossEntry" : string, true : boolean
//CHECK-NEXT:  %21 = StoreNewOwnPropertyInst %7 : object, %5 : object, "GlossList" : string, true : boolean
//CHECK-NEXT:  %22 = StoreNewOwnPropertyInst %5 : object, %3 : object, "GlossDiv" : string, true : boolean
//CHECK-NEXT:  %23 = StoreNewOwnPropertyInst %3 : object, %2 : object, "glossary" : string, true : boolean
//CHECK-NEXT:  %24 = StorePropertyInst %2 : object, globalObject : object, "json" : string
//CHECK-NEXT:  %25 = LoadStackInst %0
//CHECK-NEXT:  %26 = ReturnInst %25
//CHECK-NEXT:function_end

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


