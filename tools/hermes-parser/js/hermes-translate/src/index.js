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

export function translateFlowToJS(
  code: string,
  prettierOptions: {...} = {},
): string {
  const {ast, scopeManager} = parse(code);

  const jsAST = flowToJS(ast, code, scopeManager);

  return print(jsAST, code, prettierOptions);
}

export type {MapperOptions as FlowImportsMapperOptions};
export function translateFlowImportsTo(
  code: string,
  prettierOptions: {...} = {},
  opts: MapperOptions,
): string {
  const {ast, scopeManager} = parse(code);

  const jsAST = flowImportTo(ast, code, scopeManager, opts);

  return print(jsAST, code, prettierOptions);
}
