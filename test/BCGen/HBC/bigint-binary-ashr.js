/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O %s -dump-ir | %FileCheckOrRegen --check-prefix=CHKIR %s

// << can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown.

function numberPlusBigInt() {
  return (1+(BigInt(2)>>BigInt(1)));
}
