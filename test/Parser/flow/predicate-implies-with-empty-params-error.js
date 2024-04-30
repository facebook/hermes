/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

function foo(x: mixed): implies<> x is number {}
// CHECK: {{.*}}:10:35: error: invalid return annotation. 'implies' type guard needs to be followed by identifier
// CHECK: function foo(x: mixed): implies<> x is number {}
// CHECK:                                   ^
