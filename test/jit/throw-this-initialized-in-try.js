/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// ThrowIfThisInitialized longjmps to a catch handler in the same function,
// which reads its frame registers from memory. Check that values assigned
// before the throw survive, in the shapes where a live value could plausibly
// still be sitting in a temp register.
//
// NOTE: this is a smoke test, not a regression test for the in-try sync in
// Emitter::throwIfThisInitialized. It passes with or without that sync, and
// cannot be made to fail: ThrowIfThisInitialized is always emitted
// immediately after the super call, and callImpl already flushes every live
// FR before any call, so no JS-producible shape leaves a dirty FR between the
// call and the check. Its value is locking the behavior in for the x86-64
// port.

class Base {
  constructor() {
    this.b = 1;
  }
}

// Straight-line double super().
class Straight extends Base {
  constructor(y) {
    var x = -1;
    try {
      super();
      x = y + 1;
      super();
    } catch (e) {
      print("Straight", x);
    }
  }
}
new Straight(41);
// CHECK: Straight 42

// A single super() site executed twice by a loop.
class LoopSuper extends Base {
  constructor(y) {
    var x = -1;
    try {
      for (var i = 0; i < 2; ++i) {
        x = y + i;
        super();
      }
    } catch (e) {
      print("LoopSuper", x);
    }
  }
}
new LoopSuper(41);
// CHECK-NEXT: LoopSuper 42

// Several independent live values across the throw.
class ManyLive extends Base {
  constructor(y) {
    var a = -1,
      b = -2,
      c = -3;
    try {
      super();
      a = y + 1;
      b = y * 2;
      c = y - 1;
      super();
    } catch (e) {
      print("ManyLive", a, b, c);
    }
  }
}
new ManyLive(41);
// CHECK-NEXT: ManyLive 42 82 40

// A value derived from the object the first super() produced.
class FromThis extends Base {
  constructor(y) {
    var x = -1;
    try {
      super();
      x = this.b + y;
      super();
    } catch (e) {
      print("FromThis", x);
    }
  }
}
new FromThis(41);
// CHECK-NEXT: FromThis 42
