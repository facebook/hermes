/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

export var x = 42;

export function y() {
  return 182;
}

export default function() {
  return 352;
}

var myLongVariableName = 472;
var shortVar = 157;

export {myLongVariableName as z, shortVar};

export * from './esm-bar.js';
