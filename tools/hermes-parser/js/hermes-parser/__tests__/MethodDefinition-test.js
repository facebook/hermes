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
import {parseForSnapshot} from '../__test_utils__/parse';

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
        {
          "body": [
            {
              "body": {
                "body": [
                  {
                    "computed": false,
                    "key": {
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "method",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": {
                      "async": false,
                      "body": {
                        "body": [],
                        "type": "BlockStatement",
                      },
                      "expression": false,
                      "generator": false,
                      "id": null,
                      "params": [],
                      "predicate": null,
                      "returnType": null,
                      "type": "FunctionExpression",
                      "typeParameters": null,
                    },
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": [],
              "id": {
                "name": "C",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": [],
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [
                  {
                    "async": false,
                    "body": {
                      "body": [],
                      "directives": [],
                      "type": "BlockStatement",
                    },
                    "computed": false,
                    "generator": false,
                    "id": null,
                    "key": {
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "method",
                    "params": [],
                    "predicate": null,
                    "returnType": null,
                    "static": false,
                    "type": "ClassMethod",
                    "typeParameters": null,
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": [],
              "id": {
                "name": "C",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": [],
              "superClass": null,
              "superTypeParameters": null,
              "type": "ClassDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
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
        {
          "body": [
            {
              "body": {
                "body": [
                  {
                    "computed": false,
                    "key": {
                      "name": "constructor",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "constructor",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": {
                      "async": false,
                      "body": {
                        "body": [],
                        "type": "BlockStatement",
                      },
                      "expression": false,
                      "generator": false,
                      "id": null,
                      "params": [],
                      "predicate": null,
                      "returnType": null,
                      "type": "FunctionExpression",
                      "typeParameters": null,
                    },
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": [],
              "id": {
                "name": "C",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": [],
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [
                  {
                    "async": false,
                    "body": {
                      "body": [],
                      "directives": [],
                      "type": "BlockStatement",
                    },
                    "computed": false,
                    "generator": false,
                    "id": null,
                    "key": {
                      "name": "constructor",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "constructor",
                    "params": [],
                    "predicate": null,
                    "returnType": null,
                    "static": false,
                    "type": "ClassMethod",
                    "typeParameters": null,
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": [],
              "id": {
                "name": "C",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": [],
              "superClass": null,
              "superTypeParameters": null,
              "type": "ClassDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
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
        {
          "body": [
            {
              "body": {
                "body": [
                  {
                    "computed": false,
                    "key": {
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "get",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": {
                      "async": false,
                      "body": {
                        "body": [
                          {
                            "argument": {
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
                      "params": [],
                      "predicate": null,
                      "returnType": null,
                      "type": "FunctionExpression",
                      "typeParameters": null,
                    },
                  },
                  {
                    "computed": false,
                    "key": {
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "set",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": {
                      "async": false,
                      "body": {
                        "body": [],
                        "type": "BlockStatement",
                      },
                      "expression": false,
                      "generator": false,
                      "id": null,
                      "params": [
                        {
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
              "decorators": [],
              "id": {
                "name": "C",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": [],
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [
                  {
                    "async": false,
                    "body": {
                      "body": [
                        {
                          "argument": {
                            "type": "NumericLiteral",
                            "value": 1,
                          },
                          "type": "ReturnStatement",
                        },
                      ],
                      "directives": [],
                      "type": "BlockStatement",
                    },
                    "computed": false,
                    "generator": false,
                    "id": null,
                    "key": {
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "get",
                    "params": [],
                    "predicate": null,
                    "returnType": null,
                    "static": false,
                    "type": "ClassMethod",
                    "typeParameters": null,
                  },
                  {
                    "async": false,
                    "body": {
                      "body": [],
                      "directives": [],
                      "type": "BlockStatement",
                    },
                    "computed": false,
                    "generator": false,
                    "id": null,
                    "key": {
                      "name": "foo",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "set",
                    "params": [
                      {
                        "name": "v",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                    ],
                    "predicate": null,
                    "returnType": null,
                    "static": false,
                    "type": "ClassMethod",
                    "typeParameters": null,
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": [],
              "id": {
                "name": "C",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": [],
              "superClass": null,
              "superTypeParameters": null,
              "type": "ClassDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectBabelAlignment(testCase);
    });
  });
});
