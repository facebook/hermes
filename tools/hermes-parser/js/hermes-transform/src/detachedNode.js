/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {BaseNode, ESNode} from 'hermes-estree';

import {getVisitorKeys, isNode} from './getVisitorKeys';
import {SimpleTraverser} from './traverse/SimpleTraverser';

export opaque type DetachedNode<+T> = T;

// used by the node type function codegen
export function detachedProps<T: BaseNode>(
  parent: ?ESNode,
  props: {...},
): DetachedNode<T> {
  // $FlowExpectedError[incompatible-return]
  return {
    ...props,
    // if not provided, then we purposely don't set this here
    // and will rely on the tooling to update it as appropriate.
    // nothing should be reading from this before it's set anyway.
    parent: (parent: $FlowFixMe),
    range: [0, 0],
    loc: {
      start: {
        line: 1,
        column: 0,
      },
      end: {
        line: 1,
        column: 0,
      },
    },
  };
}

/**
 * Shallowly clones the node, but not its children.
 */
export function shallowCloneNode<T: ESNode>(
  node: T,
  newProps: $Partial<{...}> = {},
): DetachedNode<T> {
  return detachedProps(null, (Object.assign({}, node, newProps): $FlowFixMe));
}

/**
 * Deeply clones node and its entire tree.
 */
export function deepCloneNode<T: ESNode>(
  node: T,
  newProps: $Partial<{...}> = {},
): DetachedNode<T> {
  const clone: DetachedNode<T> = Object.assign(
    JSON.parse(
      JSON.stringify(node, (key, value) => {
        // null out parent pointers
        if (key === 'parent') {
          return undefined;
        }
        return value;
      }),
    ),
    newProps,
  );

  updateAllParentPointers(clone);

  // $FlowExpectedError[class-object-subtyping]
  return detachedProps(null, clone);
}

/**
 * Corrects the parent pointers in direct children of the given node
 */
export function setParentPointersInDirectChildren(
  node: DetachedNode<ESNode>,
): void {
  for (const key of getVisitorKeys(node)) {
    if (
      isNode(
        // $FlowExpectedError[prop-missing]
        node[key],
      )
    ) {
      node[key].parent = node;
    } else if (Array.isArray(node[key])) {
      for (const child of node[key]) {
        child.parent = node;
      }
    }
  }
}

/**
 * Traverses the entire subtree to ensure the parent pointers are set correctly
 */
export function updateAllParentPointers(node: ESNode | DetachedNode<ESNode>) {
  SimpleTraverser.traverse(node, {
    enter(node, parent) {
      // $FlowExpectedError[cannot-write]
      node.parent = parent;
    },
    leave() {},
  });
}
