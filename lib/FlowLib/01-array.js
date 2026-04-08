/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

@Hermes.array
class Array<T> {
  @Hermes.final
  map<U>(
    callback: (t: T, i?: number, array?: Array<T>) => U,
  ): Array<U> {
    "inline";
    var result: U[] = [];
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      var u: U = callback(t, i, this);
      result.push(u);
    }
    return result;
  }

  @Hermes.final
  filter(
    callback: (t: T, i?: number, array?: Array<T>) => any,
  ): Array<T> {
    "inline";
    var result: T[] = [];
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      if (callback(t, i, this))
        result.push(t);
    }
    return result;
  }

  @Hermes.final
  includes(searchElement: T, fromIndex?: number): bool {
    "inline";
    var len: number = this.length;
    var start: number;
    if (fromIndex !== undefined) {
      start = fromIndex;
      start = __ToIntegerOrInfinity(start);
      if (start < 0) {
        start = len + start;
        if (start < 0)
          start = 0;
      }
    } else {
      start = 0;
    }
    for (var i: number = start; i < len; ++i) {
      if (__SameValueZero(this[i], searchElement))
        return true;
    }
    return false;
  }

  @Hermes.final
  forEach(
    callback: (t: T, i?: number, array?: Array<T>) => void,
  ): void {
    "inline";
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      callback(t, i, this);
    }
  }

  @Hermes.final
  flatMap<U>(
    callback: (t: T, i?: number, array?: Array<T>) => Array<U>,
  ): Array<U> {
    "inline";
    var result: Array<U> = [];
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      // TODO: callback return type should be U | Array<U> per spec (non-array
      // values are kept as-is, arrays are flattened one level).
      // Needs runtime Array.isArray() or instanceof check in typed mode.
      var arr: Array<U> = callback(t, i, this);
      result.push(...arr);
    }
    return result;
  }

  @Hermes.final
  concat(other: Array<T>): Array<T> {
    "inline";
    return [...this, ...other];
  }

  @Hermes.final
  toString(): string {
    var len: number = this.length;
    if (len === 0)
      return "";
    var el0: T = this[0];
    var result: string = (el0 === undefined || el0 === null) ? "" : String(el0);
    for (var i: number = 1; i < len; ++i) {
      result += ",";
      var el: T = this[i];
      if (el !== undefined && el !== null)
        result += String(el);
    }
    return result;
  }
}
