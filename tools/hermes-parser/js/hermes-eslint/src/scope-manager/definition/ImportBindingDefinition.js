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
  ImportSpecifier,
  ImportDefaultSpecifier,
  ImportNamespaceSpecifier,
  ImportDeclaration,
  Identifier,
} from 'hermes-estree';

import {DefinitionType} from './DefinitionType';
import {DefinitionBase} from './DefinitionBase';

class ImportBindingDefinition extends DefinitionBase<
  typeof DefinitionType.ImportBinding,
  ImportSpecifier | ImportDefaultSpecifier | ImportNamespaceSpecifier,
  ImportDeclaration,
  Identifier,
> {
  declare +type: typeof DefinitionType.ImportBinding;

  constructor(
    name: Identifier,
    node: ImportBindingDefinition['node'],
    decl: ImportDeclaration,
  ) {
    super(DefinitionType.ImportBinding, name, node, decl);
    switch (node.type) {
      case 'ImportSpecifier':
        if (
          node.importKind === 'type' ||
          node.importKind === 'typeof' ||
          decl.importKind === 'type' ||
          decl.importKind === 'typeof'
        ) {
          this.isTypeDefinition = true;
          this.isVariableDefinition = false;
        } else {
          this.isTypeDefinition = false;
          this.isVariableDefinition = true;
        }
        break;

      case 'ImportDefaultSpecifier':
        if (decl.importKind === 'type' || decl.importKind === 'typeof') {
          this.isTypeDefinition = true;
          this.isVariableDefinition = false;
        } else {
          this.isTypeDefinition = false;
          this.isVariableDefinition = true;
        }
        break;

      case 'ImportNamespaceSpecifier':
        // not possible for a namespace import to be a type in flow
        this.isTypeDefinition = false;
        this.isVariableDefinition = true;
        break;
    }
  }

  +isTypeDefinition: boolean;
  +isVariableDefinition: boolean;
}

export {ImportBindingDefinition};
