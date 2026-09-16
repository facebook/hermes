/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

// {allowData:true} decodes a data: URL natively (no provider needed).
var w1 = new Worker("data:text/javascript,postMessage('data-src')", {
  allowData: true,
});
w1.onmessage = function (msg) {
  print("src: " + msg);
  w1.terminate();
};

// CHECK: src: data-src
