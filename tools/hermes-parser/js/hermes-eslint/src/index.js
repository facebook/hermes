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
import type ScopeManagerClass from './eslint-scope/scope-manager';

const HermesParser = require('hermes-parser');
const ScopeManager = require('./HermesScopeManager');
const VisitorKeys = require('./HermesESLintVisitorKeys');

type Options = {
  sourceType?: 'module' | 'script',
};

function parse(code: string, options?: Options): Program {
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
      e.lineNumber = e.loc.line;
      e.column = e.loc.column;
    }

    throw e;
  }
}

function parseForESLint(
  code: string,
  options?: Options,
): {
  ast: Program,
  scopeManager: ScopeManagerClass,
  visitorKeys: VisitorKeysType,
} {
  const ast = parse(code, options);

  return {
    ast,
    scopeManager: ScopeManager.create(ast),
    visitorKeys: VisitorKeys,
  };
}

export type * from './eslint-scope/types';
module.exports = {parse, parseForESLint, VisitorKeys};
