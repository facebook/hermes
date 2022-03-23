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
      get prop4() {},
      set prop5(x) {},
    })
  `;

  test('ESTree', () => {
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
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
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "literalType": "numeric",
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
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": false,
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
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
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "init",
                  "method": true,
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
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
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "get",
                  "method": false,
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [],
                    "predicate": null,
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
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "set",
                  "method": false,
                  "shorthand": false,
                  "type": "Property",
                  "value": Object {
                    "async": false,
                    "body": Object {
                      "body": Array [],
                      "type": "BlockStatement",
                    },
                    "generator": false,
                    "id": null,
                    "params": Array [
                      Object {
                        "name": "x",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                    ],
                    "predicate": null,
                    "returnType": null,
                    "type": "FunctionExpression",
                    "typeParameters": null,
                  },
                },
              ],
              "type": "ObjectExpression",
            },
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
                    name: 'prop5',
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
