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

import {parse, parseForSnapshot} from '../__test_utils__/parse';

describe('ImportExpression', () => {
  const source = `import('foo')`;

  test('ESTree', () => {
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
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
  });

  test('Babel', () => {
    // Babel converts ImportExpression to CallExpression with Import callee
    expect(parse(source, {babel: true})).toMatchObject({
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
  });
});
