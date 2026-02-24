/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s -fno-inline | %FileCheck %s --match-full-lines

function test() {
    var o = { valueOf() { return 1n; } }
    var bigint = 1n;
    var result;

    (() => result = o + bigint)();
    print(result);
    // CHECK: 2

    (() => result = o - bigint)();
    print(result);
    // CHECK-NEXT: 0

    (() => result = o * bigint)();
    print(result);
    // CHECK-NEXT: 1

    (() => result = o / bigint)();
    print(result);
    // CHECK-NEXT: 1

    (() => result = o % bigint)();
    print(result);
    // CHECK-NEXT: 0

    (() => result = o << bigint)();
    print(result);
    // CHECK-NEXT: 2

    (() => result = o >> bigint)();
    print(result);
    // CHECK-NEXT: 0

    (() => result = o | bigint)();
    print(result);
    // CHECK-NEXT: 1

    (() => result = o ^ bigint)();
    print(result);
    // CHECK-NEXT: 0

    (() => result = o & bigint)();
    print(result);
    // CHECK-NEXT: 1

    (() => result = - bigint)();
    print(result);
    // CHECK-NEXT: -1

    (() => result = ~ bigint)();
    print(result);
    // CHECK-NEXT: -2

    (() => result = o++)();
    print(result);
    // CHECK-NEXT: 1

    (() => result = o--)();
    print(result);
    // CHECK-NEXT: 2

    (() => result = ++o)();
    print(result);
    // CHECK-NEXT: 2

    (() => result = --o)();
    print(result);
    // CHECK-NEXT: 1

    var o = function() {};
    o.valueOf = function() { return 1n; };

    (() => result = o + bigint)();
    print(result);
    // CHECK: 2

}

test();

(function() {
  function foo() {
    function o() {}
    o.valueOf = () => 10n;
    return -o;
  }

  var v = foo();
  print(typeof v, v);
  // CHECK-NEXT: bigint -10
})();
