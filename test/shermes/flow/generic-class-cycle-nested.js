/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class AAA {
  constructor() {}
  foo() {
    class CCC<T> extends AAA {
      x: T;
      constructor(x: T) {
        super();
        this.x = x;
      }
    }
    let cn: CCC<number> = new CCC<number>(2);
    print(cn.x);
    let cs: CCC<string> = new CCC<string>('abc');
    print(cs.x);
  }
}

print('start');
// CHECK-LABEL: start

new AAA().foo();
// CHECK-NEXT: 2
// CHECK-NEXT: abc
