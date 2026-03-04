/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import {parseForESLint} from './eslint-scope-test-utils';

describe('ScopeManager#addGlobals', () => {
  it('adds variables to global scope and resolves global references', () => {
    const {scopeManager} = parseForESLint(`
      foo = bar + bar;
    `);

    const globalScope = scopeManager.scopes[0];

    expect(globalScope.type).toEqual('global');
    expect(globalScope.variables).toHaveLength(0);
    expect(globalScope.references).toHaveLength(3);
    expect(globalScope.through).toHaveLength(3);
    expect(globalScope.__implicit.variables.map(v => v.name)).toEqual(['foo']);
    expect(
      globalScope.__implicit.referencesLeftToResolve.map(
        ref => ref.identifier.name,
      ),
    ).toEqual(['foo', 'bar', 'bar']);

    scopeManager.addGlobals(['foo', 'bar']);

    expect(globalScope.variables).toHaveLength(2);
    expect(globalScope.set.get('foo')).toEqual(globalScope.variables[0]);
    expect(globalScope.set.get('bar')).toEqual(globalScope.variables[1]);
    expect(globalScope.references[0].resolved).toEqual(
      globalScope.variables[0],
    );
    expect(globalScope.references[1].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(globalScope.references[2].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(globalScope.variables[0].references).toHaveLength(1);
    expect(globalScope.variables[1].references).toHaveLength(2);
    expect(globalScope.through).toHaveLength(0);
    expect(globalScope.__implicit.variables).toHaveLength(0);
    expect(globalScope.__implicit.referencesLeftToResolve).toHaveLength(0);
  });

  it('adds variables to global scope and resolves inner-scope references', () => {
    const {scopeManager} = parseForESLint(`
      () => foo = bar + bar;
    `);

    const globalScope = scopeManager.scopes[0];
    const functionScope = scopeManager.scopes[1];

    expect(functionScope.type).toEqual('function');
    expect(functionScope.references).toHaveLength(3);
    expect(functionScope.references[0].resolved).toBeNull();
    expect(functionScope.references[1].resolved).toBeNull();
    expect(functionScope.references[2].resolved).toBeNull();
    expect(globalScope.through).toHaveLength(3);

    scopeManager.addGlobals(['foo', 'bar']);

    expect(globalScope.variables).toHaveLength(2);
    expect(functionScope.references[0].resolved).toEqual(
      globalScope.variables[0],
    );
    expect(functionScope.references[1].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(functionScope.references[2].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(globalScope.variables[0].references).toHaveLength(1);
    expect(globalScope.variables[1].references).toHaveLength(2);
    expect(globalScope.through).toHaveLength(0);
    expect(globalScope.__implicit.variables).toHaveLength(0);
    expect(globalScope.__implicit.referencesLeftToResolve).toHaveLength(0);
  });

  it('does not affect unrelated references', () => {
    const {scopeManager} = parseForESLint(`
      foo = bar + bar;
    `);

    const globalScope = scopeManager.scopes[0];

    scopeManager.addGlobals(['baz', 'qux']);

    expect(globalScope.variables).toHaveLength(2);
    expect(globalScope.references[0].resolved).toBeNull();
    expect(globalScope.references[1].resolved).toBeNull();
    expect(globalScope.references[2].resolved).toBeNull();
    expect(globalScope.through.map(ref => ref.identifier.name)).toEqual([
      'foo',
      'bar',
      'bar',
    ]);
    expect(globalScope.__implicit.variables.map(v => v.name)).toEqual(['foo']);
    expect(
      globalScope.__implicit.referencesLeftToResolve.map(
        ref => ref.identifier.name,
      ),
    ).toEqual(['foo', 'bar', 'bar']);
  });

  it('does not affect already declared globals', () => {
    const {scopeManager} = parseForESLint(`
      let foo = bar + bar;
      var bar;
    `);

    const globalScope = scopeManager.scopes[0];

    expect(globalScope.variables).toHaveLength(2);
    expect(globalScope.references[0].resolved).toEqual(
      globalScope.variables[0],
    );
    expect(globalScope.references[1].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(globalScope.references[2].resolved).toEqual(
      globalScope.variables[1],
    );

    scopeManager.addGlobals(['foo', 'bar']);

    expect(globalScope.variables).toHaveLength(2);
    expect(globalScope.references[0].resolved).toEqual(
      globalScope.variables[0],
    );
    expect(globalScope.references[1].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(globalScope.references[2].resolved).toEqual(
      globalScope.variables[1],
    );
    expect(globalScope.through).toHaveLength(0);
    expect(globalScope.__implicit.variables).toHaveLength(0);
    expect(globalScope.__implicit.referencesLeftToResolve).toHaveLength(0);
  });
});
