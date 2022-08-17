/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O
// RUN: %hermes -hermes-parser -dump-ir %s

// Make sure that we are not crashing on this one:
function assertEquals(x, y) { }
assertEquals(true, true ? true : false);


