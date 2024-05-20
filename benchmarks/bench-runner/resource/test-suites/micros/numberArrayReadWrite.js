/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// This tests the cost of writing small-integer numbers to/from
// the heap.  If we are compressing the numbers, this will
// be sensitive to the cost of that compression/decompression.

function copy(arr0, arr1, arrSize) {
    for (var i = 0; i < arrSize; i++) {
        arr0[i] = arr1[i];
    }
}
function doTest(outer, arrSize) {
    var arr0 = [];
    var arr1 = [];
    for (var i = 0; i < arrSize; i++) {
        arr0.push(i);
        arr1.push(i);
    }
    var start = new Date();
    for (var i = 0; i < outer; i++) {
        copy(arr0, arr1, arrSize);
        var tmpArr = arr0;
        arr0 = arr1;
        arr1 = tmpArr;
    }
    var end = new Date();
    print("Time: " + (end - start))
}
doTest(10000, 10000);
