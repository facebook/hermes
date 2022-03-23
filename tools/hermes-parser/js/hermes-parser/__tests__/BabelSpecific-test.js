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

describe('Babel-Specific Tests', () => {
  test('Babel root File node', () => {
    expect(parse('test', {babel: true})).toMatchObject({
      type: 'File',
      loc: loc(1, 0, 1, 4),
      start: 0,
      end: 4,
      program: {
        type: 'Program',
        loc: loc(1, 0, 1, 4),
        start: 0,
        end: 4,
        body: [
          {
            type: 'ExpressionStatement',
          },
        ],
        directives: [],
      },
      comments: [],
    });
  });

  test('Babel identifierName', () => {
    expect(parse('test', {babel: true})).toMatchObject({
      type: 'File',
      loc: loc(1, 0, 1, 4),
      start: 0,
      end: 4,
      program: {
        type: 'Program',
        loc: loc(1, 0, 1, 4),
        start: 0,
        end: 4,
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'Identifier',
              loc: {
                identifierName: 'test',
              },
            },
          },
        ],
        directives: [],
      },
      comments: [],
    });
  });
});
