/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

function recreateExports() {
  module.exports = {recreateExports};
}

recreateExports();

require('./cjs-caching-3.js');
