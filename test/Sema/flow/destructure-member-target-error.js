/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Destructuring into a typed member expression is narrowed at runtime when the
// source type is unknown. A statically known source type is checked instead.

let so: {s: string} = {s: ''};
let src: any = null;
let optional: {p: string | void} = {p: undefined};

// Legal: the store narrows at runtime.
({p: so.s} = src);

({p: so.s} = {p: 1});
// The default is stored into the target, so it is checked against it.
({p: so.s = 1} = optional);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}destructure-member-target-error.js:20:6: error: ft: incompatible object destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:({p: so.s} = {p: 1});
// CHECK-NEXT:     ^~~~
// CHECK-NEXT:{{.*}}destructure-member-target-error.js:22:13: error: ft: incompatible destructuring default type: cannot implicitly cast from 1 to string
// CHECK-NEXT:({p: so.s = 1} = optional);
// CHECK-NEXT:            ^
// CHECK-NEXT:Emitted 2 errors. exiting.
