/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {
  ModuleDeclaration,
  Statement,
  StatementParentArray,
} from 'hermes-estree';
import type {MutationContext} from '../MutationContext';
import type {DetachedNode} from '../../detachedNode';

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
  // TODO
}
