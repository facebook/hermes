/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck %s

var handler = {
    get get() {
        revoke();
        return ()=>{
            gc();
            return () => { print("complete"); }
        };
    }
};
let { proxy, revoke } = Proxy.revocable([], handler);
proxy.prop();
// CHECK: complete
