/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import isType from './isType';
import {isIdentifier} from '../generated/node-types';

/**
 * Check if the input `node` is definitely immutable.
 */
export default function isImmutable(node: t.Node): boolean {
  if (isType(node.type, 'Immutable')) return true;

  if (isIdentifier(node)) {
    if (node.name === 'undefined') {
      // immutable!
      return true;
    } else {
      // no idea...
      return false;
    }
  }

  return false;
}
