/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "var obj ={0" ":0,"{1..65535} ":0};" | %hermes - -target=HBC -O -gc-sanitize-handles=0 -Xjit=force -Xjit-crash-on-error
// REQUIRES: jit && !slow_debug

// The above echo generates JavaScript code like this:
// var obj = {
//   0:0,
//   1:0,
//   2:0,
//   3:0,
//   ...
//   65535:0
// };
// It's a literal object that contains 65536 literal entries.
// We want to know the JIT is able to handle this input if needed.
// -gc-sanitize-handles=0 is passed to reduce the risk of a test timeout.
