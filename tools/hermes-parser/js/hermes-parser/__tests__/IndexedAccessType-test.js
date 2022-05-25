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

describe('IndexedAccessType', () => {
  describe('Basic Indexed Access Type', () => {
    const testCase: AlignmentCase = {
      code: `
        type T = O[k]
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token T',
      },
      // babel: {expectToFail: false},
      babel: {
        // TODO - once we update the babel version we test against - we can enable this
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token, expected "]"',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "id": Object {
                "name": "T",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "right": Object {
                "indexType": Object {
                  "id": Object {
                    "name": "k",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "objectType": Object {
                  "id": Object {
                    "name": "O",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "type": "IndexedAccessType",
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
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'TypeAlias',
              right: {
                type: 'AnyTypeAnnotation',
              },
              typeParameters: null,
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Optional Indexed Access Type', () => {
    const testCase: AlignmentCase = {
      code: `
        type T = O?.[k]
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token T',
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
                "name": "T",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "right": Object {
                "indexType": Object {
                  "id": Object {
                    "name": "k",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "objectType": Object {
                  "id": Object {
                    "name": "O",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "optional": true,
                "type": "OptionalIndexedAccessType",
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
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'TypeAlias',
              right: {
                type: 'AnyTypeAnnotation',
              },
              typeParameters: null,
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });
});
