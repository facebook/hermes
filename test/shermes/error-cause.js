/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print('error cause');
// CHECK-LABEL: error cause

try {
  try {
    throw Error("err1");
  } catch (e1) {
    throw Error("err2", { cause: e1 });
  }
} catch (e2) {
  print(e2.message);
// CHECK-NEXT: err2
  print(e2.cause.message);
// CHECK-NEXT: err1
}

try {
  throw Error("err", {get cause() {
    print('getter');
    return 'foo';
  }});
// CHECK-NEXT: getter
} catch (e) {
  print(e.message);
// CHECK-NEXT: err
  print(e.cause);
// CHECK-NEXT: foo
}

try {
  throw Error("err", {});
} catch (e) {
  print(e.message);
// CHECK-NEXT: err
  print(Object.hasOwnProperty(e, 'cause'));
// CHECK-NEXT: false
}
