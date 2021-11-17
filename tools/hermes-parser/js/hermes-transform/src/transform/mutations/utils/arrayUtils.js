/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

export function insertInArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
  elements: $ReadOnlyArray<T>,
): Array<T> {
  return array.slice(0, index).concat(elements).concat(array.slice(index));
}

export function removeFromArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
): Array<T> {
  return [...array.slice(0, index), ...array.slice(index + 1)];
}

export function replaceInArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
  elements: $ReadOnlyArray<T>,
): Array<T> {
  return array
    .slice(0, index)
    .concat(elements)
    .concat(array.slice(index + 1));
}
