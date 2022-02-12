/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {
  ClassMember,
  Comment,
  ESNode,
  Expression,
  FunctionParameter,
  ModuleDeclaration,
  Statement,
  TypeAnnotationType,
} from 'hermes-estree';
import type {DetachedNode} from '../detachedNode';
import type {TransformCloneSignatures} from '../generated/TransformCloneSignatures';
import type {TransformReplaceSignatures} from '../generated/TransformReplaceSignatures';
import type {TraversalContext} from '../traverse/traverse';
import type {AddLeadingCommentsMutation} from './mutations/AddLeadingComments';
import type {AddTrailingCommentsMutation} from './mutations/AddTrailingComments';
import type {CloneCommentsToMutation} from './mutations/CloneCommentsTo';
import type {InsertStatementMutation} from './mutations/InsertStatement';
import type {RemoveCommentMutation} from './mutations/RemoveComment';
import type {RemoveNodeMutation} from './mutations/RemoveNode';
import type {RemoveStatementMutation} from './mutations/RemoveStatement';
import type {ReplaceNodeMutation} from './mutations/ReplaceNode';
import type {ReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';

import {deepCloneNode, shallowCloneNode} from '../detachedNode';
import {
  getCommentsForNode,
  isLeadingComment,
  isTrailingComment,
} from './comments/comments';
import {createAddLeadingCommentsMutation} from './mutations/AddLeadingComments';
import {createAddTrailingCommentsMutation} from './mutations/AddTrailingComments';
import {createCloneCommentsToMutation} from './mutations/CloneCommentsTo';
import {createInsertStatementMutation} from './mutations/InsertStatement';
import {createRemoveCommentMutation} from './mutations/RemoveComment';
import {createRemoveNodeMutation} from './mutations/RemoveNode';
import {createRemoveStatementMutation} from './mutations/RemoveStatement';
import {createReplaceNodeMutation} from './mutations/ReplaceNode';
import {createReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';

type Mutation = $ReadOnly<
  | AddLeadingCommentsMutation
  | AddTrailingCommentsMutation
  | CloneCommentsToMutation
  | InsertStatementMutation
  | RemoveCommentMutation
  | RemoveNodeMutation
  | RemoveStatementMutation
  | ReplaceNodeMutation
  | ReplaceStatementWithManyMutation,
>;

type SingleOrArray<+T> = T | $ReadOnlyArray<T>;

type ReplaceNodeOptions = $ReadOnly<{
  /**
   * Moves the comments from the target node to the nodetoReplaceWith.
   * Note that this does not *clone* comments, it moves them and clears out
   * the target node's comments afterward.
   */
  keepComments?: boolean,
}>;

type TransformCloneAPIs = $ReadOnly<{
  /**
   * Shallowly clones the given node.
   *
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
    <T: ESNode>(node: T): DetachedNode<T>,
    <T: ESNode>(node: ?T): DetachedNode<T> | null,
  },

  /**
   * Shallowly clones the given node and applies the given overrides.
   * {@see shallowCloneNode}
   */
  shallowCloneNodeWithOverrides: TransformCloneSignatures,

  /**
   * {@see shallowCloneNode}
   */
  shallowCloneArray: {
    <T: ESNode>(node: $ReadOnlyArray<T>): $ReadOnlyArray<DetachedNode<T>>,
    <T: ESNode>(node: ?$ReadOnlyArray<T>): ?$ReadOnlyArray<DetachedNode<T>>,
  },

  /**
   * Deeply clones the node and all its children.
   *
   * !!! Be careful about using this !!!
   * Because this is a deep clone, using it high up in the AST can
   * result in a lot of work being done.
   */
  deepCloneNode: {
    <T: ESNode>(node: T): DetachedNode<T>,
    <T: ESNode>(node: ?T): DetachedNode<T> | null,
  },

  /**
   * Deeply clones the node and all its children, then applies the
   * given overrides.
   * {@see deepCloneNode}
   */
  deepCloneNodeWithOverrides: TransformCloneSignatures,
}>;

type TransformCommentAPIs = $ReadOnly<{
  /**
   * Gets all of the comments attached to the given node.
   */
  getComments: (node: ESNode) => Array<Comment>,

  /**
   * Gets the leading comments attached to the given node.
   */
  getLeadingComments: (node: ESNode) => Array<Comment>,

  /**
   * Gets the trailing comments attached to the given node.
   */
  getTrailingComments: (node: ESNode) => Array<Comment>,

  /**
   * Clones all of the comments from the target node to the destination node.
   * As its name suggest - this will clone the comments, duplicating them
   * entirely. It will not remove the comments from the target node afterward.
   */
  cloneCommentsTo: (
    target: ESNode,
    destination: ESNode | DetachedNode<ESNode>,
  ) => void,

  /**
   * Add leading comments to the specified node.
   */
  addLeadingComments: (
    node: ESNode | DetachedNode<ESNode>,
    comments: SingleOrArray<Comment>,
  ) => void,

  /**
   * Add trailing comments to the specified node.
   */
  addTrailingComments: (
    node: ESNode | DetachedNode<ESNode>,
    comments: SingleOrArray<Comment>,
  ) => void,

  /**
   * Removes the specified comments
   */
  removeComments: (comments: SingleOrArray<Comment>) => void,
}>;

type TransformInsertAPIs = $ReadOnly<{
  /**
   * Insert `nodeToInsert` after the `target` statement.
   * The inserted nodes will be kept in the order given.
   */
  insertAfterStatement: (
    target: InsertStatementMutation['target'],
    nodeToInsert: SingleOrArray<
      DetachedNode<InsertStatementMutation['target']>,
    >,
  ) => void,

  /**
   * Insert `nodeToInsert` before the `target` statement.
   * The inserted nodes will be kept in the order given.
   */
  insertBeforeStatement: (
    target: InsertStatementMutation['target'],
    nodeToInsert: SingleOrArray<
      DetachedNode<InsertStatementMutation['target']>,
    >,
  ) => void,
}>;

type TransformRemoveAPIs = $ReadOnly<{
  /**
   * Removes a given node from the AST.
   * The set of thigns that can be removed is intentionally restricted by types.
   * This represents the set of "misc nodes" that are known to be safe to remove without outright breaking the AST.
   */
  removeNode: (node: RemoveNodeMutation['node']) => void,

  /**
   * Removes a given statement from the AST.
   */
  removeStatement: (node: RemoveStatementMutation['node']) => void,
}>;

type TransformReplaceAPIs = $ReadOnly<{
  /**
   * Replace the `target` node with the `nodeToReplaceWith` node.
   * This simply does an in-place replacement in the AST.
   */
  replaceNode: {
    // Expressions may be replaced with other expressions
    (
      target: Expression,
      nodeToReplaceWith: DetachedNode<Expression>,
      options?: ReplaceNodeOptions,
    ): void,
    // Module declarations may be replaced with statements or other module declarations
    (
      target: ModuleDeclaration,
      nodeToReplaceWith: DetachedNode<ModuleDeclaration | Statement>,
      options?: ReplaceNodeOptions,
    ): void,
    // Statement maybe be replaced with statements or module declarations
    (
      target: Statement,
      nodeToReplaceWith: DetachedNode<ModuleDeclaration | Statement>,
      options?: ReplaceNodeOptions,
    ): void,
    // Types maybe be replaced with other types
    (
      target: TypeAnnotationType,
      nodeToReplaceWith: DetachedNode<TypeAnnotationType>,
      options?: ReplaceNodeOptions,
    ): void,
    // Class members may be replaced with other class members
    (
      target: ClassMember,
      nodeToReplaceWith: DetachedNode<ClassMember>,
      options?: ReplaceNodeOptions,
    ): void,
    // Function params amy be replace with other function params
    (
      target: FunctionParameter,
      nodeToReplaceWith: DetachedNode<FunctionParameter>,
      options?: ReplaceNodeOptions,
    ): void,
  } & TransformReplaceSignatures, // allow like-for-like replacements as well

  /**
   * Replaces the `target` node with all of the `nodesToReplaceWith` nodes.
   * The nodes will be kept in the order given.
   */
  replaceStatementWithMany: (
    target: ReplaceStatementWithManyMutation['target'],
    nodesToReplaceWith: ReplaceStatementWithManyMutation['nodesToReplaceWith'],
    options?: {
      /**
       * Moves the comments from the target node to the first node in the array.
       * Note that this does not *clone* comments, it moves them and clears out
       * the target node's comments afterward.
       */
      keepComments?: boolean,
    },
  ) => void,
}>;

export type TransformContextAdditions = $ReadOnly<{
  mutations: $ReadOnlyArray<Mutation>,
  astWasMutated: boolean,

  ...TransformCommentAPIs,
  ...TransformCloneAPIs,
  ...TransformInsertAPIs,
  ...TransformRemoveAPIs,
  ...TransformReplaceAPIs,
}>;
export type TransformContext = TraversalContext<TransformContextAdditions>;

export function getTransformContext(): TransformContextAdditions {
  /**
   * The mutations in order of collection.
   */
  const mutations: Array<Mutation> = [];
  function pushMutation(mutation: ?Mutation): void {
    if (mutation != null) {
      mutations.push(mutation);
    }
  }

  const cloneAPIs: TransformCloneAPIs = {
    shallowCloneNode: ((
      node: ?ESNode,
    ): // $FlowExpectedError[incompatible-cast]
    ?DetachedNode<ESNode> => {
      if (node == null) {
        return null;
      }

      return shallowCloneNode(node);
    }: TransformCloneAPIs['shallowCloneNode']),

    // $FlowExpectedError[incompatible-exact]
    shallowCloneNodeWithOverrides: ((
      node: ?ESNode,
      newProps?: $ReadOnly<{...}>,
    ): ?DetachedNode<ESNode> => {
      if (node == null) {
        return null;
      }

      return shallowCloneNode(node, newProps);
    }: TransformCloneAPIs['shallowCloneNodeWithOverrides']),

    shallowCloneArray: (<T: ESNode>(
      nodes: ?$ReadOnlyArray<T>,
    ): // $FlowExpectedError[incompatible-cast]
    ?$ReadOnlyArray<DetachedNode<ESNode>> => {
      if (nodes == null) {
        return null;
      }

      return nodes.map(node => shallowCloneNode<T>(node));
    }: TransformCloneAPIs['shallowCloneArray']),

    deepCloneNode: ((
      node: ?ESNode,
    ): // $FlowExpectedError[incompatible-cast]
    ?DetachedNode<ESNode> => {
      if (node == null) {
        return null;
      }

      return deepCloneNode(node);
    }: TransformCloneAPIs['deepCloneNode']),

    // $FlowExpectedError[incompatible-exact]
    deepCloneNodeWithOverrides: ((
      node: ?ESNode,
      newProps?: $ReadOnly<{...}>,
    ): ?DetachedNode<ESNode> => {
      if (node == null) {
        return null;
      }

      return deepCloneNode(node, newProps);
    }: TransformCloneAPIs['deepCloneNodeWithOverrides']),
  };
  const commentAPIs: TransformCommentAPIs = {
    getComments: ((node): Array<Comment> => {
      return [...getCommentsForNode(node)];
    }: TransformCommentAPIs['getComments']),

    getLeadingComments: ((node): Array<Comment> => {
      return getCommentsForNode(node).filter(isLeadingComment);
    }: TransformCommentAPIs['getLeadingComments']),

    getTrailingComments: ((node): Array<Comment> => {
      return getCommentsForNode(node).filter(isTrailingComment);
    }: TransformCommentAPIs['getTrailingComments']),

    cloneCommentsTo: ((target, destination): void => {
      pushMutation(createCloneCommentsToMutation(target, destination));
    }: TransformCommentAPIs['cloneCommentsTo']),

    addLeadingComments: ((node, comments): void => {
      pushMutation(createAddLeadingCommentsMutation(node, toArray(comments)));
    }: TransformCommentAPIs['addLeadingComments']),

    addTrailingComments: ((node, comments): void => {
      pushMutation(createAddTrailingCommentsMutation(node, toArray(comments)));
    }: TransformCommentAPIs['addTrailingComments']),

    removeComments: ((comments): void => {
      toArray(comments).forEach(comment => {
        pushMutation(createRemoveCommentMutation(comment));
      });
    }: TransformCommentAPIs['removeComments']),
  };
  const insertAPIs: TransformInsertAPIs = {
    insertAfterStatement: ((target, nodesToInsert): void => {
      pushMutation(
        createInsertStatementMutation('after', target, toArray(nodesToInsert)),
      );
    }: TransformInsertAPIs['insertBeforeStatement']),

    insertBeforeStatement: ((target, nodesToInsert): void => {
      pushMutation(
        createInsertStatementMutation('before', target, toArray(nodesToInsert)),
      );
    }: TransformInsertAPIs['insertBeforeStatement']),
  };
  const removeAPIs: TransformRemoveAPIs = {
    removeNode: ((node): void => {
      pushMutation(createRemoveNodeMutation(node));
    }: TransformRemoveAPIs['removeNode']),

    removeStatement: ((node): void => {
      pushMutation(createRemoveStatementMutation(node));
    }: TransformRemoveAPIs['removeStatement']),
  };
  const replaceAPIs: TransformReplaceAPIs = {
    replaceNode: ((
      target: ESNode,
      nodeToReplaceWith: DetachedNode<ESNode>,
      options?: ReplaceNodeOptions,
    ): void => {
      pushMutation(
        createReplaceNodeMutation(target, nodeToReplaceWith, options),
      );
    }: TransformReplaceAPIs['replaceNode']),

    replaceStatementWithMany: ((
      target,
      nodesToReplaceWith,
      options?: ReplaceNodeOptions,
    ): void => {
      pushMutation(
        createReplaceStatementWithManyMutation(
          target,
          nodesToReplaceWith,
          options,
        ),
      );
    }: TransformReplaceAPIs['replaceStatementWithMany']),
  };

  return {
    mutations,

    // $FlowExpectedError[unsafe-getters-setters]
    get astWasMutated(): boolean {
      return mutations.length > 0;
    },

    ...cloneAPIs,
    ...commentAPIs,
    ...insertAPIs,
    ...removeAPIs,
    ...replaceAPIs,
  };
}

function toArray<T>(thing: SingleOrArray<T>): $ReadOnlyArray<T> {
  if (Array.isArray(thing)) {
    // $FlowExpectedError[incompatible-return]
    return thing;
  }
  return [thing];
}
