/**
 * Portions Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

/*
  Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
'use strict';

import type {Identifier, Node} from './ScopeManagerTypes';

const Variable = require('./variable');

const DefinitionType = {
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
};

class Definition {
  /**
   * Type of the occurrence (e.g. "Parameter", "Variable", ...).
   */
  type: string;

  /**
   * The identifier AST node of the occurrence.
   */
  name: Identifier;

  /**
   * The enclosing node of the identifier.
   */
  node: Node;

  /**
   * The enclosing statement node of the identifier.
   */
  parent: ?Node;

  /**
   * The index in the declaration statement.
   */
  index: ?number;

  /**
   * The kind of the declaration statement.
   */
  kind: ?string;

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

class CatchClauseDefinition extends Definition {
  constructor(catchNode: Node) {
    super({
      type: DefinitionType.CatchClause,
      name: catchNode.param,
      node: catchNode,
    });
  }
}

class ClassNameDefinition extends Definition {
  constructor(classNode: Node) {
    super({
      type: DefinitionType.ClassName,
      name: classNode.id,
      node: classNode,
    });
  }
}

class EnumDefinition extends Definition {
  constructor(enumDeclarationNode: Node) {
    super({
      type: DefinitionType.Enum,
      name: enumDeclarationNode.id,
      node: enumDeclarationNode,
    });
  }
}

class FunctionNameDefinition extends Definition {
  constructor(functionNode: Node) {
    super({
      type: DefinitionType.FunctionName,
      name: functionNode.id,
      node: functionNode,
    });
  }
}

class ImplicitGlobalVariableDefinition extends Definition {
  constructor(idNode: Identifier, node: Node) {
    super({
      type: DefinitionType.ImplicitGlobalVariable,
      name: idNode,
      node,
    });
  }
}

class ImportBindingDefinition extends Definition {
  constructor(
    idNode: Identifier,
    specifierNode: Node,
    importDeclarationNode: Node,
  ) {
    super({
      type: DefinitionType.ImportBinding,
      name: idNode,
      node: specifierNode,
      parent: importDeclarationNode,
    });
  }
}

class ParameterDefinition extends Definition {
  /**
   * Whether the parameter definition is a part of a rest parameter.
   */
  rest: boolean;

  constructor(
    idNode: Identifier,
    functionNode: Node,
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

class TypeDefinition extends Definition {
  constructor(idNode: Identifier, declNode: Node) {
    super({
      type: DefinitionType.Type,
      name: idNode,
      node: declNode,
    });
  }
}

class TypeParameterDefinition extends Definition {
  constructor(typeParamNode: Node) {
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
    };
    super({
      type: DefinitionType.TypeParameter,
      name: id,
      node: typeParamNode,
    });
  }
}

class VariableDefinition extends Definition {
  constructor(
    idNode: Identifier,
    declaratorNode: Node,
    declarationNode: Node,
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

module.exports = {
  CatchClauseDefinition,
  ClassNameDefinition,
  Definition,
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
