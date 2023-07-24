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

import type {ESNode} from 'hermes-estree';

import {SimpleTraverser} from '../src/traverse/SimpleTraverser';
import {parse as parseOriginal} from '../src/index';

// $FlowExpectedError[untyped-import]
import {VISITOR_KEYS as babelVisitorKeys} from '@babel/types';

export const parse: typeof parseOriginal = (source, options) => {
  return parseOriginal(source, {flow: 'all', ...options});
};

export function parseForSnapshot(
  source: string,
  options?: {preserveRange?: boolean, babel?: boolean},
): mixed {
  if (options?.babel === true) {
    return cleanASTForSnapshot(parse(source, {babel: true}).program, options);
  }

  return cleanASTForSnapshot(parse(source), options);
}

export function cleanASTForSnapshot(
  ast: ESNode,
  options?: {preserveRange?: boolean, babel?: boolean},
): mixed {
  SimpleTraverser.traverse(ast, {
    enter(node) {
      // $FlowExpectedError[cannot-write]
      delete node.loc;
      // $FlowExpectedError[cannot-write]
      delete node.parent;
      if (options?.preserveRange !== true) {
        // $FlowExpectedError[cannot-write]
        delete node.range;
      }

      if (options?.babel === true) {
        // $FlowExpectedError[prop-missing]
        delete node.start;
        // $FlowExpectedError[prop-missing]
        delete node.end;
      }
    },
    leave() {},
    visitorKeys:
      options?.babel === true
        ? {...babelVisitorKeys, BigIntLiteralTypeAnnotation: []}
        : null,
  });

  if (ast.type === 'Program') {
    return {
      type: 'Program',
      body: ast.body,
    };
  }

  return ast;
}
