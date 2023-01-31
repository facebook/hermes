/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -g --lazy --Xhermes-internal-test-methods -O %s | %FileCheck %s
// RUN: %hermes -g --lazy --Xhermes-internal-test-methods -O0 %s | %FileCheck %s

"use strict";

// isLazy should be able to handle native functions.
print(HermesInternal.isLazy(print));
// CHECK: false

// isLazy should be able to handle non-callables.
print(HermesInternal.isLazy(10));
// CHECK: false

function lazyFunc() {}

print(HermesInternal.isLazy(lazyFunc));
// CHECK: true

print(JSON.stringify(HermesInternal.getFunctionLocation(lazyFunc)));
// CHECK: {"isNative":false,"lineNumber":21,"columnNumber":1}

lazyFunc();

print(HermesInternal.isLazy(lazyFunc));
// CHECK: false

print(JSON.stringify(HermesInternal.getFunctionLocation(lazyFunc)));
// CHECK: {"isNative":false,"lineNumber":21,"columnNumber":1,"fileName":"{{.*}}regress-hermes-internal-get-lazy-function-location.js"}
