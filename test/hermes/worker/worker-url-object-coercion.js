/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

// With no provider registered, a non-buffer object is coerced with ToString and
// (no provider) treated as source. Here the object's string form is valid JS.
var codeObject = {
  toString() {
    return "postMessage('coerced-ran');";
  },
};

var worker = new Worker(codeObject);
worker.onmessage = function (msg) {
  print(msg);
  worker.terminate();
};

// CHECK: coerced-ran
