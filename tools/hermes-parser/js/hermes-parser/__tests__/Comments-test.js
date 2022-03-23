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

describe('Comments', () => {
  test('Parsing comments', () => {
    const source = '/*Block comment*/ 1; // Line comment';

    // ESTree comments in AST
    expect(parse(source)).toMatchObject({
      type: 'Program',
      comments: [
        {
          type: 'Block',
          value: 'Block comment',
          loc: {
            start: {
              line: 1,
              column: 0,
            },
            end: {
              line: 1,
              column: 17,
            },
          },
          range: [0, 17],
        },
        {
          type: 'Line',
          value: ' Line comment',
          loc: {
            start: {
              line: 1,
              column: 21,
            },
            end: {
              line: 1,
              column: 36,
            },
          },
          range: [21, 36],
        },
      ],
    });

    // Babel comments in AST
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
          },
        ],
      },
      comments: [
        {
          type: 'CommentBlock',
          value: 'Block comment',
        },
        {
          type: 'CommentLine',
          value: ' Line comment',
        },
      ],
    });
  });

  test('Interpreter directives', () => {
    // Parsing interpreter directive
    const sourceWithDirective = `#! interpreter comment
      1; /*block comment*/
    `;

    const interpreter = {
      type: 'InterpreterDirective',
      loc: loc(1, 0, 1, 22),
      value: ' interpreter comment',
    };

    expect(parse(sourceWithDirective)).toMatchObject({
      type: 'Program',
      interpreter,
      comments: [{type: 'Block', value: 'block comment'}],
    });
    expect(parse(sourceWithDirective, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        interpreter,
      },
      comments: [{type: 'CommentBlock', value: 'block comment'}],
    });

    // No interpreter directive
    const sourceWithoutDirective = '// Line comment';
    expect(parse(sourceWithoutDirective)).toMatchObject({
      type: 'Program',
      interpreter: null,
      comments: [{type: 'Line', value: ' Line comment'}],
    });
    expect(parse(sourceWithoutDirective, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        interpreter: null,
      },
      comments: [{type: 'CommentLine', value: ' Line comment'}],
    });
  });
});
