/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {transform} from '../../src/transform/transform';
import * as t from '../../src/generated/node-types';

describe('transform', () => {
  it('should do nothing (including no formatting) if no mutations are applied', () => {
    const code = 'const x = 1'; // no semi to ensure no formatting
    const result = transform(code, () => ({}));

    expect(result).toBe(code);
  });

  it('should format the code as well as mutate it', () => {
    const code = 'const x = 1';
    const result = transform(code, context => ({
      VariableDeclaration(node) {
        if (node.type !== 'VariableDeclaration') {
          return;
        }

        context.replaceStatementWithMany(node, [
          t.VariableDeclaration({
            kind: 'const',
            declarations: [
              t.VariableDeclarator({
                id: t.Identifier({name: 'y'}),
                init: t.NullLiteral(),
              }),
            ],
          }),
        ]);
      },
    }));

    // note that it should have a semicolon
    expect(result).toBe(`\
const y = null;
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

  describe('replace', () => {
    describe('single', () => {
      it('expression', () => {
        const code = 'const x = 1;';
        const result = transform(code, context => ({
          Literal(node) {
            if (node.type !== 'Literal') {
              return;
            }

            context.replaceNode(node, t.BooleanLiteral({value: true}));
          },
        }));

        expect(result).toBe(`\
const x = true;
`);
      });

      it('statement', () => {
        const code = 'const x = 1;';
        const result = transform(code, context => ({
          VariableDeclaration(node) {
            if (node.type !== 'VariableDeclaration') {
              return;
            }

            context.replaceNode(
              node,
              t.VariableDeclaration({
                declarations: [
                  t.VariableDeclarator({
                    id: t.Identifier({name: 'y'}),
                    init: t.NullLiteral(),
                  }),
                ],
                kind: 'let',
              }),
            );
          },
        }));

        expect(result).toBe(`\
let y = null;
`);
      });

      it('type', () => {
        const code = 'const x: any = 1;';
        const result = transform(code, context => ({
          AnyTypeAnnotation(node) {
            if (node.type !== 'AnyTypeAnnotation') {
              return;
            }

            context.replaceNode(node, t.NumberTypeAnnotation());
          },
        }));

        expect(result).toBe(`\
const x: number = 1;
`);
      });
    });

    describe('with many', () => {
      it('works with array parents', () => {
        const code = 'const x = 1;';
        const result = transform(code, context => ({
          VariableDeclaration(node) {
            if (node.type !== 'VariableDeclaration') {
              return;
            }

            context.replaceStatementWithMany(node, [
              t.VariableDeclaration({
                kind: 'const',
                declarations: [
                  t.VariableDeclarator({
                    id: t.Identifier({name: 'y'}),
                    init: t.NullLiteral(),
                  }),
                ],
              }),
              t.VariableDeclaration({
                kind: 'const',
                declarations: [
                  t.VariableDeclarator({
                    id: t.Identifier({name: 'z'}),
                    init: t.BooleanLiteral({value: true}),
                  }),
                ],
              }),
            ]);
          },
        }));

        expect(result).toBe(`\
const y = null;
const z = true;
`);
      });

      it('wraps statements in a BlockStatement if they were in a bodyless parent', () => {
        const code = 'if (condition) return true;';
        const result = transform(code, context => ({
          ReturnStatement(node) {
            if (node.type !== 'ReturnStatement') {
              return;
            }

            context.replaceStatementWithMany(node, [
              t.VariableDeclaration({
                kind: 'const',
                declarations: [
                  t.VariableDeclarator({
                    id: t.Identifier({
                      name: 'y',
                    }),
                    init: t.NumericLiteral({
                      value: 1,
                    }),
                  }),
                ],
              }),
              t.VariableDeclaration({
                kind: 'const',
                declarations: [
                  t.VariableDeclarator({
                    id: t.Identifier({
                      name: 'z',
                    }),
                    init: t.NumericLiteral({
                      value: 2,
                    }),
                  }),
                ],
              }),
            ]);
          },
        }));

        expect(result).toBe(`\
if (condition) {
  const y = 1;
  const z = 2;
}
`);
      });
    });
  });
});
