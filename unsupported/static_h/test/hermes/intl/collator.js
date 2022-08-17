/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("Intl.Collator Test")
// CHECK-LABEL: Intl.Collator Test

print(['Z', 'a', 'z', 'ä'].sort(new Intl.Collator('de').compare));
// CHECK-NEXT: a,ä,z,Z
print(['Z', 'a', 'z', 'ä'].sort(new Intl.Collator('sv').compare));
// CHECK-NEXT: a,z,Z,ä
print(['Z', 'a', 'z', 'ä'].sort(new Intl.Collator('sv', {caseFirst: 'upper'}).compare));
// CHECK-NEXT: a,Z,z,ä
print(['Z', 'a', 'z', 'ä'].sort(new Intl.Collator('de', { caseFirst: 'upper' } ).compare));
// CHECK-NEXT: a,ä,Z,z

print(["of", "öf"].sort(new Intl.Collator('de-DE', { } ).compare));
// CHECK-NEXT: of,öf
print(["of", "öf"].sort(new Intl.Collator('de-DE-u-co-phonebk', { } ).compare));
// CHECK-NEXT: öf,of
print(["of", "öf"].sort(new Intl.Collator('de-DE', { collation: "phonebk" } ).compare));
// CHECK-NEXT: öf,of

print(['test1', 'test2', 'test10'].sort(new Intl.Collator('en', { } ).compare));
// CHECK-NEXT: test1,test10,test2
print(['test1', 'test2', 'test10'].sort(new Intl.Collator('en', { numeric: true } ).compare));
// CHECK-NEXT: test1,test2,test10

try{ new Intl.Collator('en', { collation: "banananana" } ) } catch (e) { print(e) }
// CHECK-NEXT: RangeError: Invalid collation: banananana

var puncList = ["aa", "ab", "a,b", "a\nb", "a\na", "a  a", "a  b", "...", "", ".a.a"];
print(JSON.stringify([...puncList].sort(new Intl.Collator("en", ).compare)));
// CHECK-NEXT: ["","...",".a.a","a\na","a\nb","a  a","a  b","a,b","aa","ab"]
print(JSON.stringify([...puncList].sort(new Intl.Collator("en", { ignorePunctuation: true }).compare)));
// CHECK-NEXT: ["...","","aa","a\na","a a",".a.a","ab","a,b","a\nb","a b"]

try { Intl.Collator.prototype.resolvedOptions.call(new Intl.DateTimeFormat()) }
catch (e) { print(e) }
// CHECK-NEXT: TypeError: Intl.Collator.prototype.resolvedOptions called with incompatible 'this'
