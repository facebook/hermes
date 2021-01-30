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

const Syntax = require('estraverse').Syntax;
const esrecurse = require('esrecurse');
const Reference = require('./reference');
const Variable = require('./variable');
const PatternVisitor = require('./pattern-visitor');
const {
  CatchClauseDefinition,
  ClassNameDefinition,
  DefinitionType,
  EnumDefinition,
  FunctionNameDefinition,
  ImportBindingDefinition,
  ParameterDefinition,
  TypeDefinition,
  TypeParameterDefinition,
  VariableDefinition,
} = require('./definition');
const assert = require('assert');

// Importing ImportDeclaration.
// http://people.mozilla.org/~jorendorff/es6-draft.html#sec-moduledeclarationinstantiation
// https://github.com/estree/estree/blob/master/es6.md#importdeclaration
// FIXME: Now, we don't create module environment, because the context is
// implementation dependent.

class Importer extends esrecurse.Visitor {
  constructor(declaration, referencer) {
    super(null, referencer.options);
    this.declaration = declaration;
    this.referencer = referencer;
  }

  visitImport(id, specifier) {
    this.referencer.visitPattern(id, pattern => {
      this.referencer
        .currentScope()
        .__define(
          pattern,
          new ImportBindingDefinition(pattern, specifier, this.declaration),
        );
    });
  }

  ImportNamespaceSpecifier(node) {
    const local = node.local || node.id;

    if (local) {
      this.visitImport(local, node);
    }
  }

  ImportDefaultSpecifier(node) {
    const local = node.local || node.id;

    this.visitImport(local, node);
  }

  ImportSpecifier(node) {
    const local = node.local || node.id;

    if (node.name) {
      this.visitImport(node.name, node);
    } else {
      this.visitImport(local, node);
    }
  }
}

// Referencing variables and creating bindings.
class Referencer extends esrecurse.Visitor {
  constructor(options, scopeManager) {
    super(null, options);
    this.options = options;
    this.scopeManager = scopeManager;
    this.parent = null;
    this.isInnerMethodDefinition = false;
  }

  currentScope() {
    return this.scopeManager.__currentScope;
  }

  close(node) {
    while (this.currentScope() && node === this.currentScope().block) {
      this.scopeManager.__currentScope = this.currentScope().__close(
        this.scopeManager,
      );
    }
  }

  pushInnerMethodDefinition(isInnerMethodDefinition) {
    const previous = this.isInnerMethodDefinition;

    this.isInnerMethodDefinition = isInnerMethodDefinition;
    return previous;
  }

  popInnerMethodDefinition(isInnerMethodDefinition) {
    this.isInnerMethodDefinition = isInnerMethodDefinition;
  }

  referencingDefaultValue(pattern, assignments, maybeImplicitGlobal, init) {
    const scope = this.currentScope();

    assignments.forEach(assignment => {
      scope.__referencingValue(
        pattern,
        Reference.WRITE,
        assignment.right,
        maybeImplicitGlobal,
        init,
      );
    });
  }

  visitArray(arr) {
    if (arr) {
      for (const child of arr) {
        this.visit(child);
      }
    }
  }

  visitPattern(node, options, callback) {
    let visitPatternOptions = options;
    let visitPatternCallback = callback;

    if (typeof options === 'function') {
      visitPatternCallback = options;
      visitPatternOptions = {visitAllNodes: false};
    }

    // Call the callback at left hand identifier nodes, and collect extra nodes to visit.
    const visitor = new PatternVisitor(
      this.options,
      node,
      visitPatternCallback,
    );

    visitor.visit(node);

    // Process all unvisited nodes recursively.
    if (visitPatternOptions.visitAllNodes) {
      visitor.extraNodesToVisit.forEach(this.visit, this);
    }
  }

  visitFunction(node) {
    let i, iz;

    // FunctionDeclaration name is defined in upper scope
    // NOTE: Not referring variableScope. It is intended.
    // Since
    //  in ES5, FunctionDeclaration should be in FunctionBody.
    //  in ES6, FunctionDeclaration should be block scoped.

    if (node.type === Syntax.FunctionDeclaration) {
      // id is defined in upper scope
      this.currentScope().__define(node.id, new FunctionNameDefinition(node));
    }

    // FunctionExpression with name creates its special scope;
    // FunctionExpressionNameScope.
    if (node.type === Syntax.FunctionExpression && node.id) {
      this.scopeManager.__nestFunctionExpressionNameScope(node);
    }

    // Consider this function is in the MethodDefinition.
    this.scopeManager.__nestFunctionScope(node, this.isInnerMethodDefinition);

    const that = this;

    /**
     * Visit pattern callback
     * @param {pattern} pattern - pattern
     * @param {Object} info - info
     * @returns {void}
     */
    function visitPatternCallback(pattern, info) {
      that
        .currentScope()
        .__define(
          pattern,
          new ParameterDefinition(pattern, node, i, info.rest),
        );

      that.referencingDefaultValue(pattern, info.assignments, null, true);
    }

    // Add type parameter declarations before parameter declarations, as type
    // parameters may be used in parameter declarations.
    this.visit(node.typeParameters);

    // Process parameter declarations.
    for (i = 0, iz = node.params.length; i < iz; ++i) {
      this.visitPattern(
        node.params[i],
        {visitAllNodes: true},
        visitPatternCallback,
      );
    }

    // if there's a rest argument, add that
    if (node.rest) {
      this.visitPattern(
        {
          type: 'RestElement',
          argument: node.rest,
        },
        pattern => {
          this.currentScope().__define(
            pattern,
            new ParameterDefinition(pattern, node, node.params.length, true),
          );
        },
      );
    }

    // In TypeScript there are a number of function-like constructs which have no body,
    // so check it exists before traversing
    if (node.body) {
      // Skip BlockStatement to prevent creating BlockStatement scope.
      if (node.body.type === Syntax.BlockStatement) {
        this.visitChildren(node.body);
      } else {
        this.visit(node.body);
      }
    }

    this.close(node);
  }

  visitClass(node) {
    if (node.type === Syntax.ClassDeclaration) {
      this.currentScope().__define(node.id, new ClassNameDefinition(node));
    }

    this.visit(node.superClass);

    this.scopeManager.__nestClassScope(node);

    if (node.id) {
      this.currentScope().__define(node.id, new ClassNameDefinition(node));
    }

    this.visit(node.typeParameters);
    this.visit(node.body);

    this.close(node);
  }

  visitProperty(node) {
    let previous;

    if (node.computed) {
      this.visit(node.key);
    }

    const isMethodDefinition = node.type === Syntax.MethodDefinition;

    if (isMethodDefinition) {
      previous = this.pushInnerMethodDefinition(true);
    }
    this.visit(node.value);
    if (isMethodDefinition) {
      this.popInnerMethodDefinition(previous);
    }
  }

  visitForIn(node) {
    if (
      node.left.type === Syntax.VariableDeclaration &&
      node.left.kind !== 'var'
    ) {
      this.scopeManager.__nestForScope(node);
    }

    if (node.left.type === Syntax.VariableDeclaration) {
      this.visit(node.left);
      this.visitPattern(node.left.declarations[0].id, pattern => {
        this.currentScope().__referencingValue(
          pattern,
          Reference.WRITE,
          node.right,
          null,
          true,
        );
      });
    } else {
      this.visitPattern(node.left, {visitAllNodes: true}, (pattern, info) => {
        let maybeImplicitGlobal = null;

        if (!this.currentScope().isStrict) {
          maybeImplicitGlobal = {
            pattern,
            node,
          };
        }
        this.referencingDefaultValue(
          pattern,
          info.assignments,
          maybeImplicitGlobal,
          false,
        );
        this.currentScope().__referencingValue(
          pattern,
          Reference.WRITE,
          node.right,
          maybeImplicitGlobal,
          false,
        );
      });
    }
    this.visit(node.right);
    this.visit(node.body);

    this.close(node);
  }

  visitVariableDeclaration(variableTargetScope, node, index) {
    const decl = node.declarations[index];
    const init = decl.init;

    this.visitPattern(decl.id, {visitAllNodes: true}, (pattern, info) => {
      variableTargetScope.__define(
        pattern,
        new VariableDefinition(pattern, decl, node, index, node.kind),
      );

      this.referencingDefaultValue(pattern, info.assignments, null, true);
      if (init) {
        this.currentScope().__referencingValue(
          pattern,
          Reference.WRITE,
          init,
          null,
          true,
        );
      }
    });
  }

  AssignmentExpression(node) {
    if (PatternVisitor.isPattern(node.left)) {
      if (node.operator === '=') {
        this.visitPattern(node.left, {visitAllNodes: true}, (pattern, info) => {
          let maybeImplicitGlobal = null;

          if (!this.currentScope().isStrict) {
            maybeImplicitGlobal = {
              pattern,
              node,
            };
          }
          this.referencingDefaultValue(
            pattern,
            info.assignments,
            maybeImplicitGlobal,
            false,
          );
          this.currentScope().__referencingValue(
            pattern,
            Reference.WRITE,
            node.right,
            maybeImplicitGlobal,
            false,
          );
        });
      } else {
        this.currentScope().__referencingValue(
          node.left,
          Reference.RW,
          node.right,
        );
      }
    } else {
      this.visit(node.left);
    }
    this.visit(node.right);
  }

  CatchClause(node) {
    this.scopeManager.__nestCatchScope(node);

    this.visitPattern(node.param, {visitAllNodes: true}, (pattern, info) => {
      this.currentScope().__define(pattern, new CatchClauseDefinition(node));
      this.referencingDefaultValue(pattern, info.assignments, null, true);
    });
    this.visit(node.body);

    this.close(node);
  }

  Program(node) {
    this.scopeManager.__nestGlobalScope(node);

    if (this.scopeManager.isModule()) {
      this.scopeManager.__nestModuleScope(node);
    }

    this.visitChildren(node);
    this.close(node);
  }

  Identifier(node) {
    this.currentScope().__referencingValue(node);
    this.visitChildren(node);
  }

  UpdateExpression(node) {
    if (PatternVisitor.isPattern(node.argument)) {
      this.currentScope().__referencingValue(node.argument, Reference.RW, null);
    } else {
      this.visitChildren(node);
    }
  }

  MemberExpression(node) {
    this.visit(node.object);
    if (node.computed) {
      this.visit(node.property);
    }
  }

  Property(node) {
    this.visitProperty(node);
  }

  MethodDefinition(node) {
    this.visitProperty(node);
  }

  BreakStatement() {}

  ContinueStatement() {}

  LabeledStatement(node) {
    this.visit(node.body);
  }

  ForStatement(node) {
    // Create ForStatement declaration.
    // NOTE: In ES6, ForStatement dynamically generates
    // per iteration environment. However, escope is
    // a static analyzer, we only generate one scope for ForStatement.
    if (
      node.init &&
      node.init.type === Syntax.VariableDeclaration &&
      node.init.kind !== 'var'
    ) {
      this.scopeManager.__nestForScope(node);
    }

    this.visitChildren(node);

    this.close(node);
  }

  ClassExpression(node) {
    this.visitClass(node);
  }

  ClassDeclaration(node) {
    this.visitClass(node);
  }

  BlockStatement(node) {
    this.scopeManager.__nestBlockScope(node);

    this.visitChildren(node);

    this.close(node);
  }

  ThisExpression() {
    this.currentScope().variableScope.__detectThis();
  }

  WithStatement(node) {
    this.visit(node.object);

    // Then nest scope for WithStatement.
    this.scopeManager.__nestWithScope(node);

    this.visit(node.body);

    this.close(node);
  }

  VariableDeclaration(node) {
    const variableTargetScope =
      node.kind === 'var'
        ? this.currentScope().variableScope
        : this.currentScope();

    for (let i = 0, iz = node.declarations.length; i < iz; ++i) {
      const decl = node.declarations[i];

      this.visitVariableDeclaration(variableTargetScope, node, i);
      if (decl.init) {
        this.visit(decl.init);
      }
    }
  }

  // sec 13.11.8
  SwitchStatement(node) {
    this.visit(node.discriminant);

    this.scopeManager.__nestSwitchScope(node);

    for (let i = 0, iz = node.cases.length; i < iz; ++i) {
      this.visit(node.cases[i]);
    }

    this.close(node);
  }

  FunctionDeclaration(node) {
    this.visitFunction(node);
  }

  FunctionExpression(node) {
    this.visitFunction(node);
  }

  ForOfStatement(node) {
    this.visitForIn(node);
  }

  ForInStatement(node) {
    this.visitForIn(node);
  }

  ArrowFunctionExpression(node) {
    this.visitFunction(node);
  }

  ImportDeclaration(node) {
    assert(
      this.scopeManager.isModule(),
      'ImportDeclaration should appear when the mode is ES6 and in the module context.',
    );

    const importer = new Importer(node, this);

    importer.visit(node);
  }

  visitExportDeclaration(node) {
    if (node.source) {
      return;
    }
    if (node.declaration) {
      this.visit(node.declaration);
      return;
    }

    this.visitChildren(node);
  }

  ExportDeclaration(node) {
    this.visitExportDeclaration(node);
  }

  ExportAllDeclaration(node) {
    this.visitExportDeclaration(node);
  }

  ExportDefaultDeclaration(node) {
    this.visitExportDeclaration(node);
  }

  ExportNamedDeclaration(node) {
    this.visitExportDeclaration(node);
  }

  ExportSpecifier(node) {
    const local = node.id || node.local;

    this.visit(local);
  }

  MetaProperty() {
    // do nothing.
  }

  GenericTypeAnnotation(node) {
    if (node.id.type === Syntax.Identifier) {
      this.currentScope().__referencingType(node.id);
    } else {
      this.visit(node.id);
    }

    this.visit(node.typeParameters);
  }

  FunctionTypeAnnotation(node) {
    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visitArray(node.params);
    this.visit(node.returnType);
    this.visit(node.rest);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  QualifiedTypeIdentifier(node) {
    // Only the first component of a qualified type identifier is a reference,
    // e.g. 'Foo' in `type T = Foo.Bar.Baz`.
    if (node.qualification.type === Syntax.Identifier) {
      this.currentScope().__referencingValue(node.qualification);
    } else {
      this.visit(node.qualification);
    }
  }

  ObjectTypeProperty(node) {
    // Do not visit 'key' child if it is an identifier to prevent key being treated as a reference.
    // e.g. 'foo' is a property name in `type T = { foo: string }`.
    if (node.key.type !== Syntax.Identifier) {
      this.visit(node.key);
    }

    this.visit(node.value);
    this.visit(node.variance);
  }

  ObjectTypeIndexer(node) {
    // Do not visit 'id' child to prevent id from being treated as a reference.
    // e.g. 'foo' is an unreferenceable name for the indexer parameter in
    // `type T = { [foo: string]: number }`.
    this.visit(node.key);
    this.visit(node.value);
    this.visit(node.variance);
  }

  ObjectTypeInternalSlot(node) {
    // Do not visit 'id' child to prevent id from being treated as a reference.
    // e.g. 'foo' is an internal slot name in `type T = { [[foo]]: number }`.
    this.visit(node.value);
  }

  FunctionTypeParam(node) {
    // Do not visit 'name' child to prevent name from being treated as a reference.
    // e.g. 'foo' is a parameter name in a type that should not be treated like a
    // definition or reference in `type T = (foo: string) => void`.
    this.visit(node.typeAnnotation);
  }

  createTypeDefinition(node) {
    this.currentScope().__define(node.id, new TypeDefinition(node.id, node));
  }

  visitTypeAlias(node) {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.right);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  visitOpaqueType(node) {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.impltype);
    this.visit(node.supertype);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  visitInterfaceDeclaration(node) {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visitArray(node.extends);
    this.visit(node.body);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  TypeAlias(node) {
    this.visitTypeAlias(node);
  }

  OpaqueType(node) {
    this.visitOpaqueType(node);
  }

  InterfaceDeclaration(node) {
    this.visitInterfaceDeclaration(node);
  }

  EnumDeclaration(node) {
    this.currentScope().__define(node.id, new EnumDefinition(node.id, node));

    // Enum body cannot contain identifier references, so no need to visit body.
  }

  maybeCreateTypeScope(node) {
    if (
      node.typeParameters &&
      node.typeParameters.params &&
      node.typeParameters.params.length !== 0
    ) {
      this.scopeManager.__nestTypeScope(node);
      return true;
    }

    return false;
  }

  TypeParameter(node) {
    const def = new TypeParameterDefinition(node);
    this.currentScope().__define(def.name, def);

    this.visit(node.bound);
    this.visit(node.variance);
    this.visit(node.default);
  }

  DeclareTypeAlias(node) {
    this.visitTypeAlias(node);
  }

  DeclareOpaqueType(node) {
    this.visitOpaqueType(node);
  }

  DeclareInterface(node) {
    this.visitInterfaceDeclaration(node);
  }

  DeclareVariable(node) {
    this.currentScope().__define(
      node.id,
      new VariableDefinition(node.id, node, node, 0, 'declare'),
    );

    this.visit(node.id.typeAnnotation);
  }

  DeclareFunction(node) {
    this.currentScope().__define(node.id, new FunctionNameDefinition(node));

    this.visit(node.id.typeAnnotation);
    this.visit(node.predicate);
  }

  DeclareClass(node) {
    this.currentScope().__define(node.id, new ClassNameDefinition(node));

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visitArray(node.extends);
    this.visitArray(node.implements);
    this.visitArray(node.mixins);
    this.visit(node.body);

    if (hasTypeScope) {
      this.close(node);
    }
  }
}

module.exports = Referencer;
