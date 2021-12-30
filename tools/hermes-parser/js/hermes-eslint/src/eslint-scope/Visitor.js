/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

import type {ESNode} from 'hermes-estree';

// $FlowExpectedError[untyped-import]
const esrecurse = require('esrecurse');

export type VisitorOptions = $ReadOnly<{
  childVisitorKeys?: $ReadOnly<{[string]: $ReadOnlyArray<string>}>,
  fallback?: 'iteration' | (ESNode => void),
}>;

declare class Visitor {
  constructor(visitor: ?{...}, options?: VisitorOptions): Visitor;

  /**
   * Default method for visiting children.
   * When you need to call default visiting operation inside custom visiting
   * operation, you can use it with `this.visitChildren(node)`.
   */
  visitChildren(node: ESNode): void;

  /**
   * Dispatching node.
   */
  visit(node: ?ESNode): void;
}

module.exports = (esrecurse.Visitor: typeof Visitor);
