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

import {parse, print} from 'hermes-transform';
import {visitorKeys as tsVisitorKeys} from '@typescript-eslint/visitor-keys';
import flowToFlowDef from './flowToFlowDef';
import {flowDefToTSDef} from './flowDefToTSDef';

export function translateFlowToFlowDef(
  code: string,
  prettierOptions: {...} = {},
): string {
  const {ast, scopeManager} = parse(code);

  const [flowDefAst, mutatedCode] = flowToFlowDef(ast, code, scopeManager, {
    recoverFromErrors: true,
  });

  return print(flowDefAst, mutatedCode, prettierOptions);
}

export function translateFlowToTSDef(
  code: string,
  prettierOptions: {...} = {},
): string {
  const flowDefCode = translateFlowToFlowDef(code, prettierOptions);

  return translateFlowDefToTSDef(flowDefCode, prettierOptions);
}

export function translateFlowDefToTSDef(
  code: string,
  prettierOptions: {...} = {},
): string {
  const {ast, scopeManager} = parse(code);
  const tsAST = flowDefToTSDef(code, ast, scopeManager);

  return print(
    // $FlowExpectedError[incompatible-call] - this is fine as we're providing the visitor keys
    tsAST,
    code,
    {
      ...prettierOptions,
    },
    tsVisitorKeys,
  );
}
