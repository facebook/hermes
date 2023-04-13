/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {ESNode} from 'hermes-estree';

import {
  arrayIsEqual,
  removeFromArray,
  replaceInArray,
} from './astArrayMutationHelpers';
import {getVisitorKeys, isNode} from '../traverse/getVisitorKeys';
import {SimpleTraverser} from '../traverse/SimpleTraverser';

function getParentKey(
  target: ESNode,
  parent: ESNode,
): $ReadOnly<
  | {
      type: 'single',
      node: ESNode,
      key: string,
    }
  | {
      type: 'array',
      node: ESNode,
      key: string,
      targetIndex: number,
    },
> {
  if (parent == null) {
    throw new Error(`Expected parent node to be set on "${target.type}"`);
  }
  for (const key of getVisitorKeys(parent)) {
    if (
      isNode(
        // $FlowExpectedError[prop-missing]
        parent[key],
      )
    ) {
      if (parent[key] === target) {
        return {type: 'single', node: parent, key};
      }
    } else if (Array.isArray(parent[key])) {
      for (let i = 0; i < parent[key].length; i += 1) {
        const current = parent[key][i];
        if (current === target) {
          return {type: 'array', node: parent, key, targetIndex: i};
        }
      }
    }
  }

  // this shouldn't happen ever
  throw new Error(
    `Expected to find the ${target.type} as a direct child of the ${parent.type}.`,
  );
}

/**
 * Replace a node with a new node within an AST (via the parent pointer).
 */
export function replaceNodeOnParent(
  originalNode: ESNode,
  originalNodeParent: ESNode,
  nodeToReplaceWith: ESNode,
): void {
  const replacementParent = getParentKey(originalNode, originalNodeParent);
  const parent = replacementParent.node;
  if (replacementParent.type === 'array') {
    // $FlowExpectedError[prop-missing]
    parent[replacementParent.key] = replaceInArray(
      // $FlowExpectedError[prop-missing]
      parent[replacementParent.key],
      replacementParent.targetIndex,
      [nodeToReplaceWith],
    );
  } else {
    // $FlowExpectedError[prop-missing]
    parent[replacementParent.key] = nodeToReplaceWith;
  }
}

/**
 * Remove a node from the AST its connected to (via the parent pointer).
 */
export function removeNodeOnParent(
  originalNode: ESNode,
  originalNodeParent: ESNode,
): void {
  const replacementParent = getParentKey(originalNode, originalNodeParent);
  const parent = replacementParent.node;
  if (replacementParent.type === 'array') {
    // $FlowExpectedError[prop-missing]
    parent[replacementParent.key] = removeFromArray(
      // $FlowExpectedError[prop-missing]
      parent[replacementParent.key],
      replacementParent.targetIndex,
    );
  } else {
    // $FlowExpectedError[prop-missing]
    parent[replacementParent.key] = null;
  }
}

/**
 * Corrects the parent pointers in direct children of the given node.
 */
export function setParentPointersInDirectChildren(node: ESNode): void {
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
 * Traverses the entire subtree to ensure the parent pointers are set correctly.
 */
export function updateAllParentPointers(node: ESNode) {
  SimpleTraverser.traverse(node, {
    enter(node, parent) {
      // $FlowExpectedError[cannot-write]
      node.parent = parent;
    },
    leave() {},
  });
}

/**
 * Clone node and add new props.
 *
 * This will only create a new object if the overrides actually result in a change.
 */
export function nodeWith<T: ESNode>(node: T, overrideProps: Partial<T>): T {
  // Check if this will actually result in a change, maintaining referential equality is important.
  const willBeUnchanged = Object.entries(overrideProps).every(([key, value]) =>
    // $FlowExpectedError[prop-missing]
    Array.isArray(value) ? arrayIsEqual(node[key], value) : node[key] === value,
  );
  if (willBeUnchanged) {
    return node;
  }

  // Create new node.
  // $FlowExpectedError[cannot-spread-interface]
  const newNode: T = {
    ...node,
    ...overrideProps,
  };

  // Ensure parent pointers are correctly set within this nodes children.
  setParentPointersInDirectChildren(newNode);

  return newNode;
}

/**
 * Shallow clones node, providing a new reference for an existing node.
 */
export function shallowCloneNode<T: ESNode>(node: T): T {
  // $FlowExpectedError[cannot-spread-interface]
  const newNode: T = {...node};

  // Ensure parent pointers are correctly set within this nodes children.
  setParentPointersInDirectChildren(newNode);

  return newNode;
}

/**
 * Deeply clones node and its entire tree.
 */
export function deepCloneNode<T: ESNode>(node: T): T {
  const clone: T = JSON.parse(
    JSON.stringify(node, (key, value) => {
      // null out parent pointers
      if (key === 'parent') {
        return undefined;
      }
      return value;
    }),
  );

  updateAllParentPointers(clone);

  return clone;
}
