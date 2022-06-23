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

import type {Program} from 'hermes-estree';
import type {VisitorKeys as VisitorKeysType} from './HermesESLintVisitorKeys';
import type {PartialAnalyzeOptions, ScopeManager} from './scope-manager';

import * as HermesParser from 'hermes-parser';
import {analyze} from './scope-manager';
import VisitorKeys from './HermesESLintVisitorKeys';

type ParseForESLintOptions = $ReadOnly<{
  ...PartialAnalyzeOptions,
}>;

function parse(code: string, options?: ParseForESLintOptions): Program {
  const parserOptions = {
    allowReturnOutsideFunction: true,
    flow: 'all',
    sourceType: options?.sourceType ?? 'module',
    tokens: true,
  };

  try {
    const ast = HermesParser.parse(code, parserOptions);
    return ast;
  } catch (e) {
    // Format error location for ESLint
    if (e instanceof SyntaxError) {
      // $FlowFixMe[prop-missing]
      e.lineNumber = e.loc.line;
      // $FlowFixMe[prop-missing]
      e.column = e.loc.column;
    }

    throw e;
  }
}

type ParseForESLintReturn = {
  ast: Program,
  scopeManager: ScopeManager,
  visitorKeys: VisitorKeysType,
};
function parseForESLint(
  code: string,
  options?: ParseForESLintOptions,
): {
  ast: Program,
  scopeManager: ScopeManager,
  visitorKeys: VisitorKeysType,
} {
  const ast = parse(code, options);
  const scopeManager = analyze(ast, options);

  return {
    ast,
    scopeManager,
    visitorKeys: VisitorKeys,
  };
}

export type * from './scope-manager';
export type {ParseForESLintOptions, ParseForESLintReturn};
export {ScopeType, DefinitionType} from './scope-manager';
export {parse, parseForESLint, VisitorKeys};
