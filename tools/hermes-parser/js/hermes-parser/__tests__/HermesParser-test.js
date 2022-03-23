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
    expect(() => parse('const = 1')).toThrowError(
      expect.objectContaining({
        loc: {
          line: 1,
          column: 6,
        },
      }),
    );
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

    expect(parse(scriptSource)).toMatchObject(scriptProgram);
    expect(parse(scriptSource, {babel: true})).toMatchObject({
      type: 'File',
      program: scriptProgram,
    });

    expect(parse(scriptSource, {sourceType: 'unambiguous'})).toMatchObject(
      scriptProgram,
    );
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
