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
import {trimToBeCode} from './utils/inlineCodeHelpers';

async function translate(code: string): Promise<string> {
  const {ast, scopeManager} = await parse(code);

  const [flowDefAst, mutatedCode] = flowToFlowDef(ast, code, scopeManager, {
    recoverFromErrors: false,
  });

  return print(flowDefAst, mutatedCode, prettierConfig);
}

async function expectTranslate(
  expectCode: string,
  toBeCode: string,
): Promise<void> {
  const expectTranslateCode = await translate(expectCode);
  expect(expectTranslateCode).toBe(trimToBeCode(toBeCode));
}
async function expectTranslateUnchanged(expectCode: string): Promise<void> {
  const expectTranslateCode = await translate(expectCode);
  expect(expectTranslateCode).toBe(trimToBeCode(expectCode));
}

describe('flowToFlowDef', () => {
  describe('Comments', () => {
    it('maintain docblock', async () => {
      await expectTranslateUnchanged(
        `/**
          * @flow
          */
         export type Bar = string;`,
      );
    });
    it('maintain toplevel statement comments', async () => {
      await expectTranslate(
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
    it('strip unused function', async () => {
      await expectTranslate(
        `function foo(): void {}
         export function bar(): void { foo(); }`,
        `declare export function bar(): void;`,
      );
    });
    it('keep used TypeAlias', async () => {
      await expectTranslateUnchanged(
        `type Foo = string;
         export type Bar = Foo;`,
      );
    });
    it('keep chain of used TypeAliases', async () => {
      await expectTranslateUnchanged(
        `type Foo = string;
         type Bar = Foo;
         type Baz = Bar;
         export type Boo = Baz;`,
      );
    });
    it('keep used TypeAlias with many references', async () => {
      await expectTranslateUnchanged(
        `type Foo = string;
         export type Bar = Foo;
         export type Baz = [Bar, Foo];`,
      );
    });
    it('strip unused but shadowed deps', async () => {
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
      await expectTranslate(neverReferenced + expectedOutput, expectedOutput);
    });
    it('strip unused but shadowed deps (complex)', async () => {
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
      await expectTranslate(neverReferenced + expectedOutput, expectedOutput);
    });
  });
  describe('optimization pass', () => {
    it('strip unused import defs', async () => {
      await expectTranslate(
        `import type {Foo, Bar} from 'Foo';
         export type Baz = Foo;`,
        `import type {Foo} from 'Foo';
         export type Baz = Foo;`,
      );
    });
  });
  describe('ExportNamedDeclaration', () => {
    it('type specifier', async () => {
      await expectTranslateUnchanged(
        `type Bar = string;
         export type {Foo, Bar as Baz};`,
      );
    });
    it('type specifier with source', async () => {
      await expectTranslateUnchanged(
        `export type {Foo, Bar as Baz} from 'Baz';`,
      );
    });
    it('value specifier with source', async () => {
      await expectTranslateUnchanged(`export {Foo, Bar as Baz} from 'Baz';`);
    });
    it('all with source', async () => {
      await expectTranslateUnchanged(`export * as Foo from 'Foo';`);
    });
  });
  describe('ExportDefaultDeclaration', () => {
    it('export default function', async () => {
      await expectTranslate(
        `export default function Foo() {}`,
        `declare export default function Foo(): void;`,
      );
    });
    it('export default class', async () => {
      await expectTranslate(
        `export default class Foo {}`,
        `declare export default class Foo {}`,
      );
    });
    it('export default expression', async () => {
      await expectTranslate(
        `export default (1: number);`,
        `declare export default number;`,
      );
    });
    it('export default var', async () => {
      await expectTranslate(
        `function foo() {}
         export default foo;`,
        `declare function foo(): void;
         declare export default typeof foo;`,
      );
    });
  });
  describe('ExportAllDeclaration', () => {
    it('export basic', async () => {
      await expectTranslateUnchanged(`export * from 'Foo';`);
    });
  });
  describe('module.exports', () => {
    it('export basic', async () => {
      await expectTranslate(
        `module.exports = 1;`,
        `declare module.exports: 1;`,
      );
    });
  });
  describe('exports.*', () => {
    it('export basic', async () => {
      await expect(async () => translate(`exports.A = 1;`)).rejects
        .toThrowErrorMatchingInlineSnapshot(`
        "
        > 1 | exports.A = 1;
            | ^^^^^^^^^^^^^^ convertExport: Named CommonJS exports not supported. Use either \`module.exports = {...}\` or ES6 exports."
      `);
      await expect(async () => translate(`module.exports.A = 1;`)).rejects
        .toThrowErrorMatchingInlineSnapshot(`
        "
        > 1 | module.exports.A = 1;
            | ^^^^^^^^^^^^^^^^^^^^^ convertExport: Named CommonJS exports not supported. Use either \`module.exports = {...}\` or ES6 exports."
      `);
    });
  });
  describe('FunctionDeclation', () => {
    it('basic', async () => {
      await expectTranslate(
        `export function foo(): void {}`,
        `declare export function foo(): void;`,
      );
    });
    it('without return type', async () => {
      await expectTranslate(
        `export function foo() {}`,
        `declare export function foo(): void;`,
      );
    });
    it('with type params', async () => {
      await expectTranslate(
        `export function foo<T>(): T {}`,
        `declare export function foo<T>(): T;`,
      );
    });
    it('with params', async () => {
      await expectTranslate(
        `export function foo(bar: string, baz: number): void {}`,
        `declare export function foo(bar: string, baz: number): void;`,
      );
    });
    it('with rest params', async () => {
      await expectTranslate(
        `export function foo(bar: string, ...baz: Array<number>): void {}`,
        `declare export function foo(bar: string, ...baz: Array<number>): void;`,
      );
    });
    it('with default params', async () => {
      await expectTranslate(
        `export function foo(bar: string = 'hello'): void {}`,
        `declare export function foo(bar: string): void;`,
      );
    });
    it('with predicates', async () => {
      await expectTranslate(
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
    it('basic', async () => {
      await expectTranslateUnchanged(`export type Foo = string;`);
    });
    it('with type params', async () => {
      await expectTranslateUnchanged(
        `export type Foo<Bar: Baz, Boo> = string;`,
      );
    });
  });
  describe('OpaqueType', () => {
    it('basic', async () => {
      await expectTranslate(
        `export opaque type Foo = string;`,
        `declare export opaque type Foo;`,
      );
    });
    it('basic local', async () => {
      await expectTranslate(
        `type Foo = string;
         opaque type Bar = Foo;
         export type Baz = Bar;`,
        `declare opaque type Bar;
         export type Baz = Bar;`,
      );
    });
    it('with type params', async () => {
      await expectTranslate(
        `export opaque type Foo<Bar: Baz, Boo> = string;`,
        `declare export opaque type Foo<Bar: Baz, Boo>;`,
      );
    });
    it('with super type', async () => {
      await expectTranslate(
        `export opaque type Foo: Bar = string;`,
        `declare export opaque type Foo: Bar;`,
      );
    });
    it('with super type and type params', async () => {
      await expectTranslate(
        `export opaque type Foo<Bar: Baz, Boo>: Boa = string;`,
        `declare export opaque type Foo<Bar: Baz, Boo>: Boa;`,
      );
    });
  });
  describe('ImportDeclaration', () => {
    it('basic', async () => {
      await expectTranslateUnchanged(
        `import type {Foo} from 'foo';
         export type {Foo};`,
      );
    });
    it('type specifiers', async () => {
      await expectTranslateUnchanged(
        `import {type Foo} from 'foo';
         export type {Foo};`,
      );
    });
  });
  describe('ClassDeclaration', () => {
    it('property', async () => {
      await expectTranslate(
        `export class A {
           foo: string = '';
         }`,
        `declare export class A {
           foo: string;
         }`,
      );
      await expectTranslate(
        `export class A {
           'foo': string = '';
         }`,
        `declare export class A {
           foo: string;
         }`,
      );
      await expectTranslate(
        `export class A {
           1: string = '';
         }`,
        `declare export class A {
           1: string;
         }`,
      );
    });
    it('method', async () => {
      await expectTranslate(
        `export class A {
           foo() {}
           static bar() {}
         }`,
        `declare export class A {
           foo(): void;
           static bar(): void;
         }`,
      );
      await expectTranslate(
        `export class A {
           'foo'() {}
           static 'bar'() {}
         }`,
        `declare export class A {
           foo(): void;
           static bar(): void;
         }`,
      );
      await expectTranslate(
        `export class A {
           1() {}
           static 2() {}
         }`,
        `declare export class A {
           1(): void;
           static 2(): void;
         }`,
      );
    });
  });
  describe('InterfaceDeclaration', () => {
    it('property', async () => {
      await expectTranslate(
        `export interface A {
           foo: string;
         }`,
        `export interface A {
           foo: string;
         }`,
      );
    });
    it('method', async () => {
      await expectTranslate(
        `export interface A {
           foo(): void;
         }`,
        `export interface A {
           foo(): void;
         }`,
      );
    });
    it('local', async () => {
      await expectTranslate(
        `interface Foo {}
         export type Bar = Foo;`,
        `interface Foo {}
         export type Bar = Foo;`,
      );
    });
  });
  describe('VariableDeclaration', () => {
    it('basic type parameter', async () => {
      await expectTranslate(
        `export const foo: number = 1;`,
        `declare export const foo: number;`,
      );
    });
    it('basic typecast', async () => {
      await expectTranslate(
        `export const foo = (1: number);`,
        `declare export const foo: number;`,
      );
    });
    it('prefer type parameter', async () => {
      await expectTranslate(
        `export const foo: number = (1: any);`,
        `declare export const foo: number;`,
      );
    });
    it('with dependency', async () => {
      await expectTranslate(
        `const foo: number = 1;
         export const bar: typeof foo = 1;`,
        `declare const foo: number;
         declare export const bar: typeof foo;`,
      );
    });
    it('with imported value', async () => {
      await expectTranslate(
        `import {foo} from 'foo';
         export const bar = foo;`,
        `import {foo} from 'foo';
         declare export const bar: typeof foo;`,
      );
    });
  });
  describe('EnumDeclaration', () => {
    it('basic', async () => {
      await expectTranslateUnchanged(`export enum Foo {}`);
    });
    it('local', async () => {
      await expectTranslateUnchanged(
        `enum Foo {}
         declare export const bar: Foo;`,
      );
    });
  });
  describe('DeclareClass', () => {
    it('basic', async () => {
      await expectTranslateUnchanged(
        `declare class Foo {}
         declare export const bar: Foo;`,
      );
    });
    it('complex', async () => {
      await expectTranslateUnchanged(
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
    it('extends member expression', async () => {
      await expectTranslateUnchanged(
        `declare export class Foo<T> extends Bar.TClass<T> {}`,
      );
    });
    it('extends type cast expression', async () => {
      await expectTranslate(
        `export class Foo<T> extends (Bar: X) {}`,
        `declare export class Foo<T> extends X {}`,
      );
    });
    it('extends type cast typeof expression', async () => {
      await expectTranslate(
        `export class Foo<T> extends (Bar: typeof X) {}`,
        `declare export class Foo<T> extends X {}`,
      );
    });
  });
  describe('Expression', () => {
    async function expectTranslateExpression(
      expectExprCode: string,
      toBeExprCode: string,
    ): Promise<void> {
      await expectTranslate(
        `export const expr = ${expectExprCode};`,
        `declare export const expr: ${toBeExprCode};`,
      );
    }
    describe('Identifier', () => {
      it('basic', async () => {
        await expectTranslateExpression(`foo`, `typeof foo`);
      });
    });
    describe('ObjectExpression', () => {
      it('empty', async () => {
        await expectTranslateExpression(`{}`, `{}`);
      });
      it('methods', async () => {
        await expectTranslateExpression(`{foo() {}}`, `{foo(): void}`);
        await expectTranslateExpression(`{1() {}}`, `{1(): void}`);
        await expectTranslateExpression(`{'foo'() {}}`, `{foo(): void}`);
        await expectTranslateExpression(`{get foo() {}}`, `{get foo(): void}`);
        await expectTranslateExpression(`{get 1() {}}`, `{get 1(): void}`);
        await expectTranslateExpression(
          `{get 'foo'() {}}`,
          `{get foo(): void}`,
        );
        await expectTranslateExpression(
          `{set foo(bar: string) {}}`,
          `{set foo(bar: string): void}`,
        );
        await expectTranslateExpression(
          `{set 1(bar: string) {}}`,
          `{set 1(bar: string): void}`,
        );
        await expectTranslateExpression(
          `{set 'foo'(bar: string) {}}`,
          `{set foo(bar: string): void}`,
        );
      });
      it('properties', async () => {
        await expectTranslateExpression(`{FOO: 1}`, `{FOO: 1}`);
        await expectTranslateExpression(`{'foo-bar': 1}`, `{'foo-bar': 1}`);
        await expectTranslateExpression(`{1: 1}`, `{1: 1}`);
      });
      it('spread', async () => {
        await expectTranslateExpression(`{...a}`, `{...a}`);
      });
    });
    describe('Literals', () => {
      it('number', async () => {
        await expectTranslateExpression(`1`, `1`);
        await expectTranslateExpression(`1.99`, `1.99`);
      });
      it('string', async () => {
        await expectTranslateExpression(`'s'`, `'s'`);
      });
      it('boolean', async () => {
        await expectTranslateExpression(`true`, `true`);
      });
      it('regex', async () => {
        await expectTranslateExpression(`/a/`, `RegExp`);
      });
      it('null', async () => {
        await expectTranslateExpression(`null`, `null`);
      });
    });
    describe('TypeCastExpression', () => {
      it('basic', async () => {
        await expectTranslateExpression(`(1: number)`, `number`);
      });
    });
    describe('AsExpression', () => {
      it('basic', async () => {
        await expectTranslateExpression(`1 as number`, `number`);
      });
    });
    describe('FunctionExpression', () => {
      it('basic', async () => {
        await expectTranslateExpression(`function foo() {}`, `() => void`);
        await expectTranslateExpression(
          `function foo<T>(baz: T, bar: string) {}`,
          `<T>(baz: T, bar: string) => void`,
        );
      });
    });
    describe('ArrowFunctionExpression', () => {
      it('basic', async () => {
        await expectTranslateExpression(`() => {}`, `() => void`);
        await expectTranslateExpression(
          `<T>(baz: T, bar: string) => {}`,
          `<T>(baz: T, bar: string) => void`,
        );
      });
    });
  });
  describe('ComponentDeclaration', () => {
    it('export', async () => {
      await expectTranslate(
        `export component Foo() {}`,
        `declare export component Foo();`,
      );
    });
    it('export default', async () => {
      await expectTranslate(
        `export default component Foo() {}`,
        `declare export default component Foo();`,
      );
    });
    it('params', async () => {
      await expectTranslate(
        `export component Foo(foo: string, 'bar' as BAR?: string) {}`,
        `declare export component Foo(foo: string, 'bar'?: string);`,
      );
    });
    it('default params', async () => {
      await expectTranslate(
        `export component Foo(foo: string = '') {}`,
        `declare export component Foo(foo?: string);`,
      );
    });
    it('rest param', async () => {
      await expectTranslate(
        `export component Foo(...foo: {...}) {}`,
        `declare export component Foo(...foo: {...});`,
      );
    });
    it('destructured rest param', async () => {
      await expectTranslate(
        `export component Foo(...{foo}: {...}) {}`,
        `declare export component Foo(...rest: {...});`,
      );
    });
    it('renders type', async () => {
      await expectTranslate(
        `type T = Bar;
         export component Foo() renders T {}`,
        `type T = Bar;
         declare export component Foo() renders T;`,
      );
    });
  });
});
