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

describe('PropertyDefinition', () => {
  const testCase: AlignmentCase = {
    code: `
      class C {
        foo;
        bar = 1;
        static staticProp = 1;
      }
    `,
    espree: {
      expectToFail: false,
    },
    babel: {
      expectToFail: false,
    },
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "body": {
              "body": [
                {
                  "computed": false,
                  "declare": false,
                  "key": {
                    "name": "foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": false,
                  "tsModifiers": null,
                  "type": "PropertyDefinition",
                  "typeAnnotation": null,
                  "value": null,
                  "variance": null,
                },
                {
                  "computed": false,
                  "declare": false,
                  "key": {
                    "name": "bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": false,
                  "tsModifiers": null,
                  "type": "PropertyDefinition",
                  "typeAnnotation": null,
                  "value": {
                    "literalType": "numeric",
                    "raw": "1",
                    "type": "Literal",
                    "value": 1,
                  },
                  "variance": null,
                },
                {
                  "computed": false,
                  "declare": false,
                  "key": {
                    "name": "staticProp",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": true,
                  "tsModifiers": null,
                  "type": "PropertyDefinition",
                  "typeAnnotation": null,
                  "value": {
                    "literalType": "numeric",
                    "raw": "1",
                    "type": "Literal",
                    "value": 1,
                  },
                  "variance": null,
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
    expectBabelAlignment(testCase);
  });

  describe('With types', () => {
    describe('Property', () => {
      const testCase: AlignmentCase = {
        code: `
          class C {
            baz: F = 1;
          }
        `,
        espree: {
          // espree doesn't support types
          expectToFail: 'espree-exception',
          expectedExceptionMessage: 'Unexpected token :',
        },
        babel: {
          expectToFail: false,
        },
      };

      test('ESTree', () => {
        expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
          {
            "body": [
              {
                "body": {
                  "body": [
                    {
                      "computed": false,
                      "declare": false,
                      "key": {
                        "name": "baz",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "optional": false,
                      "static": false,
                      "tsModifiers": null,
                      "type": "PropertyDefinition",
                      "typeAnnotation": {
                        "type": "TypeAnnotation",
                        "typeAnnotation": {
                          "id": {
                            "name": "F",
                            "optional": false,
                            "type": "Identifier",
                            "typeAnnotation": null,
                          },
                          "type": "GenericTypeAnnotation",
                          "typeParameters": null,
                        },
                      },
                      "value": {
                        "literalType": "numeric",
                        "raw": "1",
                        "type": "Literal",
                        "value": 1,
                      },
                      "variance": null,
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
        expectBabelAlignment(testCase);
      });
    });
    describe('Declared Property', () => {
      const testCase: AlignmentCase = {
        code: `
          class C {
            declare bam: 1;
          }
        `,
        espree: {
          // espree doesn't support types
          expectToFail: 'espree-exception',
          expectedExceptionMessage: 'Unexpected token bam',
        },
        // babel: {expectToFail: false},
        babel: {
          // TODO - once we update the babel version we test against - we can enable this
          expectToFail: 'babel-exception',
          expectedExceptionMessage: 'Unexpected token',
        },
      };

      test('ESTree', () => {
        expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
          {
            "body": [
              {
                "body": {
                  "body": [
                    {
                      "computed": false,
                      "declare": true,
                      "key": {
                        "name": "bam",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "optional": false,
                      "static": false,
                      "tsModifiers": null,
                      "type": "PropertyDefinition",
                      "typeAnnotation": {
                        "type": "TypeAnnotation",
                        "typeAnnotation": {
                          "raw": "1",
                          "type": "NumberLiteralTypeAnnotation",
                          "value": 1,
                        },
                      },
                      "value": null,
                      "variance": null,
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
        expectBabelAlignment(testCase);
      });
    });
  });
});
