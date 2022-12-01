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
      {
        "body": [
          {
            "directive": null,
            "expression": {
              "properties": [
                {
                  "computed": false,
                  "key": {
                    "name": "prop1",
                    "optional": false,
                    "range": [
                      18,
                      23,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "range": [
                    18,
                    26,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": {
                    "literalType": "numeric",
                    "range": [
                      25,
                      26,
                    ],
                    "raw": "1",
                    "type": "Literal",
                    "value": 1,
                  },
                },
                {
                  "computed": false,
                  "key": {
                    "name": "prop2",
                    "optional": false,
                    "range": [
                      36,
                      41,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "range": [
                    36,
                    56,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": {
                    "async": false,
                    "body": {
                      "body": [],
                      "range": [
                        54,
                        56,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": [],
                    "predicate": null,
                    "range": [
                      41,
                      56,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                {
                  "computed": false,
                  "key": {
                    "name": "prop3",
                    "optional": false,
                    "range": [
                      66,
                      71,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "range": [
                    66,
                    76,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": {
                    "async": false,
                    "body": {
                      "body": [],
                      "range": [
                        74,
                        76,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": [],
                    "predicate": null,
                    "range": [
                      71,
                      76,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                {
                  "computed": false,
                  "key": {
                    "name": "prop4",
                    "optional": false,
                    "range": [
                      92,
                      97,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "range": [
                    86,
                    102,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": {
                    "async": true,
                    "body": {
                      "body": [],
                      "range": [
                        100,
                        102,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": [],
                    "predicate": null,
                    "range": [
                      97,
                      102,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                {
                  "computed": false,
                  "key": {
                    "name": "prop5",
                    "optional": false,
                    "range": [
                      116,
                      121,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "get",
                  "method": false,
                  "range": [
                    112,
                    126,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": {
                    "async": false,
                    "body": {
                      "body": [],
                      "range": [
                        124,
                        126,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": [],
                    "predicate": null,
                    "range": [
                      121,
                      126,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
                {
                  "computed": false,
                  "key": {
                    "name": "prop6",
                    "optional": false,
                    "range": [
                      140,
                      145,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "set",
                  "method": false,
                  "range": [
                    136,
                    151,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": {
                    "async": false,
                    "body": {
                      "body": [],
                      "range": [
                        149,
                        151,
                      ],
                      "type": "BlockStatement",
                    },
                    "expression": false,
                    "generator": false,
                    "id": null,
                    "params": [
                      {
                        "name": "x",
                        "optional": false,
                        "range": [
                          146,
                          147,
                        ],
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                    ],
                    "predicate": null,
                    "range": [
                      145,
                      151,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
              ],
              "range": [
                8,
                160,
              ],
              "type": "ObjectExpression",
            },
            "range": [
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
