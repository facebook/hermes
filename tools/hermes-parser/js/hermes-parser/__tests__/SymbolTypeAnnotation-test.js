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

describe('Symbol type annotation', () => {
  const testCase: AlignmentCase = {
    code: `
      type T = symbol
    `,
    espree: {
      expectToFail: 'espree-exception',
      expectedExceptionMessage: 'Unexpected token T',
    },
    babel: {expectToFail: false},
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
              "type": "SymbolTypeAnnotation",
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
              type: 'GenericTypeAnnotation',
              id: {
                type: 'Identifier',
                name: 'symbol',
              },
            },
            typeParameters: null,
          },
        ],
      },
    });
    expectBabelAlignment(testCase);
  });
});
