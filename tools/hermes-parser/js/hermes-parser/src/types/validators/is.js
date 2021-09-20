/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import shallowEqual from '../utils/shallowEqual';
import isType from './isType';

/**
 * Returns whether `node` is of given `type`.
 *
 * For better performance, use this instead of `is[Type]` when `type` is unknown.
 */
export default function is(
  type: string,
  node: t.Node | null | undefined,
  opts?: Partial<t.Node>,
) {
  if (!node) return false;

  const matches = isType(node.type, type);
  if (!matches) {
    return false;
  }

  if (typeof opts === 'undefined') {
    return true;
  } else {
    return shallowEqual(node, opts);
  }
}
