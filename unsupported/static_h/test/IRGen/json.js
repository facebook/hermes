/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [json]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = AllocArrayInst 2 : number, "GML" : string, "XML" : string
//CHECK-NEXT:  %3 = AllocObjectLiteralInst "para" : string, "A meta-markup language, used to create markup languages such as DocBook." : string, "GlossSeeAlso" : string, %2 : object
//CHECK-NEXT:  %4 = AllocObjectLiteralInst "ID" : string, "SGML" : string, "SortAs" : string, "SGML" : string, "GlossTerm" : string, "Standard Generalized Markup Language" : string, "Acronym" : string, "SGML" : string, "Abbrev" : string, "ISO 8879:1986" : string, "GlossDef" : string, %3 : object, "GlossSee" : string, "markup" : string
//CHECK-NEXT:  %5 = AllocObjectLiteralInst "GlossEntry" : string, %4 : object
//CHECK-NEXT:  %6 = AllocObjectLiteralInst "title" : string, "S" : string, "GlossList" : string, %5 : object
//CHECK-NEXT:  %7 = AllocObjectLiteralInst "title" : string, "example glossary" : string, "GlossDiv" : string, %6 : object
//CHECK-NEXT:  %8 = AllocObjectLiteralInst "glossary" : string, %7 : object
//CHECK-NEXT:  %9 = StorePropertyInst %8 : object, globalObject : object, "json" : string
//CHECK-NEXT:  %10 = LoadStackInst %0
//CHECK-NEXT:  %11 = ReturnInst %10
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


