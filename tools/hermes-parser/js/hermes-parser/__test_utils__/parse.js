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

import type {Program as ESTreeProgram} from 'hermes-estree';
import type {ESNode} from 'hermes-estree';
import type {ParserOptions} from '../src/ParserOptions';
import type {BabelFile} from '../src/babel/TransformESTreeToBabel';
import type {VisitorKeys} from '../src/generated/ESTreeVisitorKeys';

import {SimpleTraverser} from '../src/traverse/SimpleTraverser';
import {parse as parseOriginal} from '../src/index';
import {print as printAST} from 'hermes-transform';

// $FlowExpectedError[untyped-import]
import {VISITOR_KEYS as babelVisitorKeys} from '@babel/types';
// $FlowExpectedError[untyped-import]
import generate from '@babel/generator';

export const BABEL_VISITOR_KEYS: VisitorKeys = {
  ...babelVisitorKeys,
  BigIntLiteralTypeAnnotation: [],
};

const prettierConfig = Object.freeze({
  arrowParens: 'avoid',
  singleQuote: true,
  trailingComma: 'all',
  bracketSpacing: false,
  bracketSameLine: true,
  parser: 'hermes',
});

declare function parse(
  code: string,
  opts: {...ParserOptions, babel: true},
): BabelFile;
// eslint-disable-next-line no-redeclare
declare function parse(
  code: string,
  opts?:
    | {...ParserOptions, babel?: false | void}
    | {...ParserOptions, babel: false},
): ESTreeProgram;
// eslint-disable-next-line no-redeclare
export function parse(code: string, options: ParserOptions) {
  if (options?.babel === true) {
    return parseOriginal(code, {flow: 'all', ...options, babel: true});
  }

  return parseOriginal(code, {flow: 'all', ...options, babel: false});
}

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
      enableExperimentalComponentSyntax ?? true,
  };
  if (babel === true) {
    return cleanASTForSnapshot(
      parse(source, {
        babel: true,
        ...parseOpts,
      }).program,
      {babel, preserveRange, enforceLocationInformation: true},
    );
  }

  return cleanASTForSnapshot(parse(source, parseOpts), {
    babel,
    preserveRange,
  });
}

export function printForSnapshotESTree(code: string): Promise<string> {
  return printForSnapshot(code);
}
export function parseForSnapshotESTree(code: string): mixed {
  return parseForSnapshot(code);
}
export function printForSnapshotBabel(
  code: string,
  options?: {reactRuntimeTarget?: ParserOptions['reactRuntimeTarget']},
): Promise<string> {
  return printForSnapshot(code, {
    babel: true,
    reactRuntimeTarget: options?.reactRuntimeTarget,
  });
}
export function parseForSnapshotBabel(code: string): mixed {
  return parseForSnapshot(code, {babel: true});
}

export async function printForSnapshot(
  source: string,
  {
    babel,
    enableExperimentalComponentSyntax,
    reactRuntimeTarget,
  }: {
    babel?: boolean,
    enableExperimentalComponentSyntax?: boolean,
    reactRuntimeTarget?: ParserOptions['reactRuntimeTarget'],
  } = {},
): Promise<string> {
  const parseOpts = {
    enableExperimentalComponentSyntax:
      enableExperimentalComponentSyntax ?? true,
    reactRuntimeTarget,
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
  options?: {
    preserveRange?: boolean,
    babel?: boolean,
    enforceLocationInformation?: boolean,
  },
): mixed {
  SimpleTraverser.traverse(ast, {
    enter(node) {
      if (options?.enforceLocationInformation === true && node.loc == null) {
        console.log(node);
        throw new Error(
          `AST node of type "${node.type}" is missing "loc" property`,
        );
      }
      // $FlowExpectedError[cannot-write]
      delete node.loc;

      if (options?.babel === true) {
        if (
          options?.enforceLocationInformation === true &&
          // $FlowExpectedError[prop-missing]
          node.start == null
        ) {
          throw new Error(
            `AST node of type "${node.type}" is missing "start" property`,
          );
        }
        // $FlowExpectedError[prop-missing]
        delete node.start;

        // $FlowExpectedError[prop-missing]
        if (options?.enforceLocationInformation === true && node.end == null) {
          throw new Error(
            `AST node of type "${node.type}" is missing "end" property`,
          );
        }
        // $FlowExpectedError[prop-missing]
        delete node.end;

        if (
          options?.enforceLocationInformation === true &&
          node.parent != null
        ) {
          throw new Error(
            `AST node of type "${node.type}" has "parent" property`,
          );
        }

        if (
          options?.enforceLocationInformation === true &&
          node.range != null
        ) {
          throw new Error(
            `AST node of type "${node.type}" has "range" property`,
          );
        }
      } else {
        if (
          options?.enforceLocationInformation === true &&
          node.range == null
        ) {
          throw new Error(
            `AST node of type "${node.type}" is missing "range" property`,
          );
        }

        if (options?.preserveRange !== true) {
          // $FlowExpectedError[cannot-write]
          delete node.range;
        }

        // $FlowExpectedError[cannot-write]
        delete node.parent;
      }
    },
    leave() {},
    visitorKeys: options?.babel === true ? BABEL_VISITOR_KEYS : null,
  });

  if (ast.type === 'Program') {
    return {
      type: 'Program',
      body: ast.body,
    };
  }

  return ast;
}
