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
  const insertionParent = getInsertionParent(mutation.target);

  // enforce that if we are inserting module declarations - they are being inserted in a valid location
  assertValidModuleDeclarationParent(
    insertionParent.parent,
    mutation.nodesToInsert,
  );

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
    // $FlowExpectedError[incompatible-cast] -- this is enforced by assertValidModuleDeclarationParent above
    (mutation.nodesToInsert: $ReadOnlyArray<DetachedNode<Statement>>);

  const {parent, key} = insertionParent;

  // $FlowExpectedError[prop-missing]
  if (parent[key].type === 'BlockStatement') {
    // This should be impossible because the direct parent is either a `BlockStatement` or is something else.
    // If it's a `BlockStatement` then it is handled above
    throw new UnexpectedTransformationState(
      `Found a \`BlockStatement\` on \`${parent.type}.${key}\` when it should have been anything else.`,
    );
  }

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

function getInsertionParent(target: ModuleDeclaration | Statement): $ReadOnly<
  | {
      type: 'single',
      parent: StatementParentSingle,
      key: string,
    }
  | {
      type: 'array',
      parent: StatementParentArray,
      key: string,
      targetIndex: number,
    },
> {
  function assertValidStatementInsertion<
    T: $ReadOnly<interface {type: string}>,
  >(parentWithType: T, ...invalidKeys: $ReadOnlyArray<$Keys<T>>): void {
    for (const key of invalidKeys) {
      // $FlowExpectedError[prop-missing]
      const value = parentWithType[key];

      if (
        value === target ||
        (Array.isArray(value) && value.includes(target))
      ) {
        throw new InvalidInsertionError(
          `Attempted to insert a statement into \`${parentWithType.type}.${key}\`.`,
        );
      }
    }
  }
  function getAssertedIndex(key: string, arr: $ReadOnlyArray<mixed>): number {
    const idx = arr.indexOf(target);
    if (idx == null) {
      throw new InvalidInsertionError(
        `Could not find target in array of \`${parent.type}.${key}\`.`,
      );
    }
    return idx;
  }

  const parent = target.parent;
  switch (parent.type) {
    case 'IfStatement': {
      assertValidStatementInsertion(parent, 'test');
      const key = parent.consequent === target ? 'consequent' : 'alternate';
      return {type: 'single', parent, key};
    }

    case 'LabeledStatement': {
      assertValidStatementInsertion(parent, 'label');
      return {type: 'single', parent, key: 'body'};
    }

    case 'WithStatement': {
      assertValidStatementInsertion(parent, 'object');
      return {type: 'single', parent, key: 'body'};
    }

    case 'DoWhileStatement':
    case 'WhileStatement': {
      assertValidStatementInsertion(parent, 'test');
      return {type: 'single', parent, key: 'body'};
    }

    case 'ForStatement': {
      assertValidStatementInsertion(parent, 'init', 'test', 'update');
      return {type: 'single', parent, key: 'body'};
    }

    case 'ForInStatement':
    case 'ForOfStatement': {
      assertValidStatementInsertion(
        // $FlowExpectedError[prop-missing] - flow does not track properties from parent interface
        parent,
        'left',
        'right',
      );
      return {type: 'single', parent, key: 'body'};
    }

    case 'SwitchCase': {
      assertValidStatementInsertion(parent, 'test');
      return {
        type: 'array',
        parent,
        key: 'consequent',
        targetIndex: getAssertedIndex('consequent', parent.consequent),
      };
    }

    case 'BlockStatement':
    case 'Program': {
      return {
        type: 'array',
        parent,
        key: 'body',
        targetIndex: getAssertedIndex('body', parent.body),
      };
    }
  }

  throw new InvalidInsertionError(
    `Expected to find a valid statement parent, but found a parent of type "${parent.type}".`,
  );
}

function isModuleDeclaration(node: ESNode): boolean %checks {
  return (
    node.type === 'ImportDeclaration' ||
    node.type === 'ExportNamedDeclaration' ||
    node.type === 'ExportDefaultDeclaration' ||
    node.type === 'ExportAllDeclaration'
  );
}

function assertValidModuleDeclarationParent(
  target: ESNode,
  nodesToInsert: $ReadOnlyArray<DetachedNode<ModuleDeclaration | Statement>>,
): void {
  if (
    target.type === 'Program' ||
    (target.type === 'BlockStatement' && target.parent.type === 'DeclareModule')
  ) {
    return;
  }

  for (const node of nodesToInsert) {
    const esnode = asESNode(node);
    if (!isModuleDeclaration(esnode)) {
      continue;
    }

    throw new InvalidInsertionError(
      `${esnode.type} cannot be inserted into a ${target.type}.`,
    );
  }
}

function insertInArray<T>(
  array: $ReadOnlyArray<T>,
  index: number,
  elements: $ReadOnlyArray<T>,
): Array<T> {
  return array.slice(0, index).concat(elements).concat(array.slice(index));
}
