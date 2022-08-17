/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

const ScopeType = ({
  Block: 'block',
  Catch: 'catch',
  Class: 'class',
  ClassFieldInitializer: 'class-field-initializer',
  ClassStaticBlock: 'class-static-block',
  DeclareModule: 'declare-module',
  For: 'for',
  Function: 'function',
  FunctionExpressionName: 'function-expression-name',
  Global: 'global',
  Module: 'module',
  Switch: 'switch',
  Type: 'type',
  With: 'with',
}: $ReadOnly<{
  Block: 'block',
  Catch: 'catch',
  Class: 'class',
  ClassFieldInitializer: 'class-field-initializer',
  ClassStaticBlock: 'class-static-block',
  DeclareModule: 'declare-module',
  For: 'for',
  Function: 'function',
  FunctionExpressionName: 'function-expression-name',
  Global: 'global',
  Module: 'module',
  Switch: 'switch',
  Type: 'type',
  With: 'with',
}>);
type ScopeTypeType = $Values<typeof ScopeType>;

export type {ScopeTypeType};
export {ScopeType};
