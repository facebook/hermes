/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ESNode, ESQueryNodeSelectors, Program} from 'hermes-estree';
import type {ScopeManager, Scope, Variable} from 'hermes-eslint';
import type {EmitterListener} from './SafeEmitter';

import {NodeEventGenerator} from './NodeEventGenerator';
import {SafeEmitter} from './SafeEmitter';
import {SimpleTraverser} from './SimpleTraverser';

export type TraversalContext<T = void> = $ReadOnly<{
  /**
   * Gets the variables that were declared by the given node.
   */
  getDeclaredVariables: (node: ESNode) => $ReadOnlyArray<Variable>,
  /**
   * Gets the nearest variable declared for the given name declare in the
   * current or an upper scope.
   */
  getBinding: (name: string) => ?Variable,
  /**
   * Gets the scope for the given node.
   * Defaults to the currently traversed node.
   */
  getScope: (node?: ESNode) => Scope,

  ...T,
}>;

export type Visitor<T = void> = (
  context: TraversalContext<T>,
) => ESQueryNodeSelectors;

/**
 * Traverse the AST with additional context members provided by `additionalContext`.
 * @param ast the ESTree AST to traverse
 * @param scopeManager the eslint-scope compatible scope manager instance calculated using the ast
 * @param additionalContext a callback function which returns additional context members to add to the context provided to the visitor
 */
export function traverseWithContext<T = void>(
  ast: Program,
  scopeManager: ScopeManager,
  additionalContext: (TraversalContext<void>) => T,
  visitor: Visitor<T>,
): void {
  const emitter = new SafeEmitter();
  const nodeQueue: Array<{isEntering: boolean, node: ESNode}> = [];

  let currentNode: ESNode = ast;

  // set parent pointers and build up the traversal queue
  SimpleTraverser.traverse(ast, {
    enter(node, parent) {
      // $FlowExpectedError[cannot-write] - hermes doesn't set this
      node.parent = parent;
      nodeQueue.push({isEntering: true, node});
    },
    leave(node) {
      nodeQueue.push({isEntering: false, node});
    },
  });

  const getScope = (givenNode: ESNode = currentNode) => {
    // On Program node, get the outermost scope to avoid return Node.js special function scope or ES modules scope.
    const inner = givenNode.type !== 'Program';

    for (let node = givenNode; node; node = node.parent) {
      const scope = scopeManager.acquire(node, inner);

      if (scope) {
        if (scope.type === 'function-expression-name') {
          return scope.childScopes[0];
        }
        return scope;
      }
    }

    return scopeManager.scopes[0];
  };

  const traversalContextBase: TraversalContext<void> = Object.freeze({
    getDeclaredVariables: (node: ESNode) =>
      scopeManager.getDeclaredVariables(node),
    getBinding: (name: string) => {
      let currentScope = getScope();

      while (currentScope != null) {
        for (const variable of currentScope.variables) {
          if (variable.defs.length && variable.name === name) {
            return variable;
          }
        }
        currentScope = currentScope.upper;
      }

      return null;
    },
    getScope,
  });
  const traversalContext: TraversalContext<T> = Object.freeze({
    ...traversalContextBase,
    ...additionalContext(traversalContextBase),
  });

  const selectors = visitor(traversalContext);

  // add all the selectors from the visitor as listeners
  Object.keys(selectors).forEach(selector => {
    // flow doesn't want us to be general here - but it's safe
    // $FlowExpectedError[incompatible-type]
    // $FlowExpectedError[prop-missing]
    const listener: ?EmitterListener = selectors[selector];
    if (listener) {
      emitter.on(selector, listener);
    }
  });

  const eventGenerator = new NodeEventGenerator(emitter);
  nodeQueue.forEach(traversalInfo => {
    currentNode = traversalInfo.node;

    try {
      if (traversalInfo.isEntering) {
        eventGenerator.enterNode(currentNode);
      } else {
        eventGenerator.leaveNode(currentNode);
      }
    } catch (err) {
      err.currentNode = currentNode;
      throw err;
    }
  });
}

export function traverse(
  ast: Program,
  scopeManager: ScopeManager,
  visitor: Visitor<void>,
): void {
  traverseWithContext(ast, scopeManager, () => {}, visitor);
}
