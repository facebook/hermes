/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

import {addTrailingComment, cloneComment} from '../comments/comments';

export type AddTrailingCommentsMutation = $ReadOnly<{
  type: 'addTrailingComments',
  comments: $ReadOnlyArray<Comment>,
  node: ESNode | DetachedNode<ESNode>,
}>;

export function createAddTrailingCommentsMutation(
  node: AddTrailingCommentsMutation['node'],
  comments: AddTrailingCommentsMutation['comments'],
): ?AddTrailingCommentsMutation {
  if (comments.length === 0) {
    return null;
  }

  return {
    type: 'addTrailingComments',
    comments,
    node,
  };
}

export function performAddTrailingCommentsMutation(
  mutationContext: MutationContext,
  mutation: AddTrailingCommentsMutation,
): null {
  for (const originalComment of mutation.comments) {
    const comment = cloneComment(originalComment);
    mutationContext.appendCommentToSource(comment);
    addTrailingComment(mutation.node, comment);
  }

  return null;
}
