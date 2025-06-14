/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

(function (){
    let arr = [1,2,3,4];
    // Call reverse so the prototype gets cached.
    arr.reverse();
    print(JSON.stringify(arr));
    // CHECK: [4,3,2,1]

    // Test adding properties to the indexed storage of the prototype.
    arr = [1,,,4];
    let arrProto = arr.__proto__;
    arrProto[1] = 2;
    arr.reverse();
    print(JSON.stringify(arr));
    // CHECK-NEXT: [4,2,2,1]

    // Clear the indexed storage in the prototype.
    arrProto.length = 0;
    arr.reverse();
    print(JSON.stringify(arr));
    // CHECK-NEXT: [1,2,null,4]

    // Update the parent of arrProto to an object and refresh the cache.
    let newProtoPar = {};
    arrProto.__proto__ = newProtoPar;
    arr = [1,,,4];
    arr.reverse();
    print(JSON.stringify(arr));
    // CHECK-NEXT: [4,null,null,1]

    // Introduce an index like property in the prototype chain.
    newProtoPar[2] = 3;
    arr.reverse();
    print(JSON.stringify(arr));
    // CHECK-NEXT: [1,3,3,4]
})();
