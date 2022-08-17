/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -target=HBC -O %s | %FileCheck --match-full-lines %s

function strictFunc() {
    "use strict";
    print(typeof this, Object.prototype.toString.call(this));
}

function nonStrictFunc() {
    print(typeof this, Object.prototype.toString.call(this));
}

strictFunc.call(1);
//CHECK: number [object Number]
strictFunc.call(null);
//CHECK-NEXT: object [object Null]
strictFunc.call(undefined);
//CHECK-NEXT: undefined [object Undefined]

nonStrictFunc.call(1);
//CHECK-NEXT: object [object Number]
nonStrictFunc.call(null);
//CHECK-NEXT: object [object global]
nonStrictFunc.call(undefined);
//CHECK-NEXT: object [object global]
