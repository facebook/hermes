/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -dump-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

i?.<

// CHECK: {{.*}}:10:4: error: 'identifier' expected after '.' or '?.' in member expression
// CHECK: i?.<
// CHECK: ~~~^
