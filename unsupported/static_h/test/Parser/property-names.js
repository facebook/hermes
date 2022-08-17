/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s

// Make sure "weird" property names work.

({default: 10});
({get: 11});
({set: 11});
({get 1 () { return 2}});
({set 2 (x) {}});
