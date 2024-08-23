/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

"use strict";

function bench (lc, fc) {
    var n, fact;
    var res = 0;
    while (--lc >= 0) {
        n = fc;
        fact = n;
        while (--n > 1)
            fact *= n;
        res += fact;
    }
    return res;
}

for(let i = 0; i != 1000; ++i)
    bench(10, 10);
var start = new Date();
print(bench(4e6, 100))
var end = new Date();
print("Time: " + (end - start));
