/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import addComments from './addComments';

/**
 * Add comment of certain type to a node.
 */
export default function addComment<T: Object>(
  node: T,
  type: string,
  content: string,
  line?: boolean
): T {
  return addComments(node, type, [
    {
      type: line ? 'CommentLine' : 'CommentBlock',
      value: content,
    },
  ]);
}
