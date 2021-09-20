/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {
  isClassDeclaration,
  isFunctionDeclaration,
} from '../generated/node-types';
import isLet from './isLet';

/**
 * Check if the input `node` is block scoped.
 */
export default function isBlockScoped(node: t.Node): boolean {
  return isFunctionDeclaration(node) || isClassDeclaration(node) || isLet(node);
}
