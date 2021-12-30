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
  Comment,
  ESNode,
  Expression,
  ModuleDeclaration,
  Statement,
  TypeAnnotationType,
} from 'hermes-estree';
import type {DetachedNode} from '../detachedNode';
import type {TransformCloneSignatures} from '../generated/TransformCloneSignatures';
import type {TransformReplaceSignatures} from '../generated/TransformReplaceSignatures';
import type {AddLeadingCommentsMutation} from './mutations/AddLeadingComments';
import type {AddTrailingCommentsMutation} from './mutations/AddTrailingComments';
import type {CloneCommentsToMutation} from './mutations/CloneCommentsTo';
import type {InsertStatementMutation} from './mutations/InsertStatement';
import type {RemoveCommentMutation} from './mutations/RemoveComment';
import type {RemoveStatementMutation} from './mutations/RemoveStatement';
import type {ReplaceNodeMutation} from './mutations/ReplaceNode';
import type {ReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';

import {codeFrameColumns} from '@babel/code-frame';
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
import {createRemoveStatementMutation} from './mutations/RemoveStatement';
import {createReplaceNodeMutation} from './mutations/ReplaceNode';
import {createReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';

type Mutation = $ReadOnly<
  | AddLeadingCommentsMutation
  | AddTrailingCommentsMutation
  | CloneCommentsToMutation
  | InsertStatementMutation
  | RemoveCommentMutation
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

export type TransformContext = $ReadOnly<{
  mutations: $ReadOnlyArray<Mutation>,
  astWasMutated: boolean,

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
  shallowCloneNode: TransformCloneSignatures & {
    // generic passthrough cases with no options
    <T: ESNode>(node: T): DetachedNode<T>,
    <T: ESNode>(node: ?T): DetachedNode<T> | null,
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
  deepCloneNode: TransformCloneSignatures & {
    // generic passthrough cases with no options
    <T: ESNode>(node: T): DetachedNode<T>,
    <T: ESNode>(node: ?T): DetachedNode<T> | null,
  },

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
    (
      target: Expression,
      nodeToReplaceWith: DetachedNode<Expression>,
      options?: ReplaceNodeOptions,
    ): void,
    // module declarations must be replaced with statements or other module declarations
    (
      target: ModuleDeclaration,
      nodeToReplaceWith: DetachedNode<ModuleDeclaration | Statement>,
      options?: ReplaceNodeOptions,
    ): void,
    // Statement must be replaced with statements or module declarations
    (
      target: Statement,
      nodeToReplaceWith: DetachedNode<ModuleDeclaration | Statement>,
      options?: ReplaceNodeOptions,
    ): void,
    // Types must be replaced with types
    (
      target: TypeAnnotationType,
      nodeToReplaceWith: DetachedNode<TypeAnnotationType>,
      options?: ReplaceNodeOptions,
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
    options?: {
      /**
       * Moves the comments from the target node to the first node in the array.
       * Note that this does not *clone* comments, it moves them and clears out
       * the target node's comments afterward.
       */
      keepComments?: boolean,
    },
  ) => void,

  /**
   * Removes a given node from the AST.
   */
  removeStatement: (node: ModuleDeclaration | Statement) => void,

  //
  // Comment APIs
  //

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

  /**
   * Creates a full code frame for the node along with the message.
   *
   * i.e. `context.buildCodeFrame(node, 'foo')` will create a string like:
   * ```
   * 56 | function () {
   *    | ^^^^^^^^^^^^^
   * 57 | }.bind(this)
   *    | ^^ foo
   * ```
   */
  buildCodeFrame: (node: ESNode, message: string) => string,

  /**
   * Creates a simple code frame for the node along with the message.
   * Use this if you want a condensed marker for your message.
   *
   * i.e. `context.logWithNode(node, 'foo')` will create a string like:
   * ```
   * [FunctionExpression:56:44] foo
   * ```
   * (where 56:44 represents L56, Col44)
   */
  buildSimpleCodeFrame: (node: ESNode, message: string) => string,
}>;

export function getTransformContext(code: string): TransformContext {
  /**
   * The mutations in order of collection.
   */
  const mutations: Array<Mutation> = [];
  function pushMutation(mutation: ?Mutation): void {
    if (mutation != null) {
      mutations.push(mutation);
    }
  }

  return {
    mutations,

    // $FlowExpectedError[unsafe-getters-setters]
    get astWasMutated(): boolean {
      return mutations.length > 0;
    },

    // $FlowExpectedError[incompatible-exact]
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

    // $FlowExpectedError[incompatible-exact]
    deepCloneNode: ((
      node: ?ESNode,
      newProps?: $ReadOnly<{...}>,
    ): // $FlowExpectedError[incompatible-cast]
    ?DetachedNode<ESNode> => {
      if (node == null) {
        return null;
      }

      return deepCloneNode(node, newProps);
    }: TransformContext['deepCloneNode']),

    insertAfterStatement: ((target, nodesToInsert): void => {
      pushMutation(
        createInsertStatementMutation('after', target, toArray(nodesToInsert)),
      );
    }: TransformContext['insertBeforeStatement']),

    insertBeforeStatement: ((target, nodesToInsert): void => {
      pushMutation(
        createInsertStatementMutation('before', target, toArray(nodesToInsert)),
      );
    }: TransformContext['insertBeforeStatement']),

    replaceNode: ((
      target: ESNode,
      nodeToReplaceWith: DetachedNode<ESNode>,
      options?: $ReadOnly<{keepComments?: boolean}>,
    ): void => {
      pushMutation(
        createReplaceNodeMutation(target, nodeToReplaceWith, options),
      );
    }: TransformContext['replaceNode']),

    replaceStatementWithMany: ((
      target,
      nodesToReplaceWith,
      options?: $ReadOnly<{keepComments?: boolean}>,
    ): void => {
      pushMutation(
        createReplaceStatementWithManyMutation(
          target,
          nodesToReplaceWith,
          options,
        ),
      );
    }: TransformContext['replaceStatementWithMany']),

    removeStatement: ((node): void => {
      pushMutation(createRemoveStatementMutation(node));
    }: TransformContext['removeStatement']),

    //
    // Comment APIs
    //

    getComments: ((node): Array<Comment> => {
      return [...getCommentsForNode(node)];
    }: TransformContext['getComments']),

    getLeadingComments: ((node): Array<Comment> => {
      return getCommentsForNode(node).filter(isLeadingComment);
    }: TransformContext['getLeadingComments']),

    getTrailingComments: ((node): Array<Comment> => {
      return getCommentsForNode(node).filter(isTrailingComment);
    }: TransformContext['getTrailingComments']),

    cloneCommentsTo: ((target, destination): void => {
      pushMutation(createCloneCommentsToMutation(target, destination));
    }: TransformContext['cloneCommentsTo']),

    addLeadingComments: ((node, comments): void => {
      pushMutation(createAddLeadingCommentsMutation(node, toArray(comments)));
    }: TransformContext['addLeadingComments']),

    addTrailingComments: ((node, comments): void => {
      pushMutation(createAddTrailingCommentsMutation(node, toArray(comments)));
    }: TransformContext['addTrailingComments']),

    removeComments: ((comments): void => {
      toArray(comments).forEach(comment => {
        pushMutation(createRemoveCommentMutation(comment));
      });
    }: TransformContext['removeComments']),

    buildCodeFrame: (node: ESNode, message: string): string => {
      // babel uses 1-indexed columns
      const locForBabel = {
        start: {
          line: node.loc.start.line,
          column: node.loc.start.column + 1,
        },
        end: {
          line: node.loc.end.line,
          column: node.loc.end.column + 1,
        },
      };
      return codeFrameColumns(code, locForBabel, {
        linesAbove: 0,
        linesBelow: 0,
        highlightCode: process.env.NODE_ENV !== 'test',
        message: message,
      });
    },

    buildSimpleCodeFrame: (node: ESNode, message: string): string => {
      return `[${node.type}:${node.loc.start.line}:${node.loc.start.column}] ${message}`;
    },
  };
}

function toArray<T>(thing: SingleOrArray<T>): $ReadOnlyArray<T> {
  if (Array.isArray(thing)) {
    // $FlowExpectedError[incompatible-return]
    return thing;
  }
  return [thing];
}
