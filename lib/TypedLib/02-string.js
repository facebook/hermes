/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function charAt(this: string, pos: number): string {
  'inline';
  return this[pos];
}
Hermes.decorate(charAt, Hermes.builtin);

function at(this: string, index: number): string | void {
  'inline';
  return $SHBuiltin.call(globalThis.String.prototype.at, this, index) as
    | string
    | void;
}
Hermes.decorate(at, Hermes.builtin);

function charCodeAt(this: string, pos: number): number {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.charCodeAt,
    this,
    pos
  ) as number;
}
Hermes.decorate(charCodeAt, Hermes.builtin);

function codePointAt(this: string, pos: number): number | void {
  'inline';
  return $SHBuiltin.call(globalThis.String.prototype.codePointAt, this, pos) as
    | number
    | void;
}
Hermes.decorate(codePointAt, Hermes.builtin);

/// TODO: Varargs.
function concat(this: string, other: string): string {
  'inline';
  return this + other;
}
Hermes.decorate(concat, Hermes.builtin);

function endsWith(
  this: string,
  searchString: string,
  endPosition?: number
): boolean {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.endsWith,
    this,
    searchString,
    endPosition
  ) as boolean;
}
Hermes.decorate(endsWith, Hermes.builtin);

function includes(
  this: string,
  searchString: string,
  position?: number
): boolean {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.includes,
    this,
    searchString,
    position
  ) as boolean;
}
Hermes.decorate(includes, Hermes.builtin);

function indexOf(
  this: string,
  searchString: string,
  position?: number
): number {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.indexOf,
    this,
    searchString,
    position
  ) as number;
}
Hermes.decorate(indexOf, Hermes.builtin);

function lastIndexOf(
  this: string,
  searchString: string,
  position?: number
): number {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.lastIndexOf,
    this,
    searchString,
    position
  ) as number;
}
Hermes.decorate(lastIndexOf, Hermes.builtin);

function padEnd(
  this: string,
  targetLength: number,
  padString?: string
): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.padEnd,
    this,
    targetLength,
    padString
  ) as string;
}
Hermes.decorate(padEnd, Hermes.builtin);

function padStart(
  this: string,
  targetLength: number,
  padString?: string
): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.padStart,
    this,
    targetLength,
    padString
  ) as string;
}
Hermes.decorate(padStart, Hermes.builtin);

function repeat(this: string, count: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.repeat,
    this,
    count
  ) as string;
}
Hermes.decorate(repeat, Hermes.builtin);

function slice(this: string, start?: number, end?: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.slice,
    this,
    start,
    end
  ) as string;
}
Hermes.decorate(slice, Hermes.builtin);

/// TODO: Return a proper FastArray here.
function split(this: string, separator?: string, limit?: number): any {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.split,
    this,
    separator,
    limit
  );
}
Hermes.decorate(split, Hermes.builtin);

function startsWith(
  this: string,
  searchString: string,
  position?: number
): boolean {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.startsWith,
    this,
    searchString,
    position
  ) as boolean;
}
Hermes.decorate(startsWith, Hermes.builtin);

function substring(this: string, start?: number, end?: number): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.substring,
    this,
    start,
    end
  ) as string;
}
Hermes.decorate(substring, Hermes.builtin);

function toLowerCase(this: string): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.toLowerCase,
    this
  ) as string;
}
Hermes.decorate(toLowerCase, Hermes.builtin);

function toUpperCase(this: string): string {
  'inline';
  return $SHBuiltin.call(
    globalThis.String.prototype.toUpperCase,
    this
  ) as string;
}
Hermes.decorate(toUpperCase, Hermes.builtin);

function trim(this: string): string {
  'inline';
  return $SHBuiltin.call(globalThis.String.prototype.trim, this) as string;
}
Hermes.decorate(trim, Hermes.builtin);

function trimStart(this: string): string {
  'inline';
  return $SHBuiltin.call(globalThis.String.prototype.trimStart, this) as string;
}
Hermes.decorate(trimStart, Hermes.builtin);

function trimEnd(this: string): string {
  'inline';
  return $SHBuiltin.call(globalThis.String.prototype.trimEnd, this) as string;
}
Hermes.decorate(trimEnd, Hermes.builtin);

// TODO: replace / replaceAll / search / match / matchAll — need a typed
//       RegExp and typed match results.
// TODO: localeCompare / normalize / toLocaleLowerCase / toLocaleUpperCase.
// TODO: isWellFormed / toWellFormed.
// TODO: substr — Annex B.
