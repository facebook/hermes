/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

print('BigInt Binary ==');
// CHECK-LABEL: BigInt Binary ==

function printComparisonsAndResult(label, lhs, rhs) {
    print(label);
    print("==", lhs == rhs);
    print("!=", lhs != rhs);
    print("===", lhs === rhs);
    print("!==", lhs !== rhs);
}

var zero = BigInt(0);
var one = BigInt(1);

printComparisonsAndResult("0n vs 0n", zero, zero);
// CHECK-LABEL: 0n vs 0n
// CHECK: == true
// CHECK: != false
// CHECK: === true
// CHECK: !== false

printComparisonsAndResult("1n vs 1n", one, one);
// CHECK-LABEL: 1n vs 1n
// CHECK: == true
// CHECK: != false
// CHECK: === true
// CHECK: !== false

printComparisonsAndResult("1n vs true", one, true);
// CHECK-LABEL: 1n vs true
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("true vs 1n", true, one);
// CHECK-LABEL: true vs 1n
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("0n vs false", zero, false);
// CHECK-LABEL: 0n vs false
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("0n vs true", false, zero);
// CHECK-LABEL: 0n vs true
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("1n vs '1'", one, "1");
// CHECK-LABEL: 1n vs '1'
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("'1' vs 1n", "1", one);
// CHECK-LABEL: '1' vs 1n
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("0n vs '0'", zero, "0");
// CHECK-LABEL: 0n vs '0'
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("'0' vs 0n", "0", zero);
// CHECK-LABEL: '0' vs 0n
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true


printComparisonsAndResult("0n vs ''", zero, "");
// CHECK-LABEL: 0n vs ''
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("'' vs 0n", "", zero);
// CHECK-LABEL: '' vs 0n
// CHECK: == true
// CHECK: != false
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("1n vs 'invalid bigint literal'", one, "invalid bigint literal");
// CHECK-LABEL: 1n vs 'invalid bigint literal'
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("'invalid bigint literal' vs 0n", "invalid bigint literal", zero);
// CHECK-LABEL: 'invalid bigint literal' vs 0n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("1n vs NaN", one, NaN);
// CHECK-LABEL: 1n vs NaN
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("NaN vs 1n", NaN, one);
// CHECK-LABEL: NaN vs 1n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("0n vs NaN", zero, NaN);
// CHECK-LABEL: 0n vs NaN
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("NaN vs 0n", NaN, zero);
// CHECK-LABEL: NaN vs 0n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("1n vs {}", one, {});
// CHECK-LABEL: 1n vs {}
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("{} vs 1n", {}, one);
// CHECK-LABEL: {} vs 1n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("0n vs {}", zero, {});
// CHECK-LABEL: 0n vs {}
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("{} vs 0n", {}, zero);
// CHECK-LABEL: {} vs 0n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("0n vs Symbol(0)", zero, Symbol(0));
// CHECK-LABEL: 0n vs Symbol(0)
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("Symbol(0) vs 0n", Symbol(0), zero);
// CHECK-LABEL: Symbol(0) vs 0n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("1n vs Symbol(1)", zero, Symbol(1));
// CHECK-LABEL: 1n vs Symbol(1)
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true

printComparisonsAndResult("Symbol(1) vs 1n", Symbol(1), zero);
// CHECK-LABEL: Symbol(1) vs 1n
// CHECK: == false
// CHECK: != true
// CHECK: === false
// CHECK: !== true
