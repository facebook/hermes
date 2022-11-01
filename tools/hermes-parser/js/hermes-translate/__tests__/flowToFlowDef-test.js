/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import flowToFlowDef from '../src/flowToFlowDef';
// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../.prettierrc.json';
import {parse, print} from 'hermes-transform';

function translate(code: string): string {
  const {ast, scopeManager} = parse(code);

  const [flowDefAst, mutatedCode] = flowToFlowDef(ast, code, scopeManager, {
    recoverFromErrors: false,
  });

  return print(flowDefAst, mutatedCode, prettierConfig);
}

/**
 * Align code based on the shortest whitespace offset
 */
function trimToBeCode(toBeCode: string): string {
  const trimmedToBeCode = toBeCode.trim();
  const trimmedToBeCodeLines = trimmedToBeCode.split('\n');
  if (trimmedToBeCodeLines.length === 1) {
    return trimmedToBeCode + '\n';
  }

  let minSpaces = Infinity;
  const lines: Array<[number, string]> = [];
  for (let i = 1; i < trimmedToBeCodeLines.length; i++) {
    const line = trimmedToBeCodeLines[i];
    if (line === '') {
      lines.push([0, '']);
      continue;
    }
    const lineLeftTrimmed = line.trimLeft();
    const offset = line.length - lineLeftTrimmed.length;
    lines.push([offset, lineLeftTrimmed]);
    minSpaces = Math.min(minSpaces, offset);
  }
  if (minSpaces === 0) {
    return trimmedToBeCode + '\n';
  }

  let rebuiltStr = trimmedToBeCodeLines[0] + '\n';
  for (const [offset, line] of lines) {
    if (line === '') {
      rebuiltStr += '\n';
      continue;
    }
    rebuiltStr += ' '.repeat(offset - minSpaces) + line + '\n';
  }
  return rebuiltStr;
}

function expectTranslate(expectCode: string, toBeCode: string): void {
  expect(translate(expectCode)).toBe(trimToBeCode(toBeCode));
}
function expectTranslateUnchanged(expectCode: string): void {
  expect(translate(expectCode)).toBe(trimToBeCode(expectCode));
}

describe('flowToFlowDef', () => {
  describe('Comments', () => {
    it('maintain docblock', () => {
      expectTranslateUnchanged(
        `/**
          * @flow
          */
         export type Bar = string;`,
      );
    });
    it('maintain toplevel statement comments', () => {
      expectTranslate(
        `/**
          * @flow
          */

         // Comment 1
         export type Bar = string;

         // Comment 2
         export function bar(): void {}`,
        `/**
          * @flow
          */

         // Comment 1
         export type Bar = string;

         // Comment 2
         declare export function bar(): void;`,
      );
    });
  });
  describe('dependency walking', () => {
    it('strip unused function', () => {
      expectTranslate(
        `function foo(): void {}
         export function bar(): void { foo(); }`,
        `declare export function bar(): void;`,
      );
    });
    it('keep used TypeAlias', () => {
      expectTranslateUnchanged(
        `type Foo = string;
         export type Bar = Foo;`,
      );
    });
    it('keep chain of used TypeAliases', () => {
      expectTranslateUnchanged(
        `type Foo = string;
         type Bar = Foo;
         type Baz = Bar;
         export type Boo = Baz;`,
      );
    });
    it('keep used TypeAlias with many references', () => {
      expectTranslateUnchanged(
        `type Foo = string;
         export type Bar = Foo;
         export type Baz = [Bar, Foo];`,
      );
    });
    it('strip unused but shadowed deps', () => {
      // These should never be referenced by anything
      const neverReferenced = `
        type Foo = string;`;
      // Shadowing "Foo", this shouldn't create a dep on the outer "Foo"
      const expectedOutput = `
        type Bar = string;
        declare export class BazC<Foo> {
          prop1: Foo;
          prop2: Bar;
        }`;
      expectTranslate(neverReferenced + expectedOutput, expectedOutput);
    });
    it('strip unused but shadowed deps (complex)', () => {
      // These should never be referenced by anything
      const neverReferenced = `
        type T = 1;`;
      // These should all have a reference
      const expectedOutput = `
        declare class OuterClass<T1> {}
        declare class OuterMixin<T1> {}
        declare interface OuterInterface<T1> {}
        type Outer1 = number;
        type Outer2 = number;
        type Outer3 = number;
        type Outer4 = number;
        type Outer5 = number;
        type Outer6 = number;

        declare export class Foo<
            // shouldn't create a reference on the outer T
            T: Outer1 = Outer2,
            // shouldn't create a reference on the outer T
            T2: T = T,
          >
          // shouldn't create a reference on the outer T
          extends OuterClass<T>
          // shouldn't create a reference on the outer T
          mixins OuterMixin<T>
          // shouldn't create a reference on the outer T
          implements OuterInterface<T>
        {
          // shouldn't create a reference on the outer T
          prop1: T;
          // shouldn't create a reference on the outer T
          method1(): T;
          // shouldn't create a reference on the outer T
          [T]: string;

          prop2: Outer3;
          method2(): Outer4;
          [Outer5]: Outer6;

          // this should create a (circular) reference to the containing Foo
          constructor(): Foo;
        }`;
      expectTranslate(neverReferenced + expectedOutput, expectedOutput);
    });
  });
  describe('optimization pass', () => {
    it('strip unused import defs', () => {
      expectTranslate(
        `import type {Foo, Bar} from 'Foo';
         export type Baz = Foo;`,
        `import type {Foo} from 'Foo';
         export type Baz = Foo;`,
      );
    });
  });
  describe('ExportNamedDeclaration', () => {
    it('type specifier', () => {
      expectTranslateUnchanged(
        `type Bar = string;
         export type {Foo, Bar as Baz};`,
      );
    });
    it('type specifier with source', () => {
      expectTranslateUnchanged(`export type {Foo, Bar as Baz} from 'Baz';`);
    });
    it('value specifier with source', () => {
      expectTranslateUnchanged(`export {Foo, Bar as Baz} from 'Baz';`);
    });
    it('all with source', () => {
      expectTranslateUnchanged(`export * as Foo from 'Foo';`);
    });
  });
  describe('ExportDefaultDeclaration', () => {
    it('export default function', () => {
      expectTranslate(
        `export default function Foo() {}`,
        `declare export default function Foo(): void;`,
      );
    });
    it('export default class', () => {
      expectTranslate(
        `export default class Foo {}`,
        `declare export default class Foo {}`,
      );
    });
    it('export default expression', () => {
      expectTranslate(
        `export default (1: number);`,
        `declare export default number;`,
      );
    });
    it('export default var', () => {
      expectTranslate(
        `function foo() {}
         export default foo;`,
        `declare function foo(): void;
         declare export default foo;`,
      );
    });
  });
  describe('ExportAllDeclaration', () => {
    it('export basic', () => {
      expectTranslateUnchanged(`export * from 'Foo';`);
    });
  });
  describe('FunctionDeclation', () => {
    it('basic', () => {
      expectTranslate(
        `export function foo(): void {}`,
        `declare export function foo(): void;`,
      );
    });
    it('without return type', () => {
      expectTranslate(
        `export function foo() {}`,
        `declare export function foo(): void;`,
      );
    });
    it('with type params', () => {
      expectTranslate(
        `export function foo<T>(): T {}`,
        `declare export function foo<T>(): T;`,
      );
    });
    it('with params', () => {
      expectTranslate(
        `export function foo(bar: string, baz: number): void {}`,
        `declare export function foo(bar: string, baz: number): void;`,
      );
    });
    it('with rest params', () => {
      expectTranslate(
        `export function foo(bar: string, ...baz: Array<number>): void {}`,
        `declare export function foo(bar: string, ...baz: Array<number>): void;`,
      );
    });
    it('with default params', () => {
      expectTranslate(
        `export function foo(bar: string = 'hello'): void {}`,
        `declare export function foo(bar: string): void;`,
      );
    });
    it('with predicates', () => {
      expectTranslate(
        `function bar(baz: string): boolean %checks {
          return baz === '';
         }
         export function foo(): boolean %checks {
           return bar('');
         }`,
        `declare function bar(baz: string): boolean %checks(baz === '');
         declare export function foo(): boolean %checks(bar(''));`,
      );
    });
  });
  describe('TypeAlias', () => {
    it('basic', () => {
      expectTranslateUnchanged(`export type Foo = string;`);
    });
    it('with type params', () => {
      expectTranslateUnchanged(`export type Foo<Bar: Baz, Boo> = string;`);
    });
  });
  describe('OpaqueType', () => {
    it('basic', () => {
      expectTranslate(
        `export opaque type Foo = string;`,
        `declare export opaque type Foo;`,
      );
    });
    it('basic local', () => {
      expectTranslate(
        `type Foo = string;
         opaque type Bar = Foo;
         export type Baz = Bar;`,
        `declare opaque type Bar;
         export type Baz = Bar;`,
      );
    });
    it('with type params', () => {
      expectTranslate(
        `export opaque type Foo<Bar: Baz, Boo> = string;`,
        `declare export opaque type Foo<Bar: Baz, Boo>;`,
      );
    });
    it('with super type', () => {
      expectTranslate(
        `export opaque type Foo: Bar = string;`,
        `declare export opaque type Foo: Bar;`,
      );
    });
    it('with super type and type params', () => {
      expectTranslate(
        `export opaque type Foo<Bar: Baz, Boo>: Boa = string;`,
        `declare export opaque type Foo<Bar: Baz, Boo>: Boa;`,
      );
    });
  });
  describe('ImportDeclaration', () => {
    it('basic', () => {
      expectTranslateUnchanged(
        `import type {Foo} from 'foo';
         export type {Foo};`,
      );
    });
    it('type specifiers', () => {
      expectTranslateUnchanged(
        `import {type Foo} from 'foo';
         export type {Foo};`,
      );
    });
  });
  describe('ClassDeclaration', () => {
    it('property', () => {
      expectTranslate(
        `export class A {
           foo: string = '';
         }`,
        `declare export class A {
           foo: string;
         }`,
      );
    });
    it('method', () => {
      expectTranslate(
        `export class A {
           foo() {}
           static bar() {}
         }`,
        `declare export class A {
           foo(): void;
           static bar(): void;
         }`,
      );
    });
  });
  describe('InterfaceDeclaration', () => {
    it('property', () => {
      expectTranslate(
        `export interface A {
           foo: string;
         }`,
        `export interface A {
           foo: string;
         }`,
      );
    });
    it('method', () => {
      expectTranslate(
        `export interface A {
           foo(): void;
         }`,
        `export interface A {
           foo(): void;
         }`,
      );
    });
    it('local', () => {
      expectTranslate(
        `interface Foo {}
         export type Bar = Foo;`,
        `interface Foo {}
         export type Bar = Foo;`,
      );
    });
  });
  describe('VariableDeclaration', () => {
    it('basic type parameter', () => {
      expectTranslate(
        `export const foo: number = 1;`,
        `declare export var foo: number;`,
      );
    });
    it('basic typecast', () => {
      expectTranslate(
        `export const foo = (1: number);`,
        `declare export var foo: number;`,
      );
    });
    it('prefer type parameter', () => {
      expectTranslate(
        `export const foo: number = (1: any);`,
        `declare export var foo: number;`,
      );
    });
    it('with dependency', () => {
      expectTranslate(
        `const foo: number = 1;
         export const bar: typeof foo = 1;`,
        `declare var foo: number;
         declare export var bar: typeof foo;`,
      );
    });
  });
  describe('EnumDeclaration', () => {
    it('basic', () => {
      expectTranslateUnchanged(`export enum Foo {}`);
    });
    it('local', () => {
      expectTranslateUnchanged(
        `enum Foo {}
         declare export var bar: Foo;`,
      );
    });
  });
  describe('DeclareClass', () => {
    it('basic', () => {
      expectTranslateUnchanged(
        `declare class Foo {}
         declare export var bar: Foo;`,
      );
    });
    it('complex', () => {
      expectTranslateUnchanged(
        `declare export class Foo<T>
           extends TClass<T>
           mixins TMixin<T>
           implements TInterface<T>
         {
           prop1: T;
           method1(): T;
           [T]: string;
           constructor(): Foo;
         }`,
      );
    });
  });
  describe('Expression', () => {
    function expectTranslateExpression(
      expectExprCode: string,
      toBeExprCode: string,
    ): void {
      expectTranslate(
        `export const expr = ${expectExprCode};`,
        `declare export var expr: ${toBeExprCode};`,
      );
    }
    describe('Identifier', () => {
      it('basic', () => {
        expectTranslateExpression(`foo`, `foo`);
      });
    });
    describe('ObjectExpression', () => {
      it('empty', () => {
        expectTranslateExpression(`{}`, `{}`);
      });
      it('methods', () => {
        expectTranslateExpression(`{foo() {}}`, `{foo(): void}`);
        expectTranslateExpression(`{get foo() {}}`, `{get foo(): void}`);
        expectTranslateExpression(
          `{set foo(bar: string) {}}`,
          `{set foo(bar: string): void}`,
        );
      });
      it('properties', () => {
        expectTranslateExpression(`{FOO: 1}`, `{FOO: 1}`);
      });
      it('spread', () => {
        expectTranslateExpression(`{...a}`, `{...a}`);
      });
    });
    describe('Literals', () => {
      it('number', () => {
        expectTranslateExpression(`1`, `1`);
        expectTranslateExpression(`1.99`, `1.99`);
      });
      it('string', () => {
        expectTranslateExpression(`'s'`, `'s'`);
      });
      it('boolean', () => {
        expectTranslateExpression(`true`, `true`);
      });
      it('regex', () => {
        expectTranslateExpression(`/a/`, `RegExp`);
      });
      it('null', () => {
        expectTranslateExpression(`null`, `null`);
      });
    });
    describe('TypeCastExpression', () => {
      it('basic', () => {
        expectTranslateExpression(`(1: number)`, `number`);
      });
    });
    describe('FunctionExpression', () => {
      it('basic', () => {
        expectTranslateExpression(`function foo() {}`, `() => void`);
        expectTranslateExpression(
          `function foo<T>(baz: T, bar: string) {}`,
          `<T>(baz: T, bar: string) => void`,
        );
      });
    });
    describe('ArrowFunctionExpression', () => {
      it('basic', () => {
        expectTranslateExpression(`() => {}`, `() => void`);
        expectTranslateExpression(
          `<T>(baz: T, bar: string) => {}`,
          `<T>(baz: T, bar: string) => void`,
        );
      });
    });
  });
});
