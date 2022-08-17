/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

new.target;
// CHECK: {{.*}}:10:1: error: 'new.target' not in a function
// CHECK: new.target;
// CHECK: ^~~~~~~~~~
