/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -parse-jsx -dump-ast -pretty-json %s 2>&1 ) | %FileCheck %s --match-full-lines

var x = <a></a><b>y</b>;
// CHECK: {{.*}}:10:25: error: non-terminated regular expression literal
// CHECK: var x = <a></a><b>y</b>;
// CHECK:                         ^
