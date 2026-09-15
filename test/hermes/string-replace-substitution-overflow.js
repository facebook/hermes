/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: huge_memory
// RUN: %hermes -O -target=HBC -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s

// Regression test for a crash when expanding $-substitutions in
// String.prototype.replace. getSubstitution() accumulates into a
// SmallU16String whose capacity is an unsigned, so the doubling in
// grow_pod() wraps once the result passes 2^31 code units and aborts the
// process ("LLVM ERROR: out of memory") before StringPrimitive::create()
// can raise a RangeError.
//
// Each $` below expands to the 64 MiB prefix, so 33 of them build 2112 MiB
// of code units, just past that 2^31 boundary. 32 or fewer stay under it
// and raise RangeError even without the fix, so this margin is the whole
// point of the test -- shrinking the input makes it pass either way.
//
// That costs ~5 GB peak RSS when the bug is present and ~1.6 GB when it is
// not, which is why this is gated behind huge_memory instead of running by
// default. To run it:
//   LIT_OPTS="--param huge_memory=1" ninja check-hermes

var s = "x".repeat(64 * 1024 * 1024) + "a";
var t = "$`".repeat(33);
try {
  s.replace("a", t);
  print("no error");
} catch (e) {
  print("caught", e.name);
}
//CHECK: caught RangeError
