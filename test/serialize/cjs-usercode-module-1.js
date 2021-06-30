/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

exports.x = 42;
exports.y = 'asdf';
exports.z = require('./cjs-usercode-module-2.js').z;
