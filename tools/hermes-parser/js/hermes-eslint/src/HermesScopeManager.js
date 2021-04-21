/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

'use strict';

import type {Node} from './eslint-scope/ScopeManagerTypes';

const ScopeManager = require('./eslint-scope/scope-manager');
const {Definition} = require('./eslint-scope/definition');
const Referencer = require('./eslint-scope/referencer');
const VisitorKeys = require('./HermesESLintVisitorKeys');

function create(ast: Node): ScopeManager {
  const options = {
    sourceType: ast.sourceType,
    childVisitorKeys: VisitorKeys,
  };

  const scopeManager = new ScopeManager(options);
  const referencer = new Referencer(options, scopeManager);

  referencer.visit(ast);

  return scopeManager;
}

module.exports = {create};
