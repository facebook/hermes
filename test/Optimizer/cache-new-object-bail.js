/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fcache-new-object -dump-ir %s -O | %FileCheck --match-full-lines %s

// A variety of different functions which don't trigger CacheNewObjectInst
// to be emitted.

function cond(x, y) {
  if (y)
    this.y = y;
  this.x = x;
}

function assignProp(x, y) {
  x[this] = y;
}

function assignNotLit(x, y) {
  this[x] = x;
}

function usesThis(x, y) {
  this.x = x;
  this.y = this.z;
}

function noUses(x, y) {
  print(x);
}

function callArg(x, y) {
  print(this);
}

// CHECK-NOT:  %{{.*}} CacheNewObjectInst {{.*}}
