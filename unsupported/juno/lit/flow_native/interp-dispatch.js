/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

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
// CHECK: 373304861765286933565920470441822466257223788178678581982964159733362287969947809641457290407594073428410904372057793030326145493814704070178859617284533689933365248.000000
