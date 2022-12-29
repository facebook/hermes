/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O %s -dump-ir | %FileCheck --check-prefix=CHKIR %s

// Tilde (i.e., negation) can no longer be assumed to return int32 -- it returns a
// numeric when its argument's type is unknown. This means that
//
//     number + (~"thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = UnaryOperatorInst '~', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(~a));
}
