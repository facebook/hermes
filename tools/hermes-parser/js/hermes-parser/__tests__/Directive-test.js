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

describe('Directive', () => {
  test('Top level directives', () => {
    const source = `\
'use strict';
'use strict';
Foo;`;

    // ESTree top level directive nodes
    expect(parse(source)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          loc: loc(1, 0, 1, 13),
          expression: {
            type: 'Literal',
            value: 'use strict',
            loc: loc(1, 0, 1, 12),
          },
          directive: 'use strict',
        },
        {
          type: 'ExpressionStatement',
          loc: loc(2, 0, 2, 13),
          expression: {
            type: 'Literal',
            value: 'use strict',
            loc: loc(2, 0, 2, 12),
          },
          directive: 'use strict',
        },
        {
          type: 'ExpressionStatement',
          loc: loc(3, 0, 3, 4),
          expression: {
            type: 'Identifier',
            name: 'Foo',
            loc: loc(3, 0, 3, 3),
          },
        },
      ],
    });

    // Babel top level directive nodes
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            loc: loc(3, 0, 3, 4),
            expression: {
              type: 'Identifier',
              name: 'Foo',
              loc: loc(3, 0, 3, 3),
            },
          },
        ],
        directives: [
          {
            type: 'Directive',
            loc: loc(1, 0, 1, 13),
            value: {
              type: 'DirectiveLiteral',
              loc: loc(1, 0, 1, 12),
              value: 'use strict',
            },
          },
          {
            type: 'Directive',
            loc: loc(2, 0, 2, 13),
            value: {
              type: 'DirectiveLiteral',
              loc: loc(2, 0, 2, 12),
              value: 'use strict',
            },
          },
        ],
      },
    });
  });

  test('Function body directive', () => {
    const source = `\
function test() {
  'use strict';
  Foo;
}
`;

    // ESTree directive node in function body
    expect(parse(source)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'FunctionDeclaration',
          body: {
            type: 'BlockStatement',
            body: [
              {
                type: 'ExpressionStatement',
                loc: loc(2, 2, 2, 15),
                expression: {
                  type: 'Literal',
                  value: 'use strict',
                  loc: loc(2, 2, 2, 14),
                },
                directive: 'use strict',
              },
              {
                type: 'ExpressionStatement',
                loc: loc(3, 2, 3, 6),
                expression: {
                  type: 'Identifier',
                  name: 'Foo',
                  loc: loc(3, 2, 3, 5),
                },
              },
            ],
          },
        },
      ],
    });

    // Babel directive node in function body
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'FunctionDeclaration',
            body: {
              type: 'BlockStatement',
              body: [
                {
                  type: 'ExpressionStatement',
                  loc: loc(3, 2, 3, 6),
                  expression: {
                    type: 'Identifier',
                    name: 'Foo',
                    loc: loc(3, 2, 3, 5),
                  },
                },
              ],
              directives: [
                {
                  type: 'Directive',
                  loc: loc(2, 2, 2, 15),
                  value: {
                    type: 'DirectiveLiteral',
                    loc: loc(2, 2, 2, 14),
                    value: 'use strict',
                  },
                },
              ],
            },
          },
        ],
      },
    });
  });
});
