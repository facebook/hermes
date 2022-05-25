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
import {parse} from '../__test_utils__/parse';
import {loc} from '../__test_utils__/loc';

describe('TemplateLiteral', () => {
  const testCase: AlignmentCase = {
    code: `
      \`a \${b} c\`
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('ESTree', () => {
    // ESTree template literals with source locations
    expect(parse(testCase.code)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'TemplateLiteral',
            loc: loc(2, 6, 2, 16),
            range: [7, 17],
            quasis: [
              {
                type: 'TemplateElement',
                loc: loc(2, 6, 2, 11),
                range: [7, 12],
                tail: false,
                value: {
                  cooked: 'a ',
                  raw: 'a ',
                },
              },
              {
                type: 'TemplateElement',
                loc: loc(2, 12, 2, 16),
                range: [13, 17],
                tail: true,
                value: {
                  cooked: ' c',
                  raw: ' c',
                },
              },
            ],
            expressions: [
              {
                type: 'Identifier',
                loc: loc(2, 11, 2, 12),
                range: [12, 13],
                name: 'b',
              },
            ],
          },
        },
      ],
    });
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    // Babel template literals with source locations
    expect(parse(testCase.code, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'TemplateLiteral',
              loc: loc(2, 6, 2, 16),
              start: 7,
              end: 17,
              quasis: [
                {
                  type: 'TemplateElement',
                  loc: loc(2, 7, 2, 9),
                  start: 8,
                  end: 10,
                  tail: false,
                  value: {
                    cooked: 'a ',
                    raw: 'a ',
                  },
                },
                {
                  type: 'TemplateElement',
                  loc: loc(2, 13, 2, 15),
                  start: 14,
                  end: 16,
                  tail: true,
                  value: {
                    cooked: ' c',
                    raw: ' c',
                  },
                },
              ],
              expressions: [
                {
                  type: 'Identifier',
                  loc: loc(2, 11, 2, 12),
                  start: 12,
                  end: 13,
                  name: 'b',
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
