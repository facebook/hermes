/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {
  ESNode,
  ExpressionStatement,
  Identifier,
  ModuleDeclaration,
  Program,
  Statement,
} from 'hermes-estree';

import {parseForESLint} from 'hermes-eslint';
import * as t from '../../../src/generated/node-types';
import {
  createInsertStatementMutation,
  performInsertStatementMutation,
} from '../../../src/transform/mutations/InsertStatement';
import {MutationContext} from '../../../src/transform/MutationContext';
import {traverse} from '../../../src/traverse/traverse';

type StatementTypes = Statement['type'] | ModuleDeclaration['type'];
const CODE_SAMPLES: {[StatementTypes]: string} = {
  ExpressionStatement: 'foo;',
  BlockStatement: '{}',
  EmptyStatement: ';',
  DebuggerStatement: 'debugger;',
  WithStatement: 'with (foo) {}',
  ReturnStatement: 'return foo;',
  LabeledStatement: 'label: foo;',
  BreakStatement: 'break;',
  ContinueStatement: 'continue;',
  IfStatement: 'if (foo) {}',
  SwitchStatement: 'switch (foo) {}',
  ThrowStatement: 'throw foo;',
  TryStatement: 'try {} catch {}',
  WhileStatement: 'while (foo) {}',
  DoWhileStatement: 'do {} while (foo)',
  ForStatement: 'for (;;) {}',
  ForInStatement: 'for (foo in bar) {}',
  ForOfStatement: 'for (foo of bar) {}',
  TypeAlias: 'type Foo = 1;',
  OpaqueType: 'opaque type Foo = 1;',
  InterfaceDeclaration: 'interface Foo {}',
  FunctionDeclaration: 'function foo() {}',
  VariableDeclaration: 'let foo;',
  ClassDeclaration: 'class Foo {}',
  DeclareTypeAlias: 'declare type Foo = 1;',
  DeclareOpaqueType: 'declare opaque type Foo: 1;',
  DeclareInterface: 'declare interface Foo {}',
  DeclareModule: 'declare module foo {}',
  ImportDeclaration: "import foo from 'foo';",
  ExportNamedDeclaration: 'export { foo };',
  ExportDefaultDeclaration: 'export default foo;',
  ExportAllDeclaration: "export * from 'foo';",
  DeclareExportDeclaration: 'declare export default foo;',
  DeclareExportAllDeclaration: "declare export * from 'foo';",
  DeclareModuleExports: 'declare module.exports: Foo;',
};
const MODULE_DECLARATIONS = [
  'ImportDeclaration',
  'ExportNamedDeclaration',
  'ExportDefaultDeclaration',
  'ExportAllDeclaration',
  'DeclareExportDeclaration',
  'DeclareExportAllDeclaration',
  'DeclareModuleExports',
];
const LOOP_ONLY_STATEMENTS = ['BreakStatement', 'ContinueStatement'];
const DEFAULT_SKIP_STATEMENTS = [
  ...MODULE_DECLARATIONS,
  ...LOOP_ONLY_STATEMENTS,
];

function parseAndGetAstAndNode(
  type: StatementTypes,
  code: string,
): {
  ast: Program,
  target: Statement | ModuleDeclaration,
} {
  const {ast, scopeManager} = parseForESLint(code, {
    sourceType: 'module',
  });

  let target: Statement | ModuleDeclaration | null = null;
  traverse(ast, scopeManager, () => ({
    // $FlowExpectedError[invalid-computed-prop] - this is guaranteed safe
    [type](node) {
      target = node;
    },
  }));
  if (target == null) {
    throw new Error(`Couldn't find a ${type} node in the parsed AST.`);
  }

  return {ast, target};
}

describe('InsertStatement', () => {
  function test({
    wrapCode,
    getAssertionObject,
    skipTypes = DEFAULT_SKIP_STATEMENTS,
  }: {
    wrapCode: (code: string) => string,
    getAssertionObject: (nodes: [mixed, mixed]) => mixed,
    skipTypes?: $ReadOnlyArray<StatementTypes>,
  }) {
    function loopSamples(side: 'before' | 'after') {
      for (const type of Object.keys(CODE_SAMPLES)) {
        if (skipTypes.includes(type)) {
          continue;
        }

        it(type, () => {
          const {ast, target} = parseAndGetAstAndNode(
            type,
            wrapCode(CODE_SAMPLES[type]),
          );
          const nodeToInsert = t.ExpressionStatement({
            expression: t.Identifier({
              name: 'inserted',
            }),
          });
          const mutation = createInsertStatementMutation(side, target, [
            nodeToInsert,
          ]);
          performInsertStatementMutation(new MutationContext(), mutation);
          expect(ast).toMatchObject(
            getAssertionObject(
              side === 'before'
                ? [nodeToInsert, target]
                : [target, nodeToInsert],
            ),
          );
        });
      }
    }

    describe('before', () => {
      loopSamples('before');
    });

    describe('after', () => {
      loopSamples('after');
    });
  }

  describe('Parent: Program', () => {
    test({
      wrapCode: c => c,
      getAssertionObject: body => ({
        type: 'Program',
        body,
      }),
      // we are allowed to have ModuleDeclarations here
      skipTypes: LOOP_ONLY_STATEMENTS,
    });
  });

  describe('Parent: IfStatement', () => {
    describe('BlockStatement body', () => {
      test({
        wrapCode: c => `if (cond) { ${c} }`,
        getAssertionObject: body => ({
          type: 'Program',
          body: [
            {
              type: 'IfStatement',
              consequent: {
                type: 'BlockStatement',
                body,
              },
            },
          ],
        }),
      });
    });

    describe('Statement body', () => {
      test({
        wrapCode: c => `if (cond) ${c}`,
        getAssertionObject: body => ({
          type: 'Program',
          body: [
            {
              type: 'IfStatement',
              consequent: {
                type: 'BlockStatement',
                body,
              },
            },
          ],
        }),
        skipTypes: [
          // all of these are non-sensical without a body
          'BlockStatement',
          'TypeAlias',
          'OpaqueType',
          'InterfaceDeclaration',
          'FunctionDeclaration',
          'VariableDeclaration',
          'ClassDeclaration',
          'DeclareTypeAlias',
          'DeclareOpaqueType',
          'DeclareInterface',
          'DeclareModule',
          ...DEFAULT_SKIP_STATEMENTS,
        ],
      });
    });
  });

  describe('Parent: ForStatement', () => {
    describe('BlockStatement body', () => {
      test({
        wrapCode: c => `for (;;) { ${c} }`,
        getAssertionObject: body => ({
          type: 'Program',
          body: [
            {
              type: 'ForStatement',
              body: {
                type: 'BlockStatement',
                body,
              },
            },
          ],
        }),
        skipTypes: MODULE_DECLARATIONS,
      });
    });

    describe('Statement body', () => {
      test({
        wrapCode: c => `for (;;) ${c}`,
        getAssertionObject: body => ({
          type: 'Program',
          body: [
            {
              type: 'ForStatement',
              body: {
                type: 'BlockStatement',
                body,
              },
            },
          ],
        }),
        skipTypes: [
          // all of these are non-sensical without a body
          'BlockStatement',
          'TypeAlias',
          'OpaqueType',
          'InterfaceDeclaration',
          'FunctionDeclaration',
          'VariableDeclaration',
          'ClassDeclaration',
          'DeclareTypeAlias',
          'DeclareOpaqueType',
          'DeclareInterface',
          'DeclareModule',
          ...MODULE_DECLARATIONS,
        ],
      });
    });
  });

  describe('Parent: SwitchCase', () => {
    test({
      wrapCode: c => `switch (thing) { case c: ${c} }`,
      getAssertionObject: body => ({
        type: 'Program',
        body: [
          {
            type: 'SwitchStatement',
            cases: [
              {
                type: 'SwitchCase',
                consequent: body,
              },
            ],
          },
        ],
      }),
      // BreakStatements are allowed inside a SwitchCase
      skipTypes: ['ContinueStatement', ...MODULE_DECLARATIONS],
    });
  });

  describe('Parent: DeclareModule', () => {
    test({
      wrapCode: c => `declare module foo { ${c} }`,
      getAssertionObject: body => ({
        type: 'Program',
        body: [
          {
            type: 'DeclareModule',
            body: {
              type: 'BlockStatement',
              body,
            },
          },
        ],
      }),
      // only "declare" statements are allowed
      skipTypes: Object.keys(CODE_SAMPLES).filter(
        t => !t.startsWith('Declare'),
      ),
    });
  });
});
