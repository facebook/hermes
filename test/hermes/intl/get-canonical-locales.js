/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get canonical locales test");
// CHECK-LABEL: get canonical locales test

print(Intl.getCanonicalLocales(["EN-us", "Fr"]));
// CHECK-NEXT: en-US,fr

print(Intl.getCanonicalLocales(["zh-zh", "ZH"]));
// CHECK-NEXT: zh-ZH,zh

print(Intl.getCanonicalLocales(["cmn-hans-cn-t-ca-u-ca-a-blt-x-t-u"]));
// CHECK-NEXT: cmn-Hans-CN-a-blt-t-ca-u-ca-x-t-u

print(Intl.getCanonicalLocales(["en-us-u-asd-a-tbd-0-abc"]));
// CHECK-NEXT: en-US-0-abc-a-tbd-u-asd

try {
  Intl.getCanonicalLocales("en_uk");
} catch (exception) {
  print(exception);
}
// CHECK-NEXT: RangeError: Invalid language tag: en_uk

try {
  Intl.getCanonicalLocales("und-t-en-us-t-en-us");
} catch (exception) {
  print(exception);
}
// CHECK-NEXT: RangeError: Invalid language tag: und-t-en-us-t-en-us

try {
  Intl.getCanonicalLocales('EN-Us-u-x-test');
} catch (exception) {
  print(exception);
}
// CHECK-NEXT: RangeError: Invalid language tag: EN-Us-u-x-test

print(Intl.getCanonicalLocales("und-T-en-arab-us-z1-100-a1-101-zebra"));
// CHECK-NEXT: und-t-en-arab-us-a1-101-zebra-z1-100

print(Intl.getCanonicalLocales([]).length);
//CHECK-NEXT: 0
