/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hermesc -dump-ir -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines

var obj = {__proto__:{}, a:1, __proto__:{}}
//CHECK: {{.*}}__proto__duplicated.js:10:31: error: __proto__ was set multiple times in the object definition.
//CHECK-NEXT: var obj = {__proto__:{}, a:1, __proto__:{}}
//CHECK-NEXT:                               ^~~~~~~~~~~~
//CHECK: {{.*}}__proto__duplicated.js:10:12: note: The first definition was here.
//CHECK-NEXT: var obj = {__proto__:{}, a:1, __proto__:{}}
//CHECK-NEXT:            ^~~~~~~~~~~~
//CHECK-NEXT: Emitted 1 errors. exiting.
