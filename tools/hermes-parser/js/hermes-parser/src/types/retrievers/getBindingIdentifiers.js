/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {
  isExportDeclaration,
  isIdentifier,
  isDeclaration,
  isFunctionDeclaration,
  isFunctionExpression,
  isExportAllDeclaration,
} from '../generated/node-types';
import bindingIdentifiers from '../definitions/bindingIdentifiers';

/**
 * Return a list of binding identifiers associated with the input `node`.
 */
export default function getBindingIdentifiers(
  node: t.Node,
  duplicates?: boolean,
  outerOnly?: boolean,
): Record<string, t.Identifier> | Record<string, Array<t.Identifier>> {
  let search = [].concat(node);
  const ids = Object.create(null);

  while (search.length) {
    const id = search.shift();
    if (!id) continue;

    const keys = bindingIdentifiers[id.type];

    if (isIdentifier(id)) {
      if (duplicates) {
        const _ids = (ids[id.name] = ids[id.name] || []);
        _ids.push(id);
      } else {
        ids[id.name] = id;
      }
      continue;
    }

    if (isExportDeclaration(id) && !isExportAllDeclaration(id)) {
      if (isDeclaration(id.declaration)) {
        search.push(id.declaration);
      }
      continue;
    }

    if (outerOnly) {
      if (isFunctionDeclaration(id)) {
        search.push(id.id);
        continue;
      }

      if (isFunctionExpression(id)) {
        continue;
      }
    }

    if (keys) {
      for (let i = 0; i < keys.length; i++) {
        const key = keys[i];
        if (id[key]) {
          search = search.concat(id[key]);
        }
      }
    }
  }

  // $FlowIssue Object.create() seems broken
  return ids;
}
