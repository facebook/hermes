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

import type {Program} from 'hermes-estree';

const ScopeManager = require('./eslint-scope/scope-manager');
const Referencer = require('./eslint-scope/referencer');
const VisitorKeys = require('./HermesESLintVisitorKeys');

function create(ast: Program): ScopeManager {
  const scopeManager = new ScopeManager({
    sourceType: ast.sourceType,
    childVisitorKeys: VisitorKeys,
  });
  const referencer = new Referencer(
    {
      childVisitorKeys: VisitorKeys,
    },
    scopeManager,
  );

  referencer.visit(ast);

  return scopeManager;
}

module.exports = {create};
