/**
 * Portions Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/*
 Copyright (C) 2014 Yusuke Suzuki <utatane.tea@gmail.com>

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

const {parseForESLint} = require('./eslint-scope-test-utils');

describe('ES6 destructuring assignments', () => {
  it('Pattern in var in ForInStatement', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                for (var [a, b, c] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('c');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[2]);
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[3]);
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);
  });

  it('Pattern in let in ForInStatement', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                for (let [a, b, c] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(3); // [global, function, for]

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[2];
    expect(scope.type).toEqual('for');
    expect(scope.variables).toHaveLength(3);
    expect(scope.variables[0].name).toEqual('a');
    expect(scope.variables[1].name).toEqual('b');
    expect(scope.variables[2].name).toEqual('c');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[0]);
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[1]);
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[2]);
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);
    expect(scope.references[3].resolved).toBeNull();
  });

  it('Pattern with default values in var in ForInStatement', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                for (var [a, b, c = d] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(2);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'd',
    );
    expect(scope.implicit.referencesLeftToResolve[1].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('c');
    expect(scope.references).toHaveLength(6);
    expect(scope.references[0].identifier.name).toEqual('c');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].writeExpr.name).toEqual('d');
    expect(scope.references[0].resolved).toEqual(scope.variables[3]);
    expect(scope.references[1].identifier.name).toEqual('d');
    expect(scope.references[1].isWrite()).toBe(false);
    expect(scope.references[2].identifier.name).toEqual('a');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[1]);
    expect(scope.references[3].identifier.name).toEqual('b');
    expect(scope.references[3].isWrite()).toBe(true);
    expect(scope.references[3].resolved).toEqual(scope.variables[2]);
    expect(scope.references[4].identifier.name).toEqual('c');
    expect(scope.references[4].isWrite()).toBe(true);
    expect(scope.references[4].writeExpr.name).toEqual('array');
    expect(scope.references[4].resolved).toEqual(scope.variables[3]);
    expect(scope.references[5].identifier.name).toEqual('array');
    expect(scope.references[5].isWrite()).toBe(false);
  });

  it('Pattern with default values in let in ForInStatement', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                for (let [a, b, c = d] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(3); // [global, function, for]

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(2);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'd',
    );
    expect(scope.implicit.referencesLeftToResolve[0].from.type).toEqual('for');
    expect(scope.implicit.referencesLeftToResolve[1].identifier.name).toEqual(
      'array',
    );
    expect(scope.implicit.referencesLeftToResolve[1].from.type).toEqual('for');

    scope = scopeManager.scopes[2];
    expect(scope.type).toEqual('for');
    expect(scope.variables).toHaveLength(3);
    expect(scope.variables[0].name).toEqual('a');
    expect(scope.variables[1].name).toEqual('b');
    expect(scope.variables[2].name).toEqual('c');
    expect(scope.references).toHaveLength(6);
    expect(scope.references[0].identifier.name).toEqual('c');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].writeExpr.name).toEqual('d');
    expect(scope.references[0].resolved).toEqual(scope.variables[2]);
    expect(scope.references[1].identifier.name).toEqual('d');
    expect(scope.references[1].isWrite()).toBe(false);
    expect(scope.references[2].identifier.name).toEqual('a');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].writeExpr.name).toEqual('array');
    expect(scope.references[2].resolved).toEqual(scope.variables[0]);
    expect(scope.references[3].identifier.name).toEqual('b');
    expect(scope.references[3].isWrite()).toBe(true);
    expect(scope.references[3].writeExpr.name).toEqual('array');
    expect(scope.references[3].resolved).toEqual(scope.variables[1]);
    expect(scope.references[4].identifier.name).toEqual('c');
    expect(scope.references[4].isWrite()).toBe(true);
    expect(scope.references[4].writeExpr.name).toEqual('array');
    expect(scope.references[4].resolved).toEqual(scope.variables[2]);
    expect(scope.references[5].identifier.name).toEqual('array');
    expect(scope.references[5].isWrite()).toBe(false);
    expect(scope.references[5].resolved).toBeNull();
  });

  it('Pattern with nested default values in var in ForInStatement', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                for (var [a, [b, c = d] = e] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(3);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'd',
    );
    expect(scope.implicit.referencesLeftToResolve[1].identifier.name).toEqual(
      'e',
    );
    expect(scope.implicit.referencesLeftToResolve[2].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('c');
    expect(scope.references).toHaveLength(9);
    expect(scope.references[0].identifier.name).toEqual('b');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].writeExpr.name).toEqual('e');
    expect(scope.references[0].resolved).toEqual(scope.variables[2]);
    expect(scope.references[1].identifier.name).toEqual('c');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].writeExpr.name).toEqual('e');
    expect(scope.references[1].resolved).toEqual(scope.variables[3]);
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].writeExpr.name).toEqual('d');
    expect(scope.references[2].resolved).toEqual(scope.variables[3]);
    expect(scope.references[3].identifier.name).toEqual('d');
    expect(scope.references[3].isWrite()).toBe(false);
    expect(scope.references[4].identifier.name).toEqual('e');
    expect(scope.references[4].isWrite()).toBe(false);
    expect(scope.references[5].identifier.name).toEqual('a');
    expect(scope.references[5].isWrite()).toBe(true);
    expect(scope.references[5].writeExpr.name).toEqual('array');
    expect(scope.references[5].resolved).toEqual(scope.variables[1]);
    expect(scope.references[6].identifier.name).toEqual('b');
    expect(scope.references[6].isWrite()).toBe(true);
    expect(scope.references[6].writeExpr.name).toEqual('array');
    expect(scope.references[6].resolved).toEqual(scope.variables[2]);
    expect(scope.references[7].identifier.name).toEqual('c');
    expect(scope.references[7].isWrite()).toBe(true);
    expect(scope.references[7].writeExpr.name).toEqual('array');
    expect(scope.references[7].resolved).toEqual(scope.variables[3]);
    expect(scope.references[8].identifier.name).toEqual('array');
    expect(scope.references[8].isWrite()).toBe(false);
  });

  it('Pattern with nested default values in let in ForInStatement', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                for (let [a, [b, c = d] = e] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(3); // [global, function, for]

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(3);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'd',
    );
    expect(scope.implicit.referencesLeftToResolve[0].from.type).toEqual('for');
    expect(scope.implicit.referencesLeftToResolve[1].identifier.name).toEqual(
      'e',
    );
    expect(scope.implicit.referencesLeftToResolve[1].from.type).toEqual('for');
    expect(scope.implicit.referencesLeftToResolve[2].identifier.name).toEqual(
      'array',
    );
    expect(scope.implicit.referencesLeftToResolve[2].from.type).toEqual('for');

    scope = scopeManager.scopes[2];
    expect(scope.type).toEqual('for');
    expect(scope.variables).toHaveLength(3);
    expect(scope.variables[0].name).toEqual('a');
    expect(scope.variables[1].name).toEqual('b');
    expect(scope.variables[2].name).toEqual('c');
    expect(scope.references).toHaveLength(9);
    expect(scope.references[0].identifier.name).toEqual('b');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].writeExpr.name).toEqual('e');
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('c');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].writeExpr.name).toEqual('e');
    expect(scope.references[1].resolved).toEqual(scope.variables[2]);
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].writeExpr.name).toEqual('d');
    expect(scope.references[2].resolved).toEqual(scope.variables[2]);
    expect(scope.references[3].identifier.name).toEqual('d');
    expect(scope.references[3].isWrite()).toBe(false);
    expect(scope.references[4].identifier.name).toEqual('e');
    expect(scope.references[4].isWrite()).toBe(false);
    expect(scope.references[5].identifier.name).toEqual('a');
    expect(scope.references[5].isWrite()).toBe(true);
    expect(scope.references[5].writeExpr.name).toEqual('array');
    expect(scope.references[5].resolved).toEqual(scope.variables[0]);
    expect(scope.references[6].identifier.name).toEqual('b');
    expect(scope.references[6].isWrite()).toBe(true);
    expect(scope.references[6].writeExpr.name).toEqual('array');
    expect(scope.references[6].resolved).toEqual(scope.variables[1]);
    expect(scope.references[7].identifier.name).toEqual('c');
    expect(scope.references[7].isWrite()).toBe(true);
    expect(scope.references[7].writeExpr.name).toEqual('array');
    expect(scope.references[7].resolved).toEqual(scope.variables[2]);
    expect(scope.references[8].identifier.name).toEqual('array');
    expect(scope.references[8].isWrite()).toBe(false);
    expect(scope.references[8].resolved).toBeNull();
  });

  it('Pattern with default values in var in ForInStatement (separate declarations)', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var a, b, c;
                for ([a, b, c = d] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(2);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'd',
    );
    expect(scope.implicit.referencesLeftToResolve[1].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('c');
    expect(scope.references).toHaveLength(6);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[2]);
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].writeExpr.name).toEqual('d');
    expect(scope.references[2].resolved).toEqual(scope.variables[3]);
    expect(scope.references[3].identifier.name).toEqual('c');
    expect(scope.references[3].isWrite()).toBe(true);
    expect(scope.references[3].writeExpr.name).toEqual('array');
    expect(scope.references[3].resolved).toEqual(scope.variables[3]);
    expect(scope.references[4].identifier.name).toEqual('d');
    expect(scope.references[4].isWrite()).toBe(false);
    expect(scope.references[5].identifier.name).toEqual('array');
    expect(scope.references[5].isWrite()).toBe(false);
  });

  it('Pattern with default values in var in ForInStatement (separate declarations and with MemberExpression)', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var obj;
                for ([obj.a, obj.b, obj.c = d] in array);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(2);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'd',
    );
    expect(scope.implicit.referencesLeftToResolve[1].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(2);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('obj');
    expect(scope.references).toHaveLength(5);
    expect(scope.references[0].identifier.name).toEqual('obj'); // obj.a
    expect(scope.references[0].isWrite()).toBe(false);
    expect(scope.references[0].isRead()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('obj'); // obj.b
    expect(scope.references[1].isWrite()).toBe(false);
    expect(scope.references[1].isRead()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[1]);
    expect(scope.references[2].identifier.name).toEqual('obj'); // obj.c
    expect(scope.references[2].isWrite()).toBe(false);
    expect(scope.references[2].isRead()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[1]);
    expect(scope.references[3].identifier.name).toEqual('d');
    expect(scope.references[3].isWrite()).toBe(false);
    expect(scope.references[3].isRead()).toBe(true);
    expect(scope.references[4].identifier.name).toEqual('array');
    expect(scope.references[4].isWrite()).toBe(false);
    expect(scope.references[4].isRead()).toBe(true);
  });

  it('ArrayPattern in var', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var [a, b, c] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('c');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[2]);
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[3]);
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);
  });

  it('SpreadElement in var', () => {
    let {ast, scopeManager} = parseForESLint(`
            (function () {
                var [a, b, ...rest] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('rest');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[2]);
    expect(scope.references[2].identifier.name).toEqual('rest');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[3]);
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);

    ({ast, scopeManager} = parseForESLint(`
            (function () {
                var [a, b, ...[c, d, ...rest]] = array;
            }());
        `));

    expect(scopeManager.scopes).toHaveLength(2);

    scope = scopeManager.scopes[0];
    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');

    expect(scope.variables).toHaveLength(6);
    const expectedVariableNames = ['arguments', 'a', 'b', 'c', 'd', 'rest'];

    for (let index = 0; index < expectedVariableNames.length; index++) {
      expect(scope.variables[index].name).toEqual(expectedVariableNames[index]);
    }

    expect(scope.references).toHaveLength(6);
    const expectedReferenceNames = ['a', 'b', 'c', 'd', 'rest'];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
      expect(scope.references[index].isWrite()).toBe(true);
    }
    expect(scope.references[5].identifier.name).toEqual('array');
    expect(scope.references[5].isWrite()).toBe(false);
  });

  it('ObjectPattern in var', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var {
                    shorthand,
                    key: value,
                    hello: {
                        world
                    }
                } = object;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'object',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('shorthand');
    expect(scope.variables[2].name).toEqual('value');
    expect(scope.variables[3].name).toEqual('world');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('shorthand');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('value');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[2]);
    expect(scope.references[2].identifier.name).toEqual('world');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[3]);
    expect(scope.references[3].identifier.name).toEqual('object');
    expect(scope.references[3].isWrite()).toBe(false);
  });

  it('complex pattern in var', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var {
                    shorthand,
                    key: [ a, b, c, d, e ],
                    hello: {
                        world
                    }
                } = object;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'object',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(8);
    const expectedVariableNames = [
      'arguments',
      'shorthand',
      'a',
      'b',
      'c',
      'd',
      'e',
      'world',
    ];

    for (let index = 0; index < expectedVariableNames.length; index++) {
      expect(scope.variables[index].name).toEqual(expectedVariableNames[index]);
    }
    expect(scope.references).toHaveLength(8);
    const expectedReferenceNames = [
      'shorthand',
      'a',
      'b',
      'c',
      'd',
      'e',
      'world',
    ];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
      expect(scope.references[index].isWrite()).toBe(true);
    }
    expect(scope.references[7].identifier.name).toEqual('object');
    expect(scope.references[7].isWrite()).toBe(false);
  });

  it('ArrayPattern in AssignmentExpression', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                [a, b, c] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(4);
    expect(
      scope.implicit.referencesLeftToResolve.map(left => left.identifier.name),
    ).toEqual(['a', 'b', 'c', 'array']);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(1);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toBeNull();
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toBeNull();
    expect(scope.references[2].identifier.name).toEqual('c');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toBeNull();
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);
  });

  it('ArrayPattern with MemberExpression in AssignmentExpression', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var obj;
                [obj.a, obj.b, obj.c] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(2);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('obj');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('obj');
    expect(scope.references[0].isWrite()).toBe(false);
    expect(scope.references[0].isRead()).toBe(true);
    expect(scope.references[0].resolved).toEqual(scope.variables[1]);
    expect(scope.references[1].identifier.name).toEqual('obj');
    expect(scope.references[1].isWrite()).toBe(false);
    expect(scope.references[1].isRead()).toBe(true);
    expect(scope.references[1].resolved).toEqual(scope.variables[1]);
    expect(scope.references[2].identifier.name).toEqual('obj');
    expect(scope.references[2].isWrite()).toBe(false);
    expect(scope.references[2].isRead()).toBe(true);
    expect(scope.references[2].resolved).toEqual(scope.variables[1]);
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);
    expect(scope.references[3].isRead()).toBe(true);
  });

  it('SpreadElement in AssignmentExpression', () => {
    let {ast, scopeManager} = parseForESLint(`
            (function () {
                [a, b, ...rest] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(4);
    expect(
      scope.implicit.referencesLeftToResolve.map(left => left.identifier.name),
    ).toEqual(['a', 'b', 'rest', 'array']);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(1);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toBeNull();
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toBeNull();
    expect(scope.references[2].identifier.name).toEqual('rest');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toBeNull();
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);

    ({ast, scopeManager} = parseForESLint(`
            (function () {
                [a, b, ...[c, d, ...rest]] = array;
            }());
        `));

    expect(scopeManager.scopes).toHaveLength(2);

    scope = scopeManager.scopes[0];
    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(6);
    expect(
      scope.implicit.referencesLeftToResolve.map(left => left.identifier.name),
    ).toEqual(['a', 'b', 'c', 'd', 'rest', 'array']);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');

    expect(scope.variables).toHaveLength(1);
    expect(scope.variables[0].name).toEqual('arguments');

    expect(scope.references).toHaveLength(6);
    const expectedReferenceNames = ['a', 'b', 'c', 'd', 'rest'];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
      expect(scope.references[index].isWrite()).toBe(true);
      expect(scope.references[index].resolved).toBeNull();
    }
    expect(scope.references[5].identifier.name).toEqual('array');
    expect(scope.references[5].isWrite()).toBe(false);
  });

  it('SpreadElement with MemberExpression in AssignmentExpression', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                [a, b, ...obj.rest] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(4);
    expect(
      scope.implicit.referencesLeftToResolve.map(left => left.identifier.name),
    ).toEqual(['a', 'b', 'obj', 'array']);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(1);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toBeNull();
    expect(scope.references[1].identifier.name).toEqual('b');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toBeNull();
    expect(scope.references[2].identifier.name).toEqual('obj');
    expect(scope.references[2].isWrite()).toBe(false);
    expect(scope.references[3].identifier.name).toEqual('array');
    expect(scope.references[3].isWrite()).toBe(false);
  });

  it('ObjectPattern in AssignmentExpression', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                ({
                    shorthand,
                    key: value,
                    hello: {
                        world
                    }
                } = object);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(4);
    expect(
      scope.implicit.referencesLeftToResolve.map(left => left.identifier.name),
    ).toEqual(['shorthand', 'value', 'world', 'object']);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(1);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.references).toHaveLength(4);
    expect(scope.references[0].identifier.name).toEqual('shorthand');
    expect(scope.references[0].isWrite()).toBe(true);
    expect(scope.references[0].resolved).toBeNull();
    expect(scope.references[1].identifier.name).toEqual('value');
    expect(scope.references[1].isWrite()).toBe(true);
    expect(scope.references[1].resolved).toBeNull();
    expect(scope.references[2].identifier.name).toEqual('world');
    expect(scope.references[2].isWrite()).toBe(true);
    expect(scope.references[2].resolved).toBeNull();
    expect(scope.references[3].identifier.name).toEqual('object');
    expect(scope.references[3].isWrite()).toBe(false);
  });

  it('complex pattern in AssignmentExpression', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                ({
                    shorthand,
                    key: [ a, b, c, d, e ],
                    hello: {
                        world
                    }
                } = object);
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(8);
    expect(
      scope.implicit.referencesLeftToResolve.map(left => left.identifier.name),
    ).toEqual(['shorthand', 'a', 'b', 'c', 'd', 'e', 'world', 'object']);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(1);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.references).toHaveLength(8);
    const expectedReferenceNames = [
      'shorthand',
      'a',
      'b',
      'c',
      'd',
      'e',
      'world',
    ];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
      expect(scope.references[index].isWrite()).toBe(true);
    }
    expect(scope.references[7].identifier.name).toEqual('object');
    expect(scope.references[7].isWrite()).toBe(false);
  });

  it('ArrayPattern in parameters', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function ([a, b, c]) {
            }(array));
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(1);
    expect(scope.references[0].identifier.name).toEqual('array');
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('c');
    expect(scope.references).toHaveLength(0);
  });

  it('SpreadElement in parameters', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function ([a, b, ...rest], ...rest2) {
            }(array));
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(1);
    expect(scope.references[0].identifier.name).toEqual('array');
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'array',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(5);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('a');
    expect(scope.variables[2].name).toEqual('b');
    expect(scope.variables[3].name).toEqual('rest');
    expect(scope.variables[3].defs[0].rest).toBe(true);
    expect(scope.variables[4].name).toEqual('rest2');
    expect(scope.variables[4].defs[0].rest).toBe(true);
    expect(scope.references).toHaveLength(0);
  });

  it('ObjectPattern in parameters', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function ({
                    shorthand,
                    key: value,
                    hello: {
                        world
                    }
                }) {
            }(object));
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(1);
    expect(scope.references[0].identifier.name).toEqual('object');
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'object',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(4);
    expect(scope.variables[0].name).toEqual('arguments');
    expect(scope.variables[1].name).toEqual('shorthand');
    expect(scope.variables[2].name).toEqual('value');
    expect(scope.variables[3].name).toEqual('world');
    expect(scope.references).toHaveLength(0);
  });

  it('complex pattern in parameters', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function ({
                    shorthand,
                    key: [ a, b, c, d, e ],
                    hello: {
                        world
                    }
                }) {
            }(object));
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(1);
    expect(scope.references[0].identifier.name).toEqual('object');
    expect(scope.implicit.referencesLeftToResolve).toHaveLength(1);
    expect(scope.implicit.referencesLeftToResolve[0].identifier.name).toEqual(
      'object',
    );

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(8);
    const expectedVariableNames = [
      'arguments',
      'shorthand',
      'a',
      'b',
      'c',
      'd',
      'e',
      'world',
    ];

    for (let index = 0; index < expectedVariableNames.length; index++) {
      expect(scope.variables[index].name).toEqual(expectedVariableNames[index]);
    }
    expect(scope.references).toHaveLength(0);
  });

  it('default values and patterns in var', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var [a, b, c, d = 20 ] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(5);
    const expectedVariableNames = ['arguments', 'a', 'b', 'c', 'd'];

    for (let index = 0; index < expectedVariableNames.length; index++) {
      expect(scope.variables[index].name).toEqual(expectedVariableNames[index]);
    }
    expect(scope.references).toHaveLength(6);
    const expectedReferenceNames = [
      'a',
      'b',
      'c',
      'd', // assign 20
      'd', // assign array
      'array',
    ];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
    }
  });

  it('default values containing references and patterns in var', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var [a, b, c, d = e ] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(5);
    const expectedVariableNames = ['arguments', 'a', 'b', 'c', 'd'];

    for (let index = 0; index < expectedVariableNames.length; index++) {
      expect(scope.variables[index].name).toEqual(expectedVariableNames[index]);
    }
    expect(scope.references).toHaveLength(7);
    const expectedReferenceNames = [
      'a', // assign array
      'b', // assign array
      'c', // assign array
      'd', // assign e
      'd', // assign array
      'e',
      'array',
    ];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
    }
  });

  it('nested default values containing references and patterns in var', () => {
    const {ast, scopeManager} = parseForESLint(`
            (function () {
                var [a, b, [c, d = e] = f ] = array;
            }());
        `);

    expect(scopeManager.scopes).toHaveLength(2);

    let scope = scopeManager.scopes[0];

    expect(scope.type).toEqual('global');
    expect(scope.variables).toHaveLength(0);
    expect(scope.references).toHaveLength(0);

    scope = scopeManager.scopes[1];
    expect(scope.type).toEqual('function');
    expect(scope.variables).toHaveLength(5);
    const expectedVariableNames = ['arguments', 'a', 'b', 'c', 'd'];

    for (let index = 0; index < expectedVariableNames.length; index++) {
      expect(scope.variables[index].name).toEqual(expectedVariableNames[index]);
    }
    expect(scope.references).toHaveLength(10);
    const expectedReferenceNames = [
      'a', // assign array
      'b', // assign array
      'c', // assign f
      'c', // assign array
      'd', // assign f
      'd', // assign e
      'd', // assign array
      'e',
      'f',
      'array',
    ];

    for (let index = 0; index < expectedReferenceNames.length; index++) {
      expect(scope.references[index].identifier.name).toEqual(
        expectedReferenceNames[index],
      );
    }
  });
});
