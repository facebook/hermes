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
  BaseNode,
  BlockStatement,
  ESNode,
  ModuleDeclaration,
  Statement,
  StatementParentArray,
  StatementParentSingle,
} from 'hermes-estree';
import type {MutationContext} from '../MutationContext';
import type {DetachedNode} from '../../detachedNode';

import {getStatementParent} from './utils/getStatementParent';
import {isValidModuleDeclarationParent} from './utils/isValidModuleDeclarationParent';
import {InvalidInsertionError, UnexpectedTransformationState} from '../Errors';
import {asESNode} from '../../detachedNode';
import * as t from '../../generated/node-types';

export type InsertStatementMutation = $ReadOnly<{
  type: 'insertStatement',
  side: 'before' | 'after',
  target: ModuleDeclaration | Statement,
  nodesToInsert: $ReadOnlyArray<DetachedNode<Statement | ModuleDeclaration>>,
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
  const insertionParent = getStatementParent(mutation.target);

  // enforce that if we are inserting module declarations - they are being inserted in a valid location
  if (
    !isValidModuleDeclarationParent(
      insertionParent.parent,
      mutation.nodesToInsert,
    )
  ) {
    throw new InvalidInsertionError(
      `import/export cannot be inserted into a ${insertionParent.parent.type}.`,
    );
  }

  mutationContext.markMutation(insertionParent.parent, insertionParent.key);

  if (insertionParent.type === 'array') {
    const parent: interface {
      [string]: $ReadOnlyArray<DetachedNode<Statement | ModuleDeclaration>>,
    } = insertionParent.parent;
    switch (mutation.side) {
      case 'before': {
        parent[insertionParent.key] = insertInArray(
          parent[insertionParent.key],
          insertionParent.targetIndex - 1,
          mutation.nodesToInsert,
        );
        break;
      }

      case 'after': {
        parent[insertionParent.key] = insertInArray(
          parent[insertionParent.key],
          insertionParent.targetIndex + 1,
          mutation.nodesToInsert,
        );
        break;
      }
    }

    // ensure the parent pointers are correctly set to the new parent
    for (const statement of mutation.nodesToInsert) {
      // $FlowExpectedError[cannot-write] - intentionally mutating the AST
      asESNode(statement).parent = parent;
    }
    return;
  }

  const statementsToInsert =
    // $FlowExpectedError[incompatible-cast] -- this is enforced by isValidModuleDeclarationParent above
    (mutation.nodesToInsert: $ReadOnlyArray<DetachedNode<Statement>>);

  const {parent, key} = insertionParent;

  // $FlowExpectedError[prop-missing]
  const statementToWrap = parent[key];
  // we need to wrap this key in a BlockStatement so we can insert the new statement
  const blockStatement = t.BlockStatement({
    body:
      mutation.side === 'before'
        ? [...statementsToInsert, statementToWrap]
        : [statementToWrap, ...statementsToInsert],
    parent: insertionParent.parent,
  });

  (insertionParent.parent: interface {[string]: mixed})[insertionParent.key] =
    blockStatement;
}

function insertInArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
  elements: $ReadOnlyArray<T>,
): Array<T> {
  return array.slice(0, index).concat(elements).concat(array.slice(index));
}
