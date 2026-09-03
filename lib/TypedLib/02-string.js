/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function charAt(this: string, pos: number): string {
  return this[pos];
}
Hermes.decorate(charAt, Hermes.builtin);

function charCodeAt(this: string, pos: number): number {
  return globalThis.String.prototype.charCodeAt.call(this, pos) as number;
}
Hermes.decorate(charCodeAt, Hermes.builtin);
