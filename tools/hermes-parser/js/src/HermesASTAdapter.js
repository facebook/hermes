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

/**
 * The base class for transforming the Hermes AST to the desired output format.
 * Extended by concrete adapters which output an ESTree or Babel AST.
 */
class HermesASTAdapter {
  /**
   * Transform the input Hermes AST to the desired output format.
   * This modifies the input AST in place instead of constructing a new AST.
   */
  transform(program) {
    this.visitNode(program);

    // Comments are not traversed via visitor keys
    for (const comment of program.comments) {
      this.fixSourceLocation(comment);
    }

    return program;
  }

  visitNode(node) {
    if (node == null) {
      return;
    }

    this.fixSourceLocation(node);

    const visitorKeys = HERMES_AST_VISITOR_KEYS[node.type];
    for (const key in visitorKeys) {
      const childType = visitorKeys[key];
      if (childType === NODE_CHILD) {
        const child = node[key];
        if (child != null) {
          this.visitNode(node[key]);
        }
      } else if (childType === NODE_LIST_CHILD) {
        for (const child of node[key]) {
          if (child != null) {
            this.visitNode(child);
          }
        }
      }
    }
  }

  /**
   * Update the source location for this node depending on the output AST format.
   * This can modify the input node in-place. Overriden in child classes.
   */
  fixSourceLocation(node) {
    throw new Error('Implemented in subclasses');
  }
}

module.exports = HermesASTAdapter;
