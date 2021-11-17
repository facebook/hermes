/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {transform as transformOriginal} from '../../src/transform/transform';
import * as t from '../../src/generated/node-types';
// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../../.prettierrc.json';

function transform(code, visitors) {
  return transformOriginal(code, visitors, prettierConfig);
}

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
          context.removeStatement(node);
        },
      }));

      expect(result).toBe(`\
console.log('I will survive');
`);
    });

    it('wraps statements in a BlockStatement if they were in a bodyless parent', () => {
      const code = 'if (condition) return true;';
      const result = transform(code, context => ({
        ReturnStatement(node) {
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

  describe('complex transforms', () => {
    it('should support transforms on the same subtree', () => {
      const code = `\
class Foo {
  method(): () => () => Foo {
    function bar(): () => Foo {
      function baz(): Foo {
        return this;
      }
      return baz;
    }
    return bar;
  }
}
      `;

      // transform which replaces all function declarations with arrow functions
      const result = transform(code, context => ({
        'FunctionDeclaration[id]'(node) {
          context.replaceNode(
            node,
            t.VariableDeclaration({
              kind: 'const',
              declarations: [
                t.VariableDeclarator({
                  id: context.shallowCloneNode(node.id),
                  init: t.ArrowFunctionExpression({
                    async: node.async,
                    body: context.shallowCloneNode(node.body),
                    expression: false,
                    params: context.shallowCloneArray(node.params),
                    predicate: context.shallowCloneNode(node.predicate),
                    returnType: context.shallowCloneNode(node.returnType),
                    typeParameters: context.shallowCloneNode(
                      node.typeParameters,
                    ),
                  }),
                }),
              ],
            }),
          );
        },
      }));

      expect(result).toBe(`\
class Foo {
  method(): () => () => Foo {
    const bar = (): (() => Foo) => {
      const baz = (): Foo => {
        return this;
      };
      return baz;
    };
    return bar;
  }
}
`);
    });

    it('should fail if you attempt to insert before a removed node', () => {
      const code = `\
if (true) call();
      `;

      expect(() =>
        transform(code, context => ({
          ExpressionStatement(node) {
            context.replaceNode(
              node,
              t.ExpressionStatement({
                expression: t.StringLiteral({
                  value: 'removed',
                }),
              }),
            );

            context.insertBeforeStatement(
              node,
              t.ExpressionStatement({
                expression: t.StringLiteral({
                  value: 'inserted',
                }),
              }),
            );
          },
        })),
      ).toThrowErrorMatchingInlineSnapshot(
        `"Expected to find the target \\"ExpressionStatement\\" on the \\"IfStatement.alternate\\", but found a different node. This likely means that you attempted to mutate around the target after it was deleted/replaced."`,
      );
    });

    it('should allow insertion before removal', () => {
      const code = `\
if (true) call();
      `;

      const result = transform(code, context => ({
        ExpressionStatement(node) {
          context.insertBeforeStatement(
            node,
            t.ExpressionStatement({
              expression: t.StringLiteral({
                value: 'inserted',
              }),
            }),
          );

          context.replaceNode(
            node,
            t.ExpressionStatement({
              expression: t.StringLiteral({
                value: 'removed',
              }),
            }),
          );
        },
      }));

      expect(result).toBe(`\
if (true) {
  ('inserted');
  ('removed');
}
`);
    });
  });

  describe('comments', () => {
    describe('attachment', () => {
      it('should attach comments so they are maintained during an insertion', () => {
        const code = `
// leading comment
statement(); // inline comment
`;

        const result = transform(code, context => ({
          ExpressionStatement(node) {
            context.insertBeforeStatement(
              node,
              t.ExpressionStatement({
                expression: t.StringLiteral({
                  value: 'before',
                }),
              }),
            );
            context.insertAfterStatement(
              node,
              t.ExpressionStatement({
                expression: t.StringLiteral({
                  value: 'after',
                }),
              }),
            );
          },
        }));

        expect(result).toBe(`\
('before');
// leading comment
statement(); // inline comment
('after');
`);
      });

      it('should attach comments so they are removed when the associated node is removed', () => {
        const code = `
// this should remain leading #1
const x = 1; // this should remain inline #1
// leading comment to be deleted
statement(); // inline comment to be deleted
// this should remain leading #2
const y = 1; // this should remain inline #2
`;

        const result = transform(code, context => ({
          ExpressionStatement(node) {
            context.removeStatement(node);
          },
        }));

        expect(result).toBe(`\
// this should remain leading #1
const x = 1; // this should remain inline #1
// this should remain leading #2
const y = 1; // this should remain inline #2
`);
      });

      it('should clone comments when nodes are cloned', () => {
        const code = `
// leading comment to be duplicated
statement(); // inline comment to be duplicated
`;

        const result = transform(code, context => ({
          ExpressionStatement(node) {
            context.insertBeforeStatement(node, context.shallowCloneNode(node));
          },
        }));

        expect(result).toBe(`\
// leading comment to be duplicated
statement(); // inline comment to be duplicated
// leading comment to be duplicated
statement(); // inline comment to be duplicated
`);
      });

      it('should attach comments so they are removed when the associated node is replaced (by default)', () => {
        const code = `
// this should remain leading #1
const x = 1; // this should remain inline #1
// leading comment to be deleted
statement(); // inline comment to be deleted
// this should remain leading #2
const y = 1; // this should remain inline #2
`;

        const result = transform(code, context => ({
          ExpressionStatement(node) {
            context.replaceNode(
              node,
              t.ExpressionStatement({
                expression: t.StringLiteral({
                  value: 'inserted',
                }),
              }),
            );
          },
        }));

        expect(result).toBe(`\
// this should remain leading #1
const x = 1; // this should remain inline #1
('inserted');
// this should remain leading #2
const y = 1; // this should remain inline #2
`);
      });

      it('should optionally attach comments so they are kept when the associated statement is replaced', () => {
        const code = `
// this should remain leading #1
const x = 1; // this should remain inline #1
// leading comment to be deleted
statement(); // inline comment to be deleted
// this should remain leading #2
const y = 1; // this should remain inline #2
`;

        const result = transform(code, context => ({
          ExpressionStatement(node) {
            context.replaceStatementWithMany(
              node,
              [
                t.ExpressionStatement({
                  expression: t.StringLiteral({
                    value: 'inserted1',
                  }),
                }),
                t.ExpressionStatement({
                  expression: t.StringLiteral({
                    value: 'inserted2',
                  }),
                }),
                t.ExpressionStatement({
                  expression: t.StringLiteral({
                    value: 'inserted3',
                  }),
                }),
              ],
              {keepComments: true},
            );
          },
        }));

        expect(result).toBe(`\
// this should remain leading #1
const x = 1; // this should remain inline #1
// leading comment to be deleted
('inserted1'); // inline comment to be deleted
('inserted2');
('inserted3');
// this should remain leading #2
const y = 1; // this should remain inline #2
`);
      });

      it('should optionally attach comments so they are kept when the associated node is replaced', () => {
        const code = `
// this should remain leading #1
const x = 1; // this should remain inline #1
// leading comment to be deleted
statement(); // inline comment to be deleted
// this should remain leading #2
const y = 1; // this should remain inline #2
`;

        const result = transform(code, context => ({
          ExpressionStatement(node) {
            context.replaceNode(
              node,
              t.ExpressionStatement({
                expression: t.StringLiteral({
                  value: 'inserted1',
                }),
              }),
              {keepComments: true},
            );
          },
        }));

        expect(result).toBe(`\
// this should remain leading #1
const x = 1; // this should remain inline #1
// leading comment to be deleted
('inserted1'); // inline comment to be deleted
// this should remain leading #2
const y = 1; // this should remain inline #2
`);
      });
    });

    describe('addition', () => {
      it('should allow leading comment addition', () => {
        const code = `\
const x = 1;
const y = 2;`;
        const result = transform(code, context => ({
          Identifier(node) {
            if (node.name === 'x') {
              context.addLeadingComments(node.parent.parent, [
                t.LineComment({value: 'line'}),
              ]);
            } else if (node.name === 'y') {
              context.addLeadingComments(node.parent.parent, [
                t.BlockComment({value: 'block'}),
              ]);
            }
          },
        }));

        expect(result).toBe(`\
//line
const x = 1;
/*block*/ const y = 2;
`);
      });

      it('should allow trailing comment addition', () => {
        const code = `\
const x = 1;
const y = 2;`;
        const result = transform(code, context => ({
          Identifier(node) {
            if (node.name === 'x') {
              context.addTrailingComments(node.parent.parent, [
                t.LineComment({value: 'line'}),
              ]);
            } else if (node.name === 'y') {
              context.addTrailingComments(node.parent.parent, [
                t.BlockComment({value: 'block'}),
              ]);
            }
          },
        }));

        expect(result).toBe(`\
const x = 1;
//line
const y = 2;
/*block*/
`);
      });

      it('should allow comment addition to new, detached nodes', () => {
        const code = 'const x = 1;';
        const result = transform(code, context => ({
          VariableDeclaration(node) {
            const newNode = t.ExpressionStatement({
              expression: t.StringLiteral({value: 'inserted'}),
            });
            context.insertBeforeStatement(node, newNode);

            context.addLeadingComments(newNode, [
              t.LineComment({value: 'leading line'}),
              t.BlockComment({value: 'leading block'}),
            ]);
            context.addTrailingComments(newNode, [
              t.LineComment({value: 'trailing line'}),
              t.BlockComment({value: 'trailing block'}),
            ]);
          },
        }));

        expect(result).toBe(`\
//leading line
/*leading block*/
('inserted');
//trailing line
/*trailing block*/
const x = 1;
`);
      });
    });

    describe('removal', () => {
      it('shoud allow removal of leading comments', () => {
        const code = `\
/*block*/
const x = 1;
//line
const y = 2;`;
        const result = transform(code, context => ({
          VariableDeclaration(node) {
            const coment = context.getComments(node)[0];
            context.removeComments(coment);
          },
        }));

        expect(result).toBe(`\
const x = 1;
const y = 2;
`);
      });

      it('shoud allow removal of trailing comments', () => {
        const code = `\
const x = 1; /*block*/
const y = 2; //line`;
        const result = transform(code, context => ({
          VariableDeclaration(node) {
            const coment = context.getComments(node)[0];
            context.removeComments(coment);
          },
        }));

        expect(result).toBe(`\
const x = 1;
const y = 2;
`);
      });
    });
  });
});
