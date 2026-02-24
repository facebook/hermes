/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// header.js + footer.js wraps around other js lib files to create a local scope
// so that local variables / functions defined within cannot be modified by users
(function() {
  'use strict';

  // This object will be returned as the completion value of executing internal
  // bytecode (this IIFE). It provides a way to return JS values to native code.
  // E.g. the only use currently is to register JSFunctions as JS builtins that
  // "CallBuiltin" opcode can call.
  var internalBytecodeResult = {};
