/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O %s -dump-ir | %FileCheck --check-prefix=CHKIR %s

// << can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown.

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '>>', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(BigInt(2)>>BigInt(1)));
}
