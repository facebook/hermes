/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {ESNode} from 'hermes-estree';

import visitorKeys from '../HermesESLintVisitorKeys';
import {Referencer} from './referencer';
import {ScopeManager} from './ScopeManager';

type AnalyzeOptions = $ReadOnly<{
  /**
   * Whether the whole script is executed under node.js environment.
   * When enabled, the scope manager adds a function scope immediately following the global scope.
   * Defaults to `false`.
   */
  globalReturn: boolean,

  /**
   * The identifier that's used for JSX Element creation (after transpilation).
   * This should not be a member expression - just the root identifier (i.e. use "React" instead of "React.createElement").
   * Defaults to `"React"`.
   */
  jsxPragma: string | null,

  /**
   * The identifier that's used for JSX fragment elements (after transpilation).
   * If `null`, assumes transpilation will always use a member on `jsxFactory` (i.e. React.Fragment).
   * This should not be a member expression - just the root identifier (i.e. use "h" instead of "h.Fragment").
   * Defaults to `null`.
   */
  jsxFragmentName: string | null,

  /**
   * The source type of the script.
   */
  sourceType: 'script' | 'module',
}>;
type PartialAnalyzeOptions = $ReadOnly<$Partial<AnalyzeOptions>>;

const DEFAULT_OPTIONS: AnalyzeOptions = {
  globalReturn: false,
  jsxPragma: 'React',
  jsxFragmentName: null,
  sourceType: 'module',
};

/**
 * Takes an AST and returns the analyzed scopes.
 */
function analyze(
  tree: ESNode,
  providedOptions?: PartialAnalyzeOptions,
): ScopeManager {
  const scopeManager = new ScopeManager({
    globalReturn: providedOptions?.globalReturn ?? DEFAULT_OPTIONS.globalReturn,
    sourceType: providedOptions?.sourceType ?? DEFAULT_OPTIONS.sourceType,
  });
  const referencer = new Referencer(
    {
      childVisitorKeys: visitorKeys,
      jsxPragma:
        providedOptions?.jsxPragma === undefined
          ? DEFAULT_OPTIONS.jsxPragma
          : providedOptions.jsxPragma,
      jsxFragmentName:
        providedOptions?.jsxFragmentName ?? DEFAULT_OPTIONS.jsxFragmentName,
    },
    scopeManager,
  );

  referencer.visit(tree);

  return scopeManager;
}

export type {AnalyzeOptions, PartialAnalyzeOptions};
export {analyze};
