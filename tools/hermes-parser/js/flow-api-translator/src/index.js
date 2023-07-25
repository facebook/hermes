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

import type {MapperOptions} from './flowImportTo';

import {parse, print} from 'hermes-transform';
import {visitorKeys as tsVisitorKeys} from '@typescript-eslint/visitor-keys';
import flowToFlowDef from './flowToFlowDef';
import {flowDefToTSDef} from './flowDefToTSDef';
import {flowToJS} from './flowToJS';
import {flowImportTo} from './flowImportTo';

export async function translateFlowToFlowDef(
  code: string,
  prettierOptions: {...} = {},
): Promise<string> {
  const {ast, scopeManager} = await parse(code);

  const [flowDefAst, mutatedCode] = flowToFlowDef(ast, code, scopeManager, {
    recoverFromErrors: true,
  });

  return print(flowDefAst, mutatedCode, prettierOptions);
}

export async function translateFlowToTSDef(
  code: string,
  prettierOptions: {...} = {},
): Promise<string> {
  const flowDefCode = await translateFlowToFlowDef(code, prettierOptions);

  return translateFlowDefToTSDef(flowDefCode, prettierOptions);
}

export async function translateFlowDefToTSDef(
  code: string,
  prettierOptions: {...} = {},
): Promise<string> {
  const {ast, scopeManager} = await parse(code);
  const [tsAST, mutatedCode] = flowDefToTSDef(code, ast, scopeManager, {
    recoverFromErrors: true,
  });

  return print(
    // $FlowExpectedError[incompatible-call] - this is fine as we're providing the visitor keys
    tsAST,
    mutatedCode,
    {
      ...prettierOptions,
    },
    tsVisitorKeys,
  );
}

export async function translateFlowToJS(
  code: string,
  prettierOptions: {...} = {},
): Promise<string> {
  const {ast, scopeManager} = await parse(code);

  const jsAST = flowToJS(ast, code, scopeManager);

  return print(jsAST, code, prettierOptions);
}

export type {MapperOptions as FlowImportsMapperOptions};
export async function translateFlowImportsTo(
  code: string,
  prettierOptions: {...} = {},
  opts: MapperOptions,
): Promise<string> {
  const {ast, scopeManager} = await parse(code);

  const jsAST = flowImportTo(ast, code, scopeManager, opts);

  return print(jsAST, code, prettierOptions);
}
