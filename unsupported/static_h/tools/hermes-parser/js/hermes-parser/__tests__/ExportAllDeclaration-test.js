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

describe('ExportAllDeclaration', () => {
  describe('Unnamed', () => {
    const testCase: AlignmentCase = {
      code: `
        export * from 'z'
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "exportKind": "value",
              "exported": null,
              "source": Object {
                "literalType": "string",
                "raw": "'z'",
                "type": "Literal",
                "value": "z",
              },
              "type": "ExportAllDeclaration",
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
              type: 'ExportAllDeclaration',
              exportKind: 'value',
              source: {
                type: 'StringLiteral',
                value: 'z',
              },
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Renamed', () => {
    const testCase: AlignmentCase = {
      code: `
        export * as y from 'z'
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "exportKind": "value",
              "exported": Object {
                "name": "y",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "source": Object {
                "literalType": "string",
                "raw": "'z'",
                "type": "Literal",
                "value": "z",
              },
              "type": "ExportAllDeclaration",
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
              type: 'ExportNamedDeclaration',
              exportKind: 'value',
              specifiers: [
                {
                  type: 'ExportNamespaceSpecifier',
                  exported: {
                    type: 'Identifier',
                    name: 'y',
                  },
                },
              ],
              source: {
                type: 'StringLiteral',
                value: 'z',
              },
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });
});
