/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {ESNode} from 'hermes-estree';
import type {VisitorKeys} from '../../HermesESLintVisitorKeys';

import visitorKeys from '../../HermesESLintVisitorKeys';

type VisitorOptions = $ReadOnly<{
  childVisitorKeys?: VisitorKeys | null,
}>;

function isNode(node: mixed): boolean %checks {
  return (
    typeof node === 'object' && node != null && typeof node.type === 'string'
  );
}
type NodeChildValue = string | boolean | number | ESNode;
type NodeChildValueOrArray = NodeChildValue | $ReadOnlyArray<NodeChildValue>;

/* abstract */ class VisitorBase {
  +_childVisitorKeys: VisitorKeys;
  constructor(options: VisitorOptions) {
    this._childVisitorKeys = options.childVisitorKeys ?? visitorKeys;
  }

  /**
   * Default method for visiting children.
   * @param node the node whose children should be visited
   * @param exclude a list of keys to not visit
   */
  visitChildren(node: ?ESNode, excludeArr: $ReadOnlyArray<string> = []): void {
    if (node == null || node.type == null) {
      return;
    }

    if (node.type === 'VariableDeclaration') {
      console.trace('wtf');
    }

    const exclude = new Set<string>(excludeArr.concat(['parent']));
    const children = this._childVisitorKeys[node.type] ?? Object.keys(node);
    for (const key of children) {
      if (exclude.has(key)) {
        continue;
      }

      // $FlowExpectedError[prop-missing]
      const child: ?NodeChildValueOrArray = node[key];
      if (child == null) {
        continue;
      }

      if (Array.isArray(child)) {
        for (const subChild of child) {
          if (isNode(subChild)) {
            this.visit(subChild);
          }
        }
      } else if (isNode(child)) {
        this.visit(child);
      }
    }
  }

  /**
   * Dispatching node.
   */
  visit(node: ?ESNode): void {
    if (node == null || node.type == null) {
      return;
    }

    // $FlowExpectedError[prop-missing] - subclasses declare the relevant methods
    const visitor: ?(node: ESNode) => void = this[node.type];
    if (visitor) {
      visitor.call(this, node);
      // assume the visitor function handled the children appropriately
      return;
    }

    this.visitChildren(node);
  }

  visitArray: (?$ReadOnlyArray<ESNode>) => void = (arr): void => {
    if (arr == null) {
      return;
    }

    for (const child of arr) {
      this.visit(child);
    }
  };
}

export type {VisitorOptions, VisitorKeys};
export {VisitorBase};
