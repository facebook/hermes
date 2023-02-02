/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -dump-ir %s 2>&1 | %FileCheck %s --match-full-lines

//CHECK:{{.*}}warning: the property "color" was set multiple times in the object definition.
//CHECK-NEXT:var x = { color: 10, color: 20, color: 30 };
//CHECK-NEXT:                     ^~~~~~~~~
//CHECK-NEXT:{{.*}}note: The first definition was here.
//CHECK-NEXT:var x = { color: 10, color: 20, color: 30 };
//CHECK-NEXT:          ^~~~~~~~~
//CHECK-NEXT:{{.*}}warning: the property "color" was set multiple times in the object definition.
//CHECK-NEXT:var x = { color: 10, color: 20, color: 30 };
//CHECK-NEXT:                                ^~~~~~~~~
//CHECK-NEXT:{{.*}}note: The first definition was here.
//CHECK-NEXT:var x = { color: 10, color: 20, color: 30 };
//CHECK-NEXT:          ^~~~~~~~~
var x = { color: 10, color: 20, color: 30 };

//CHECK:{{.*}}warning: the property "color" was set multiple times in the object definition.
//CHECK-NEXT:var y = { color: 10, color: 20, __proto__: {}};
//CHECK-NEXT:                     ^~~~~~~~~
//CHECK-NEXT:{{.*}}note: The first definition was here.
//CHECK-NEXT:var y = { color: 10, color: 20, __proto__: {}};
//CHECK-NEXT:          ^~~~~~~~~
var y = { color: 10, color: 20, __proto__: {}};
