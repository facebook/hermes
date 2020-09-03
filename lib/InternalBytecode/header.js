/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// header.js + footer.js wraps around other js lib files to create a local scope
// so that local variables / functions defined within cannot be modified by users
(function() {
  'use strict';
