/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const HermesParser = require('hermes-parser');
const ScopeManager = require('./HermesScopeManager');
const VisitorKeys = require('./HermesESLintVisitorKeys');

function parse(code, options = {}) {
  const parserOptions = {sourceType: options.sourceType, tokens: true};
  return HermesParser.parse(code, parserOptions);
}

function parseForESLint(code, options = {}) {
  const ast = parse(code, options);

  return {
    ast,
    scopeManager: ScopeManager.create(ast),
    visitorKeys: VisitorKeys,
  };
}

module.exports = {parse, parseForESLint};
