/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

type A = hook (this: T) => void;
// CHECK: {{.*}}:10:16: error: hooks do not support 'this' constraints
// CHECK-NEXT: type A = hook (this: T) => void;
// CHECK-NEXT:                ^~~~
