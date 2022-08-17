/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This benchmark tests the speed of array writes for a small array.

function writeNumbers(array) {
    var i = 0;
    while (i < array.length) {
        // Assume the array's length is evenly divisible by 10 to avoid length
        // check overhead.
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
        array[i++] = i;
    }
    return i;
}

function run(numTimes) {
    var totalSum = 0;
    var arr = new Array(10);
    for (var i = 0; i < numTimes; i++) {
        totalSum += writeNumbers(arr);
    }
    return totalSum;
}

print(run(1000000));
