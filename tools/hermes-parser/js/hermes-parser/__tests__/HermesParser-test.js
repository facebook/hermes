/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const {parse} = require('hermes-parser');

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

function parseAsFlow(source, options = {}) {
  options.flow = 'all';
  return parse(source, options);
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
              type: 'Literal',
              value: 1,
            }),
          }),
        ],
      }),
    ],
  });
});

describe('Parse errors', () => {
  test('Basic', () => {
    expect(() => parse('const = 1')).toThrow(
      new SyntaxError(
        `'identifier' expected in declaration (1:6)
const = 1
~~~~~~^`,
      ),
    );
  });

  test('Has error location', () => {
    try {
      parse('const = 1');
      fail('Expected parse error to be thrown');
    } catch (e) {
      expect(e.loc).toMatchObject({
        line: 1,
        column: 6,
      });
    }
  });

  test('Source line with non-ASCII characters', () => {
    // Parse error does not include caret line for non-ASCII characters
    expect(() => parse('/*\u0176*/ const = 1')).toThrow(
      new SyntaxError(
        `'identifier' expected in declaration (1:13)
/*\u0176*/ const = 1`,
      ),
    );
  });

  test('Error with notes', () => {
    // Parse error with additional notes
    const source = `class C {
  constructor() { 1 }
  constructor() { 2 }
}`;
    expect(() => parse(source)).toThrow(
      new SyntaxError(
        `duplicate constructors in class (3:2)
  constructor() { 2 }
  ^~~~~~~~~~~~~~~~~~~
note: first constructor definition (2:2)
  constructor() { 1 }
  ^~~~~~~~~~~~~~~~~~~`,
      ),
    );
  });
});

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

test('Source locations', () => {
  // ESTree source locations include range
  expect(parse('Foo')).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        loc: {
          source: null,
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
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          loc: {
            source: null,
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
    },
  });

  // Filename is included in source location if provided
  expect(parse('Foo', {sourceFilename: 'FooTest.js'})).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        loc: {
          source: 'FooTest.js',
          start: {
            line: 1,
            column: 0,
          },
          end: {
            line: 1,
            column: 3,
          },
        },
      },
    ],
  });

  // Code points that will be encoded as 1, 2, 3, and 4 byte UTF-8 characters
  // within Hermes are translated back to UTF-16 code unit source locations.
  const unicode = `'foo1';
'foo\u00a7';
'foo\u2014';
'foo\ud83d\udea6';
`;
  expect(parse(unicode)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          loc: loc(1, 0, 1, 6),
          range: [0, 6],
          value: 'foo1',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          loc: loc(2, 0, 2, 6),
          range: [8, 14],
          value: 'foo\u00a7',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          loc: loc(3, 0, 3, 6),
          range: [16, 22],
          value: 'foo\u2014',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          loc: loc(4, 0, 4, 7),
          range: [24, 31],
          value: 'foo\ud83d\udea6',
        },
      },
    ],
  });
});

test('Program source locations', () => {
  const source = `
A;
/*comment*/`;

  // ESTree program location only includes program body
  expect(parse(source)).toMatchObject({
    type: 'Program',
    loc: loc(2, 0, 2, 2),
    range: [1, 3],
  });

  // Babel program location starts at beginning of source and includes trailing comments
  expect(parse(source, {babel: true})).toMatchObject({
    type: 'File',
    loc: loc(1, 0, 3, 11),
    start: 0,
    end: 15,
    program: {
      type: 'Program',
      loc: loc(1, 0, 3, 11),
      start: 0,
      end: 15,
    },
  });
});

describe('Program source type', () => {
  const moduleProgram = {
    type: 'Program',
    sourceType: 'module',
  };
  const scriptProgram = {
    type: 'Program',
    sourceType: 'script',
  };

  test('hardcoded', () => {
    expect(parse('Foo', {sourceType: 'module'})).toMatchObject(moduleProgram);
    expect(parse('Foo', {sourceType: 'script'})).toMatchObject(scriptProgram);
  });

  test('detect module type', () => {
    // Verify that every value import or export results in module source type
    const moduleSources = [
      `import Foo from 'foo'`,
      `export default 1`,
      `export const foo = 1`,
      `export * from 'foo'`,
    ];

    for (const moduleSource of moduleSources) {
      expect(parse(moduleSource)).toMatchObject(moduleProgram);
      expect(parse(moduleSource, {babel: true})).toMatchObject({
        type: 'File',
        program: moduleProgram,
      });

      expect(parse(moduleSource, {sourceType: 'unambiguous'})).toMatchObject(
        moduleProgram,
      );
    }
  });

  test('detect script type', () => {
    // Verify that type imports and exports do not result in module source type
    const scriptSource = `
      import type Foo from 'foo';
      export type T = any;
      export type * from 'foo';
    `;

    expect(parseAsFlow(scriptSource)).toMatchObject(scriptProgram);
    expect(parseAsFlow(scriptSource, {babel: true})).toMatchObject({
      type: 'File',
      program: scriptProgram,
    });

    expect(parse(scriptSource, {sourceType: 'unambiguous'})).toMatchObject(
      scriptProgram,
    );
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
                type: 'Literal',
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
    },
  });
});

test('Literals', () => {
  const source = `
    null;
    10;
    0.56283;
    "test";
    true;
    /foo/g;
  `;

  // ESTree AST literal nodes
  expect(parse(source)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: null,
          raw: 'null',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: 10,
          raw: '10',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: 0.56283,
          raw: '0.56283',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: 'test',
          raw: '"test"',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: true,
          raw: 'true',
        },
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: new RegExp('foo', 'g'),
          raw: '/foo/g',
          regex: {
            pattern: 'foo',
            flags: 'g',
          },
        },
      },
    ],
  });

  // ESTree AST with invalid RegExp literal
  expect(parse('/foo/qq')).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: null,
          regex: {
            pattern: 'foo',
            flags: 'qq',
          },
        },
      },
    ],
  });

  // Babel AST literal nodes
  expect(parse(source, {babel: true})).toMatchObject({
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'NullLiteral',
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'NumericLiteral',
            value: 10,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'NumericLiteral',
            value: 0.56283,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'StringLiteral',
            value: 'test',
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'BooleanLiteral',
            value: true,
          },
        },
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'RegExpLiteral',
            pattern: 'foo',
            flags: 'g',
          },
        },
      ],
    },
  });
});

test('Arrays', () => {
  const source = 'const [a,,b] = [1,,2];';

  // ESTree AST array nodes
  expect(parse(source)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'VariableDeclaration',
        declarations: [
          {
            type: 'VariableDeclarator',
            id: {
              type: 'ArrayPattern',
              elements: [
                {
                  type: 'Identifier',
                  name: 'a',
                },
                null,
                {
                  type: 'Identifier',
                  name: 'b',
                },
              ],
            },
            init: {
              type: 'ArrayExpression',
              elements: [
                {
                  type: 'Literal',
                  value: 1,
                },
                null,
                {
                  type: 'Literal',
                  value: 2,
                },
              ],
            },
          },
        ],
      },
    ],
  });

  // Babel AST array nodes
  expect(parse(source, {babel: true})).toMatchObject({
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'VariableDeclaration',
          declarations: [
            {
              type: 'VariableDeclarator',
              id: {
                type: 'ArrayPattern',
                elements: [
                  {
                    type: 'Identifier',
                    name: 'a',
                  },
                  null,
                  {
                    type: 'Identifier',
                    name: 'b',
                  },
                ],
              },
              init: {
                type: 'ArrayExpression',
                elements: [
                  {
                    type: 'NumericLiteral',
                    value: 1,
                  },
                  null,
                  {
                    type: 'NumericLiteral',
                    value: 2,
                  },
                ],
              },
            },
          ],
        },
      ],
    },
  });
});

test('Template literals', () => {
  const source = '`a ${b} c`';

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

test('This type annotation', () => {
  const source = `
    type t1 = this;
    type t2 = this<T>;
    type t3 = T.this;
    type t4 = this.T;
  `;

  const thisAlias = {
    type: 'TypeAlias',
    right: {
      type: 'ThisTypeAnnotation',
    },
  };
  const genericAlias = {
    type: 'TypeAlias',
    right: {
      type: 'GenericTypeAnnotation',
    },
  };
  const expectedProgram = {
    type: 'Program',
    body: [thisAlias, genericAlias, genericAlias, genericAlias],
  };

  expect(parseAsFlow(source)).toMatchObject(expectedProgram);
  expect(parseAsFlow(source, {babel: true})).toMatchObject({
    type: 'File',
    program: expectedProgram,
  });
});

test('Method definitions', () => {
  const source = `
    class C {
      foo() {}
    }
  `;

  // ESTree AST contains MethodDefinition containing a FunctionExpression value
  expect(parse(source)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ClassDeclaration',
        body: {
          type: 'ClassBody',
          body: [
            {
              type: 'MethodDefinition',
              key: {
                type: 'Identifier',
                name: 'foo',
              },
              value: {
                type: 'FunctionExpression',
                id: null,
                params: [],
                body: {
                  type: 'BlockStatement',
                  body: [],
                },
                async: false,
                generator: false,
                returnType: null,
                typeParameters: null,
                predicate: null,
              },
            },
          ],
        },
      },
    ],
  });

  // Babel AST has ClassMethod containing all properties of FunctionExpression
  expect(parse(source, {babel: true})).toMatchObject({
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ClassDeclaration',
          body: {
            type: 'ClassBody',
            body: [
              {
                type: 'ClassMethod',
                key: {
                  type: 'Identifier',
                  name: 'foo',
                },
                id: null,
                params: [],
                body: {
                  type: 'BlockStatement',
                  body: [],
                },
                async: false,
                generator: false,
                returnType: null,
                typeParameters: null,
                predicate: null,
              },
            ],
          },
        },
      ],
    },
  });
});

test('Object properties', () => {
  const source = `
    ({
      prop1: 1,
      prop2: function() {},
      prop3() {},
      get prop4() {},
      set prop5(x) {},
    })
  `;

  expect(parse(source, {babel: true})).toMatchObject({
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'ObjectExpression',
            properties: [
              {
                type: 'ObjectProperty',
                key: {
                  type: 'Identifier',
                  name: 'prop1',
                },
                value: {
                  type: 'NumericLiteral',
                  value: 1,
                },
                computed: false,
                shorthand: false,
              },
              {
                type: 'ObjectProperty',
                key: {
                  type: 'Identifier',
                  name: 'prop2',
                },
                value: {
                  type: 'FunctionExpression',
                },
                computed: false,
                shorthand: false,
              },
              {
                type: 'ObjectMethod',
                key: {
                  type: 'Identifier',
                  name: 'prop3',
                },
                kind: 'method',
                id: null,
                params: [],
                body: {
                  type: 'BlockStatement',
                  body: [],
                },
                async: false,
                generator: false,
                returnType: null,
                typeParameters: null,
                predicate: null,
              },
              {
                type: 'ObjectMethod',
                key: {
                  type: 'Identifier',
                  name: 'prop4',
                },
                kind: 'get',
                id: null,
                params: [],
                body: {
                  type: 'BlockStatement',
                  body: [],
                },
                async: false,
                generator: false,
                returnType: null,
                typeParameters: null,
                predicate: null,
              },
              {
                type: 'ObjectMethod',
                key: {
                  type: 'Identifier',
                  name: 'prop5',
                },
                kind: 'set',
                id: null,
                params: [
                  {
                    type: 'Identifier',
                    name: 'x',
                  },
                ],
                body: {
                  type: 'BlockStatement',
                  body: [],
                },
                async: false,
                generator: false,
                returnType: null,
                typeParameters: null,
                predicate: null,
              },
            ],
          },
        },
      ],
    },
  });
});

test('Rest element', () => {
  const source = `
    function test1(...rest: string) {}
    function test2([...rest: string]) {}
  `;

  expect(parseAsFlow(source, {babel: true})).toMatchObject({
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'FunctionDeclaration',
          params: [
            {
              type: 'RestElement',
              argument: {
                type: 'Identifier',
                name: 'rest',
                typeAnnotation: null,
              },
              typeAnnotation: {type: 'TypeAnnotation'},
            },
          ],
        },
        {
          type: 'FunctionDeclaration',
          params: [
            {
              type: 'ArrayPattern',
              elements: [
                {
                  type: 'RestElement',
                  argument: {
                    type: 'Identifier',
                    name: 'rest',
                    typeAnnotation: null,
                  },
                  typeAnnotation: {type: 'TypeAnnotation'},
                },
              ],
            },
          ],
        },
      ],
    },
  });
});

test('Import expression', () => {
  const source = `import('foo')`;
  expect(parse(source)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'ImportExpression',
          source: {
            type: 'Literal',
            value: 'foo',
          },
        },
      },
    ],
  });

  // Babel converts ImportExpression to CallExpression with Import callee
  expect(parse(source, {babel: true})).toMatchObject({
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'CallExpression',
            callee: {
              type: 'Import',
            },
            arguments: [{type: 'StringLiteral', value: 'foo'}],
          },
        },
      ],
    },
  });
});

describe('Import declaration', () => {
  test('Value importKind converted to null', () => {
    const source = `import {Foo, type Bar, typeof Baz} from 'Foo'`;
    const program = {
      type: 'Program',
      body: [
        {
          type: 'ImportDeclaration',
          importKind: 'value',
          specifiers: [
            {
              type: 'ImportSpecifier',
              local: {
                type: 'Identifier',
                name: 'Foo',
              },
              importKind: null,
            },
            {
              type: 'ImportSpecifier',
              local: {
                type: 'Identifier',
                name: 'Bar',
              },
              importKind: 'type',
            },
            {
              type: 'ImportSpecifier',
              local: {
                type: 'Identifier',
                name: 'Baz',
              },
              importKind: 'typeof',
            },
          ],
        },
      ],
    };

    expect(parse(source)).toMatchObject(program);
    expect(parse(source, {babel: true})).toMatchObject({type: 'File', program});
  });
});

test('Unicode strings and identifiers', () => {
  const source = `
    // Null byte in middle of string
    'foo \0 bar';
    // Unicode directly in text
    const 𩸽 = '𩸽';
    // Unicode literals
    const a\u033f = '\u033f';
    // Surrogate pairs (invalid individual UTF-8 code points)
    '\ud83d\udea6';
  `;

  expect(parse(source)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: 'foo \0 bar',
        },
      },
      {
        type: 'VariableDeclaration',
        declarations: [
          {
            type: 'VariableDeclarator',
            id: {
              type: 'Identifier',
              name: '𩸽',
            },
            init: {
              type: 'Literal',
              value: '𩸽',
            },
          },
        ],
      },
      {
        type: 'VariableDeclaration',
        declarations: [
          {
            type: 'VariableDeclarator',
            id: {
              type: 'Identifier',
              name: 'a\u033f',
            },
            init: {
              type: 'Literal',
              value: '\u033f',
            },
          },
        ],
      },
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: '\ud83d\udea6',
        },
      },
    ],
  });
});

test('Symbol type annotation', () => {
  expect(parse(`type T = symbol`, {babel: true})).toMatchObject({
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
});

test('Semantic validation', () => {
  // Semantic validator catches errors
  expect(() => parse(`return 1;`)).toThrow(
    new SyntaxError(
      `'return' not in a function (1:0)
return 1;
^~~~~~~~~`,
    ),
  );

  // But invalid regexps are not reported
  expect(parse(`/(((/;`)).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ExpressionStatement',
        expression: {
          type: 'Literal',
          value: null,
          regex: {
            pattern: '(((',
            flags: '',
          },
        },
      },
    ],
  });
});

test('Private properties', () => {
  // Private property uses are not supported
  expect(() => parse(`foo.#private`)).toThrow(
    new SyntaxError('Private properties are not supported (1:4)'),
  );

  // Private property definitions are not supported
  expect(() => parse(`class C { #private }`)).toThrow(
    new SyntaxError('Private properties are not supported (1:10)'),
  );
});

test('Flow pragma detection', () => {
  const parsedAsFlow = {
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'CallExpression',
          },
        },
      ],
    },
  };

  const notParsedAsFlow = {
    type: 'File',
    program: {
      type: 'Program',
      body: [
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'BinaryExpression',
          },
        },
      ],
    },
  };

  // In the presence of a Flow pragma, ambiguous expressions are parsed as Flow
  const withFlowPragma = `
    // @flow
    foo<T>(x);
  `;
  expect(parse(withFlowPragma, {babel: true})).toMatchObject(parsedAsFlow);
  expect(parse(withFlowPragma, {babel: true, flow: 'all'})).toMatchObject(
    parsedAsFlow,
  );

  // Without a Flow pragma present, ambiguous expressions are not parsed as Flow
  // by default, but are parsed as flow in `flow: all` mode.
  const withoutFlowPragma = `
    foo<T>(x);
  `;
  expect(parse(withoutFlowPragma, {babel: true})).toMatchObject(
    notParsedAsFlow,
  );
  expect(parse(withoutFlowPragma, {babel: true, flow: 'all'})).toMatchObject(
    parsedAsFlow,
  );

  // Flow pragma can appear after directives
  const flowPragmaAfterDirective = `
    'use strict';
    /* @flow */
    foo<T>(x);
  `;
  expect(parse(flowPragmaAfterDirective, {babel: true})).toMatchObject(
    parsedAsFlow,
  );

  // Flow pragma must appear before the first statement, so ambiguous expression
  // is not parsed as Flow type syntax.
  const flowPragmaAfterStatement = `
    foo<T>(x);
    // @flow
  `;
  expect(parse(flowPragmaAfterStatement, {babel: true})).toMatchObject(
    notParsedAsFlow,
  );

  // @flow must be followed by a word boundary for it to count as a Flow pragma
  const malformedFlowPragmas = `
    // @flo
    // @floww
    // @flow1
    // @flow_strict
    foo<T>(x);
  `;
  expect(parse(malformedFlowPragmas, {babel: true})).toMatchObject(
    notParsedAsFlow,
  );
});

test('Tokens', () => {
  const source = `var foo = 'str'; 1 === true`;
  expect(parse(source, {tokens: true})).toMatchObject({
    type: 'Program',
    tokens: [
      {
        type: 'Keyword',
        loc: loc(1, 0, 1, 3),
        value: 'var',
      },
      {
        type: 'Identifier',
        loc: loc(1, 4, 1, 7),
        value: 'foo',
      },
      {
        type: 'Punctuator',
        loc: loc(1, 8, 1, 9),
        value: '=',
      },
      {
        type: 'String',
        loc: loc(1, 10, 1, 15),
        value: "'str'",
      },
      {
        type: 'Punctuator',
        loc: loc(1, 15, 1, 16),
        value: ';',
      },
      {
        type: 'Numeric',
        loc: loc(1, 17, 1, 18),
        value: '1',
      },
      {
        type: 'Punctuator',
        loc: loc(1, 19, 1, 22),
        value: '===',
      },
      {
        type: 'Boolean',
        loc: loc(1, 23, 1, 27),
        value: 'true',
      },
    ],
  });
});

test('Allow return outside function', () => {
  expect(() => parse('return 1')).toThrow(
    new SyntaxError(
      `'return' not in a function (1:0)
return 1
^~~~~~~~`,
    ),
  );

  expect(parse('return 1', {allowReturnOutsideFunction: true})).toMatchObject({
    type: 'Program',
    body: [
      {
        type: 'ReturnStatement',
        argument: {
          type: 'Literal',
          value: 1,
        },
      },
    ],
  });
});

describe('This type annotations', () => {
  test('Removed in Babel mode', () => {
    const params = [
      {
        type: 'Identifier',
        name: 'param',
        typeAnnotation: {
          typeAnnotation: {type: 'NumberTypeAnnotation'},
        },
      },
    ];

    expect(
      parse(
        `
          function f1(this: string, param: number) {}
          (function f2(this: string, param: number) {});
        `,
        {babel: true},
      ),
    ).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'FunctionDeclaration',
            id: {name: 'f1'},
            params,
          },
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'FunctionExpression',
              id: {name: 'f2'},
              params,
            },
          },
        ],
      },
    });
  });

  test('Preserved in ESTree mode', () => {
    const params = [
      {
        type: 'Identifier',
        name: 'this',
        typeAnnotation: {
          typeAnnotation: {type: 'StringTypeAnnotation'},
        },
      },
      {
        type: 'Identifier',
        name: 'param',
        typeAnnotation: {
          typeAnnotation: {type: 'NumberTypeAnnotation'},
        },
      },
    ];

    expect(
      parse(
        `
          function f1(this: string, param: number) {}
          (function f2(this: string, param: number) {});
        `,
      ),
    ).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'FunctionDeclaration',
          id: {name: 'f1'},
          params,
        },
        {
          type: 'ExpressionStatement',
          expression: {
            type: 'FunctionExpression',
            id: {name: 'f2'},
            params,
          },
        },
      ],
    });
  });
});

describe('Indexed Access Type annotations in Babel', () => {
  test('Basic Indexed Access Type', () => {
    expect(parse(`type T = O[k]`, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'TypeAlias',
            right: {
              type: 'AnyTypeAnnotation',
            },
            typeParameters: null,
          },
        ],
      },
    });
  });

  test('Optional Indexed Access Type', () => {
    expect(parse(`type T = O?.[k]`, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'TypeAlias',
            right: {
              type: 'AnyTypeAnnotation',
            },
            typeParameters: null,
          },
        ],
      },
    });
  });
});
