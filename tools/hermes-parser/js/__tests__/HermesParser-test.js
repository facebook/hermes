/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

jest.disableAutomock();

const {parse} = require('../build/HermesParser');

test('Can parse simple file', () => {
  expect(parse('const x = 1')).toMatchObject({
    type: 'Program',
    body: [
      expect.objectContaining({
        type: 'VariableDeclaration',
        kind: 'const',
        declarations: [
          expect.objectContaining({
            type: 'VariableDeclarator',
            id: expect.objectContaining({
              type: 'Identifier',
              name: 'x',
            }),
            init: expect.objectContaining({
              type: 'NumericLiteral',
              value: 1,
            }),
          }),
        ],
      }),
    ],
  });
});

test('Parse error is thrown', () => {
  expect(() => parse('const = 1')).toThrow(
    `1:6: 'identifier' expected in declaration
const = 1
~~~~~~^
`,
  );
});

test('Parse error does not include caret line for non-ASCII characters', () => {
  expect(() => parse('/*\u0176*/ const = 1')).toThrow(
    `1:13: 'identifier' expected in declaration
/*\u0176*/ const = 1
`,
  );
});

test('Parsing comments', () => {
  expect(parse('/*Block comment*/ 1; // Line comment')).toMatchObject({
    type: 'Program',
    comments: [
      {
        type: 'Block',
        value: 'Block comment',
      },
      {
        type: 'Line',
        value: ' Line comment',
      },
    ],
  });
});
