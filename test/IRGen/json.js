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
// CHECK-NEXT:       DeclareGlobalVarInst "json": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst "example glossary": string, %4: object, "title": string, true: boolean
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst "S": string, %6: object, "title": string, true: boolean
// CHECK-NEXT:  %8 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 7: number, empty: any
// CHECK-NEXT:        StoreNewOwnPropertyInst "SGML": string, %9: object, "ID": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "SGML": string, %9: object, "SortAs": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "Standard Generalized Markup Language": string, %9: object, "GlossTerm": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "SGML": string, %9: object, "Acronym": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "ISO 8879:1986": string, %9: object, "Abbrev": string, true: boolean
// CHECK-NEXT:  %15 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:        StoreNewOwnPropertyInst "A meta-markup language, used to create markup languages such as DocBook.": string, %15: object, "para": string, true: boolean
// CHECK-NEXT:  %17 = AllocArrayInst (:object) 2: number, "GML": string, "XML": string
// CHECK-NEXT:        StoreNewOwnPropertyInst %17: object, %15: object, "GlossSeeAlso": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %15: object, %9: object, "GlossDef": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "markup": string, %9: object, "GlossSee": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %9: object, %8: object, "GlossEntry": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %8: object, %6: object, "GlossList": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %6: object, %4: object, "GlossDiv": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst %4: object, %3: object, "glossary": string, true: boolean
// CHECK-NEXT:        StorePropertyLooseInst %3: object, globalObject: object, "json": string
// CHECK-NEXT:  %26 = LoadStackInst (:any) %1: any
// CHECK-NEXT:        ReturnInst %26: any
// CHECK-NEXT:function_end
