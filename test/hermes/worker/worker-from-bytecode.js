/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Prove that a Worker constructed from precompiled Hermes bytecode actually
// EXECUTES the bytecode (not merely that construction succeeds). The worker
// body is compiled to bytecode, embedded as a Uint8Array (`BC`), and run; the
// worker posts a sentinel back which the CLI event loop delivers to us.

// RUN: echo 'onmessage = function() { postMessage("ran-from-bytecode"); };' > %t.body.js
// RUN: %hermesc -emit-binary -out %t.hbc %t.body.js
// RUN: %python %S/Inputs/hbc_to_js.py %t.hbc %t.pre.js
// RUN: cat %t.pre.js %s > %t.run.js
// RUN: %hermes %t.run.js | %FileCheck %s --match-full-lines

// `BC` is prepended by the RUN pipeline: the bytecode of the worker body above.
var worker = new Worker(BC.buffer);
worker.onmessage = function (msg) {
  print("bytecode: " + msg);
  worker.terminate();
};
worker.postMessage("go");

// CHECK: bytecode: ran-from-bytecode
