/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ModuleDeclaration, Statement} from 'hermes-estree';
import type {MutationContext} from '../MutationContext';
import type {DetachedNode} from '../../detachedNode';

import {replaceInArray} from './utils/arrayUtils';
import {getStatementParent} from './utils/getStatementParent';
import {isValidModuleDeclarationParent} from './utils/isValidModuleDeclarationParent';
import {InvalidReplacementError} from '../Errors';
import {asESNode} from '../../detachedNode';
import * as t from '../../generated/node-types';

export type ReplaceStatementWithManyMutation = $ReadOnly<{
  type: 'replaceStatementWithMany',
  target: ModuleDeclaration | Statement,
  nodesToReplaceWith: $ReadOnlyArray<
    DetachedNode<ModuleDeclaration | Statement>,
  >,
}>;

export function createReplaceStatementWithManyMutation(
  target: ReplaceStatementWithManyMutation['target'],
  nodesToReplaceWith: ReplaceStatementWithManyMutation['nodesToReplaceWith'],
): ReplaceStatementWithManyMutation {
  return {
    type: 'replaceStatementWithMany',
    target,
    nodesToReplaceWith,
  };
}

export function performReplaceStatementWithManyMutation(
  mutationContext: MutationContext,
  mutation: ReplaceStatementWithManyMutation,
): void {
  const replacementParent = getStatementParent(mutation.target);

  // enforce that if we are replacing with module declarations - they are being inserted in a valid location
  if (
    !isValidModuleDeclarationParent(
      replacementParent.parent,
      mutation.nodesToReplaceWith,
    )
  ) {
    throw new InvalidReplacementError(
      `import/export cannot be replaced into a ${replacementParent.parent.type}.`,
    );
  }

  mutationContext.markDeletion(mutation.target);

  if (replacementParent.type === 'array') {
    const parent: interface {
      [string]: $ReadOnlyArray<DetachedNode<Statement | ModuleDeclaration>>,
    } = replacementParent.parent;
    parent[replacementParent.key] = replaceInArray(
      parent[replacementParent.key],
      replacementParent.targetIndex,
      mutation.nodesToReplaceWith,
    );

    // ensure the parent pointers are correctly set to the new parent
    for (const statement of mutation.nodesToReplaceWith) {
      // $FlowExpectedError[cannot-write] - intentionally mutating the AST
      asESNode(statement).parent = parent;
    }
    return;
  }

  const statementsToReplaceWith =
    // $FlowExpectedError[incompatible-cast] -- this is enforced by isValidModuleDeclarationParent above
    (mutation.nodesToReplaceWith: $ReadOnlyArray<DetachedNode<Statement>>);

  // we need to wrap the nodes in a BlockStatement as before there was only 1 node
  const blockStatement = t.BlockStatement({
    body: statementsToReplaceWith,
    parent: replacementParent.parent,
  });

  (replacementParent.parent: interface {[string]: mixed})[
    replacementParent.key
  ] = blockStatement;
}
