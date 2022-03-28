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

describe('Object properties', () => {
  const source = `
    ({
      prop1: 1,
      prop2: function() {},
      prop3() {},
      async prop4() {},
      get prop5() {},
      set prop6(x) {},
    })
  `;

  test('ESTree', () => {
    expect(parseForSnapshot(source, {preserveRange: true}))
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
                      14,
                      19,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "range": Array [
                    14,
                    22,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "literalType": "numeric",
                    "range": Array [
                      21,
                      22,
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
                      30,
                      35,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "range": Array [
                    30,
                    50,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        48,
                        50,
                      ],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      35,
                      50,
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
                      58,
                      63,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "range": Array [
                    58,
                    68,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        66,
                        68,
                      ],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      63,
                      68,
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
                      82,
                      87,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "range": Array [
                    76,
                    92,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": true,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        90,
                        92,
                      ],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      87,
                      92,
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
                      104,
                      109,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "get",
                  "method": false,
                  "range": Array [
                    100,
                    114,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        112,
                        114,
                      ],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
                    "range": Array [
                      109,
                      114,
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
                      126,
                      131,
                    ],
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "set",
                  "method": false,
                  "range": Array [
                    122,
                    137,
                  ],
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "range": Array [
                        135,
                        137,
                      ],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [
                      Object {
                        "name": "x",
                        "optional": false,
                        "range": Array [
                          132,
                          133,
                        ],
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                    ],
                    "predicate": null,
                    "range": Array [
                      131,
                      137,
                    ],
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
              ],
              "range": Array [
                6,
                144,
              ],
              "type": "ObjectExpression",
            },
            "range": Array [
              5,
              145,
            ],
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
  });

  test('Babel', () => {
    expect(parse(source, {babel: true})).toMatchObject({
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
  });
});
