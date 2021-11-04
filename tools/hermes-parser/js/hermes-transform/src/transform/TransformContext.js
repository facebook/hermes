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
  ESNode,
  ModuleDeclaration,
  Statement,
  StatementParentArray,
} from 'hermes-estree';
import type {InsertStatementMutation} from './mutations/InsertStatement';
import type {RemoveNodeMutation} from './mutations/RemoveNode';
import type {ReplaceNodeMutation} from './mutations/ReplaceNode';
import type {ReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';
import type {DetachedNode} from '../detachedNode';

import {getVisitorKeys} from '../getVisitorKeys';
import {createInsertStatementMutation} from './mutations/InsertStatement';
import {createReplaceNodeMutation} from './mutations/ReplaceNode';
import {createReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';
import {createRemoveNodeMutation} from './mutations/RemoveNode';
import {SimpleTraverser} from '../traverse/SimpleTraverser';
import {deepCloneNode, shallowCloneNode} from '../detachedNode';

type Mutation = $ReadOnly<
  | InsertStatementMutation
  | ReplaceNodeMutation
  | ReplaceStatementWithManyMutation
  | RemoveNodeMutation,
>;

type SingleOrArray<T> = T | $ReadOnlyArray<T>;

export type TransformContext = $ReadOnly<{
  mutations: $ReadOnlyArray<Mutation>,

  /**
   * Shallowly clones the given node and applies the given overrides.
   * !!! Be careful about using this !!!
   * This does not clone children nodes. This means that if you keep
   * the original node in the AST then you will have two trees in the
   * AST which refer to the exact same node objects; which will lead
   * to ***undefined*** behaviour.
   *
   * You should only use this if:
   * 1) the node is a simple leaf (eg literals, identifiers, etc)
   * 2) you are 100% removing the original node from the AST
   *
   * If you want to literally duplicate a node to place somewhere else
   * in the AST, then use `deepCloneNode` instead.
   */
  shallowCloneNode: typeof shallowCloneNode,

  /**
   * Deeply clones the node and all its children, then applies the
   * given overrides.
   * !!! Be careful about using this !!!
   * Because this is a deep clone, using it high up in the AST can
   * result in a lot of work being done.
   */
  deepCloneNode: typeof deepCloneNode,

  /**
   * Insert `nodeToInsert` after the `target` statement.
   * The inserted nodes will be kept in the order given.
   */
  insertAfterStatement: (
    target: ModuleDeclaration | Statement,
    nodeToInsert: SingleOrArray<DetachedNode<ModuleDeclaration | Statement>>,
  ) => void,

  /**
   * Insert `nodeToInsert` before the `target` statement.
   * The inserted nodes will be kept in the order given.
   */
  insertBeforeStatement: (
    target: ModuleDeclaration | Statement,
    nodeToInsert: SingleOrArray<DetachedNode<ModuleDeclaration | Statement>>,
  ) => void,

  /**
   * Replace the `target` node with the `nodeToReplaceWith` node.
   * This simply does an in-place replacement in the AST.
   */
  replaceNode: (
    target: ESNode,
    nodeToReplaceWith: DetachedNode<ESNode>,
  ) => void,

  /**
   * Replaces the `target` node with all of the `nodesToReplaceWith` nodes.
   * The nodes will be kept in the order given.
   */
  replaceStatementWithMany: (
    target: ModuleDeclaration | Statement,
    nodesToReplaceWith: $ReadOnlyArray<
      DetachedNode<ModuleDeclaration | Statement>,
    >,
  ) => void,

  /**
   * Removes a given node from the AST.
   */
  removeNode: {
    (node: ModuleDeclaration | Statement): void,
  },
}>;

export function getTransformContext(): TransformContext {
  /**
   * The mutations in order of collection.
   */
  const mutations: Array<Mutation> = [];

  return {
    mutations,

    shallowCloneNode: (shallowCloneNode: TransformContext['shallowCloneNode']),

    deepCloneNode: (deepCloneNode: TransformContext['deepCloneNode']),

    insertAfterStatement: ((target, nodesToInsert): void => {
      mutations.push(
        createInsertStatementMutation('after', target, toArray(nodesToInsert)),
      );
    }: TransformContext['insertBeforeStatement']),

    insertBeforeStatement: ((target, nodesToInsert): void => {
      mutations.push(
        createInsertStatementMutation('before', target, toArray(nodesToInsert)),
      );
    }: TransformContext['insertBeforeStatement']),

    replaceNode: ((target, nodeToReplaceWith): void => {
      mutations.push(createReplaceNodeMutation(target, nodeToReplaceWith));
    }: TransformContext['replaceNode']),

    replaceStatementWithMany: ((target, nodesToReplaceWith): void => {
      mutations.push(
        createReplaceStatementWithManyMutation(target, nodesToReplaceWith),
      );
    }: TransformContext['replaceStatementWithMany']),

    removeNode: ((node): void => {
      mutations.push(createRemoveNodeMutation(node));
    }: TransformContext['removeNode']),
  };
}

function toArray<T: ESNode>(
  thing: SingleOrArray<DetachedNode<T>>,
): $ReadOnlyArray<DetachedNode<T>> {
  if (Array.isArray(thing)) {
    return thing;
  }
  return [thing];
}
