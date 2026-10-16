/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Intended location: test/Sema/flow/class-static-super-error.js
// RUN: (! %shermes -typed -dump-sema -ferror-limit=0 %s 2>&1) | %FileCheck %s

class SuperErrorBase {
  instanceOnly: number = 1;
}
class SuperErrorChild extends SuperErrorBase {
  static read(): number {
    return super.instanceOnly;
  }
  @Hermes.final
  static genericRead<T>(value: T): number {
    return super.instanceOnly;
  }
}
SuperErrorChild.genericRead<number>(1);

// CHECK: error: ft: static property instanceOnly not defined in class SuperErrorBase
// CHECK: error: ft: static property instanceOnly not defined in class SuperErrorBase
