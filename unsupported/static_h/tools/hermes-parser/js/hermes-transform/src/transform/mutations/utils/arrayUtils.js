/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

function assertArrayBounds<T>(array: $ReadOnlyArray<T>, index: number): void {
  if (index < 0 || index >= array.length) {
    throw new Error(
      `Invalid Mutation: Tried to mutate an elements array with an out of bounds index. Index: ${index}, Array Size: ${array.length}`,
    );
  }
}

export function insertInArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
  elements: $ReadOnlyArray<T>,
): Array<T> {
  if (index === array.length) {
    // Support the insert at end of array case.
    return array.concat(elements);
  }
  assertArrayBounds(array, index);
  return array.slice(0, index).concat(elements).concat(array.slice(index));
}

export function removeFromArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
): Array<T> {
  assertArrayBounds(array, index);
  return [...array.slice(0, index), ...array.slice(index + 1)];
}

export function replaceInArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
  elements: $ReadOnlyArray<T>,
): Array<T> {
  assertArrayBounds(array, index);
  return array
    .slice(0, index)
    .concat(elements)
    .concat(array.slice(index + 1));
}
