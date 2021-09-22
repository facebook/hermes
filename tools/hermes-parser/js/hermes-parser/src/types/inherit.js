/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

export default function inherit(
  key: string,
  child: Object,
  parent: Object,
): void {
  if (child && parent) {
    child[key] = Array.from(
      new Set([].concat(child[key], parent[key]).filter(Boolean)),
    );
  }
}
