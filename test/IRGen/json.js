/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "json": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst "example glossary": string, %5: object, "title": string, true: boolean
// CHECK-NEXT:  %7 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst "S": string, %7: object, "title": string, true: boolean
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %10 = AllocObjectInst (:object) 7: number, empty: any
// CHECK-NEXT:        StoreNewOwnPropertyInst "SGML": string, %10: object, "ID": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "SGML": string, %10: object, "SortAs": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "Standard Generalized Markup Language": string, %10: object, "GlossTerm": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "SGML": string, %10: object, "Acronym": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "ISO 8879:1986": string, %10: object, "Abbrev": string, true: boolean
// CHECK-NEXT:  %16 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:        StoreNewOwnPropertyInst "A meta-markup language, used to create markup languages such as DocBook.": string, %16: object, "para": string, true: boolean
// CHECK-NEXT:  %18 = AllocArrayInst (:object) 2: number, "GML": string, "XML": string
// CHECK-NEXT:        StoreNewOwnPropertyInst %18: object, %16: object, "GlossSeeAlso": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %16: object, %10: object, "GlossDef": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "markup": string, %10: object, "GlossSee": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %10: object, %9: object, "GlossEntry": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %9: object, %7: object, "GlossList": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %7: object, %5: object, "GlossDiv": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %5: object, %4: object, "glossary": string, true: boolean
// CHECK-NEXT:        StorePropertyLooseInst %4: object, globalObject: object, "json": string
// CHECK-NEXT:  %27 = LoadStackInst (:any) %2: any
// CHECK-NEXT:        ReturnInst %27: any
// CHECK-NEXT:function_end
