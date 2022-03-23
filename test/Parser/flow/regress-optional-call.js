/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

i?.<T>+

// CHECK: {{.*}}:10:7: error: '(' expected after type arguments in optional call
// CHECK: i?.<T>+
// CHECK: ~~~~~~^
