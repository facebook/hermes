/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

{
    function promotedFunction(endRecursion) {
        if(endRecursion) {
            promotedFunction(true);
            return;
        }
        print ('SUCCESS');
    }

    callback = promotedFunction;
}

{
    function promotedFunction(endRecursion) {
        if(endRecursion) {
            promotedFunction(true);
            return;
        }
        print ('FAIL');
    }
}

callback(false);
//CHECK: SUCCESS
