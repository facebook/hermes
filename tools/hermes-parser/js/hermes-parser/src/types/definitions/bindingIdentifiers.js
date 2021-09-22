/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/**
 * Mapping of types to their identifier keys.
 */
export default {
  DeclareClass: ['id'],
  DeclareFunction: ['id'],
  DeclareModule: ['id'],
  DeclareVariable: ['id'],
  DeclareInterface: ['id'],
  DeclareTypeAlias: ['id'],
  DeclareOpaqueType: ['id'],
  InterfaceDeclaration: ['id'],
  TypeAlias: ['id'],
  OpaqueType: ['id'],

  CatchClause: ['param'],
  LabeledStatement: ['label'],
  UnaryExpression: ['argument'],
  AssignmentExpression: ['left'],

  ImportSpecifier: ['local'],
  ImportNamespaceSpecifier: ['local'],
  ImportDefaultSpecifier: ['local'],
  ImportDeclaration: ['specifiers'],

  ExportSpecifier: ['exported'],
  ExportNamespaceSpecifier: ['exported'],
  ExportDefaultSpecifier: ['exported'],

  FunctionDeclaration: ['id', 'params'],
  FunctionExpression: ['id', 'params'],
  ArrowFunctionExpression: ['params'],
  ObjectMethod: ['params'],
  ClassMethod: ['params'],
  ClassPrivateMethod: ['params'],

  ForInStatement: ['left'],
  ForOfStatement: ['left'],

  ClassDeclaration: ['id'],
  ClassExpression: ['id'],

  RestElement: ['argument'],
  UpdateExpression: ['argument'],

  ObjectProperty: ['value'],

  AssignmentPattern: ['left'],
  ArrayPattern: ['elements'],
  ObjectPattern: ['properties'],

  VariableDeclaration: ['declarations'],
  VariableDeclarator: ['id'],
};
