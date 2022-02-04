/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Comment, ESNode, Program} from 'hermes-estree';
import type {DetachedNode} from '../../detachedNode';

// $FlowExpectedError[untyped-import]
import {attach as untypedAttach} from './prettier/main/comments';
// $FlowExpectedError[untyped-import]
import {locEnd, locStart} from './prettier/language-js/loc';
// $FlowExpectedError[untyped-import]
import printer from './prettier/language-js/printer-estree';
import {
  // $FlowExpectedError[untyped-import]
  addLeadingComment as untypedAddLeadingComment,
  // $FlowExpectedError[untyped-import]
  addTrailingComment as untypedAddTrailingComment,
} from './prettier/common/util';

export type Options = $ReadOnly<{}>;

export function attachComments(
  comments: $ReadOnlyArray<Comment>,
  ast: Program,
  text: string,
): void {
  untypedAttach(comments, ast, text, {
    locStart,
    locEnd,
    printer,
  });
}

export function moveCommentsToNewNode(
  oldNode: ESNode,
  newNode: DetachedNode<ESNode>,
): void {
  setCommentsOnNode(newNode, getCommentsForNode(oldNode));
  setCommentsOnNode(oldNode, []);
}

export function setCommentsOnNode(
  node: ESNode | DetachedNode<ESNode>,
  comments: $ReadOnlyArray<Comment>,
): void {
  // $FlowExpectedError - this property is secretly added by prettier.
  node.comments = comments;
}

export function addCommentsToNode(
  node: ESNode | DetachedNode<ESNode>,
  comments: $ReadOnlyArray<Comment>,
): void {
  // $FlowExpectedError - this property is secretly added by prettier.
  node.comments = node.comments ?? [];
  // $FlowExpectedError
  (node.comments: Array<Comment>).push(...comments);
}

export function getCommentsForNode(
  node: ESNode | DetachedNode<ESNode>,
): $ReadOnlyArray<Comment> {
  // $FlowExpectedError - this property is secretly added by prettier.
  return node.comments ?? [];
}

export function isLeadingComment(comment: Comment): boolean {
  // $FlowExpectedError - this property is secretly added by prettier.
  return comment.leading === true;
}
export function isTrailingComment(comment: Comment): boolean {
  // $FlowExpectedError - this property is secretly added by prettier.
  return comment.trailing === true;
}

export function addLeadingComment(
  node: ESNode | DetachedNode<ESNode>,
  comment: Comment,
): void {
  untypedAddLeadingComment(node, comment);
}

export function addTrailingComment(
  node: ESNode | DetachedNode<ESNode>,
  comment: Comment,
): void {
  untypedAddTrailingComment(node, comment);
}

export function cloneComment<T: Comment>(comment: T): T {
  // $FlowExpectedError[incompatible-return]
  return {
    type: comment.type,
    value: comment.value,
    loc: comment.loc,
    range: comment.range,
  };
}

export function cloneCommentWithMarkers<T: Comment>(comment: T): T {
  // $FlowExpectedError[incompatible-return]
  return {
    type: comment.type,
    value: comment.value,
    loc: comment.loc,
    range: comment.range,
    leading: isLeadingComment(comment),
    trailing: isTrailingComment(comment),
  };
}

export function appendCommentToSource(code: string, comment: Comment): string {
  // prettier slices comments directly from the source code when printing
  // https://github.com/prettier/prettier/blob/5f0ee39fa03532c85bd1c35291450fe7ac3667b3/src/language-js/print/comment.js#L15-L20
  // this means that we need to have any appended comments directly in the
  // source code or else prettier will slice nothing and bork up the transform

  let commentText = comment.value;
  switch (comment.type) {
    case 'Block':
      commentText = `/*${commentText}*/`;
      break;
    case 'Line':
      commentText = `//${commentText}`;
      break;
  }

  let newCode = code;
  newCode += '\n';
  const start = newCode.length;
  newCode += commentText;
  const end = newCode.length;

  // $FlowExpectedError[cannot-write]
  comment.range = [start, end];

  return newCode;
}
