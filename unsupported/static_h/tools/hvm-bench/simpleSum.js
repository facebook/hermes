/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function doSum() {
    var sum = 0;
    for (var i = 0; i < 100000; i++) {
        sum += i;
    }
    return sum;
}

function doSumNTimes(n) {
    var sum = 0;
    for (var i = 0; i < n; i++) {
        sum += doSum()
    }
    return sum;
}

print(doSumNTimes(10000));
