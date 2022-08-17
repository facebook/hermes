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

describe('ThisTypeAnnotation', () => {
  const testCase: AlignmentCase = {
    code: `
      type t1 = this;
      type t3 = T.this;
      type t4 = this.T;
    `,
    espree: {
      // espree doesn't support types
      expectToFail: 'espree-exception',
      expectedExceptionMessage: 'Unexpected token t1',
    },
    // babel: {expectToFail: false},
    babel: {
      // TODO - once we update the babel version we test against - we can enable this
      expectToFail: 'babel-exception',
      expectedExceptionMessage: 'Unexpected token, expected ";"',
    },
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "id": Object {
              "name": "t1",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": Object {
              "type": "ThisTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          Object {
            "id": Object {
              "name": "t3",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": Object {
              "id": Object {
                "id": Object {
                  "name": "this",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "qualification": Object {
                  "name": "T",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "QualifiedTypeIdentifier",
              },
              "type": "GenericTypeAnnotation",
              "typeParameters": null,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          Object {
            "id": Object {
              "name": "t4",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": Object {
              "id": Object {
                "id": Object {
                  "name": "T",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "qualification": Object {
                  "name": "this",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "QualifiedTypeIdentifier",
              },
              "type": "GenericTypeAnnotation",
              "typeParameters": null,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    const thisAlias = {
      type: 'TypeAlias',
      right: {
        type: 'ThisTypeAnnotation',
      },
    };
    const genericAlias = {
      type: 'TypeAlias',
      right: {
        type: 'GenericTypeAnnotation',
      },
    };
    const expectedProgram = {
      type: 'Program',
      body: [thisAlias, genericAlias, genericAlias],
    };
    expect(parse(testCase.code, {babel: true})).toMatchObject({
      type: 'File',
      program: expectedProgram,
    });
    expectBabelAlignment(testCase);
  });

  describe('this with generic', () => {
    const testCase: AlignmentCase = {
      code: `
        type t2 = this<T>;
      `,
      espree: {
        // espree doesn't support types
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token t2',
      },
      babel: {
        // babel crashes on this when it really shouldn't.
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token, expected ";"',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "id": Object {
                "name": "t2",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "right": Object {
                "id": Object {
                  "name": "this",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "GenericTypeAnnotation",
                "typeParameters": Object {
                  "params": Array [
                    Object {
                      "id": Object {
                        "name": "T",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "type": "GenericTypeAnnotation",
                      "typeParameters": null,
                    },
                  ],
                  "type": "TypeParameterInstantiation",
                },
              },
              "type": "TypeAlias",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      const genericAlias = {
        type: 'TypeAlias',
        right: {
          type: 'GenericTypeAnnotation',
        },
      };
      const expectedProgram = {
        type: 'Program',
        body: [genericAlias],
      };
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: expectedProgram,
      });
      expectBabelAlignment(testCase);
    });
  });
});

describe('ThisTypeAnnotation as a function parameter', () => {
  test('Removed in Babel mode', () => {
    const params = [
      {
        type: 'Identifier',
        name: 'param',
        typeAnnotation: {
          typeAnnotation: {type: 'NumberTypeAnnotation'},
        },
      },
    ];

    expect(
      parse(
        `
          function f1(this: string, param: number) {}
          (function f2(this: string, param: number) {});
        `,
        {babel: true},
      ),
    ).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'FunctionDeclaration',
            id: {name: 'f1'},
            params,
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'FunctionExpression',
              id: {name: 'f2'},
              params,
            },
          },
        ],
      },
    });
  });

  test('Preserved in ESTree mode', () => {
    const source = `
      function f1(this: string, param: number) {}
      (function f2(this: string, param: number) {});
    `;

    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
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
              "name": "f1",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "params": Array [
              Object {
                "name": "this",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": Object {
                  "type": "TypeAnnotation",
                  "typeAnnotation": Object {
                    "type": "StringTypeAnnotation",
                  },
                },
              },
              Object {
                "name": "param",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": Object {
                  "type": "TypeAnnotation",
                  "typeAnnotation": Object {
                    "type": "NumberTypeAnnotation",
                  },
                },
              },
            ],
            "predicate": null,
            "returnType": null,
            "type": "FunctionDeclaration",
            "typeParameters": null,
          },
          Object {
            "directive": null,
            "expression": Object {
              "async": false,
              "body": Object {
                "body": Array [],
                "type": "BlockStatement",
              },
              "expression": false,
              "generator": false,
              "id": Object {
                "name": "f2",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": Array [
                Object {
                  "name": "this",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": Object {
                    "type": "TypeAnnotation",
                    "typeAnnotation": Object {
                      "type": "StringTypeAnnotation",
                    },
                  },
                },
                Object {
                  "name": "param",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": Object {
                    "type": "TypeAnnotation",
                    "typeAnnotation": Object {
                      "type": "NumberTypeAnnotation",
                    },
                  },
                },
              ],
              "predicate": null,
              "returnType": null,
              "type": "FunctionExpression",
              "typeParameters": null,
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
  });
});
