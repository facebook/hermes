/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

function foo<const>() {}
// CHECK: {{.*}}:10:19: error: 'identifier' expected in type parameter
