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

import type {
  DeclareVariable,
  VariableDeclarator,
  VariableDeclaration,
  Identifier,
} from 'hermes-estree';

import {DefinitionType} from './DefinitionType';
import {DefinitionBase} from './DefinitionBase';

class VariableDefinition extends DefinitionBase<
  typeof DefinitionType.Variable,
  DeclareVariable | VariableDeclarator,
  DeclareVariable | VariableDeclaration,
  Identifier,
> {
  declare +type: typeof DefinitionType.Variable;

  constructor(
    name: Identifier,
    node: VariableDefinition['node'],
    decl: DeclareVariable | VariableDeclaration,
  ) {
    super(DefinitionType.Variable, name, node, decl);
  }

  +isTypeDefinition: boolean = false;
  +isVariableDefinition: boolean = true;
}

export {VariableDefinition};
