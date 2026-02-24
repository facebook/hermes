/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -O -target=HBC %s | %FileCheck --match-full-lines %s

// The switch in this function has a backwards branch for case "4", because of
// the continue statement.  Make sure this works.
function stringSwitch(x) {
  'noinline'
  var i = 0;
  for (;;) {
    if (++i == 3) return 1000;
    switch (x) {
        case "0":
            return 32;
        case "1":
            return 342;
        case "2":
            return 322;
        case "3":
            return 132;
        case "4":
            continue;
        case "5":
            return 362;
        case "6":
            return 323;
        case "7":
            return 3234;
        case "8":
            return 2332;
        case "9":
            return 3642;
        case "10":
            return 3211;
        case "11":
            return 2332;
        case "12":
            return 3243;
        case "13":
            return 3254;
        case "14":
            return 3342;
        case "15":
            return 3523;
        case "16":
            return 3352;
    }
  }
} 

print(stringSwitch("3"));
// CHECK: 132
print(stringSwitch("4"));
// CHECK: 1000


