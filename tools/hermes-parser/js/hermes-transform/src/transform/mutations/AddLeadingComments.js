/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Comment, ESNode} from 'hermes-estree';
import type {DetachedNode} from '../../detachedNode';
import type {MutationContext} from '../MutationContext';

import {addLeadingComment, cloneComment} from '../comments/comments';

export type AddLeadingCommentsMutation = $ReadOnly<{
  type: 'addLeadingComments',
  comments: $ReadOnlyArray<Comment>,
  node: ESNode | DetachedNode<ESNode>,
}>;

export function createAddLeadingCommentsMutation(
  node: AddLeadingCommentsMutation['node'],
  comments: AddLeadingCommentsMutation['comments'],
): ?AddLeadingCommentsMutation {
  if (comments.length === 0) {
    return null;
  }

  return {
    type: 'addLeadingComments',
    comments,
    node,
  };
}

export function performAddLeadingCommentsMutation(
  mutationContext: MutationContext,
  mutation: AddLeadingCommentsMutation,
): null {
  for (const originalComment of mutation.comments) {
    const comment = cloneComment(originalComment);
    mutationContext.appendCommentToSource(comment);
    addLeadingComment(mutation.node, comment);
  }

  return null;
}
