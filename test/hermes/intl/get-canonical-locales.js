/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

print("get canonical locales test");
// CHECK-LABEL: get canonical locales test

print(Intl.getCanonicalLocales(["EN-us", "Fr"]));
// CHECK-NEXT: en-US,fr

print(Intl.getCanonicalLocales(["zh-zh", "ZH"]));
// CHECK-NEXT: zh-ZH,zh

print(Intl.getCanonicalLocales(["aam", "zyb", "art-lojban"]));
// CHECK-NEXT: aas,za,jbo

try {
  Intl.getCanonicalLocales(["EN_US"]);
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK-NEXT: Caught{{.*}}

try {
  Intl.getCanonicalLocales([]);
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK-NEXT: Succeeded

try {
  Intl.getCanonicalLocales([""]);
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK-NEXT: Caught{{.*}}

// Invalid unicode
try {
  Intl.getCanonicalLocales(["\ud800bc"]);
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK-NEXT: Caught{{.*}}
