/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-sema -fno-std-globals 2>&1) | %FileCheck --match-full-lines %s

// Duplicate fields
class DupField {
  #a;
  #a;
// CHECK:{{.*}}private-declaration-dup-error.js:13:3: error: Duplicate private identifier declaration.
// CHECK-NEXT:  #a;
// CHECK-NEXT:  ^~
}

// Duplicate field and method
class DupFieldMethod {
  #b;
  #b() {}
// CHECK:{{.*}}private-declaration-dup-error.js:22:3: error: Duplicate private identifier declaration.
// CHECK-NEXT:  #b() {}
// CHECK-NEXT:  ^~
}

// Duplicate field and accessor
class DupFieldAccessor {
  #c;
  get #c() {}
// CHECK:{{.*}}private-declaration-dup-error.js:31:7: error: Duplicate private identifier declaration.
// CHECK-NEXT:  get #c() {}
// CHECK-NEXT:      ^~
}

// Duplicate field and accessor
class DupAccessors {
  get #d() {}
  get #d() {}
// CHECK:{{.*}}private-declaration-dup-error.js:40:7: error: Duplicate private identifier declaration.
// CHECK-NEXT:  get #d() {}
// CHECK-NEXT:      ^~
}

// Static/non-static accessor with same private name.
class DiffVisibilityAccessors {
  get #e() {}
  static set #e(v) {}
// CHECK:{{.*}}private-declaration-dup-error.js:49:14: error: static and non-static private accessor with the same name
// CHECK-NEXT:  static set #e(v) {}
// CHECK-NEXT:             ^~
}

