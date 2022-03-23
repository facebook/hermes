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

describe('Array', () => {
  const source = 'const [a,,b] = [1,,2];';
  test('ESTree', () => {
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "declarations": Array [
              Object {
                "id": Object {
                  "elements": Array [
                    Object {
                      "name": "a",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    null,
                    Object {
                      "name": "b",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                  ],
                  "type": "ArrayPattern",
                  "typeAnnotation": null,
                },
                "init": Object {
                  "elements": Array [
                    Object {
                      "literalType": "numeric",
                      "raw": "1",
                      "type": "Literal",
                      "value": 1,
                    },
                    null,
                    Object {
                      "literalType": "numeric",
                      "raw": "2",
                      "type": "Literal",
                      "value": 2,
                    },
                  ],
                  "trailingComma": false,
                  "type": "ArrayExpression",
                },
                "type": "VariableDeclarator",
              },
            ],
            "kind": "const",
            "type": "VariableDeclaration",
          },
        ],
        "type": "Program",
      }
    `);
  });

  test('Babel', () => {
    // Babel AST array nodes
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'VariableDeclaration',
            declarations: [
              {
                type: 'VariableDeclarator',
                id: {
                  type: 'ArrayPattern',
                  elements: [
                    {
                      type: 'Identifier',
                      name: 'a',
                    },
                    null,
                    {
                      type: 'Identifier',
                      name: 'b',
                    },
                  ],
                },
                init: {
                  type: 'ArrayExpression',
                  elements: [
                    {
                      type: 'NumericLiteral',
                      value: 1,
                    },
                    null,
                    {
                      type: 'NumericLiteral',
                      value: 2,
                    },
                  ],
                },
              },
            ],
          },
        ],
      },
    });
  });
});
