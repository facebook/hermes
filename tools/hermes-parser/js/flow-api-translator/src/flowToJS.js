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

import type {ScopeManager} from 'hermes-eslint';
import type {Program} from 'hermes-estree';
import type {ParserOptions} from 'hermes-parser';

import {Transforms} from 'hermes-parser';
import {astNodeMutationHelpers} from 'hermes-parser';
import {removeAtFlowFromDocblock} from './utils/DocblockUtils';

const {nodeWith} = astNodeMutationHelpers;

function stripAtFlow(ast: Program, _options: ParserOptions): Program {
  const docblock = ast.docblock;
  if (docblock == null) {
    return ast;
  }

  return nodeWith(ast, {
    docblock: removeAtFlowFromDocblock(docblock),
  });
}

export function flowToJS(
  sourceAST: Program,
  _code: string,
  _scopeManager: ScopeManager,
): Program {
  return [
    Transforms.stripComponentSyntax,
    Transforms.stripFlowTypes,
    stripAtFlow,
  ].reduce((ast, transform) => transform(ast, {}), sourceAST);
}
