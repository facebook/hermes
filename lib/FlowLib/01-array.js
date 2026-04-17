/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

@Hermes.array
class Array<T> {
  @Hermes.final
  map<U, This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => U,
    thisArg?: This,
  ): Array<U> {
    "inline";
    var result: U[] = [];
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      var u: U = $SHBuiltin.call(callback, thisArg, t, i, this);
      result.push(u);
    }
    return result;
  }

  @Hermes.final
  filter<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => any,
    thisArg?: This,
  ): Array<T> {
    "inline";
    var result: T[] = [];
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      if ($SHBuiltin.call(callback, thisArg, t, i, this))
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
  forEach<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => void,
    thisArg?: This,
  ): void {
    "inline";
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      $SHBuiltin.call(callback, thisArg, t, i, this);
    }
  }

  @Hermes.final
  flatMap<U, This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => Array<U>,
    thisArg?: This,
  ): Array<U> {
    "inline";
    var result: Array<U> = [];
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      // TODO: callback return type should be U | Array<U> per spec (non-array
      // values are kept as-is, arrays are flattened one level).
      // Needs runtime Array.isArray() or instanceof check in typed mode.
      var arr: Array<U> = $SHBuiltin.call(callback, thisArg, t, i, this);
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

  @Hermes.final
  indexOf(searchElement: T, fromIndex?: number): number {
    "inline";
    var len: number = this.length;
    var start: number;
    if (fromIndex !== undefined) {
      start = __ToIntegerOrInfinity(fromIndex);
      if (start < 0) {
        start = len + start;
        if (start < 0)
          start = 0;
      }
    } else {
      start = 0;
    }
    for (var i: number = start; i < len; ++i) {
      if (this[i] === searchElement)
        return i;
    }
    return -1;
  }

  @Hermes.final
  lastIndexOf(searchElement: T, fromIndex?: number): number {
    "inline";
    var len: number = this.length;
    var start: number;
    if (fromIndex !== undefined) {
      start = __ToIntegerOrInfinity(fromIndex);
      if (start < 0)
        start = len + start;
      if (start >= len)
        start = len - 1;
    } else {
      start = len - 1;
    }
    for (var i: number = start; i >= 0; --i) {
      if (this[i] === searchElement)
        return i;
    }
    return -1;
  }

  // TODO: at.
  // TODO: every / some.
  // TODO: find / findIndex.
  // TODO: findLast / findLastIndex.
  // TODO: join.
  // TODO: slice.
  // TODO: toReversed.
  // TODO: with.
  // TODO: fill.
  // TODO: reverse.
  // TODO: copyWithin.
  // TODO: reduce / reduceRight.
  // TODO: flat — needs recursive type flattening.
  // TODO: splice / toSpliced — no varargs support.
  // TODO: push / unshift — no varargs support (push is special-cased
  //   internally).
  // TODO: pop / shift — cannot assign this.length in FlowLib.
  // TODO: sort — complex comparator typing.
  // TODO: isArray / of / from — static methods.
  // TODO: keys / values / entries — requires Symbol.iterator naming.
  // TODO: toLocaleString — low value.
}
