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

import type {
  DefinitionTypeType,
  ParseForESLintOptions,
  ScopeTypeType,
} from '../src';

import {parseForESLint} from '../src';

/**
 * Utility to check that scope manager produces correct scopes and variables.
 *
 * Scopes are passed as an array of objects, starting with the module scope,
 * where each object has a scope type and array of variables, Each variable is
 * an object with a name, optional reference count, and optional definition type.
 */
export function verifyHasScopes(
  code: string,
  expectedScopes: $ReadOnlyArray<{
    type: ScopeTypeType,
    variables: $ReadOnlyArray<{
      name: string,
      type: ?DefinitionTypeType,
      referenceCount: ?number,
      eslintUsed?: boolean,
    }>,
  }>,
  parserOptions?: ParseForESLintOptions,
) {
  const {scopeManager} = parseForESLint(code, parserOptions);

  // report as an array so that it's easier to debug the tests
  // otherwise you get a cryptic failure that just says "expected 1 but received 2"
  expect(scopeManager.scopes.map(s => s.type)).toEqual([
    // Global scope (at index 0 of actual scopes) is not passed as an expected scope
    'global',
    ...expectedScopes.map(s => s.type),
  ]);

  for (let i = 0; i < expectedScopes.length; i++) {
    const actualScope = scopeManager.scopes[i + 1];
    const expectedScope = expectedScopes[i];

    expect(actualScope.type).toEqual(expectedScope.type);
    // report as an object so that it's easier to debug the tests
    expect({
      type: actualScope.type,
      variables: actualScope.variables.map(v => v.name),
    }).toEqual({
      type: actualScope.type,
      variables: expectedScope.variables.map(v => v.name),
    });

    for (let j = 0; j < expectedScope.variables.length; j++) {
      const expectedVariable = expectedScope.variables[j];
      const actualVariable = actualScope.variables[j];

      expect(actualVariable.name).toEqual(expectedVariable.name);

      if (expectedVariable.referenceCount != null) {
        const cnt = expectedVariable.referenceCount;
        // report as an object so that it's easier to debug the tests
        expect({
          type: expectedVariable.type,
          name: actualVariable.name,
          refCount: actualVariable.references.length,
        }).toEqual({
          type: expectedVariable.type,
          name: actualVariable.name,
          refCount: cnt,
        });
      }

      if (expectedVariable.type != null) {
        expect(actualVariable.defs).toHaveLength(1);
        expect(actualVariable.defs[0].type).toEqual(expectedVariable.type);
      }

      if (expectedVariable.eslintUsed != null) {
        expect(actualVariable.eslintUsed).toBe(expectedVariable.eslintUsed);
      }
    }
  }
}
