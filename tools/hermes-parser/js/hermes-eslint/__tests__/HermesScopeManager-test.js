/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const {parseForESLint} = require('hermes-eslint');

describe('Source type option', () => {
  test('script', () => {
    const {ast, scopeManager} = parseForESLint('Foo', {sourceType: 'script'});

    expect(ast.sourceType).toEqual('script');
    expect(scopeManager.scopes).toHaveLength(1);
    expect(scopeManager.scopes[0].type).toEqual('global');
  });

  test('module', () => {
    const {ast, scopeManager} = parseForESLint('Foo', {sourceType: 'module'});

    expect(ast.sourceType).toEqual('module');
    expect(scopeManager.scopes).toHaveLength(2);
    expect(scopeManager.scopes[0].type).toEqual('global');
    expect(scopeManager.scopes[1].type).toEqual('module');
  });
});
