/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

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

print(bench(4e6, 100));
