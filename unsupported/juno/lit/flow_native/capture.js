/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

function fn1(param1){
    var var1 = 1;
    function fn2(param2){
        var var2 = 2;
        function fn3(param3){
            var var3 = 3;
            return var1 + var2 + var3 + param1 + param2 + param3;
        }
        return fn3;
    }
    return fn2;
}

fn1(4)(5)(6);
