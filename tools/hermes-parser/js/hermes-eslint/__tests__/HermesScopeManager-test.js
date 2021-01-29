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
const {ScopeType} = require('../dist/eslint-scope/scope');

/**
 * Utility to check that scope manager produces correct scopes and variables.
 *
 * Scopes are passed as an array of objects, starting with the module scope,
 * where each object has a scope type and array of variables, Each variable is
 * an object with a name, optional reference count, and optional definition type.
 */
function verifyHasScopes(code, expectedScopes) {
  const {scopeManager} = parseForESLint(code);

  for (let i = 0; i < expectedScopes.length; i++) {
    // Skip global scope at index 0 and start at module scope at index 1
    const actualScope = scopeManager.scopes[i + 1];
    const expectedScope = expectedScopes[i];

    expect(actualScope.type).toEqual(expectedScope.type);
    expect(actualScope.variables).toHaveLength(expectedScope.variables.length);

    for (let j = 0; j < expectedScope.variables.length; j++) {
      const expectedVariable = expectedScope.variables[j];
      const actualVariable = actualScope.variables[j];

      expect(actualVariable.name).toEqual(expectedVariable.name);

      if (expectedVariable.referenceCount != null) {
        expect(actualVariable.references).toHaveLength(
          expectedVariable.referenceCount,
        );
      }

      if (expectedVariable.type != null) {
        expect(actualVariable.defs).toHaveLength(1);
        expect(actualVariable.defs[0].type).toEqual(expectedVariable.type);
      }
    }
  }
}

describe('Source type option', () => {
  test('script', () => {
    const {ast, scopeManager} = parseForESLint('Foo', {sourceType: 'script'});

    expect(ast.sourceType).toEqual('script');
    expect(scopeManager.scopes).toHaveLength(1);
    expect(scopeManager.scopes[0].type).toEqual(ScopeType.Global);
  });

  test('module', () => {
    const {ast, scopeManager} = parseForESLint('Foo', {sourceType: 'module'});

    expect(ast.sourceType).toEqual('module');
    expect(scopeManager.scopes).toHaveLength(2);
    expect(scopeManager.scopes[0].type).toEqual(ScopeType.Global);
    expect(scopeManager.scopes[1].type).toEqual(ScopeType.Module);
  });

  test('defaults to module', () => {
    const {ast, scopeManager} = parseForESLint('Foo');

    expect(ast.sourceType).toEqual('module');
    expect(scopeManager.scopes).toHaveLength(2);
    expect(scopeManager.scopes[0].type).toEqual(ScopeType.Global);
    expect(scopeManager.scopes[1].type).toEqual(ScopeType.Module);
  });
});

describe('Type and value references', () => {
  function verifyValueAndTypeReferences(code, name, definitionType) {
    const {scopeManager} = parseForESLint(code);

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
    // Verify there is a module scope, variable, and reference
    expect(scopeManager.scopes).toHaveLength(2);

    const scope = scopeManager.scopes[1];
    expect(scope.type).toEqual(ScopeType.Module);
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

    // Verify there is a module scope, variable, and reference
    expect(scopeManager.scopes).toHaveLength(2);

    const scope = scopeManager.scopes[1];
    expect(scope.type).toEqual(ScopeType.Module);
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
});

describe('QualifiedTypeIdentifier', () => {
  test('References value', () => {
    const {scopeManager} = parseForESLint(`
      import * as Foo from 'Foo';
      (1: Foo.Bar);
    `);

    // Verify that scope contains single value reference to 'Foo'
    const scope = scopeManager.scopes[1];
    expect(scope.variables).toHaveLength(1);

    const variable = scope.variables[0];
    expect(variable.name).toEqual('Foo');
    expect(variable.references).toHaveLength(1);
    expect(variable.references[0].isValueReference()).toBe(true);
  });
});

describe('Identifiers not mistakenly treated as references', () => {
  function verifyHasReferences(code, references) {
    const {scopeManager} = parseForESLint(code);

    // Module scope should contain variables with the given reference counts
    const scope = scopeManager.scopes[1];
    expect(scope.variables).toHaveLength(references.length);

    for (let i = 0; i < references.length; i++) {
      const {name, count} = references[i];
      const variable = scope.variables[i];

      expect(variable.name).toEqual(name);
      expect(variable.references).toHaveLength(count);
    }
  }

  test('Enum body', () => {
    verifyHasReferences(
      `
        import Foo from 'Foo';
        enum E {
          Foo
        }
      `,
      [
        {name: 'Foo', count: 0},
        {name: 'E', count: 0},
      ],
    );
  });

  test('QualifiedTypeIdentifier', () => {
    verifyHasReferences(
      `
        import * as Foo from 'Foo';
        import Bar from 'Bar';
        import Baz from 'Baz';

        (1: Foo.Bar.Baz);
      `,
      [
        {name: 'Foo', count: 1},
        {name: 'Bar', count: 0},
        {name: 'Baz', count: 0},
      ],
    );
  });

  test('ObjectTypeProperty', () => {
    verifyHasReferences(
      `
        import Foo from 'Foo';
        import Bar from 'Bar';
        (1: { Foo: Bar });
      `,
      [
        {name: 'Foo', count: 0},
        {name: 'Bar', count: 1},
      ],
    );
  });

  test('ObjectTypeIndexer', () => {
    verifyHasReferences(
      `
        import Foo from 'Foo';
        import Bar from 'Bar';
        (1: { [Foo: Bar]: string });
      `,
      [
        {name: 'Foo', count: 0},
        {name: 'Bar', count: 1},
      ],
    );
  });

  test('ObjectTypeInternalSlot', () => {
    verifyHasReferences(
      `
        import Foo from 'Foo';
        import Bar from 'Bar';
        (1: { [[Foo]]: Bar });
      `,
      [
        {name: 'Foo', count: 0},
        {name: 'Bar', count: 1},
      ],
    );
  });

  test('FunctionTypeParam', () => {
    verifyHasReferences(
      `
        import Foo from 'Foo';
        import Bar from 'Bar';
        (1: (Foo: Bar) => void);
      `,
      [
        {name: 'Foo', count: 0},
        {name: 'Bar', count: 1},
      ],
    );
  });
});

describe('Type parameters', () => {
  test('Definition creates Identifier node', () => {
    const {scopeManager} = parseForESLint(`type Foo<T> = T;`);

    // Type parameter defined in type scope
    const scope = scopeManager.scopes[2];
    expect(scope.type).toEqual(ScopeType.Type);

    // Definition contains Identifier with correct name and location
    const id = scope.variables[0].defs[0].name;
    expect(id.name).toEqual('T');
    expect(id.loc).toMatchObject({
      start: {
        line: 1,
        column: 9,
      },
      end: {
        line: 1,
        column: 10,
      },
    });
    expect(id.parent.type).toEqual('TypeParameter');
  });

  test('TypeScope not created if there are no type parameters', () => {
    const {scopeManager} = parseForESLint(`type T = T;`);

    expect(scopeManager.scopes).toHaveLength(2);
    expect(scopeManager.scopes[0].type).toEqual(ScopeType.Global);
    expect(scopeManager.scopes[1].type).toEqual(ScopeType.Module);
  });

  test('TypeAlias', () => {
    // Type alias contains type parameter in Type scope
    verifyHasScopes(`type Foo<T> = T;`, [
      {
        type: ScopeType.Module,
        variables: [
          {
            name: 'Foo',
            type: DefinitionType.Type,
            referenceCount: 0,
          },
        ],
      },
      {
        type: ScopeType.Type,
        variables: [
          {
            name: 'T',
            type: DefinitionType.TypeParameter,
            referenceCount: 1,
          },
        ],
      },
    ]);
  });

  test('OpaqueType', () => {
    // Opaque type contains type parameter in Type scope
    verifyHasScopes(`opaque type Foo<T> = T;`, [
      {
        type: ScopeType.Module,
        variables: [
          {
            name: 'Foo',
            type: DefinitionType.Type,
            referenceCount: 0,
          },
        ],
      },
      {
        type: ScopeType.Type,
        variables: [
          {
            name: 'T',
            type: DefinitionType.TypeParameter,
            referenceCount: 1,
          },
        ],
      },
    ]);
  });

  test('InterfaceDeclaration', () => {
    // Interface declaration contains type parameter in Type scope
    verifyHasScopes(`interface Foo<T> { prop: T }`, [
      {
        type: ScopeType.Module,
        variables: [
          {
            name: 'Foo',
            type: DefinitionType.Type,
            referenceCount: 0,
          },
        ],
      },
      {
        type: ScopeType.Type,
        variables: [
          {
            name: 'T',
            type: DefinitionType.TypeParameter,
            referenceCount: 1,
          },
        ],
      },
    ]);
  });

  test('FunctionTypeAnnotation', () => {
    // FunctionTypeAnnotation contains type parameter in Type scope
    verifyHasScopes(`(1: <T>(T) => void);`, [
      {
        type: ScopeType.Module,
        variables: [],
      },
      {
        type: ScopeType.Type,
        variables: [
          {
            name: 'T',
            type: DefinitionType.TypeParameter,
            referenceCount: 1,
          },
        ],
      },
    ]);
  });

  test('Function', () => {
    // Function contains type parameter in Function scope alongside value parameter
    verifyHasScopes(
      `
        function foo<T>(x) {
          (x: T);
        }
      `,
      [
        {
          type: ScopeType.Module,
          variables: [
            {
              name: 'foo',
              type: DefinitionType.Function,
              referenceCount: 0,
            },
          ],
        },
        {
          type: ScopeType.Function,
          variables: [
            {
              name: 'arguments',
              referenceCount: 0,
            },
            {
              name: 'T',
              type: DefinitionType.TypeParameter,
              referenceCount: 1,
            },
            {
              name: 'x',
              type: DefinitionType.Parameter,
              referenceCount: 1,
            },
          ],
        },
      ],
    );
  });

  test('Class', () => {
    // Class contains type parameter in Class scope
    verifyHasScopes(
      `
        class C<T> {
          prop: T;
        }
      `,
      [
        {
          type: ScopeType.Module,
          variables: [
            {
              name: 'C',
              type: DefinitionType.Class,
              referenceCount: 0,
            },
          ],
        },
        {
          type: ScopeType.Class,
          variables: [
            {
              name: 'C',
              type: DefinitionType.Class,
              referenceCount: 0,
            },
            {
              name: 'T',
              type: DefinitionType.TypeParameter,
              referenceCount: 1,
            },
          ],
        },
      ],
    );
  });
});
