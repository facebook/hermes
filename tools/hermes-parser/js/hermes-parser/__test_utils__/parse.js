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
import {print as printAST} from 'hermes-transform';

// $FlowExpectedError[untyped-import]
import {VISITOR_KEYS as babelVisitorKeys} from '@babel/types';
// $FlowExpectedError[untyped-import]
import generate from '@babel/generator';

const prettierConfig = Object.freeze({
  arrowParens: 'avoid',
  singleQuote: true,
  trailingComma: 'all',
  bracketSpacing: false,
  bracketSameLine: true,
  parser: 'hermes',
});

export const parse: typeof parseOriginal = (source, options) => {
  return parseOriginal(source, {flow: 'all', ...options});
};

export function parseForSnapshot(
  source: string,
  {
    babel,
    preserveRange,
    enableExperimentalComponentSyntax,
  }: {
    preserveRange?: boolean,
    babel?: boolean,
    enableExperimentalComponentSyntax?: boolean,
  } = {},
): mixed {
  const parseOpts = {
    enableExperimentalComponentSyntax:
      enableExperimentalComponentSyntax ?? false,
  };
  if (babel === true) {
    return cleanASTForSnapshot(
      parse(source, {
        babel: true,
        ...parseOpts,
      }).program,
      {babel, preserveRange},
    );
  }

  return cleanASTForSnapshot(parse(source, parseOpts), {
    babel,
    preserveRange,
  });
}

export async function printForSnapshot(
  source: string,
  {
    babel,
    enableExperimentalComponentSyntax,
  }: {
    babel?: boolean,
    enableExperimentalComponentSyntax?: boolean,
  } = {},
): Promise<string> {
  const parseOpts = {
    enableExperimentalComponentSyntax:
      enableExperimentalComponentSyntax ?? false,
  };
  if (babel === true) {
    const ast = parse(source, {
      babel: true,
      ...parseOpts,
    }).program;
    return generate(ast).code;
  }

  const ast = parse(source, parseOpts);
  const output = await printAST(ast, source, prettierConfig);
  return output.trim();
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
