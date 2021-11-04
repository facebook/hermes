/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ESNode} from 'hermes-estree';
import type {MutationContext} from '../MutationContext';
import type {DetachedNode} from '../../detachedNode';

export type ReplaceNodeMutation = $ReadOnly<{
  type: 'replaceNode',
  target: ESNode,
  nodeToReplaceWith: DetachedNode<ESNode>,
}>;

export function createReplaceNodeMutation(
  target: ReplaceNodeMutation['target'],
  nodeToReplaceWith: ReplaceNodeMutation['nodeToReplaceWith'],
): ReplaceNodeMutation {
  return {
    type: 'replaceNode',
    target,
    nodeToReplaceWith,
  };
}

export function performReplaceNodeMutation(
  mutationContext: MutationContext,
  mutation: ReplaceNodeMutation,
): void {
  // TODO
}
