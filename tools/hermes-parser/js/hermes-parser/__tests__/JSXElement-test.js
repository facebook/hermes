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

describe('JSXElement', () => {
  const testCase: AlignmentCase = {
    code: '<Component<string> />',
    espree: {
      expectToFail: 'espree-exception',
      expectedExceptionMessage: 'Unexpected token <',
    },
    babel: {
      expectToFail: 'babel-exception',
      expectedExceptionMessage: 'Unexpected token',
    },
  };
  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "directive": null,
            "expression": {
              "children": [],
              "closingElement": null,
              "openingElement": {
                "attributes": [],
                "name": {
                  "name": "Component",
                  "type": "JSXIdentifier",
                },
                "selfClosing": true,
                "type": "JSXOpeningElement",
                "typeArguments": {
                  "params": [
                    {
                      "type": "StringTypeAnnotation",
                    },
                  ],
                  "type": "TypeParameterInstantiation",
                },
              },
              "type": "JSXElement",
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    expect(parseForSnapshot(testCase.code, {babel: true}))
      .toMatchInlineSnapshot(`
      {
        "body": [
          {
            "directive": null,
            "expression": {
              "children": [],
              "closingElement": null,
              "openingElement": {
                "attributes": [],
                "name": {
                  "name": "Component",
                  "type": "JSXIdentifier",
                },
                "selfClosing": true,
                "type": "JSXOpeningElement",
              },
              "type": "JSXElement",
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    expectBabelAlignment(testCase);
  });
});
