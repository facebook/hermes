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

describe('Literal', () => {
  const source = `
    null;
    10;
    0.56283;
    "test";
    true;
    /foo/g;
    4321n;
    12_34n;
  `;

  test('ESTree', () => {
    const ast = parseForSnapshot(source);
    expect(ast).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "directive": null,
            "expression": Object {
              "literalType": "null",
              "raw": "null",
              "type": "Literal",
              "value": null,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "literalType": "numeric",
              "raw": "10",
              "type": "Literal",
              "value": 10,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "literalType": "numeric",
              "raw": "0.56283",
              "type": "Literal",
              "value": 0.56283,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "literalType": "string",
              "raw": "\\"test\\"",
              "type": "Literal",
              "value": "test",
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "literalType": "boolean",
              "raw": "true",
              "type": "Literal",
              "value": true,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "literalType": "regexp",
              "raw": "/foo/g",
              "regex": Object {
                "flags": "g",
                "pattern": "foo",
              },
              "type": "Literal",
              "value": /foo/g,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "bigint": "4321",
              "literalType": "bigint",
              "raw": "4321n",
              "type": "Literal",
              "value": 4321n,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "bigint": "1234",
              "literalType": "bigint",
              "raw": "12_34n",
              "type": "Literal",
              "value": 1234n,
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    // Also assert that the literal's `.value` is the correct instance type
    expect(parse(source)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            value: null,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            value: 10,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            value: 0.56283,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            value: 'test',
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            value: true,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            value: new RegExp('foo', 'g'),
          },
        },
        // we don't yet emit the bigint value
        {
          type: 'ExpressionStatement',
          expression: {
            // $FlowExpectedError[cannot-resolve-name] - not supported by flow yet
            value: BigInt(4321),
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            // $FlowExpectedError[cannot-resolve-name] - not supported by flow yet
            value: BigInt(1234),
          },
        },
      ],
    });

    // ESTree AST with invalid RegExp literal
    expect(parse('/foo/qq')).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'Literal',
            value: null,
            regex: {
              pattern: 'foo',
              flags: 'qq',
            },
          },
        },
      ],
    });
  });

  test('Babel', () => {
    // Babel AST literal nodes
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'NullLiteral',
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'NumericLiteral',
              value: 10,
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'NumericLiteral',
              value: 0.56283,
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'StringLiteral',
              value: 'test',
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'BooleanLiteral',
              value: true,
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'RegExpLiteral',
              pattern: 'foo',
              flags: 'g',
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'BigIntLiteral',
              bigint: '4321n',
            },
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'BigIntLiteral',
              bigint: '12_34n',
            },
          },
        ],
      },
    });
  });
});

test('Allow JSX String literals', () => {
  expect(parse(`<foo a="abc &amp; def" />;`)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'JSXElement',
          openingElement: {
            type: 'JSXOpeningElement',
            attributes: [
              {
                type: 'JSXAttribute',
                value: {
                  type: 'Literal',
                  value: 'abc & def',
                  raw: '"abc &amp; def"',
                },
              },
            ],
          },
        },
      },
    ],
  });
});
