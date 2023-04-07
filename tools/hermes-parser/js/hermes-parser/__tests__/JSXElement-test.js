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
    expect(parse(testCase.code)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          loc: {
            source: null,
            start: {line: 1, column: 0},
            end: {line: 1, column: 21},
          },
          range: [0, 21],
          expression: {
            type: 'JSXElement',
            loc: {
              source: null,
              start: {line: 1, column: 0},
              end: {line: 1, column: 21},
            },
            range: [0, 21],
            openingElement: {
              type: 'JSXOpeningElement',
              loc: {
                source: null,
                start: {line: 1, column: 0},
                end: {line: 1, column: 21},
              },
              range: [0, 21],
              name: {
                type: 'JSXIdentifier',
                loc: {
                  source: null,
                  start: {line: 1, column: 1},
                  end: {line: 1, column: 10},
                },
                range: [1, 10],
                name: 'Component',
              },
              attributes: [],
              selfClosing: true,
              typeArguments: {
                type: 'TypeParameterInstantiation',
                loc: {
                  source: null,
                  start: {line: 1, column: 10},
                  end: {line: 1, column: 18},
                },
                range: [10, 18],
                params: [
                  {
                    type: 'StringTypeAnnotation',
                    loc: {
                      source: null,
                      start: {line: 1, column: 11},
                      end: {line: 1, column: 17},
                    },
                    range: [11, 17],
                  },
                ],
              },
            },
            closingElement: null,
            children: [],
          },
          directive: null,
        },
      ],
    });
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    expectBabelAlignment(testCase);
  });
});
