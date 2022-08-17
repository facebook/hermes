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

describe('RestElement', () => {
  const testCase: AlignmentCase = {
    code: `
      function test1(...rest) {}
      function test2([...rest]) {}
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "async": false,
            "body": Object {
              "body": Array [],
              "type": "BlockStatement",
            },
            "expression": false,
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
                  "typeAnnotation": null,
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
            "expression": false,
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
                      "typeAnnotation": null,
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
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    expect(parse(testCase.code, {babel: true})).toMatchObject({
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
                  },
                ],
              },
            ],
          },
        ],
      },
    });
    expectBabelAlignment(testCase);
  });

  describe('with type', () => {
    const testCase: AlignmentCase = {
      code: `
        function test1(...rest: string) {}
        function test2([...rest: string]) {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token :',
      },
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "async": false,
              "body": Object {
                "body": Array [],
                "type": "BlockStatement",
              },
              "expression": false,
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
              "expression": false,
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
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
      expectBabelAlignment(testCase);
    });
  });

  describe('with type but not an identifier', () => {
    const testCase: AlignmentCase = {
      code: `
        function test1(...[]: string) {}
        function test2([...[]: string]) {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token :',
      },
      babel: {
        // there's no way for us to get the end location right in this case :(
        expectToFail: 'ast-diff',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "async": false,
              "body": Object {
                "body": Array [],
                "type": "BlockStatement",
              },
              "expression": false,
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
                    "elements": Array [],
                    "type": "ArrayPattern",
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
              "expression": false,
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
                        "elements": Array [],
                        "type": "ArrayPattern",
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
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
                    type: 'ArrayPattern',
                    elements: [],
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
                        type: 'ArrayPattern',
                        elements: [],
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
      expectBabelAlignment(testCase);
    });
  });
});
