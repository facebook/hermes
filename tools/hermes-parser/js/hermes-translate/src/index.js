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
import flowToFlowDef from './flowToFlowDef';

export function translate(code: string, prettierOptions: {...} = {}): string {
  const {ast, scopeManager} = parse(code);

  const [flowDefAst, mutatedCode] = flowToFlowDef(ast, code, scopeManager, {
    recoverFromErrors: true,
  });

  return print(flowDefAst, mutatedCode, prettierOptions);
}
