/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This benchmark tests the speed of array reads for a large array.

// This returns an array that looks like [1, 2, 3, ..., length].
function makeLargeArray(length) {
    return new Array(length).fill().map(function (_, i) { return i + 1; });
}

function sum(array) {
    var sum = 0;
    var i = 0;
    while (i < array.length) {
        // Assume the array's length is evenly divisible by 10 to avoid length
        // check overhead.
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
        sum += array[i++];
    }
    return sum;
}

function run(numTimes) {
    var totalSum = 0;
    // Hardcode ten thousand elements as a large array. This can be adjusted
    // if it is too small.
    var arr = makeLargeArray(10000);
    for (var i = 0; i < numTimes; i++) {
        totalSum += sum(arr);
    }
    return totalSum;
}

print(run(10000));
