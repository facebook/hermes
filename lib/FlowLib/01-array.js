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
