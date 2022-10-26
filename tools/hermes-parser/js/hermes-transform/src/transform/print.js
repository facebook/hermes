/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {MaybeDetachedNode} from '../detachedNode';
import type {Program, ESNode} from 'hermes-estree';

import {SimpleTraverser} from 'hermes-parser';
import * as prettier from 'prettier';
import {
  addCommentsToNode,
  getLeadingCommentsForNode,
} from './comments/comments';

export function print(
  ast: MaybeDetachedNode<Program>,
  originalCode: string,
  prettierOptions: {...} = {},
): string {
  // $FlowExpectedError[incompatible-type] This is now safe to access.
  const program: Program = ast;

  // The docblock comment is never attached to any AST nodes, since its technically
  // attached to the program. However this is specific to our AST and in order for
  // prettier to correctly print it we need to attach it to the first node in the
  // program body.
  if (program.docblock != null && program.body.length > 0) {
    const firstNode = program.body[0];
    const docblockComment = program.docblock.comment;
    const leadingComments = getLeadingCommentsForNode(firstNode);
    if (!leadingComments.includes(docblockComment)) {
      addCommentsToNode(firstNode, [docblockComment], 'leading');
    }
  }

  SimpleTraverser.traverse(program, {
    enter(node) {
      // prettier fully expects the parent pointers are NOT set and
      // certain cases can crash due to prettier infinite-looping
      // whilst naively traversing the parent property
      // https://github.com/prettier/prettier/issues/11793
      // $FlowExpectedError[cannot-write]
      delete node.parent;

      // prettier currently relies on the AST being in the old-school, deprecated AST format for optional chaining
      // so we have to apply their transform to our AST so it can actually format it.
      if (node.type === 'ChainExpression') {
        const newNode = transformChainExpression(node.expression);
        // $FlowExpectedError[cannot-write]
        delete node.expression;
        // $FlowExpectedError[prop-missing]
        // $FlowExpectedError[cannot-write]
        Object.assign(node, newNode);
      }
    },
    leave() {},
  });

  // we need to delete the comments prop or else prettier will do
  // its own attachment pass after the mutation and duplicate the
  // comments on each node, borking the output
  // $FlowExpectedError[cannot-write]
  delete program.comments;

  return prettier.format(
    originalCode,
    // $FlowExpectedError[incompatible-exact] - we don't want to create a dependency on the prettier types
    {
      ...prettierOptions,
      parser() {
        return program;
      },
    },
  );
}

// https://github.com/prettier/prettier/blob/d962466a828f8ef51435e3e8840178d90b7ec6cd/src/language-js/parse/postprocess/index.js#L161-L182
function transformChainExpression(node: ESNode) {
  switch (node.type) {
    case 'CallExpression':
      // $FlowExpectedError[cannot-write]
      node.type = 'OptionalCallExpression';
      // $FlowExpectedError[cannot-write]
      node.callee = transformChainExpression(node.callee);
      break;

    case 'MemberExpression':
      // $FlowExpectedError[cannot-write]
      node.type = 'OptionalMemberExpression';
      // $FlowExpectedError[cannot-write]
      node.object = transformChainExpression(node.object);
      break;
    // No default
  }

  return node;
}
