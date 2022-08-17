/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This benchmark tests the speed of array reads for a small array.
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
    var arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    for (var i = 0; i < numTimes; i++) {
        totalSum += sum(arr);
    }
    return totalSum;
}

print(run(1000000));
