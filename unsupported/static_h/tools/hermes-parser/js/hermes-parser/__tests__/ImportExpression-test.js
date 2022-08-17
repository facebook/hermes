/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {
  expectBabelAlignment,
  expectEspreeAlignment,
} from '../__test_utils__/alignment-utils';
import {parse, parseForSnapshot} from '../__test_utils__/parse';

describe('ImportExpression', () => {
  const testCase: AlignmentCase = {
    code: `
      import('foo')
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "directive": null,
            "expression": Object {
              "attributes": null,
              "source": Object {
                "literalType": "string",
                "raw": "'foo'",
                "type": "Literal",
                "value": "foo",
              },
              "type": "ImportExpression",
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    // Babel converts ImportExpression to CallExpression with Import callee
    expect(parse(testCase.code, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'CallExpression',
              callee: {
                type: 'Import',
              },
              arguments: [{type: 'StringLiteral', value: 'foo'}],
            },
          },
        ],
      },
    });
    expectBabelAlignment(testCase);
  });
});
