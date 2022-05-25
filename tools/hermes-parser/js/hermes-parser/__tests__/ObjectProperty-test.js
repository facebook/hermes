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

describe('Object properties', () => {
  const testCase: AlignmentCase = {
    code: `
      ({
        prop1: 1,
        prop2: function() {},
        prop3() {},
        async prop4() {},
        get prop5() {},
        set prop6(x) {},
      })
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code, {preserveRange: true}))
      .toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "directive": null,
            "expression": Object {
              "properties": Array [
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "prop1",
                    "optional": false,
                    "range": Array [
                      18,
                      23,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "range": Array [
                    18,
                    26,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "literalType": "numeric",
                    "range": Array [
                      25,
                      26,
                    ],
                    "raw": "1",
                    "type": "Literal",
                    "value": 1,
                  },
                },
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "prop2",
                    "optional": false,
                    "range": Array [
                      36,
                      41,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "range": Array [
                    36,
                    56,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        54,
                        56,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      41,
                      56,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "prop3",
                    "optional": false,
                    "range": Array [
                      66,
                      71,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "range": Array [
                    66,
                    76,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        74,
                        76,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      71,
                      76,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "prop4",
                    "optional": false,
                    "range": Array [
                      92,
                      97,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "range": Array [
                    86,
                    102,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": true,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        100,
                        102,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      97,
                      102,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "prop5",
                    "optional": false,
                    "range": Array [
                      116,
                      121,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "get",
                  "method": false,
                  "range": Array [
                    112,
                    126,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        124,
                        126,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      121,
                      126,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "prop6",
                    "optional": false,
                    "range": Array [
                      140,
                      145,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "set",
                  "method": false,
                  "range": Array [
                    136,
                    151,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        149,
                        151,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": Array [
                      Object {
                        "name": "x",
                        "optional": false,
                        "range": Array [
                          146,
                          147,
                        ],
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                    ],
                    "predicate": null,
                    "range": Array [
                      145,
                      151,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
              ],
              "range": Array [
                8,
                160,
              ],
              "type": "ObjectExpression",
            },
            "range": Array [
              7,
              161,
            ],
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    expect(parse(testCase.code, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'ObjectExpression',
              properties: [
                {
                  type: 'ObjectProperty',
                  key: {
                    type: 'Identifier',
                    name: 'prop1',
                  },
                  value: {
                    type: 'NumericLiteral',
                    value: 1,
                  },
                  computed: false,
                  shorthand: false,
                },
                {
                  type: 'ObjectProperty',
                  key: {
                    type: 'Identifier',
                    name: 'prop2',
                  },
                  value: {
                    type: 'FunctionExpression',
                  },
                  computed: false,
                  shorthand: false,
                },
                {
                  type: 'ObjectMethod',
                  key: {
                    type: 'Identifier',
                    name: 'prop3',
                  },
                  kind: 'method',
                  id: null,
                  params: [],
                  body: {
                    type: 'BlockStatement',
                    body: [],
                  },
                  async: false,
                  generator: false,
                  returnType: null,
                  typeParameters: null,
                  predicate: null,
                },
                {
                  type: 'ObjectMethod',
                  key: {
                    type: 'Identifier',
                    name: 'prop4',
                  },
                  kind: 'method',
                  id: null,
                  params: [],
                  body: {
                    type: 'BlockStatement',
                    body: [],
                  },
                  async: true,
                  generator: false,
                  returnType: null,
                  typeParameters: null,
                  predicate: null,
                },
                {
                  type: 'ObjectMethod',
                  key: {
                    type: 'Identifier',
                    name: 'prop5',
                  },
                  kind: 'get',
                  id: null,
                  params: [],
                  body: {
                    type: 'BlockStatement',
                    body: [],
                  },
                  async: false,
                  generator: false,
                  returnType: null,
                  typeParameters: null,
                  predicate: null,
                },
                {
                  type: 'ObjectMethod',
                  key: {
                    type: 'Identifier',
                    name: 'prop6',
                  },
                  kind: 'set',
                  id: null,
                  params: [
                    {
                      type: 'Identifier',
                      name: 'x',
                    },
                  ],
                  body: {
                    type: 'BlockStatement',
                    body: [],
                  },
                  async: false,
                  generator: false,
                  returnType: null,
                  typeParameters: null,
                  predicate: null,
                },
              ],
            },
          },
        ],
      },
    });
    expectBabelAlignment(testCase);
  });
});
