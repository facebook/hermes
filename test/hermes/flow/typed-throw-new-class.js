/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

class CustomClass {
  name: string;
  message: string;
  constructor(message: string) {
    this.message = message;
    this.name = "CustomClass";
  }
}

try {
  throw new CustomClass("This is a custom error");
} catch (e) {
  print(e.message);
  print(e.name);
}

// CHECK: This is a custom error
// CHECK-NEXT: CustomClass
