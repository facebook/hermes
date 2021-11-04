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

export type RemoveNodeMutation = $ReadOnly<{
  type: 'removeNode',
  node: ESNode,
}>;

export function createRemoveNodeMutation(
  node: RemoveNodeMutation['node'],
): RemoveNodeMutation {
  return {
    type: 'removeNode',
    node,
  };
}

export function performRemoveNodeMutation(
  mutationContext: MutationContext,
  mutation: RemoveNodeMutation,
): void {
  // TODO
}
