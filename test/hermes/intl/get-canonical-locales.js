/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

print(Intl.getCanonicalLocales(["cmn-hans-cn-t-ca-u-ca-x-t-u"]));
// CHECK-NEXT: cmn-Hans-CN-t-ca-u-ca-x-t-u

print(Intl.getCanonicalLocales([]).length);
//CHECK-NEXT: 0
