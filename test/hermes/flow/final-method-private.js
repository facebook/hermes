/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// @Hermes.final on a private method.
class Box {
  val: number;

  constructor(v: number) {
    this.val = v;
  }

  @Hermes.final
  #double(): number {
    return this.val * 2;
  }

  getDouble(): number {
    return this.#double();
  }
}

let b: Box = new Box(5);
print(b.getDouble());
// CHECK: 10
