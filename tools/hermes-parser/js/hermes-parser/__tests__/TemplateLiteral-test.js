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

import {parse} from '../__test_utils__/parse';
import {loc} from '../__test_utils__/loc';

describe('TemplateLiteral', () => {
  const source = '`a ${b} c`';

  test('ESTree', () => {
    // ESTree template literals with source locations
    expect(parse(source)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'TemplateLiteral',
            loc: loc(1, 0, 1, 10),
            range: [0, 10],
            quasis: [
              {
                type: 'TemplateElement',
                loc: loc(1, 0, 1, 5),
                range: [0, 5],
                tail: false,
                value: {
                  cooked: 'a ',
                  raw: 'a ',
                },
              },
              {
                type: 'TemplateElement',
                loc: loc(1, 6, 1, 10),
                range: [6, 10],
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
                loc: loc(1, 5, 1, 6),
                range: [5, 6],
                name: 'b',
              },
            ],
          },
        },
      ],
    });
  });

  test('Babel', () => {
    // Babel template literals with source locations
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'TemplateLiteral',
              loc: loc(1, 0, 1, 10),
              start: 0,
              end: 10,
              quasis: [
                {
                  type: 'TemplateElement',
                  loc: loc(1, 1, 1, 3),
                  start: 1,
                  end: 3,
                  tail: false,
                  value: {
                    cooked: 'a ',
                    raw: 'a ',
                  },
                },
                {
                  type: 'TemplateElement',
                  loc: loc(1, 7, 1, 9),
                  start: 7,
                  end: 9,
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
                  loc: loc(1, 5, 1, 6),
                  start: 5,
                  end: 6,
                  name: 'b',
                },
              ],
            },
          },
        ],
      },
    });
  });
});
