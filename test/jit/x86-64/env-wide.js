/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s > %t.int && %hermes -Xjit=force %s > %t.jit && diff %t.int %t.jit
// RUN: %hermes %s > %t.int && %hermes -Xjit=force -Xjit-emit-type-asserts %s > %t.jit2 && diff %t.int %t.jit2
// RUN: %hermes -O0 %s > %t.int0 && %hermes -O0 -Xjit=force %s > %t.jit0 && diff %t.int0 %t.jit0
// RUN: %hermes -Xjit=force -Xdump-jitcode=2 %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// An environment wide enough to run every path of emit_environment_init's
// slot fill. With 11 slots it does one single store, one pair of stores and
// two iterations of the 4-slot vector loop, so a wrong displacement in any of
// them leaves a slot unfilled or writes outside the cell. The closure is
// created, which is a GC safepoint, before any slot is assigned, and the
// slots are assigned only when `set` is true, so the closure reads slots that
// only the fill wrote.
function mkWide(set) {
  var s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;
  var g = function () {
    return (
      (s0 === void 0 ? 0 : s0 + 1) +
      (s1 === void 0 ? 0 : s1 + 1) +
      (s2 === void 0 ? 0 : s2 + 1) +
      (s3 === void 0 ? 0 : s3 + 1) +
      (s4 === void 0 ? 0 : s4 + 1) +
      (s5 === void 0 ? 0 : s5 + 1) +
      (s6 === void 0 ? 0 : s6 + 1) +
      (s7 === void 0 ? 0 : s7 + 1) +
      (s8 === void 0 ? 0 : s8 + 1) +
      (s9 === void 0 ? 0 : s9 + 1) +
      (s10 === void 0 ? 0 : s10 + 1)
    );
  };
  if (set) {
    s0 = 0;
    s1 = 1;
    s2 = 2;
    s3 = 3;
    s4 = 4;
    s5 = 5;
    s6 = 6;
    s7 = 7;
    s8 = 8;
    s9 = 9;
    s10 = 10;
  }
  return g;
}

// Allocate enough wide environments to force young-generation collections
// with some still live, so the GC scans slots the fill wrote. churn calls
// mkWide, so it declines; the environments are still built by compiled code.
function churn(iters) {
  var keep = null;
  var acc = 0;
  for (var i = 0; i < iters; ++i) {
    var g = mkWide((i & 1) === 0);
    acc = acc + g();
    if ((i & 4095) === 0)
      keep = g;
  }
  return acc + keep();
}

print(mkWide(true)());
// CHECK: JIT successfully compiled FunctionID 1, 'mkWide'
// CHECK: JIT successfully compiled FunctionID 3, 'g'
// CHECK-NEXT: 66
print(mkWide(false)());
// CHECK-NEXT: 0
print(churn(30000));
// CHECK: 990066
