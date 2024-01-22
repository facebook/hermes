/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

export function join(arr: string[], sep: string): string {
  let result: string = '';
  for (let i: number = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}

export function reduce<TInput, TAcc>(
  arr: TInput[],
  fn: (acc: TAcc, item: TInput, index: number) => TAcc,
  initialValue: TAcc,
): TAcc {
  let acc = initialValue;
  for (let i = 0, e = arr.length; i < e; ++i) {
    acc = fn(acc, arr[i], i);
  }
  return acc;
}

export function map<TInput, TOutput>(
  arr: TInput[],
  fn: (item: TInput, index: number) => TOutput,
): TOutput[] {
  const output: TOutput[] = [];
  for (let i = 0, e = arr.length; i < e; ++i) {
    output.push(fn(arr[i], i));
  }
  return output;
}

export function includes<T>(arr: T[], searchElement: T): boolean {
  for (let i = 0, e = arr.length; i < e; ++i) {
    if (arr[i] === searchElement) {
      return true;
    }
  }
  return false;
}
