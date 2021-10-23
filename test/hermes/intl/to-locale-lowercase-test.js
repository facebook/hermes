/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get locale lowercase test");
// CHECK-LABEL: get locale lowercase test

print(`${'TEST'.toLocaleLowerCase()}`);
// CHECK-NEXT: test

print(`${'İstanbul'.toLocaleLowerCase('en-US')}`);
// CHECK-NEXT: i̇stanbul

print(`${'CAFÉ'.toLocaleLowerCase('en-NZ')}`);
// CHECK-NEXT: café

print(`${',̆'.toLocaleLowerCase('cs-CZ')}`);
// CHECK-NEXT: ,̆

print(`${''.toLocaleLowerCase('en-US').length}`);
// CHECK-NEXT: 0

try {
    const wrongDataType = 26;
    wrongDataType.getCanonicalLocales(["en-GB"]);
    print("Succeeded");
  } catch (e) {
    print("Caught", e.name, e.message);
  }
  // CHECK-NEXT: Caught{{.*}}
