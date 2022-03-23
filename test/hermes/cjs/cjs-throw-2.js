/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

// Throw if the main module doesn't have x set to 5.

var x = require('./cjs-throw-1.js').x;
if (x !== 5) throw new Error("INVALID X");
exports.y = 42;
