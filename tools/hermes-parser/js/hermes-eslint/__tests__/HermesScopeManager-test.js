/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const {parseForESLint} = require('hermes-eslint');
const {DefinitionType} = require('../dist/eslint-scope/definition');

describe('Source type option', () => {
  test('script', () => {
    const {ast, scopeManager} = parseForESLint('Foo', {sourceType: 'script'});

    expect(ast.sourceType).toEqual('script');
    expect(scopeManager.scopes).toHaveLength(1);
    expect(scopeManager.scopes[0].type).toEqual('global');
  });

  test('module', () => {
    const {ast, scopeManager} = parseForESLint('Foo', {sourceType: 'module'});

    expect(ast.sourceType).toEqual('module');
    expect(scopeManager.scopes).toHaveLength(2);
    expect(scopeManager.scopes[0].type).toEqual('global');
    expect(scopeManager.scopes[1].type).toEqual('module');
  });
});

describe('Type and value references', () => {
  function verifyValueAndTypeReferences(code, name, definitionType) {
    const {scopeManager} = parseForESLint(code, {sourceType: 'module'});

    // Verify that scope contains a single variable
    const scope = scopeManager.scopes[1];
    expect(scope.variables).toHaveLength(1);
    const variable = scope.variables[0];

    // Verify that variable has correct name and definition type
    expect(variable.name).toEqual(name);
    expect(variable.defs).toHaveLength(1);
    expect(variable.defs[0].type).toEqual(definitionType);

    // Variable has both a value and type reference
    expect(variable.references).toHaveLength(2);
    expect(variable.references[0].isValueReference()).toBe(true);
    expect(variable.references[1].isTypeReference()).toBe(true);
  }

  verifyValueAndTypeReferences(
    `
      class C {}
      (C: Class<C>);
    `,
    'C',
    DefinitionType.ClassName,
  );
  verifyValueAndTypeReferences(
    `
      import V from 'V';
      (V: V);
    `,
    'V',
    DefinitionType.ImportBinding,
  );
  verifyValueAndTypeReferences(
    `
      enum E {
        A
      }
      (E.A: E);
    `,
    'E',
    DefinitionType.Enum,
  );
});

describe('Type definitions', () => {
  function verifyTypeDefinition(scopeManager) {
    // Verify there is a single scope, variable, and reference
    expect(scopeManager.scopes).toHaveLength(1);

    const scope = scopeManager.scopes[0];
    expect(scope.variables).toHaveLength(1);
    expect(scope.references).toHaveLength(1);

    const variable = scope.variables[0];
    const reference = scope.references[0];
    expect(variable.name).toEqual('T');

    // Verify that reference is resolved
    expect(variable.references).toHaveLength(1);
    expect(variable.references[0]).toBe(reference);
    expect(reference.resolved).toBe(variable);
    expect(reference.isTypeReference()).toBe(true);

    // Verify there is one TypeDefinition
    expect(variable.defs).toHaveLength(1);
    expect(variable.defs[0].type).toEqual(DefinitionType.Type);
  }

  test('TypeAlias', () => {
    const {scopeManager} = parseForESLint(`
      type T = number;
      (1: T);
    `);
    verifyTypeDefinition(scopeManager);
  });

  test('OpaqueType', () => {
    const {scopeManager} = parseForESLint(`
      opaque type T = number;
      (1: T);
    `);
    verifyTypeDefinition(scopeManager);
  });

  test('InterfaceDeclaration', () => {
    const {scopeManager} = parseForESLint(`
      interface T {}
      (1: T);
    `);
    verifyTypeDefinition(scopeManager);
  });
});

describe('Enums', () => {
  test('Definition', () => {
    const {scopeManager} = parseForESLint(`
      enum E {}
      E;
    `);

    // Verify there is a single scope, variable, and reference
    expect(scopeManager.scopes).toHaveLength(1);

    const scope = scopeManager.scopes[0];
    expect(scope.variables).toHaveLength(1);
    expect(scope.references).toHaveLength(1);

    const variable = scope.variables[0];
    const reference = scope.references[0];
    expect(variable.name).toEqual('E');

    // Verify that reference is resolved
    expect(variable.references).toHaveLength(1);
    expect(variable.references[0]).toBe(reference);
    expect(reference.resolved).toBe(variable);
    expect(reference.isValueReference()).toBe(true);

    // Verify there is one Enum definition
    expect(variable.defs).toHaveLength(1);
    expect(variable.defs[0].type).toEqual(DefinitionType.Enum);
  });

  test('No references in body', () => {
    const {scopeManager} = parseForESLint(`
      class Foo {};
      enum E {
        Foo
      }
    `);

    // Verify that scope contains variable 'Foo'
    const scope = scopeManager.scopes[0];
    expect(scope.variables).toHaveLength(2);
    expect(scope.variables[0].name).toEqual('Foo');

    // But enum member does not count as reference to 'Foo'
    expect(scope.references).toHaveLength(0);
    expect(scope.variables[0].references).toHaveLength(0);
  });
});
