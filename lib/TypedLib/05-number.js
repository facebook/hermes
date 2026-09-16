/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function toFixed(this: number, digits?: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.Number.prototype.toFixed,
    this,
    digits
  ) as string;
}
Hermes.decorate(toFixed, Hermes.builtin);

function toPrecision(this: number, precision?: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.Number.prototype.toPrecision,
    this,
    precision
  ) as string;
}
Hermes.decorate(toPrecision, Hermes.builtin);

function toExponential(this: number, digits?: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.Number.prototype.toExponential,
    this,
    digits
  ) as string;
}
Hermes.decorate(toExponential, Hermes.builtin);

function toString(this: number, radix?: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.Number.prototype.toString,
    this,
    radix
  ) as string;
}
Hermes.decorate(toString, Hermes.builtin);
