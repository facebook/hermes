/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -gc-sanitize-handles=0 -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s -Wx,-gc-sanitize-handles=0 | %FileCheck --match-full-lines %s

// Regression test for an integer overflow when expanding $-substitutions in
// String.prototype.replace. Each `$`` expands to the portion of the input
// string before the match, so a long input string with a long replacement
// template can build a result larger than MAX_STRING_LENGTH. The intermediate
// SmallVector used to grow past its capacity limit and abort the process
// ("SmallVector capacity overflow during allocation") instead of throwing. The
// result is now bounded and a RangeError is thrown instead.

var s = "x".repeat(1024 * 1024) + "a";
var t = "$`".repeat(300);
try {
  s.replace("a", t);
  print("no error");
} catch (e) {
  print("caught", e.name);
}
//CHECK: caught RangeError
