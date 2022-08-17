/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s
// RUN: %hermes -hermes-parser -dump-ir %s -O

  for (a[1?2:31] in x) {
  }
