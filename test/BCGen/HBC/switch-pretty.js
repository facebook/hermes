/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode -pretty-disassemble -O %s | %FileCheckOrRegen --match-full-lines %s

function g() {}

function f(x) {
    switch (x) {
        case 0:
            return 32;
        case 1:
            return 342;
        case 2:
            return 322;
        case 3:
            return 132;
        case 4:
            g();
            return 342;
        case 5:
            return 362;
        case 6:
            return 323;
        case 7:
            return 3234;
        case 8:
            return 2332;
        case 9:
            return 3642;
        case 10:
            return 3211;
        case 11:
            return 2332;
        case 12:
            return 3243;
        case 13:
            return 3254;
        case 14:
            return 3342;
        case 15:
            return 3523;
        case 16:
            return 3352;
    }
    switch (x) {
        case 1:
            return 342;
        case 2:
            return 322;
        case 3:
            return 132;
        case 4:
            g();
            return 342;
        case 8:
            return 2332;
        case 9:
            return 3642;
        case 10:
            return 3211;
        case 11:
            return 2332;
        case 12:
            return 3243;
        case 13:
            return 3254;
        case 14:
            return 3342;
        default:
            g();
            break;
    }
}
