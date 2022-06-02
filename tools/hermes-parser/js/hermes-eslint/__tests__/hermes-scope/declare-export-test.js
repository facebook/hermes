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

import {DefinitionType, ScopeType} from '../../src';
import {verifyHasScopes} from '../../__test_utils__/verifyHasScopes';

describe('declare export', () => {
  it('variable', () => {
    verifyHasScopes(
      `
        declare export var foo: string;
      `,
      [
        {
          type: ScopeType.Module,
          variables: [
            {
              name: 'foo',
              type: DefinitionType.Variable,
              referenceCount: 0,
              eslintUsed: true,
            },
          ],
        },
      ],
    );
  });
  it('function', () => {
    verifyHasScopes(
      `
        declare export function foo(): void;
      `,
      [
        {
          type: ScopeType.Module,
          variables: [
            {
              name: 'foo',
              type: DefinitionType.FunctionName,
              referenceCount: 0,
              eslintUsed: true,
            },
          ],
        },
      ],
    );
  });
  it('type', () => {
    // you can only declare export types within a module
    verifyHasScopes(
      `
        declare module mod {
          declare export type foo = 1;
        }
        export {};
      `,
      [
        {
          type: ScopeType.Module,
          variables: [],
        },
        {
          type: ScopeType.DeclareModule,
          variables: [
            {
              name: 'foo',
              type: DefinitionType.Type,
              referenceCount: 0,
              eslintUsed: true,
            },
          ],
        },
      ],
    );
  });
  it('opaque type', () => {
    verifyHasScopes(
      `
        declare export opaque type foo: 1;
      `,
      [
        {
          type: ScopeType.Module,
          variables: [
            {
              name: 'foo',
              type: DefinitionType.Type,
              referenceCount: 0,
              eslintUsed: true,
            },
          ],
        },
      ],
    );
  });
  it('interface', () => {
    verifyHasScopes(
      `
        declare export interface foo {};
      `,
      [
        {
          type: ScopeType.Module,
          variables: [
            {
              name: 'foo',
              type: DefinitionType.Type,
              referenceCount: 0,
              eslintUsed: true,
            },
          ],
        },
      ],
    );
  });
});
