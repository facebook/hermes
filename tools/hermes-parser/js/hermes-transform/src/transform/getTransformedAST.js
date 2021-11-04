/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {
  ESNode,
  BlockStatement,
  Expression,
  Program,
  Statement,
  ReturnStatement,
} from 'hermes-estree';
import type {TransformContext} from './TransformContext';
import type {Visitor} from '../traverse/traverse';

import {parseForESLint} from 'hermes-eslint';
import {traverseWithContext} from '../traverse/traverse';
import {getVisitorKeys} from '../getVisitorKeys';
import {MutationContext} from './MutationContext';
import {getTransformContext} from './TransformContext';
import {performInsertStatementMutation} from './mutations/InsertStatement';
import {performRemoveNodeMutation} from './mutations/RemoveNode';
import {performReplaceNodeMutation} from './mutations/ReplaceNode';
import {performReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';

export function getTransformedAST(
  code: string,
  visitors: Visitor<TransformContext>,
): {
  ast: Program,
  astWasMutated: boolean,
} {
  const {ast, scopeManager} = parseForESLint(code, {
    sourceType: 'module',
  });

  // traverse the AST and colllect the mutations
  const transformContext = getTransformContext();
  traverseWithContext(ast, scopeManager, () => transformContext, visitors);

  // apply the mutations to the AST
  const mutationContext = new MutationContext();
  for (const mutation of transformContext.mutations) {
    switch (mutation.type) {
      case 'insertStatement': {
        performInsertStatementMutation(mutationContext, mutation);
        break;
      }

      case 'replaceNode': {
        performReplaceNodeMutation(mutationContext, mutation);
        break;
      }

      case 'replaceStatementWithMany': {
        performReplaceStatementWithManyMutation(mutationContext, mutation);
        break;
      }

      case 'removeNode': {
        performRemoveNodeMutation(mutationContext, mutation);
        break;
      }
    }
  }

  return {
    ast,
    astWasMutated: transformContext.mutations.length > 0,
  };
}
