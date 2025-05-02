/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir -Xcustom-opt=simplestackpromotion,mem2reg,simplifycfg,codemotion,cachenewobject %s -O | %FileCheck --match-full-lines %s

// This tests that the following two situations prevent using CacheNewObject.
// 1. The initializing stores do not dominate the return.
// 2. The first access to `this` does not dominate the return, which could be an
//    issue if the implementation does a dominator tree traversal starting at
//    that access.
function NonDominatingInit(flag) {
  if (flag){
    this.a = 0
    this.b = 1;
    this.c = 2;
  }
  return;
}

// CHECK-NOT:  {{.*}} CacheNewObjectInst {{.*}}
