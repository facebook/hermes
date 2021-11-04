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

export type InsertStatementMutation = $ReadOnly<{
  type: 'insertStatement',
  side: 'before' | 'after',
  target: ModuleDeclaration | Statement,
  nodesToInsert: $ReadOnlyArray<Statement | ModuleDeclaration>,
}>;

export function createInsertStatementMutation(
  side: InsertStatementMutation['side'],
  target: InsertStatementMutation['target'],
  nodesToInsert: InsertStatementMutation['nodesToInsert'],
): InsertStatementMutation {
  return {
    type: 'insertStatement',
    side,
    target,
    nodesToInsert,
  };
}

export function performInsertStatementMutation(
  mutationContext: MutationContext,
  mutation: InsertStatementMutation,
): void {
  // TODO
}
