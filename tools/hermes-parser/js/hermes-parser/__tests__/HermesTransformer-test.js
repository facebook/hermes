/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

import {parse, types, transformFromAstSync} from 'hermes-parser';

describe('Traversal', () => {
  test('Replacement', () => {
    const source = '(foo());';
    let called = false;
    const plugin = ({types: t}) => {
      return {
        visitor: {
          CallExpression(path, state) {
            expect(t).toBe(types);
            expect(path.get('callee').node.name).toEqual('foo');
            path.get('callee').replaceWith(t.Identifier('bar'));
            called = true;
          },
        },
      };
    };
    const {ast} = transformFromAstSync(parse(source, {babel: true}), source, {
      plugins: [plugin],
    });
    expect(ast).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'CallExpression',
              callee: {
                type: 'Identifier',
                name: 'bar',
              },
            },
          },
        ],
      },
    });
    expect(called).toBe(true);
  });

  test('Accepts options', () => {
    const source = '(foo());';
    let called = false;

    const ast = parse(source, {babel: true});
    transformFromAstSync(ast, source, {
      plugins: [
        [
          {
            visitor: {
              CallExpression(path, state) {
                expect(state.file.ast).toBe(ast);
                expect(state.file.code).toBe(source);
                expect(state.opts.someRandomKey).toBe(true);
                called = true;
              },
            },
          },
          {someRandomKey: true},
        ],
      ],
    });

    expect(called).toBe(true);
  });

  test('Scope information works', () => {
    const source = 'function A() { B; } A();';
    let called = false;

    const ast = parse(source, {babel: true});
    transformFromAstSync(ast, source, {
      plugins: [
        {
          visitor: {
            CallExpression(path, state) {
              const calleePath = path.get('callee');
              const {name, loc} = calleePath.node;

              const binding = path.scope.getBinding(name);
              expect(binding.path.node.type).toBe('FunctionDeclaration');
              expect(binding.references).toBe(1);
              expect(binding.referencePaths[0].node.loc).toEqual(loc);
              called = true;
            },
          },
        },
      ],
    });

    expect(called).toBe(true);
  });
});
