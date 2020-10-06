/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

jest.disableAutomock();

const {parse} = require('../build/index');

/**
 * Utility for quickly creating source locations inline.
 */
function loc(startLine, startColumn, endLine, endColumn) {
  return {
    start: {
      line: startLine,
      column: startColumn,
    },
    end: {
      line: endLine,
      column: endColumn,
    },
  };
}

test('Can parse simple file', () => {
  expect(parse('const x = 1')).toMatchObject({
    type: 'Program',
    body: [
      expect.objectContaining({
        type: 'VariableDeclaration',
        kind: 'const',
        declarations: [
          expect.objectContaining({
            type: 'VariableDeclarator',
            id: expect.objectContaining({
              type: 'Identifier',
              name: 'x',
            }),
            init: expect.objectContaining({
              type: 'NumericLiteral',
              value: 1,
            }),
          }),
        ],
      }),
    ],
  });
});

test('Parse error is thrown', () => {
  expect(() => parse('const = 1')).toThrow(
    `1:6: 'identifier' expected in declaration
const = 1
~~~~~~^
`,
  );
});

test('Parse error does not include caret line for non-ASCII characters', () => {
  expect(() => parse('/*\u0176*/ const = 1')).toThrow(
    `1:13: 'identifier' expected in declaration
/*\u0176*/ const = 1
`,
  );
});

test('Parsing comments', () => {
  expect(parse('/*Block comment*/ 1; // Line comment')).toMatchObject({
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
});

test('Source locations', () => {
  // ESTree source locations include range
  expect(parse('Foo')).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        loc: {
          start: {
            line: 1,
            column: 0,
          },
          end: {
            line: 1,
            column: 3,
          },
        },
        range: [0, 3],
      },
    ],
  });

  // Babel source locations include start/end properties
  expect(parse('Foo', {babel: true})).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        loc: {
          start: {
            line: 1,
            column: 0,
          },
          end: {
            line: 1,
            column: 3,
          },
        },
        start: 0,
        end: 3,
      },
    ],
  });
});

test('Top level directives', () => {
  const source = `'use strict';
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
          type: 'StringLiteral',
          value: 'use strict',
          loc: loc(1, 0, 1, 12),
        },
        directive: 'use strict',
      },
      {
        type: 'ExpressionStatement',
        loc: loc(2, 0, 2, 13),
        expression: {
          type: 'StringLiteral',
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
  });
});

test('Function body directive', () => {
  const source = `function test() {
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
              loc: loc(2, 4, 2, 17),
              expression: {
                type: 'StringLiteral',
                value: 'use strict',
                loc: loc(2, 4, 2, 16),
              },
              directive: 'use strict',
            },
            {
              type: 'ExpressionStatement',
              loc: loc(3, 4, 3, 8),
              expression: {
                type: 'Identifier',
                name: 'Foo',
                loc: loc(3, 4, 3, 7),
              },
            },
          ],
        },
      },
    ],
  });

  // Babel directive node in function body
  expect(parse(source, {babel: true})).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'FunctionDeclaration',
        body: {
          type: 'BlockStatement',
          body: [
            {
              type: 'ExpressionStatement',
              loc: loc(3, 4, 3, 8),
              expression: {
                type: 'Identifier',
                name: 'Foo',
                loc: loc(3, 4, 3, 7),
              },
            },
          ],
          directives: [
            {
              type: 'Directive',
              loc: loc(2, 4, 2, 17),
              value: {
                type: 'DirectiveLiteral',
                loc: loc(2, 4, 2, 16),
                value: 'use strict',
              },
            },
          ],
        },
      },
    ],
  });
});
