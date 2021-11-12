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
  Expression,
  ModuleDeclaration,
  Statement,
  TypeAnnotationType,
} from 'hermes-estree';
import type {InsertStatementMutation} from './mutations/InsertStatement';
import type {RemoveStatementMutation} from './mutations/RemoveStatement';
import type {ReplaceNodeMutation} from './mutations/ReplaceNode';
import type {ReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';
import type {DetachedNode} from '../detachedNode';

import {createInsertStatementMutation} from './mutations/InsertStatement';
import {createReplaceNodeMutation} from './mutations/ReplaceNode';
import {createReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';
import {createRemoveStatementMutation} from './mutations/RemoveStatement';
import {deepCloneNode, shallowCloneNode} from '../detachedNode';
import type {TransformReplaceSignatures} from '../generated/TransformReplaceSignatures';

type Mutation = $ReadOnly<
  | InsertStatementMutation
  | ReplaceNodeMutation
  | ReplaceStatementWithManyMutation
  | RemoveStatementMutation,
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
  shallowCloneNode: {
    <T: ESNode>(node: T, newProps?: $Shape<T>): DetachedNode<T>,
    <T: ESNode>(node: ?T, newProps?: $Shape<T>): ?DetachedNode<T>,
  },

  /**
   * {@see shallowCloneNode}
   */
  shallowCloneArray: {
    <T: ESNode>(node: $ReadOnlyArray<T>): $ReadOnlyArray<DetachedNode<T>>,
    <T: ESNode>(node: ?$ReadOnlyArray<T>): ?$ReadOnlyArray<DetachedNode<T>>,
  },

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
  replaceNode: {
    // expressions must be replaced with other expressions
    (target: Expression, nodeToReplaceWith: DetachedNode<Expression>): void,
    // module declarations must be replaced with statements or other module declarations
    (
      target: ModuleDeclaration,
      nodeToReplaceWith: DetachedNode<ModuleDeclaration | Statement>,
    ): void,
    // Statement must be replaced with statements or module declarations
    (
      target: Statement,
      nodeToReplaceWith: DetachedNode<ModuleDeclaration | Statement>,
    ): void,
    // Types must be replaced with types
    (
      target: TypeAnnotationType,
      nodeToReplaceWith: DetachedNode<TypeAnnotationType>,
    ): void,
  } & TransformReplaceSignatures, // allow like-for-like replacements as well

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
  removeStatement: {
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

    // $FlowExpectedError[class-object-subtyping]
    shallowCloneNode: ((
      node: ?ESNode,
      newProps?: $ReadOnly<{...}>,
    ): // $FlowExpectedError[incompatible-cast]
    ?DetachedNode<ESNode> => {
      if (node == null) {
        return null;
      }

      return shallowCloneNode(node, newProps);
    }: TransformContext['shallowCloneNode']),

    shallowCloneArray: (<T: ESNode>(
      nodes: ?$ReadOnlyArray<T>,
    ): // $FlowExpectedError[incompatible-cast]
    ?$ReadOnlyArray<DetachedNode<ESNode>> => {
      if (nodes == null) {
        return null;
      }

      return nodes.map(node => shallowCloneNode<T>(node));
    }: TransformContext['shallowCloneArray']),

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

    replaceNode: ((
      target: ESNode,
      nodeToReplaceWith: DetachedNode<ESNode>,
    ): void => {
      mutations.push(createReplaceNodeMutation(target, nodeToReplaceWith));
    }: TransformContext['replaceNode']),

    replaceStatementWithMany: ((target, nodesToReplaceWith): void => {
      mutations.push(
        createReplaceStatementWithManyMutation(target, nodesToReplaceWith),
      );
    }: TransformContext['replaceStatementWithMany']),

    removeStatement: ((node): void => {
      mutations.push(createRemoveStatementMutation(node));
    }: TransformContext['removeStatement']),
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
