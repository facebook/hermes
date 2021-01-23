/**
 * Portions Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/*
 Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
'use strict';

const {parse} = require('../../dist');
const {analyze} = require('../../dist/eslint-scope');

describe('impliedStrict option', () => {
  it('ensures all user scopes are strict if ecmaVersion >= 5', () => {
    const ast = parse(`
            function foo() {
                function bar() {
                    "use strict";
                }
            }
        `);

    const scopeManager = analyze(ast, {ecmaVersion: 5, impliedStrict: true});

    expect(scopeManager.scopes).toHaveLength(3);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.block.type).toEqual('Program');
    expect(scope.isStrict).toBe(true);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.block.type).toEqual('FunctionDeclaration');
    expect(scope.isStrict).toBe(true);

    scope = scopeManager.scopes[2];
    expect(scope.type).toEqual('function');
    expect(scope.block.type).toEqual('FunctionDeclaration');
    expect(scope.isStrict).toBe(true);
  });

  it('ensures impliedStrict option is only effective when ecmaVersion option >= 5', () => {
    const ast = parse(`
            function foo() {}
        `);

    const scopeManager = analyze(ast, {ecmaVersion: 3, impliedStrict: true});

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.block.type).toEqual('Program');
    expect(scope.isStrict).toBe(false);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.block.type).toEqual('FunctionDeclaration');
    expect(scope.isStrict).toBe(false);
  });

  it('omits a nodejs global scope when ensuring all user scopes are strict', () => {
    const ast = parse(`
            function foo() {}
        `);

    const scopeManager = analyze(ast, {
      ecmaVersion: 5,
      nodejsScope: true,
      impliedStrict: true,
    });

    expect(scopeManager.scopes).toHaveLength(3);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.block.type).toEqual('Program');
    expect(scope.isStrict).toBe(false);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.block.type).toEqual('Program');
    expect(scope.isStrict).toBe(true);

    scope = scopeManager.scopes[2];
    expect(scope.type).toEqual('function');
    expect(scope.block.type).toEqual('FunctionDeclaration');
    expect(scope.isStrict).toBe(true);
  });

  it('omits a module global scope when ensuring all user scopes are strict', () => {
    const ast = parse('function foo() {}');

    const scopeManager = analyze(ast, {
      ecmaVersion: 6,
      impliedStrict: true,
      sourceType: 'module',
    });

    expect(scopeManager.scopes).toHaveLength(3);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.block.type).toEqual('Program');
    expect(scope.isStrict).toBe(false);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('module');
    expect(scope.isStrict).toBe(true);

    scope = scopeManager.scopes[2];
    expect(scope.type).toEqual('function');
    expect(scope.block.type).toEqual('FunctionDeclaration');
    expect(scope.isStrict).toBe(true);
  });
});
