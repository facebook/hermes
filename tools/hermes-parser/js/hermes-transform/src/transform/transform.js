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
import type {Visitor} from '../traverse/traverse';
import type {TransformContextAdditions} from './TransformContext';

import * as prettier from 'prettier';
import {getTransformedAST} from './getTransformedAST';
import {SimpleTraverser} from '../traverse/SimpleTraverser';

export type TransformVisitor = Visitor<TransformContextAdditions>;

export function transform(
  originalCode: string,
  visitors: TransformVisitor,
  prettierOptions: {...} = {},
): string {
  const {ast, astWasMutated, mutatedCode} = getTransformedAST(
    originalCode,
    visitors,
  );

  if (!astWasMutated) {
    return originalCode;
  }

  SimpleTraverser.traverse(ast, {
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
  delete ast.comments;

  return prettier.format(
    mutatedCode,
    // $FlowExpectedError[incompatible-exact] - we don't want to create a dependency on the prettier types
    {
      ...prettierOptions,
      parser() {
        return ast;
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
