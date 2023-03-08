/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {MapperOptions} from '../src/flowImportTo';

// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../.prettierrc.json';
import {parse, print} from 'hermes-transform';
import {trimToBeCode} from './utils/inlineCodeHelpers';
import {flowImportTo} from '../src/flowImportTo';

function translate(code: string, opts: MapperOptions): string {
  const {ast, scopeManager} = parse(code);

  const flowDefAst = flowImportTo(ast, code, scopeManager, opts);

  return print(flowDefAst, code, prettierConfig);
}

function expectModules(expectCode: string, toBeModules: Array<string>): void {
  const foundModules = [];
  translate(expectCode, {
    sourceMapper({module}) {
      foundModules.push(module);
      return module;
    },
  });
  expect(foundModules).toEqual(toBeModules);
}
function expectTranslate(
  expectCode: string,
  opts: MapperOptions,
  toBeCode: string,
): void {
  expect(translate(expectCode, opts)).toBe(trimToBeCode(toBeCode));
}

describe('flowImportTo', () => {
  it('ImportDeclaration', () => {
    expectModules(
      `import A from 'X1';
       import * as A from 'X2';
       import {A} from 'X3';
       import type {A} from 'X4';
       import typeof {A} from 'X5';`,
      ['X1', 'X2', 'X3', 'X4', 'X5'],
    );
    expectTranslate(
      `import A from 'X1';`,
      {sourceMapper: _ => 'A'},
      `import A from 'A';`,
    );
  });
  it('ExportNamedDeclaration', () => {
    expectModules(`export {A} from 'X1';`, ['X1']);
  });
  it('DeclareExportNamedDeclaration', () => {
    expectModules(`declare export {A} from 'X1';`, ['X1']);
  });
  it('ExportAllDeclaration', () => {
    expectModules(`export * from 'X1';`, ['X1']);
  });
  it('DeclareExportAllDeclaration', () => {
    expectModules(`declare export * from 'X1';`, ['X1']);
  });
});
