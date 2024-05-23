/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This benchmark tests the speed of array reads using small literal indices.
function sum(array, n) {
    var sum = 0;
    var i = 0;
    while (i < n) {
        // Assume the array's length is evenly divisible by 10 to avoid length
        // check overhead.
        sum += array[0];
        sum += array[1];
        sum += array[2];
        sum += array[3];
        sum += array[4];
        sum += array[5];
        sum += array[6];
        sum += array[7];
        sum += array[8];
        sum += array[9];
        i++;
    }
    return sum;
}

function run(numTimes, innerNumTimes) {
    var totalSum = 0;
    var arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    for (var i = 0; i < numTimes; i++) {
        totalSum += sum(arr, innerNumTimes);
    }
    return totalSum;
}

var start = new Date();
print(run(10000, 1000));
var end = new Date();
print("Time: " + (end - start));
