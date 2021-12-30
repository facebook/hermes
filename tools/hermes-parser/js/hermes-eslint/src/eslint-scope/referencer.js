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
  ESNode,
  AssignmentPattern,
  AssignmentExpression,
  CatchClause,
  Program,
  Identifier,
  UpdateExpression,
  TypeParameter,
  DeclareTypeAlias,
  DeclareOpaqueType,
  DeclareInterface,
  DeclareVariable,
  DeclareFunction,
  DeclareClass,
  DeclareModule,
  MemberExpression,
  OptionalMemberExpression,
  Property,
  MethodDefinition,
  LabeledStatement,
  ForStatement,
  ClassExpression,
  ClassDeclaration,
  ClassProperty,
  ClassPrivateProperty,
  BlockStatement,
  WithStatement,
  VariableDeclaration,
  SwitchStatement,
  FunctionDeclaration,
  FunctionExpression,
  ForOfStatement,
  ForInStatement,
  ArrowFunctionExpression,
  ImportDeclaration,
  ExportAllDeclaration,
  ExportDefaultDeclaration,
  ExportNamedDeclaration,
  ExportSpecifier,
  GenericTypeAnnotation,
  FunctionTypeAnnotation,
  QualifiedTypeIdentifier,
  ObjectTypeProperty,
  ObjectTypeIndexer,
  ObjectTypeInternalSlot,
  FunctionTypeParam,
  TypeAlias,
  OpaqueType,
  InterfaceDeclaration,
  EnumDeclaration,
  ImportSpecifier,
} from 'hermes-estree';
import type {VisitorOptions} from './Visitor';
import type ScopeManager from './scope-manager';
import type {Scope} from './scope';
import type {PatternVisitorCallback} from './pattern-visitor';

const {ReadWriteFlag} = require('./reference');
const {PatternVisitor, isPattern} = require('./pattern-visitor');
const {
  CatchClauseDefinition,
  ClassNameDefinition,
  EnumDefinition,
  FunctionNameDefinition,
  ImportBindingDefinition,
  ParameterDefinition,
  TypeDefinition,
  TypeParameterDefinition,
  VariableDefinition,
} = require('./definition');
const Visitor = require('./Visitor');
const assert = require('assert');

// Importing ImportDeclaration.
// http://people.mozilla.org/~jorendorff/es6-draft.html#sec-moduledeclarationinstantiation
// https://github.com/estree/estree/blob/master/es6.md#importdeclaration
// FIXME: Now, we don't create module environment, because the context is
// implementation dependent.

class Importer extends Visitor {
  +declaration;
  +referencer: Referencer;

  constructor(declaration, referencer: Referencer) {
    super(null, referencer.options);
    this.declaration = declaration;
    this.referencer = referencer;
  }

  visitImport(id: Identifier, specifier: ImportSpecifier) {
    this.referencer.visitPattern(id, pattern => {
      this.referencer
        .currentScopeAssert()
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
class Referencer extends Visitor {
  +options: VisitorOptions;
  +scopeManager: ScopeManager;
  +parent: null;
  isInnerMethodDefinition: boolean;

  constructor(options: VisitorOptions, scopeManager: ScopeManager) {
    super(null, options);
    this.options = options;
    this.scopeManager = scopeManager;
    this.parent = null;
    this.isInnerMethodDefinition = false;
  }

  currentScopeAssert(): Scope {
    if (this.scopeManager.__currentScope == null) {
      throw new Error('Expected there to be a current scope');
    }
    return this.scopeManager.__currentScope;
  }

  currentScope(): ?Scope {
    return this.scopeManager.__currentScope;
  }

  close(node: ESNode): void {
    while (this.currentScope() && node === this.currentScopeAssert().block) {
      this.scopeManager.__currentScope = this.currentScopeAssert().__close(
        this.scopeManager,
      );
    }
  }

  pushInnerMethodDefinition(isInnerMethodDefinition: boolean): boolean {
    const previous = this.isInnerMethodDefinition;

    this.isInnerMethodDefinition = isInnerMethodDefinition;
    return previous;
  }

  popInnerMethodDefinition(isInnerMethodDefinition: ?boolean): void {
    this.isInnerMethodDefinition = isInnerMethodDefinition === true;
  }

  referencingDefaultValue(
    pattern: Identifier,
    assignments: $ReadOnlyArray<AssignmentPattern | AssignmentExpression>,
    maybeImplicitGlobal: null | {node: ESNode, pattern: Identifier},
    init: boolean,
  ): void {
    const scope = this.currentScopeAssert();

    assignments.forEach(assignment => {
      scope.__referencingValue(
        pattern,
        ReadWriteFlag.WRITE,
        assignment.right,
        maybeImplicitGlobal,
        init,
      );
    });
  }

  visitArray(arr: $ReadOnlyArray<ESNode>): void {
    if (arr) {
      for (const child of arr) {
        this.visit(child);
      }
    }
  }

  visitPattern(
    node: ESNode,
    optionsOrCallback:
      | $ReadOnly<{...VisitorOptions, visitAllNodes?: boolean}>
      | PatternVisitorCallback,
    callback?: PatternVisitorCallback,
  ): void {
    let visitAllNodes: boolean;
    let visitPatternCallback: PatternVisitorCallback;

    if (typeof optionsOrCallback === 'function') {
      visitPatternCallback = optionsOrCallback;
      visitAllNodes = false;
    } else {
      if (callback == null) {
        throw new Error('Missing expected callback');
      }
      visitPatternCallback = callback;
      visitAllNodes = optionsOrCallback.visitAllNodes ?? false;
    }

    // Call the callback at left hand identifier nodes, and collect extra nodes to visit.
    const visitor = new PatternVisitor(
      this.options,
      node,
      visitPatternCallback,
    );

    visitor.visit(node);

    // Process all unvisited nodes recursively.
    if (visitAllNodes) {
      visitor.extraNodesToVisit.forEach(node => this.visit(node));
    }
  }

  visitFunction(
    node: FunctionDeclaration | FunctionExpression | ArrowFunctionExpression,
  ): void {
    // FunctionDeclaration name is defined in upper scope
    // NOTE: Not referring variableScope. It is intended.
    // Since
    //  in ES5, FunctionDeclaration should be in FunctionBody.
    //  in ES6, FunctionDeclaration should be block scoped.

    if (node.type === 'FunctionDeclaration' && node.id) {
      const id = node.id;
      // id is defined in upper scope
      this.currentScopeAssert().__define(
        id,
        new FunctionNameDefinition(id, node),
      );
    }

    // If type parameters exist, add them to type scope before function expression name.
    if (
      node.typeParameters != null &&
      node.typeParameters.params.length !== 0
    ) {
      const typeParameters = node.typeParameters;
      const parentScope = this.currentScopeAssert();

      const typeScope = this.scopeManager.__nestTypeScope(node);
      this.visit(typeParameters);

      // Forward future defines in type scope to parent scope
      // $FlowExpectedError[cannot-write]
      typeScope.__define = function (node, def) {
        return parentScope.__define(node, def);
      };
    }

    // Return type may reference type parameters but not parameters or function expression name.
    this.visit(node.returnType);

    // FunctionExpression with name creates its special scope;
    // FunctionExpressionNameScope.
    if (node.type === 'FunctionExpression' && node.id) {
      this.scopeManager.__nestFunctionExpressionNameScope(node);
    }

    // Consider this function is in the MethodDefinition.
    this.scopeManager.__nestFunctionScope(node, this.isInnerMethodDefinition);

    const that = this;

    function visitPatternCallback(i, pattern, info) {
      // If the first parameter for a function has name 'this' it is a Flow
      // type annotation and not a parameter than can be referenced by name.
      if (i === 0 && pattern.name === 'this') {
        return;
      }

      that
        .currentScopeAssert()
        .__define(
          pattern,
          new ParameterDefinition(pattern, node, i, info.rest),
        );

      that.referencingDefaultValue(pattern, info.assignments, null, true);
    }

    // Process parameter declarations.
    for (let i = 0, iz = node.params.length; i < iz; ++i) {
      this.visitPattern(
        node.params[i],
        {visitAllNodes: true},
        (pattern, info) => visitPatternCallback(i, pattern, info),
      );
    }

    this.visit(node.predicate);

    // In TypeScript there are a number of function-like constructs which have no body,
    // so check it exists before traversing
    if (node.body) {
      // Skip BlockStatement to prevent creating BlockStatement scope.
      if (node.body.type === 'BlockStatement') {
        this.visitChildren(node.body);
      } else {
        this.visit(node.body);
      }
    }

    this.close(node);
  }

  visitClass(node: ClassDeclaration | ClassExpression): void {
    if (node.type === 'ClassDeclaration' && node.id) {
      const id = node.id;
      this.currentScopeAssert().__define(id, new ClassNameDefinition(id, node));
    }

    this.visit(node.superClass);

    this.scopeManager.__nestClassScope(node);

    if (node.id) {
      const id = node.id;
      this.currentScopeAssert().__define(id, new ClassNameDefinition(id, node));
    }

    this.visit(node.typeParameters);
    this.visit(node.superTypeParameters);
    this.visitArray(node.implements);
    this.visit(node.body);

    this.close(node);
  }

  visitProperty(node: Property | MethodDefinition): void {
    let previous;

    if (node.computed) {
      this.visit(node.key);
    }

    const isMethodDefinition = node.type === 'MethodDefinition';

    if (isMethodDefinition) {
      previous = this.pushInnerMethodDefinition(true);
    }
    this.visit(node.value);
    if (isMethodDefinition) {
      this.popInnerMethodDefinition(previous);
    }
  }

  visitClassProperty(node: ClassProperty | ClassPrivateProperty): void {
    // private properties cannot be computed
    if (node.type === 'ClassProperty' && node.computed) {
      this.visit(node.key);
    }

    this.visit(node.value);
    this.visit(node.variance);
    this.visit(node.typeAnnotation);
  }

  visitForIn(node: ForOfStatement | ForInStatement): void {
    if (node.left.type === 'VariableDeclaration' && node.left.kind !== 'var') {
      this.scopeManager.__nestForScope(node);
    }

    if (node.left.type === 'VariableDeclaration') {
      const decl = node.left;
      this.visit(decl);
      this.visitPattern(decl.declarations[0].id, pattern => {
        this.currentScopeAssert().__referencingValue(
          pattern,
          ReadWriteFlag.WRITE,
          node.right,
          null,
          true,
        );
      });
    } else {
      this.visitPattern(node.left, {visitAllNodes: true}, (pattern, info) => {
        let maybeImplicitGlobal = null;

        if (!this.currentScopeAssert().isStrict) {
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
        this.currentScopeAssert().__referencingValue(
          pattern,
          ReadWriteFlag.WRITE,
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

  visitVariableDeclaration(
    variableTargetScope: Scope,
    node: VariableDeclaration,
    index: number,
  ): void {
    const decl = node.declarations[index];
    const init = decl.init;

    this.visitPattern(decl.id, {visitAllNodes: true}, (pattern, info) => {
      variableTargetScope.__define(
        pattern,
        new VariableDefinition(pattern, decl, node, index, node.kind),
      );

      this.referencingDefaultValue(pattern, info.assignments, null, true);
      if (init) {
        this.currentScopeAssert().__referencingValue(
          pattern,
          ReadWriteFlag.WRITE,
          init,
          null,
          true,
        );
      }
    });
  }

  AssignmentExpression(node: AssignmentExpression): void {
    const left = node.left;
    if (isPattern(left)) {
      if (node.operator === '=') {
        this.visitPattern(left, {visitAllNodes: true}, (pattern, info) => {
          let maybeImplicitGlobal = null;

          if (!this.currentScopeAssert().isStrict) {
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
          this.currentScopeAssert().__referencingValue(
            pattern,
            ReadWriteFlag.WRITE,
            node.right,
            maybeImplicitGlobal,
            false,
          );
        });
      } else if (left.type === 'Identifier') {
        this.currentScopeAssert().__referencingValue(
          left,
          ReadWriteFlag.RW,
          node.right,
        );
      }
    } else {
      this.visit(node.left);
    }
    this.visit(node.right);
  }

  CatchClause(node: CatchClause): void {
    this.scopeManager.__nestCatchScope(node);

    if (node.param) {
      this.visitPattern(node.param, {visitAllNodes: true}, (pattern, info) => {
        this.currentScopeAssert().__define(
          pattern,
          new CatchClauseDefinition(pattern, node),
        );
        this.referencingDefaultValue(pattern, info.assignments, null, true);
      });
    }
    this.visit(node.body);

    this.close(node);
  }

  Program(node: Program): void {
    this.scopeManager.__nestGlobalScope(node);

    if (this.scopeManager.isModule()) {
      this.scopeManager.__nestModuleScope(node);
    }

    this.visitChildren(node);
    this.close(node);
  }

  Identifier(node: Identifier): void {
    this.currentScopeAssert().__referencingValue(node);
    this.visitChildren(node);
  }

  UpdateExpression(node: UpdateExpression): void {
    const argument = node.argument;
    if (isPattern(argument)) {
      this.currentScopeAssert().__referencingValue(
        argument,
        ReadWriteFlag.RW,
        null,
      );
    } else {
      this.visitChildren(node);
    }
  }

  visitMemberExpression(
    node: MemberExpression | OptionalMemberExpression,
  ): void {
    this.visit(node.object);
    if (node.computed) {
      this.visit(node.property);
    }
  }

  MemberExpression(node: MemberExpression): void {
    this.visitMemberExpression(node);
  }

  OptionalMemberExpression(node: OptionalMemberExpression): void {
    this.visitMemberExpression(node);
  }

  Property(node: Property): void {
    this.visitProperty(node);
  }

  MethodDefinition(node: MethodDefinition): void {
    this.visitProperty(node);
  }

  BreakStatement(): void {}

  ContinueStatement(): void {}

  LabeledStatement(node: LabeledStatement): void {
    this.visit(node.body);
  }

  ForStatement(node: ForStatement): void {
    // Create ForStatement declaration.
    // NOTE: In ES6, ForStatement dynamically generates
    // per iteration environment. However, escope is
    // a static analyzer, we only generate one scope for ForStatement.
    if (
      node.init &&
      node.init.type === 'VariableDeclaration' &&
      node.init.kind !== 'var'
    ) {
      this.scopeManager.__nestForScope(node);
    }

    this.visitChildren(node);

    this.close(node);
  }

  ClassExpression(node: ClassExpression): void {
    this.visitClass(node);
  }

  ClassDeclaration(node: ClassDeclaration): void {
    this.visitClass(node);
  }

  ClassProperty(node: ClassProperty): void {
    this.visitClassProperty(node);
  }

  ClassPrivateProperty(node: ClassPrivateProperty): void {
    this.visitClassProperty(node);
  }

  BlockStatement(node: BlockStatement): void {
    this.scopeManager.__nestBlockScope(node);

    this.visitChildren(node);

    this.close(node);
  }

  ThisExpression(): void {
    this.currentScopeAssert().variableScope.__detectThis();
  }

  WithStatement(node: WithStatement): void {
    this.visit(node.object);

    // Then nest scope for WithStatement.
    this.scopeManager.__nestWithScope(node);

    this.visit(node.body);

    this.close(node);
  }

  VariableDeclaration(node: VariableDeclaration): void {
    const variableTargetScope =
      node.kind === 'var'
        ? this.currentScopeAssert().variableScope
        : this.currentScopeAssert();

    for (let i = 0, iz = node.declarations.length; i < iz; ++i) {
      const decl = node.declarations[i];

      this.visitVariableDeclaration(variableTargetScope, node, i);
      if (decl.init) {
        this.visit(decl.init);
      }
    }
  }

  // sec 13.11.8
  SwitchStatement(node: SwitchStatement): void {
    this.visit(node.discriminant);

    this.scopeManager.__nestSwitchScope(node);

    for (let i = 0, iz = node.cases.length; i < iz; ++i) {
      this.visit(node.cases[i]);
    }

    this.close(node);
  }

  FunctionDeclaration(node: FunctionDeclaration): void {
    this.visitFunction(node);
  }

  FunctionExpression(node: FunctionExpression): void {
    this.visitFunction(node);
  }

  ForOfStatement(node: ForOfStatement): void {
    this.visitForIn(node);
  }

  ForInStatement(node: ForInStatement): void {
    this.visitForIn(node);
  }

  ArrowFunctionExpression(node: ArrowFunctionExpression): void {
    this.visitFunction(node);
  }

  ImportDeclaration(node: ImportDeclaration): void {
    assert(
      this.scopeManager.isModule(),
      'ImportDeclaration should appear when the mode is ES6 and in the module context.',
    );

    const importer = new Importer(node, this);

    importer.visit(node);
  }

  visitExportDeclaration(
    node:
      | ExportAllDeclaration
      | ExportDefaultDeclaration
      | ExportNamedDeclaration,
  ): void {
    if (node.type !== 'ExportDefaultDeclaration' && node.source) {
      return;
    }
    if (node.type !== 'ExportAllDeclaration' && node.declaration) {
      this.visit(node.declaration);
      return;
    }

    this.visitChildren(node);
  }

  ExportAllDeclaration(node: ExportAllDeclaration): void {
    this.visitExportDeclaration(node);
  }

  ExportDefaultDeclaration(node: ExportDefaultDeclaration): void {
    this.visitExportDeclaration(node);
  }

  ExportNamedDeclaration(node: ExportNamedDeclaration): void {
    this.visitExportDeclaration(node);
  }

  ExportSpecifier(node: ExportSpecifier): void {
    this.visit(node.local);
  }

  MetaProperty(): void {
    // do nothing.
  }

  GenericTypeAnnotation(node: GenericTypeAnnotation): void {
    if (node.id.type === 'Identifier') {
      this.currentScopeAssert().__referencingType(node.id);
    } else {
      this.visit(node.id);
    }

    this.visit(node.typeParameters);
  }

  FunctionTypeAnnotation(node: FunctionTypeAnnotation): void {
    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visitArray(node.params);
    this.visit(node.returnType);
    this.visit(node.rest);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  QualifiedTypeIdentifier(node: QualifiedTypeIdentifier): void {
    // Only the first component of a qualified type identifier is a reference,
    // e.g. 'Foo' in `type T = Foo.Bar.Baz`.
    if (node.qualification.type === 'Identifier') {
      const qualification = node.qualification;
      this.currentScopeAssert().__referencingValue(qualification);
    } else {
      this.visit(node.qualification);
    }
  }

  ObjectTypeProperty(node: ObjectTypeProperty): void {
    // Do not visit 'key' child if it is an identifier to prevent key being treated as a reference.
    // e.g. 'foo' is a property name in `type T = { foo: string }`.
    if (node.key.type !== 'Identifier') {
      this.visit(node.key);
    }

    this.visit(node.value);
    this.visit(node.variance);
  }

  ObjectTypeIndexer(node: ObjectTypeIndexer): void {
    // Do not visit 'id' child to prevent id from being treated as a reference.
    // e.g. 'foo' is an unreferenceable name for the indexer parameter in
    // `type T = { [foo: string]: number }`.
    this.visit(node.key);
    this.visit(node.value);
    this.visit(node.variance);
  }

  ObjectTypeInternalSlot(node: ObjectTypeInternalSlot): void {
    // Do not visit 'id' child to prevent id from being treated as a reference.
    // e.g. 'foo' is an internal slot name in `type T = { [[foo]]: number }`.
    this.visit(node.value);
  }

  FunctionTypeParam(node: FunctionTypeParam): void {
    // Do not visit 'name' child to prevent name from being treated as a reference.
    // e.g. 'foo' is a parameter name in a type that should not be treated like a
    // definition or reference in `type T = (foo: string) => void`.
    this.visit(node.typeAnnotation);
  }

  createTypeDefinition(
    node:
      | DeclareTypeAlias
      | DeclareOpaqueType
      | DeclareInterface
      | TypeAlias
      | OpaqueType
      | InterfaceDeclaration,
  ): void {
    this.currentScopeAssert().__define(
      node.id,
      new TypeDefinition(node.id, node),
    );
  }

  visitTypeAlias(node: DeclareTypeAlias | TypeAlias): void {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.right);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  visitOpaqueType(node: DeclareOpaqueType | OpaqueType): void {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.impltype);
    this.visit(node.supertype);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  visitInterfaceDeclaration(
    node: DeclareInterface | InterfaceDeclaration,
  ): void {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visitArray(node.extends);
    this.visit(node.body);

    if (hasTypeScope) {
      this.close(node);
    }
  }

  TypeAlias(node: TypeAlias): void {
    this.visitTypeAlias(node);
  }

  OpaqueType(node: OpaqueType): void {
    this.visitOpaqueType(node);
  }

  InterfaceDeclaration(node: InterfaceDeclaration): void {
    this.visitInterfaceDeclaration(node);
  }

  EnumDeclaration(node: EnumDeclaration): void {
    this.currentScopeAssert().__define(
      node.id,
      new EnumDefinition(node.id, node),
    );

    // Enum body cannot contain identifier references, so no need to visit body.
  }

  maybeCreateTypeScope(
    node:
      | DeclareTypeAlias
      | DeclareOpaqueType
      | DeclareInterface
      | DeclareClass
      | FunctionTypeAnnotation
      | TypeAlias
      | OpaqueType
      | InterfaceDeclaration,
  ): boolean {
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

  TypeParameter(node: TypeParameter): void {
    const def = new TypeParameterDefinition(node);
    this.currentScopeAssert().__define(def.name, def);

    this.visit(node.bound);
    this.visit(node.variance);
    this.visit(node.default);
  }

  DeclareTypeAlias(node: DeclareTypeAlias): void {
    this.visitTypeAlias(node);
  }

  DeclareOpaqueType(node: DeclareOpaqueType): void {
    this.visitOpaqueType(node);
  }

  DeclareInterface(node: DeclareInterface): void {
    this.visitInterfaceDeclaration(node);
  }

  DeclareVariable(node: DeclareVariable): void {
    this.currentScopeAssert().__define(
      node.id,
      new VariableDefinition(node.id, node, node, 0, 'declare'),
    );

    this.visit(node.id.typeAnnotation);
  }

  DeclareFunction(node: DeclareFunction): void {
    this.currentScopeAssert().__define(
      node.id,
      new FunctionNameDefinition(node.id, node),
    );

    this.visit(node.id.typeAnnotation);
    this.visit(node.predicate);
  }

  DeclareClass(node: DeclareClass): void {
    this.currentScopeAssert().__define(
      node.id,
      new ClassNameDefinition(node.id, node),
    );

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

  DeclareModule(node: DeclareModule): void {
    this.scopeManager.__nestDeclareModuleScope(node);

    // Do not visit 'id', since module name is neither a reference nor a
    // definition that can be referenced.
    this.visit(node.body);

    this.close(node);
  }
}

module.exports = Referencer;
