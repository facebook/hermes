/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// A read of a provably-uninitialized IDZ field is constant-folded to a
// LiteralUninit load by the optimizer; it must throw ReferenceError rather than
// crash code generation. The read is inside a function whose local object never
// escapes, so the optimizer can prove the field is uninitialized and fold the
// load to LiteralUninit (a top-level "new C().f" stays a real field load and
// would not exercise the LiteralUninit codegen path).

class C {
  f: C;
  constructor() {}
}

function go(): C {
  let c = new C();
  return c.f;
}

try {
  go();
} catch (e) {
  print(e.name);
}
// CHECK: ReferenceError
