/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// This file contains methods responsible for dealing with comments.
import * as t from '../../types';

/**
 * Share comments amongst siblings.
 */

export function shareCommentsWithSiblings() {
  // NOTE: this assumes numbered keys
  if (typeof this.key === 'string') return;

  const node = this.node;
  if (!node) return;

  const trailing = node.trailingComments;
  const leading = node.leadingComments;
  if (!trailing && !leading) return;

  const prev = this.getSibling(this.key - 1);
  const next = this.getSibling(this.key + 1);
  const hasPrev = Boolean(prev.node);
  const hasNext = Boolean(next.node);
  if (hasPrev && !hasNext) {
    prev.addComments('trailing', trailing);
  } else if (hasNext && !hasPrev) {
    next.addComments('leading', leading);
  }
}

export function addComment(type: string, content: string, line?: boolean) {
  t.addComment(this.node, type, content, line);
}

/**
 * Give node `comments` of the specified `type`.
 */

export function addComments(type: string, comments: Array) {
  t.addComments(this.node, type, comments);
}
