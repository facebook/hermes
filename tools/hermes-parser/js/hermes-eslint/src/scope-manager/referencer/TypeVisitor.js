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
  DeclareClass,
  DeclaredPredicate,
  DeclareExportDeclaration,
  DeclareFunction,
  DeclareInterface,
  DeclareModule,
  DeclareModuleExports,
  DeclareOpaqueType,
  DeclareTypeAlias,
  DeclareVariable,
  ESNode,
  FunctionTypeAnnotation,
  FunctionTypeParam,
  GenericTypeAnnotation,
  Identifier,
  InterfaceDeclaration,
  ObjectTypeIndexer,
  ObjectTypeInternalSlot,
  ObjectTypeProperty,
  OpaqueType,
  QualifiedTypeIdentifier,
  TypeAlias,
  TypeofTypeAnnotation,
  TypeParameter,
} from 'hermes-estree';
import type {Referencer} from './Referencer';

import {Visitor} from './Visitor';
import {
  ClassNameDefinition,
  FunctionNameDefinition,
  TypeDefinition,
  TypeParameterDefinition,
  VariableDefinition,
} from '../definition';
import type {TypeAnnotationType} from 'hermes-estree';

class TypeVisitor extends Visitor {
  +_referencer: Referencer;

  constructor(referencer: Referencer) {
    super(referencer);
    this._referencer = referencer;
  }

  static visit(referencer: Referencer, node: ESNode): void {
    const typeReferencer = new TypeVisitor(referencer);
    typeReferencer.visit(node);
  }

  ///////////////////
  // Visit helpers //
  ///////////////////

  createTypeDefinition(
    node:
      | DeclareTypeAlias
      | DeclareOpaqueType
      | DeclareInterface
      | TypeAlias
      | OpaqueType
      | InterfaceDeclaration,
  ): void {
    this._referencer
      .currentScope()
      .defineIdentifier(node.id, new TypeDefinition(node.id, node));
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
      this._referencer.scopeManager.nestTypeScope(node);
      return true;
    }

    return false;
  }

  visitTypeAlias(node: DeclareTypeAlias | TypeAlias): void {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.right);

    if (hasTypeScope) {
      this._referencer.close(node);
    }
  }

  visitOpaqueType(node: DeclareOpaqueType | OpaqueType): void {
    this.createTypeDefinition(node);

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.impltype);
    this.visit(node.supertype);

    if (hasTypeScope) {
      this._referencer.close(node);
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
      this._referencer.close(node);
    }
  }

  /////////////////////
  // Visit selectors //
  /////////////////////

  DeclareClass(node: DeclareClass): void {
    this._referencer
      .currentScope()
      .defineIdentifier(node.id, new ClassNameDefinition(node.id, node));

    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visitArray(node.extends);
    this.visitArray(node.implements);
    this.visitArray(node.mixins);
    this.visit(node.body);

    if (hasTypeScope) {
      this._referencer.close(node);
    }
  }

  DeclaredPredicate(_: DeclaredPredicate): void {
    // Declared predicates are complicated - they can technically reference external
    // **values** and they can also reference the function type parameters.
    // These are rarely written by hand, and only usually in type declaration code - so
    // we just ignore them for simplicity's sake.
  }

  DeclareExportDeclaration(node: DeclareExportDeclaration): void {
    if (node.declaration) {
      const declaration = node.declaration;
      this.visit(declaration);

      // `declare export` variables are to be considered used by default
      // non-`declare` exported names are handled natively by ESLint's rule
      // as this is flow-specific syntax, we just handle it here for portability
      for (const variable of this._referencer.scopeManager.getDeclaredVariables(
        declaration,
      )) {
        variable.eslintUsed = true;
      }
    } else {
      for (const specifier of node.specifiers) {
        // can only reference values
        this._referencer.currentScope().referenceValue(specifier.local);
        // also ignore the exported name
      }
    }
  }

  DeclareModuleExports(node: DeclareModuleExports): void {
    this.visit(node.typeAnnotation);
  }

  DeclareFunction(node: DeclareFunction): void {
    this._referencer
      .currentScope()
      .defineIdentifier(node.id, new FunctionNameDefinition(node.id, node));

    // the function type is stored as an annotation on the ID
    this.visit(node.id.typeAnnotation);
    this.visit(node.predicate);
  }

  DeclareInterface(node: DeclareInterface): void {
    this.visitInterfaceDeclaration(node);
  }

  DeclareModule(node: DeclareModule): void {
    this._referencer.scopeManager.nestDeclareModuleScope(node);

    // Do not visit 'id', since module name is neither a reference nor a
    // definition that can be referenced.
    this.visit(node.body);

    this._referencer.close(node);
  }

  DeclareOpaqueType(node: DeclareOpaqueType): void {
    this.visitOpaqueType(node);
  }

  DeclareTypeAlias(node: DeclareTypeAlias): void {
    this.visitTypeAlias(node);
  }

  DeclareVariable(node: DeclareVariable): void {
    this._referencer
      .currentScope()
      .defineIdentifier(node.id, new VariableDefinition(node.id, node, node));

    this.visit(node.id.typeAnnotation);
  }

  FunctionTypeAnnotation(node: FunctionTypeAnnotation): void {
    const hasTypeScope = this.maybeCreateTypeScope(node);

    this.visit(node.typeParameters);
    this.visit(node.this);
    this.visitArray(node.params);
    this.visit(node.returnType);
    this.visit(node.rest);

    if (hasTypeScope) {
      this._referencer.close(node);
    }
  }

  FunctionTypeParam(node: FunctionTypeParam): void {
    // Do not visit 'name' child to prevent name from being treated as a reference.
    // e.g. 'foo' is a parameter name in a type that should not be treated like a
    // definition or reference in `type T = (foo: string) => void`.
    this.visit(node.typeAnnotation);
  }

  GenericTypeAnnotation(node: GenericTypeAnnotation): void {
    this.visit(node.id);
    this.visit(node.typeParameters);
  }

  Identifier(node: Identifier): void {
    this._referencer.currentScope().referenceType(node);
  }

  InterfaceDeclaration(node: InterfaceDeclaration): void {
    this.visitInterfaceDeclaration(node);
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

  ObjectTypeProperty(node: ObjectTypeProperty): void {
    // Do not visit 'key' child if it is an identifier to prevent key being treated as a reference.
    // e.g. 'foo' is a property name in `type T = { foo: string }`.
    if (node.key.type !== 'Identifier') {
      this.visit(node.key);
    }

    this.visit(node.value);
    this.visit(node.variance);
  }

  OpaqueType(node: OpaqueType): void {
    this.visitOpaqueType(node);
  }

  QualifiedTypeIdentifier(node: QualifiedTypeIdentifier): void {
    // Only the first component of a qualified type identifier is a reference,
    // e.g. 'Foo' in `type T = Foo.Bar.Baz`.
    let currentNode = node.qualification;
    while (currentNode.type !== 'Identifier') {
      currentNode = currentNode.qualification;
    }

    // qualified names *usually* only reference values like
    //     import * as Foo from 'foo';
    //     type T = Foo.Bar;
    // however, it is possible for a module to do something like
    //     class Class { ... }
    //     export default { Class }
    // meaning this is also valid
    //     import type Foo from 'foo';
    //     type T = Foo.Class;
    this._referencer.currentScope().referenceDualValueType(currentNode);
  }

  TypeAlias(node: TypeAlias): void {
    this.visitTypeAlias(node);
  }

  TypeofTypeAnnotation(node: TypeofTypeAnnotation): void {
    const identifier = (() => {
      let currentNode: TypeAnnotationType | Identifier = node.argument;
      while (currentNode.type !== 'Identifier') {
        switch (currentNode.type) {
          case 'GenericTypeAnnotation':
            currentNode = currentNode.id;
            break;

          case 'QualifiedTypeIdentifier':
            currentNode = currentNode.qualification;
            break;
        }
      }
      return currentNode;
    })();

    // typeof annotations can only reference values!
    this._referencer.currentScope().referenceValue(identifier);
  }

  TypeParameter(node: TypeParameter): void {
    const def = new TypeParameterDefinition(node);
    this._referencer.currentScope().defineIdentifier(def.name, def);

    this.visit(node.bound);
    this.visit(node.variance);
    this.visit(node.default);
  }
}

export {TypeVisitor};
