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

describe('MethodDefinition', () => {
  describe('method', () => {
    const testCase: AlignmentCase = {
      code: `
        class C {
          foo() {}
        }
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      // ESTree AST contains MethodDefinition containing a FunctionExpression value
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                      "expression": false,
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
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Babel AST has ClassMethod containing all properties of FunctionExpression
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
      expectBabelAlignment(testCase);
    });
  });

  describe('constructor', () => {
    const testCase: AlignmentCase = {
      code: `
        class C {
          constructor() {}
        }
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      // ESTree AST contains MethodDefinition containing a FunctionExpression value
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "body": Object {
                "body": Array [
                  Object {
                    "computed": false,
                    "key": Object {
                      "name": "constructor",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "constructor",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": Object {
                      "async": false,
                      "body": Object {
                        "body": Array [],
                        "type": "BlockStatement",
                      },
                      "expression": false,
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
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Babel AST has ClassMethod containing all properties of FunctionExpression
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
                      name: 'constructor',
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
                    // the kind should be set correctly
                    kind: 'constructor',
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

  describe('accessors', () => {
    const testCase: AlignmentCase = {
      code: `
        class C {
          get foo() { return 1; }
          set foo(v) { }
        }
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      // ESTree AST contains MethodDefinition containing a FunctionExpression value
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                    "kind": "get",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": Object {
                      "async": false,
                      "body": Object {
                        "body": Array [
                          Object {
                            "argument": Object {
                              "literalType": "numeric",
                              "raw": "1",
                              "type": "Literal",
                              "value": 1,
                            },
                            "type": "ReturnStatement",
                          },
                        ],
                        "type": "BlockStatement",
                      },
                      "expression": false,
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
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "set",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": Object {
                      "async": false,
                      "body": Object {
                        "body": Array [],
                        "type": "BlockStatement",
                      },
                      "expression": false,
                      "generator": false,
                      "id": null,
                      "params": Array [
                        Object {
                          "name": "v",
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
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Babel AST has ClassMethod containing all properties of FunctionExpression
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
                      body: [
                        {
                          type: 'ReturnStatement',
                          argument: {
                            type: 'NumericLiteral',
                          },
                        },
                      ],
                    },
                    async: false,
                    generator: false,
                    returnType: null,
                    typeParameters: null,
                    predicate: null,
                    // the kind should be set correctly
                    kind: 'get',
                  },
                  {
                    type: 'ClassMethod',
                    key: {
                      type: 'Identifier',
                      name: 'foo',
                    },
                    id: null,
                    params: [
                      {
                        type: 'Identifier',
                        name: 'v',
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
                    // the kind should be set correctly
                    kind: 'set',
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
});
