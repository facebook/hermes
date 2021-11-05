/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Identifier} from 'hermes-estree';

import {transform} from '../../src/transform/transform';
import * as t from '../../src/generated/node-types';

describe('transform', () => {
  it('should do nothing (including no formatting) if no mutations are applied', () => {
    const code = 'const x = 1'; // no semi to ensure no formatting
    const result = transform(code, () => ({}));

    expect(result).toBe(code);
  });

  // TODO - turn this on when we have the replaceNode mutation
  it.skip('should format the code as well as mutate it', () => {
    const code = 'const x = 1'; // no semi to ensure no formatting
    const result = transform(code, context => ({
      Identifier(node) {
        if (node.type !== 'Identifier') {
          return;
        }

        context.replaceNode(
          node,
          t.Identifier({
            name: 'y',
          }),
        );
      },
    }));

    expect(result).toBe(`\
const y = 1;
`);
  });

  describe('insert', () => {
    it('works with the insertBeforeStatement mutation', () => {
      const code = 'const x = 1;';
      const result = transform(code, context => ({
        VariableDeclaration(node) {
          if (node.type !== 'VariableDeclaration') {
            return;
          }

          context.insertBeforeStatement(
            node,
            t.VariableDeclaration({
              kind: 'const',
              declarations: [
                t.VariableDeclarator({
                  id: t.Identifier({
                    name: 'y',
                  }),
                  init: t.NumericLiteral({
                    value: 1,
                    raw: '1',
                  }),
                }),
              ],
            }),
          );
        },
      }));

      expect(result).toBe(`\
const y = 1;
const x = 1;
`);
    });

    it('works with the insertAfterStatement mutation', () => {
      const code = 'const x = 1;';
      const result = transform(code, context => ({
        VariableDeclaration(node) {
          if (node.type !== 'VariableDeclaration') {
            return;
          }

          context.insertAfterStatement(
            node,
            t.VariableDeclaration({
              kind: 'const',
              declarations: [
                t.VariableDeclarator({
                  id: t.Identifier({
                    name: 'y',
                  }),
                  init: t.NumericLiteral({
                    value: 1,
                    raw: '1',
                  }),
                }),
              ],
            }),
          );
        },
      }));

      expect(result).toBe(`\
const x = 1;
const y = 1;
`);
    });

    it('wraps statements in a BlockStatement if they were in a bodyless parent', () => {
      const code = 'if (condition) return true;';
      const result = transform(code, context => ({
        ReturnStatement(node) {
          if (node.type !== 'ReturnStatement') {
            return;
          }

          context.insertBeforeStatement(
            node,
            t.VariableDeclaration({
              kind: 'const',
              declarations: [
                t.VariableDeclarator({
                  id: t.Identifier({
                    name: 'y',
                  }),
                  init: t.NumericLiteral({
                    value: 1,
                    raw: '1',
                  }),
                }),
              ],
            }),
          );
        },
      }));

      expect(result).toBe(`\
if (condition) {
  const y = 1;
  return true;
}
`);
    });
  });

  describe('remove', () => {
    it('works with the removeStatement mutation', () => {
      const code = 'const x = 1; console.log("I will survive");';
      const result = transform(code, context => ({
        VariableDeclaration(node) {
          if (node.type !== 'VariableDeclaration') {
            return;
          }

          context.removeStatement(node);
        },
      }));

      expect(result).toBe(`\
console.log("I will survive");
`);
    });

    it('wraps statements in a BlockStatement if they were in a bodyless parent', () => {
      const code = 'if (condition) return true;';
      const result = transform(code, context => ({
        ReturnStatement(node) {
          if (node.type !== 'ReturnStatement') {
            return;
          }

          context.removeStatement(node);
        },
      }));

      expect(result).toBe(`\
if (condition) {
}
`);
    });
  });
});
