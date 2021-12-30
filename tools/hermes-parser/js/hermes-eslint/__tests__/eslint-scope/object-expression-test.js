/**
 * Portions Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/*
 Copyright JS Foundation and other contributors, https://js.foundation
 Copyright (C) 2012-2013 Yusuke Suzuki (twitter: @Constellation) and other contributors.

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

const HermesScopeManager = require('../../dist/HermesScopeManager');

describe('object expression', () => {
  it("doesn't require property type", () => {
    // Hardcoded AST.  Escope adds an extra "Property"
    // key/value to ObjectExpressions, so we're not using
    // it parse a program string.
    const ast = {
      type: 'Program',
      body: [
        {
          type: 'VariableDeclaration',
          declarations: [
            {
              type: 'VariableDeclarator',
              id: {
                type: 'Identifier',
                name: 'a',
              },
              init: {
                type: 'ObjectExpression',
                properties: [
                  {
                    kind: 'init',
                    key: {
                      type: 'Identifier',
                      name: 'foo',
                    },
                    value: {
                      type: 'Identifier',
                      name: 'a',
                    },
                  },
                ],
              },
            },
          ],
        },
      ],
    };

    const scope = HermesScopeManager.create(ast).scopes[0];

    expect(scope.variables).toHaveLength(1);
    expect(scope.references).toHaveLength(2);
    expect(scope.variables[0].name).toEqual('a');
    expect(scope.references[0].identifier.name).toEqual('a');
    expect(scope.references[1].identifier.name).toEqual('a');
  });
});
