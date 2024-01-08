/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines %s

var obj = {
  get b() {},
  set b(x) {},
  get c() {},
  set d(x) {},
};
