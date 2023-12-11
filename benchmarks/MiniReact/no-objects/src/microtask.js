/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

let microtaskQueue = [];

export function drainMicrotaskQueue(): void {
  for (let i = 0; i < microtaskQueue.length; i++) {
    microtaskQueue[i]();
    microtaskQueue[i] = undefined;
  }
  microtaskQueue = [];
}

export function queueMicrotask(callback: () => void): void {
  microtaskQueue.push(callback);
}
