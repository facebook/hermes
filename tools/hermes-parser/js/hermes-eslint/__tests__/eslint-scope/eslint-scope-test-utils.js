/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const {parseForESLint: parse} = require('../../dist');
const {ScopeType} = require('../../dist/eslint-scope/scope');

/**
 * Tests forked from eslint-scope expect a default sourceType of 'script', not 'module'.
 * Preserve this behavior to avoid updating all eslint-scope tests.
 */
function parseForESLint(code, options = {}) {
  if (options.sourceType == null) {
    options.sourceType = 'script';
  }

  return parse(code, options);
}

test('eslint-scope tests default to script sourceType', () => {
  const {ast, scopeManager} = parseForESLint('Foo');

  expect(ast.sourceType).toEqual('script');
  expect(scopeManager.scopes).toHaveLength(1);
  expect(scopeManager.scopes[0].type).toEqual(ScopeType.Global);
});

module.exports = {parseForESLint};
