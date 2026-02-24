/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// This was failing in ASAN due to a use-after-free in BytecodeGenerator,
// which didn't set references correctly if function HBC generation failed.
// The HBC generation fails in unoptimized IR because the function is so large.

var s = '';
for (var i = 0; i < 70000; i++)
    s += 'function x' + i + '() { x' + i + '(); }\n';
try {
  eval("(function() { " + s + " })();");
} catch (e) {
  print(e.name);
}
// CHECK: SyntaxError
