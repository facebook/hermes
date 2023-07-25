/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {
  expectBabelAlignment,
  expectEspreeAlignment,
} from '../__test_utils__/alignment-utils';
import {parseForSnapshot} from '../__test_utils__/parse';

const parserOpts = {enableExperimentalComponentSyntax: true};

describe('ComponentTypeAnnotation', () => {
  const testCase: AlignmentCase = {
    code: `
      type T = component();
    `,
    espree: {
      expectToFail: 'espree-exception',
      expectedExceptionMessage: 'Unexpected token T',
    },
    babel: {
      expectToFail: 'babel-exception',
      expectedExceptionMessage: 'Unexpected token',
    },
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code, parserOpts)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "id": {
              "name": "T",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "params": [],
              "rendersType": null,
              "rest": null,
              "type": "ComponentTypeAnnotation",
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
    expect(parseForSnapshot(testCase.code, {babel: true, ...parserOpts}))
      .toMatchInlineSnapshot(`
      {
        "body": [
          {
            "id": {
              "name": "T",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "AnyTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
    expectBabelAlignment(testCase);
  });
});
