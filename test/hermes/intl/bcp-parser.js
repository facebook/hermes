/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("test bcp-47 parser");
// CHECK-LABEL: test bcp-47 parser

const characterTypeTest = ["f","7","-","ç",'$'];

print(characterTypeTest.map((c) => Intl.isASCIILetter(c)));
// CHECK-NEXT: true,false,false,false,false

print(characterTypeTest.map((c) => Intl.isASCIIDigit(c)));
// CHECK-NEXT: false,true,false,false,false

print(characterTypeTest.map((c) => Intl.isASCIILetterOrDigit(c)));
// CHECK-NEXT: true,true,false,false,false

print(characterTypeTest.map((c) => Intl.isSubtagSeparator(c)));
// CHECK-NEXT: false,false,true,false,false

// const languageTagTest = ["en","engus","Arab","US","001","1999", ]
// 
// print(languageTagTest.map(s) => Intl.isUnicodeLanguageSubtag(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false
// 
// print(languageTagTest.map(s) => Intl.isUnicodeScriptSubtag(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false
// 
// print(languageTagTest.map(s) => Intl.isUnicodeRegionSubtag(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false
// 
// print(languageTagTest.map(s) => Intl.isUnicodeVariantSubtag(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false
// 
// print(languageTagTest.map(s) => Intl.isUnicodeExtensionAttribute(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false
// 
// print(languageTagTest.map(s) => Intl.isUnicodeExtensionKey(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,falseß
// 
// print(languageTagTest.map(s) => Intl.isUnicodeExtensionType(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false
// 
// print(languageTagTest.map(s) => Intl.isUnicodeLanguageSubtag(s, 0, s.length()));
// // CHECK-NEXT: true,true,false,true,false,false

let parser = Intl.LanguageTagParser("en-US").parseLocaleId()
print(parser.toParsedString());
