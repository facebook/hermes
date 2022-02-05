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

describe('RestElement', () => {
  const source = `
    function test1(...rest: string) {}
    function test2([...rest: string]) {}
  `;

  test('ESTree', () => {
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "async": false,
            "body": Object {
              "body": Array [],
              "type": "BlockStatement",
            },
            "generator": false,
            "id": Object {
              "name": "test1",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "params": Array [
              Object {
                "argument": Object {
                  "name": "rest",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": Object {
                    "type": "TypeAnnotation",
                    "typeAnnotation": Object {
                      "type": "StringTypeAnnotation",
                    },
                  },
                },
                "type": "RestElement",
              },
            ],
            "predicate": null,
            "returnType": null,
            "type": "FunctionDeclaration",
            "typeParameters": null,
          },
          Object {
            "async": false,
            "body": Object {
              "body": Array [],
              "type": "BlockStatement",
            },
            "generator": false,
            "id": Object {
              "name": "test2",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "params": Array [
              Object {
                "elements": Array [
                  Object {
                    "argument": Object {
                      "name": "rest",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": Object {
                        "type": "TypeAnnotation",
                        "typeAnnotation": Object {
                          "type": "StringTypeAnnotation",
                        },
                      },
                    },
                    "type": "RestElement",
                  },
                ],
                "type": "ArrayPattern",
                "typeAnnotation": null,
              },
            ],
            "predicate": null,
            "returnType": null,
            "type": "FunctionDeclaration",
            "typeParameters": null,
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
            type: 'FunctionDeclaration',
            params: [
              {
                type: 'RestElement',
                argument: {
                  type: 'Identifier',
                  name: 'rest',
                  typeAnnotation: null,
                },
                typeAnnotation: {type: 'TypeAnnotation'},
              },
            ],
          },
          {
            type: 'FunctionDeclaration',
            params: [
              {
                type: 'ArrayPattern',
                elements: [
                  {
                    type: 'RestElement',
                    argument: {
                      type: 'Identifier',
                      name: 'rest',
                      typeAnnotation: null,
                    },
                    typeAnnotation: {type: 'TypeAnnotation'},
                  },
                ],
              },
            ],
          },
        ],
      },
    });
  });
});
