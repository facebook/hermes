/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xflow-parser %s | %FileCheck %s --match-full-lines
// REQUIRES: flowparser
// XFAIL: *

var obj = {
  ['asdf']: function() {}
}

print(obj.asdf.name);
// CHECK: asdf
