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

import {replaceInArray} from './utils/arrayUtils';
import {InvalidReplacementError} from '../Errors';
import {asESNode} from '../../detachedNode';
import {getVisitorKeys, isNode} from '../../getVisitorKeys';

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
  const replacementParent = getParentKey(mutation.target);

  mutationContext.markDeletion(mutation.target);

  // NOTE: currently this mutation assumes you're doing the right thing.
  // it does no runtime checks and provides no guarantees about the
  // correctness of the resulting code.
  // TODO: maybe add some runtime checks based on codegenned predicates?

  if (replacementParent.type === 'array') {
    const parent: interface {
      [string]: $ReadOnlyArray<DetachedNode<ESNode>>,
    } = replacementParent.parent;
    parent[replacementParent.key] = replaceInArray(
      parent[replacementParent.key],
      replacementParent.targetIndex,
      [mutation.nodeToReplaceWith],
    );
    return;
  } else {
    (replacementParent.parent: interface {[string]: mixed})[
      replacementParent.key
    ] = mutation.nodeToReplaceWith;
  }

  // update the parent pointer
  // $FlowExpectedError[cannot-write] - intentionally mutating the AST
  asESNode(mutation.nodeToReplaceWith).parent = replacementParent.parent;
}

function getParentKey(target: ESNode): $ReadOnly<
  | {
      type: 'single',
      parent: ESNode,
      key: string,
    }
  | {
      type: 'array',
      parent: ESNode,
      key: string,
      targetIndex: number,
    },
> {
  const parent = target.parent;
  for (const key of getVisitorKeys(parent)) {
    if (
      isNode(
        // $FlowExpectedError[prop-missing]
        parent[key],
      )
    ) {
      if (parent[key] === target) {
        return {type: 'single', parent, key};
      }
    } else if (Array.isArray(parent[key])) {
      for (let i = 0; i < parent[key].length; i += 1) {
        if (parent[key][i] === target) {
          return {type: 'array', parent, key, targetIndex: i};
        }
      }
    }
  }

  // this shouldn't happen ever
  throw new InvalidReplacementError(
    `Expected to find the ${target.type} as a direct child of the ${target.type}.`,
  );
}
