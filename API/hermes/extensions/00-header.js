/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// header.js + footer.js wraps around extension JS files to create a local scope
// so that local variables / functions defined within cannot be modified by users.
// The IIFE returns an object with setup functions keyed by extension name.
(function() {
  'use strict';
  var extensions = {};
