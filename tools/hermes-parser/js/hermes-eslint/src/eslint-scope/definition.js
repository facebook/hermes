/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {
  AFunction,
  CatchClause,
  ClassDeclaration,
  ClassExpression,
  DeclareClass,
  DeclareFunction,
  DeclareInterface,
  DeclareOpaqueType,
  DeclareTypeAlias,
  DeclareVariable,
  EnumDeclaration,
  ESNode as Node,
  FunctionDeclaration,
  FunctionExpression,
  Identifier,
  ImportDeclaration,
  ImportDefaultSpecifier,
  ImportNamespaceSpecifier,
  ImportSpecifier,
  InterfaceDeclaration,
  OpaqueType,
  TypeAlias,
  TypeParameter,
  VariableDeclaration,
  VariableDeclarator,
} from 'hermes-estree';

const DefinitionType = ({
  CatchClause: 'CatchClause',
  ClassName: 'ClassName',
  Enum: 'Enum',
  FunctionName: 'FunctionName',
  ImplicitGlobalVariable: 'ImplicitGlobalVariable',
  ImportBinding: 'ImportBinding',
  Parameter: 'Parameter',
  Type: 'Type',
  TypeParameter: 'TypeParameter',
  Variable: 'Variable',
}: $ReadOnly<{
  CatchClause: 'CatchClause',
  ClassName: 'ClassName',
  Enum: 'Enum',
  FunctionName: 'FunctionName',
  ImplicitGlobalVariable: 'ImplicitGlobalVariable',
  ImportBinding: 'ImportBinding',
  Parameter: 'Parameter',
  Type: 'Type',
  TypeParameter: 'TypeParameter',
  Variable: 'Variable',
}>);

class DefinitionBase {
  /**
   * Type of the occurrence (e.g. "Parameter", "Variable", ...).
   */
  +type: string;

  /**
   * The identifier AST node of the occurrence.
   */
  +name: Identifier;

  /**
   * The enclosing node of the identifier.
   */
  +node: Node;

  /**
   * The enclosing statement node of the identifier.
   */
  +parent: ?Node;

  /**
   * The index in the declaration statement.
   */
  +index: ?number;

  /**
   * The kind of the declaration statement.
   */
  +kind: ?string;

  constructor({
    type,
    name,
    node,
    parent,
    index,
    kind,
  }: {
    type: string,
    name: Identifier,
    node: Node,
    parent?: ?Node,
    index?: number,
    kind?: string,
  }) {
    this.type = type;
    this.name = name;
    this.node = node;
    this.parent = parent;
    this.index = index;
    this.kind = kind;
  }
}

class CatchClauseDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['CatchClause'];
  declare +node: CatchClause;
  declare +parent: null;

  constructor(idNode: Identifier, catchNode: CatchClauseDefinition['node']) {
    super({
      type: DefinitionType.CatchClause,
      name: idNode,
      node: catchNode,
    });
  }
}

class ClassNameDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['ClassName'];
  declare +node: ClassDeclaration | ClassExpression | DeclareClass;
  declare +parent: null;

  constructor(idNode: Identifier, classNode: ClassNameDefinition['node']) {
    super({
      type: DefinitionType.ClassName,
      name: idNode,
      node: classNode,
    });
  }
}

class EnumDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['Enum'];
  declare +node: EnumDeclaration;
  declare +parent: null;

  constructor(idNode: Identifier, enumDeclarationNode: EnumDefinition['node']) {
    super({
      type: DefinitionType.Enum,
      name: idNode,
      node: enumDeclarationNode,
    });
  }
}

class FunctionNameDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['FunctionName'];
  declare +node: FunctionDeclaration | FunctionExpression | DeclareFunction;
  declare +parent: null;

  constructor(
    idNode: Identifier,
    functionNode: FunctionNameDefinition['node'],
  ) {
    super({
      type: DefinitionType.FunctionName,
      name: idNode,
      node: functionNode,
    });
  }
}

class ImplicitGlobalVariableDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['ImplicitGlobalVariable'];
  declare +node: Node;
  declare +parent: null;

  constructor(
    idNode: Identifier,
    node: ImplicitGlobalVariableDefinition['node'],
  ) {
    super({
      type: DefinitionType.ImplicitGlobalVariable,
      name: idNode,
      node,
    });
  }
}

class ImportBindingDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['ImportBinding'];
  declare +node:
    | ImportSpecifier
    | ImportDefaultSpecifier
    | ImportNamespaceSpecifier;
  declare +parent: ImportDeclaration;

  constructor(
    idNode: Identifier,
    specifierNode: ImportBindingDefinition['node'],
    importDeclarationNode: ImportDeclaration,
  ) {
    super({
      type: DefinitionType.ImportBinding,
      name: idNode,
      node: specifierNode,
      parent: importDeclarationNode,
    });
  }
}

class ParameterDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['Parameter'];
  declare +node: AFunction;
  declare +parent: null;

  /**
   * Whether the parameter definition is a part of a rest parameter.
   */
  rest: boolean;

  constructor(
    idNode: Identifier,
    functionNode: ParameterDefinition['node'],
    index: number,
    rest: boolean,
  ) {
    super({
      type: DefinitionType.Parameter,
      name: idNode,
      node: functionNode,
      index,
    });

    this.rest = rest;
  }
}

class TypeDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['Type'];
  declare +node:
    | DeclareTypeAlias
    | DeclareOpaqueType
    | DeclareInterface
    | TypeAlias
    | OpaqueType
    | InterfaceDeclaration;
  declare +parent: null;

  constructor(idNode: Identifier, declNode: TypeDefinition['node']) {
    super({
      type: DefinitionType.Type,
      name: idNode,
      node: declNode,
    });
  }
}

class TypeParameterDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['TypeParameter'];
  declare +node: TypeParameter;
  declare +parent: null;

  constructor(typeParamNode: TypeParameterDefinition['node']) {
    // The ScopeManager API expects an Identifier node that can be referenced
    // for each definition. TypeParameter nodes do not actually contain an
    // Identifier node, so we create a fake one with the correct name,
    // location, and parent so that it is still usable with the ScopeManager.
    const id = {
      type: 'Identifier',
      loc: typeParamNode.loc,
      name: typeParamNode.name,
      parent: typeParamNode,
      range: typeParamNode.range,
      optional: false,
      typeAnnotation: null,
    };
    super({
      type: DefinitionType.TypeParameter,
      name: id,
      node: typeParamNode,
    });
  }
}

class VariableDefinition extends DefinitionBase {
  declare +type: (typeof DefinitionType)['Variable'];
  declare +node: DeclareVariable | VariableDeclarator;
  declare +parent: DeclareVariable | VariableDeclaration;

  constructor(
    idNode: Identifier,
    declaratorNode: VariableDefinition['node'],
    declarationNode: VariableDefinition['parent'],
    index: number,
    kind: string,
  ) {
    super({
      type: DefinitionType.Variable,
      name: idNode,
      node: declaratorNode,
      parent: declarationNode,
      index,
      kind,
    });
  }
}

export type Definition =
  | CatchClauseDefinition
  | ClassNameDefinition
  | EnumDefinition
  | FunctionNameDefinition
  | ImplicitGlobalVariableDefinition
  | ImportBindingDefinition
  | ParameterDefinition
  | TypeDefinition
  | TypeParameterDefinition
  | VariableDefinition;

module.exports = {
  CatchClauseDefinition,
  ClassNameDefinition,
  DefinitionType,
  EnumDefinition,
  FunctionNameDefinition,
  ImplicitGlobalVariableDefinition,
  ImportBindingDefinition,
  ParameterDefinition,
  TypeDefinition,
  TypeParameterDefinition,
  VariableDefinition,
};
