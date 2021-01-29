/**
 * Portions Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
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
  constructor({type, name, node, parent, index, kind}) {
    /**
     * @member {String} Definition#type - type of the occurrence (e.g. "Parameter", "Variable", ...).
     */
    this.type = type;

    /**
     * @member {espree.Identifier} Definition#name - the identifier AST node of the occurrence.
     */
    this.name = name;

    /**
     * @member {espree.Node} Definition#node - the enclosing node of the identifier.
     */
    this.node = node;

    /**
     * @member {espree.Node?} Definition#parent - the enclosing statement node of the identifier.
     */
    this.parent = parent;

    /**
     * @member {Number?} Definition#index - the index in the declaration statement.
     */
    this.index = index;

    /**
     * @member {String?} Definition#kind - the kind of the declaration statement.
     */
    this.kind = kind;
  }
}

class CatchClauseDefinition extends Definition {
  constructor(catchNode) {
    super({
      type: DefinitionType.CatchClause,
      name: catchNode.param,
      node: catchNode,
    });
  }
}

class ClassNameDefinition extends Definition {
  constructor(classNode) {
    super({
      type: DefinitionType.ClassName,
      name: classNode.id,
      node: classNode,
    });
  }
}

class EnumDefinition extends Definition {
  constructor(enumDeclarationNode) {
    super({
      type: DefinitionType.Enum,
      name: enumDeclarationNode.id,
      node: enumDeclarationNode,
    });
  }
}

class FunctionNameDefinition extends Definition {
  constructor(functionNode) {
    super({
      type: DefinitionType.FunctionName,
      name: functionNode.id,
      node: functionNode,
    });
  }
}

class ImplicitGlobalVariableDefinition extends Definition {
  constructor(idNode, node) {
    super({
      type: DefinitionType.ImplicitGlobalVariable,
      name: idNode,
      node,
    });
  }
}

class ImportBindingDefinition extends Definition {
  constructor(idNode, specifierNode, importDeclarationNode) {
    super({
      type: DefinitionType.ImportBinding,
      name: idNode,
      node: specifierNode,
      parent: importDeclarationNode,
    });
  }
}

class ParameterDefinition extends Definition {
  constructor(idNode, functionNode, index, rest) {
    super({
      type: DefinitionType.Parameter,
      name: idNode,
      node: functionNode,
      index,
    });

    /**
     * Whether the parameter definition is a part of a rest parameter.
     * @member {boolean} ParameterDefinition#rest
     */
    this.rest = rest;
  }
}

class TypeDefinition extends Definition {
  constructor(idNode, declNode) {
    super({
      type: DefinitionType.Type,
      name: idNode,
      node: declNode,
    });
  }
}

class TypeParameterDefinition extends Definition {
  constructor(typeParamNode) {
    // The ScopeManager API expects an Identifier node that can be referenced
    // for each definition. TypeParameter nodes do not actually contain an
    // Identifier node, so we create a fake one with the correct name,
    // location, and parent so that it is still usable with the ScopeManager.
    const id = {
      type: 'Identifier',
      loc: typeParamNode.loc,
      name: typeParamNode.name,
      parent: typeParamNode,
    };
    super({
      type: DefinitionType.TypeParameter,
      name: id,
      node: typeParamNode,
    });
  }
}

class VariableDefinition extends Definition {
  constructor(idNode, declaratorNode, declarationNode, index, kind) {
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
