/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// SameValueZero intrinsic.
function __SameValueZero(x: any, y: any): bool {
  "inline";
  if (x !== x && y !== y) {
    return true;
  }
  return x === y;
}

/// ToIntegerOrInfinity (ES2022 7.1.5).
function __ToIntegerOrInfinity(n: number): number {
  "inline";
  if (n !== n) return 0;
  var trunc: number = n - (n % 1);
  // Infinity % 1 is NaN, so account for that and return Infinity.
  if (trunc !== trunc) return n;
  return trunc;
}

/// Spec ToString. Primitives use AddEmptyString, avoiding the global String()
/// lookup and call. Objects go through String() because `"" + v` would call
/// valueOf() before toString().
function __ToString<T>(v: T): string {
  "inline";
  const a: any = v;
  if (typeof a === "object" || typeof a === "function") return String(a);
  return "" + a;
}
