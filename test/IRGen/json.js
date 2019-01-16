// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [json]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = AllocObjectInst 1 : number
//CHECK-NEXT:  %3 = AllocObjectInst 2 : number
//CHECK-NEXT:  %4 = StoreOwnPropertyInst "example glossary" : string, %3 : object, "title" : string
//CHECK-NEXT:  %5 = AllocObjectInst 2 : number
//CHECK-NEXT:  %6 = StoreOwnPropertyInst "S" : string, %5 : object, "title" : string
//CHECK-NEXT:  %7 = AllocObjectInst 1 : number
//CHECK-NEXT:  %8 = AllocObjectInst 7 : number
//CHECK-NEXT:  %9 = StoreOwnPropertyInst "SGML" : string, %8 : object, "ID" : string
//CHECK-NEXT:  %10 = StoreOwnPropertyInst "SGML" : string, %8 : object, "SortAs" : string
//CHECK-NEXT:  %11 = StoreOwnPropertyInst "Standard Generalized Markup Language" : string, %8 : object, "GlossTerm" : string
//CHECK-NEXT:  %12 = StoreOwnPropertyInst "SGML" : string, %8 : object, "Acronym" : string
//CHECK-NEXT:  %13 = StoreOwnPropertyInst "ISO 8879:1986" : string, %8 : object, "Abbrev" : string
//CHECK-NEXT:  %14 = AllocObjectInst 2 : number
//CHECK-NEXT:  %15 = StoreOwnPropertyInst "A meta-markup language, used to create markup languages such as DocBook." : string, %14 : object, "para" : string
//CHECK-NEXT:  %16 = AllocArrayInst 2 : number, "GML" : string, "XML" : string
//CHECK-NEXT:  %17 = StoreOwnPropertyInst %16 : object, %14 : object, "GlossSeeAlso" : string
//CHECK-NEXT:  %18 = StoreOwnPropertyInst %14 : object, %8 : object, "GlossDef" : string
//CHECK-NEXT:  %19 = StoreOwnPropertyInst "markup" : string, %8 : object, "GlossSee" : string
//CHECK-NEXT:  %20 = StoreOwnPropertyInst %8 : object, %7 : object, "GlossEntry" : string
//CHECK-NEXT:  %21 = StoreOwnPropertyInst %7 : object, %5 : object, "GlossList" : string
//CHECK-NEXT:  %22 = StoreOwnPropertyInst %5 : object, %3 : object, "GlossDiv" : string
//CHECK-NEXT:  %23 = StoreOwnPropertyInst %3 : object, %2 : object, "glossary" : string
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


