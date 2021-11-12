/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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

export function attachCommentsToNewNode(
  oldNode: ESNode,
  newNode: DetachedNode<ESNode>,
): void {
  /*
  $FlowExpectedError - this property is secretly added by prettier.
  It is an array of the comment nodes.
  */
  newNode.comments = oldNode.comments;
}
