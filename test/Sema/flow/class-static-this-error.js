/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
// RUN: (! %shermes -typed -dump-sema -ferror-limit=0 %s 2>&1) | %FileCheck %s

class ThisError {
  instanceOnly: number = 1;
  static read(): number {
    return this.instanceOnly;
  }
  @Hermes.final
  static genericRead<T>(value: T): number {
    return this.instanceOnly;
  }
}
ThisError.genericRead < number > (1);

// CHECK: error: ft: static property instanceOnly not defined in class ThisError
// CHECK: error: ft: static property instanceOnly not defined in class ThisError
