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

describe('Literal', () => {
  const testCase: AlignmentCase = {
    code: `
      null;
      10;
      0.56283;
      "test";
      true;
      /foo/g;
      4321n;
      12_34n;
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('Emitted `.value` type is correct', () => {
    // Also assert that the literal's `.value` is the correct instance type
    expect(parse(testCase.code)).toMatchObject({
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
  });

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "directive": null,
            "expression": {
              "literalType": "null",
              "raw": "null",
              "type": "Literal",
              "value": null,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "literalType": "numeric",
              "raw": "10",
              "type": "Literal",
              "value": 10,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "literalType": "numeric",
              "raw": "0.56283",
              "type": "Literal",
              "value": 0.56283,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "literalType": "string",
              "raw": ""test"",
              "type": "Literal",
              "value": "test",
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "literalType": "boolean",
              "raw": "true",
              "type": "Literal",
              "value": true,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "literalType": "regexp",
              "raw": "/foo/g",
              "regex": {
                "flags": "g",
                "pattern": "foo",
              },
              "type": "Literal",
              "value": /foo/g,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "bigint": "4321",
              "literalType": "bigint",
              "raw": "4321n",
              "type": "Literal",
              "value": 4321n,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
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
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    // Babel AST literal nodes
    expect(parseForSnapshot(testCase.code, {babel: true}))
      .toMatchInlineSnapshot(`
      {
        "body": [
          {
            "directive": null,
            "expression": {
              "type": "NullLiteral",
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "type": "NumericLiteral",
              "value": 10,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "type": "NumericLiteral",
              "value": 0.56283,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "type": "StringLiteral",
              "value": "test",
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "type": "BooleanLiteral",
              "value": true,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "flags": "g",
              "pattern": "foo",
              "type": "RegExpLiteral",
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "bigint": "4321n",
              "type": "BigIntLiteral",
              "value": 4321n,
            },
            "type": "ExpressionStatement",
          },
          {
            "directive": null,
            "expression": {
              "bigint": "12_34n",
              "type": "BigIntLiteral",
              "value": 1234n,
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    expectBabelAlignment(testCase);
  });

  describe('RegExp', () => {
    const testCase: AlignmentCase = {
      code: `
        /foo/qq
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Invalid regular expression flag',
      },
      babel: {
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Invalid regular expression flag',
      },
    };

    test('ESTree', () => {
      // ESTree AST with invalid RegExp literal
      expect(parse(testCase.code)).toMatchObject({
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
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expectBabelAlignment(testCase);
    });
  });
});

describe('JSX String Literals', () => {
  const testCase: AlignmentCase = {
    code: `
      <foo a="abc &amp; def" />;
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };
  test('ESTree', () => {
    expect(parse(testCase.code)).toMatchObject({
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
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    expectBabelAlignment(testCase);
  });
});
