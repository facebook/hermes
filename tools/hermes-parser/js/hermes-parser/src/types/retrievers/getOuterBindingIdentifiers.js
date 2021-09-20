/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import getBindingIdentifiers from './getBindingIdentifiers';

export default function getOuterBindingIdentifiers(
  node: t.Node,
  duplicates: boolean,
): Record<string, t.Identifier> | Record<string, Array<t.Identifier>> {
  return getBindingIdentifiers(node, duplicates, true);
}
