/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const {
  HERMES_AST_VISITOR_KEYS,
  NODE_CHILD,
  NODE_LIST_CHILD,
} = require('./HermesASTVisitorKeys');

function visitNode(node) {
  if (node == null) {
    return;
  }

  const visitorKeys = HERMES_AST_VISITOR_KEYS[node.type];
  for (const key in visitorKeys) {
    const childType = visitorKeys[key];
    if (childType === NODE_CHILD) {
      const child = node[key];
      if (child != null) {
        visitNode(node[key]);
      }
    } else if (childType === NODE_LIST_CHILD) {
      for (const child of node[key]) {
        if (child != null) {
          visitNode(child);
        }
      }
    }
  }
}

module.exports = visitNode;
