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

import {removeFromArray} from './utils/arrayUtils';
import {getStatementParent} from './utils/getStatementParent';
import {InvalidRemovalError} from '../Errors';
import * as t from '../../generated/node-types';

export type RemoveStatementMutation = $ReadOnly<{
  type: 'removeStatement',
  node: ModuleDeclaration | Statement,
}>;

export function createRemoveStatementMutation(
  node: RemoveStatementMutation['node'],
): RemoveStatementMutation {
  return {
    type: 'removeStatement',
    node,
  };
}

export function performRemoveStatementMutation(
  mutationContext: MutationContext,
  mutation: RemoveStatementMutation,
): void {
  const removalParent = getStatementParent(mutation.node);

  mutationContext.markDeletion(mutation.node);

  if (removalParent.type === 'array') {
    const parent: interface {
      [string]: $ReadOnlyArray<DetachedNode<Statement | ModuleDeclaration>>,
    } = removalParent.parent;
    parent[removalParent.key] = removeFromArray(
      parent[removalParent.key],
      removalParent.targetIndex,
    );
    return;
  }

  // The parent has a 1:1 relationship on this key, so we can't just
  // remove the node. Instead we replace it with an empty block statement.
  // We COULD throw an error here and make the codemodder write a stricter
  // codemod - but we decided to add this bit of magic to make it easier
  // to write codemods.
  // Worst case it creates some dead code that can be easily detected
  // and cleaned up later.
  const blockStatement = t.BlockStatement({
    body: [],
    parent: removalParent.parent,
  });

  (removalParent.parent: interface {[string]: mixed})[removalParent.key] =
    blockStatement;
}
