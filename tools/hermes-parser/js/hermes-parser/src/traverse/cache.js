/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

export let path = new WeakMap();
export let scope = new WeakMap();

export function clear() {
  clearPath();
  clearScope();
}

export function clearPath() {
  path = new WeakMap();
}

export function clearScope() {
  scope = new WeakMap();
}
