/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */

// header.js + footer.js wraps around other js lib files to create a local scaope
// so that local variables / functions defined wihtin cannot be modified by users
(function() {
  'use strict';

  var _TypeError = TypeError;
  var _MathFloor = Math.floor;
