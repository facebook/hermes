/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | (! %hermes - 2>&1 -gc-sanitize-handles=0 ) | %FileCheck --match-full-lines %s
// REQUIRES: !slow_debug && !gc_malloc

// Print a huge object literal with 300,000 entries.

print("globalThis.obj = {");

for(let i = 0; i < 300_000; ++i)
    print ("a" + i, ": null,");

print("};")

// CHECK: Uncaught RangeError: Object has more than {{.+}} properties
