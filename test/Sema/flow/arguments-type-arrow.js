/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-std-globals -typed -dump-sema %s

// Test that arrows use the outer function's 'arguments' binding.
function untypedOuterArrow(x) {
  var f = (): any => arguments[0];
}
