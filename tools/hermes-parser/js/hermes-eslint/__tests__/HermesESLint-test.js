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

test('Parser produces ESTree AST', () => {
  expect(parseForESLint('const x = 1').ast).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'VariableDeclaration',
        kind: 'const',
        declarations: [
          {
            type: 'VariableDeclarator',
            id: {
              type: 'Identifier',
              name: 'x',
            },
            init: {
              type: 'Literal',
              value: 1,
            },
          },
        ],
      },
    ],
    tokens: [
      {
        type: 'Keyword',
        value: 'const',
      },
      {
        type: 'Identifier',
        value: 'x',
      },
      {
        type: 'Punctuator',
        value: '=',
      },
      {
        type: 'Numeric',
        value: '1',
      },
    ],
  });
});

test('Parser allows return outside function', () => {
  expect(parseForESLint('return 1').ast).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ReturnStatement',
        argument: {
          type: 'Literal',
          value: 1,
        },
      },
    ],
  });
});
