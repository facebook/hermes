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
    return this.join();
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

  @Hermes.final
  at(index: number): T | void {
    "inline";
    var len: number = this.length;
    index = __ToIntegerOrInfinity(index);
    if (index >= 0) {
      return index < len ? this[index] : undefined;
    } else {
      index += len;
      return index >= 0 ? this[index] : undefined;
    }
  }

  @Hermes.final
  every<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => boolean,
    thisArg?: This,
  ): boolean {
    "inline";
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      if (!$SHBuiltin.call(callback, thisArg, t, i, this))
        return false;
    }
    return true;
  }

  @Hermes.final
  some<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => boolean,
    thisArg?: This,
  ): boolean {
    "inline";
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      if ($SHBuiltin.call(callback, thisArg, t, i, this))
        return true;
    }
    return false;
  }

  @Hermes.final
  find<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => boolean,
    thisArg?: This,
  ): T | void {
    "inline";
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      if ($SHBuiltin.call(callback, thisArg, t, i, this))
        return t;
    }
    return undefined;
  }

  @Hermes.final
  findIndex<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => boolean,
    thisArg?: This,
  ): number {
    "inline";
    for (var i: number = 0, e = this.length; i < e; ++i) {
      var t: T = this[i];
      if ($SHBuiltin.call(callback, thisArg, t, i, this))
        return i;
    }
    return -1;
  }

  @Hermes.final
  findLast<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => boolean,
    thisArg?: This,
  ): T | void {
    "inline";
    for (var i: number = this.length - 1; i >= 0; --i) {
      var t: T = this[i];
      if ($SHBuiltin.call(callback, thisArg, t, i, this))
        return t;
    }
    return undefined;
  }

  @Hermes.final
  findLastIndex<This>(
    callback: (this: This, t: T, i: number, array: Array<T>) => boolean,
    thisArg?: This,
  ): number {
    "inline";
    for (var i: number = this.length - 1; i >= 0; --i) {
      var t: T = this[i];
      if ($SHBuiltin.call(callback, thisArg, t, i, this))
        return i;
    }
    return -1;
  }

  @Hermes.final
  join(separator?: string): string {
    "inline";
    var len: number = this.length;
    if (len === 0)
      return "";
    var sep: string = separator !== undefined ? separator : ",";
    var el0: T = this[0];
    var result: string = (el0 === undefined || el0 === null) ? "" : String(el0);
    for (var i: number = 1; i < len; ++i) {
      result += sep;
      var el: T = this[i];
      if (el !== undefined && el !== null)
        result += String(el);
    }
    return result;
  }

  @Hermes.final
  slice(start?: number, end?: number): Array<T> {
    var len: number = this.length;
    var s: number = start !== undefined ? __ToIntegerOrInfinity(start) : 0;
    if (s < 0) {
      s += len;
      if (s < 0)
        s = 0;
    }
    var e: number = end !== undefined ? __ToIntegerOrInfinity(end) : len;
    if (e < 0)
      e += len;
    if (e > len)
      e = len;
    var result: T[] = [];
    for (var i: number = s; i < e; ++i) {
      result.push(this[i]);
    }
    return result;
  }

  @Hermes.final
  toReversed(): Array<T> {
    "inline";
    var result: Array<T> = [];
    for (var i: number = this.length - 1; i >= 0; --i) {
      result.push(this[i]);
    }
    return result;
  }

  @Hermes.final
  with(index: number, value: T): Array<T> {
    "inline";
    var len: number = this.length;
    var idx: number = __ToIntegerOrInfinity(index);
    if (idx < 0) {
      idx += len;
      if (idx < 0)
        throw new RangeError("Invalid index");
    } else if (idx >= len) {
      throw new RangeError("Invalid index");
    }
    // Spread the whole array and then overwrite.
    // Avoids branching every iteration.
    var result: T[] = [...this];
    result[idx] = value;
    return result;
  }

  @Hermes.final
  fill(value: T, start?: number, end?: number): Array<T> {
    "inline";
    var len: number = this.length;
    var s: number = start !== undefined ? __ToIntegerOrInfinity(start) : 0;
    if (s < 0) {
      s += len;
      if (s < 0)
        s = 0;
    }
    var e: number = end !== undefined ? __ToIntegerOrInfinity(end) : len;
    if (e < 0)
      e += len;
    if (e > len)
      e = len;
    for (var i: number = s; i < e; ++i) {
      this[i] = value;
    }
    return this;
  }

  @Hermes.final
  reverse(): Array<T> {
    "inline";
    var lo: number = 0;
    var hi: number = this.length - 1;
    while (lo < hi) {
      var tmp: T = this[lo];
      this[lo] = this[hi];
      this[hi] = tmp;
      ++lo;
      --hi;
    }
    return this;
  }

  @Hermes.final
  copyWithin(target: number, start: number, end?: number): Array<T> {
    var len: number = this.length;
    var to: number = __ToIntegerOrInfinity(target);
    if (to < 0) {
      to = len + to;
      if (to < 0)
        to = 0;
    }
    var from: number = __ToIntegerOrInfinity(start);
    if (from < 0) {
      from = len + from;
      if (from < 0)
        from = 0;
    }
    var fin: number;
    if (end !== undefined) {
      fin = __ToIntegerOrInfinity(end);
      if (fin < 0)
        fin = len + fin;
      if (fin > len)
        fin = len;
    } else {
      fin = len;
    }
    var count: number = fin - from;
    if (count <= 0)
      return this;
    if (to + count > len)
      count = len - to;
    if (from < to) {
      var i: number = count - 1;
      while (i >= 0) {
        this[to + i] = this[from + i];
        --i;
      }
    } else {
      for (var j: number = 0; j < count; ++j) {
        this[to + j] = this[from + j];
      }
    }
    return this;
  }

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
