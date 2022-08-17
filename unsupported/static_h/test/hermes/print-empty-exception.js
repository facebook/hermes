/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -O %s 2>&1 ) | %FileCheck %s

var error = {
  stack: {
    toString: function() { return "" },
  },
  toString: function() { return "MY TOSTRING" },
};

throw error;
// CHECK: MY TOSTRING
