/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import bindingIdentifiers from '../definitions/bindingIdentifiers';

/**
 * Check if the input `node` is a binding identifier.
 */
export default function isBinding(
  node: t.Node,
  parent: t.Node,
  grandparent?: t.Node,
): boolean {
  if (
    grandparent &&
    node.type === 'Identifier' &&
    parent.type === 'ObjectProperty' &&
    grandparent.type === 'ObjectExpression'
  ) {
    // We need to special-case this, because getBindingIdentifiers
    // has an ObjectProperty->value entry for destructuring patterns.
    return false;
  }

  const keys = bindingIdentifiers[parent.type];
  if (keys) {
    for (let i = 0; i < keys.length; i++) {
      const key = keys[i];
      const val = parent[key];
      if (Array.isArray(val)) {
        if (val.indexOf(node) >= 0) return true;
      } else {
        if (val === node) return true;
      }
    }
  }

  return false;
}
