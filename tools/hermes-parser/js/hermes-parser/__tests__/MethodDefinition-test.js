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

describe('MethodDefinition', () => {
  const source = `
    class C {
      foo() {}
    }
  `;

  test('ESTree', () => {
    // ESTree AST contains MethodDefinition containing a FunctionExpression value
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "body": Object {
              "body": Array [
                Object {
                  "computed": false,
                  "key": Object {
                    "name": "foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "kind": "method",
                  "static": false,
                  "type": "MethodDefinition",
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
              ],
              "type": "ClassBody",
            },
            "decorators": Array [],
            "id": Object {
              "name": "C",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "implements": Array [],
            "superClass": null,
            "superTypeParameters": null,
            "type": "ClassDeclaration",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
  });

  test('Babel', () => {
    // Babel AST has ClassMethod containing all properties of FunctionExpression
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ClassDeclaration',
            body: {
              type: 'ClassBody',
              body: [
                {
                  type: 'ClassMethod',
                  key: {
                    type: 'Identifier',
                    name: 'foo',
                  },
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
              ],
            },
          },
        ],
      },
    });
  });
});
