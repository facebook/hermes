/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @noflow
 * @format
 * @generated
 */

/*
 * !!! GENERATED FILE !!!
 *
 * Any manual changes to this file will be overwritten. To regenerate run `yarn build`.
 */

// lint directives to let us do some basic validation of generated files
/* eslint no-undef: 'error', no-unused-vars: ['error', {vars: "local"}], no-redeclare: 'error' */
/* global $NonMaybeType, $Partial, $ReadOnly, $ReadOnlyArray */

'use strict';

export const NODE_CHILD = 'Node';
export const NODE_LIST_CHILD = 'NodeList';
export const HERMES_AST_VISITOR_KEYS = {
  AnyTypeAnnotation: {},
  ArrayExpression: {
    elements: 'NodeList',
  },
  ArrayPattern: {
    elements: 'NodeList',
    typeAnnotation: 'Node',
  },
  ArrayTypeAnnotation: {
    elementType: 'Node',
  },
  ArrowFunctionExpression: {
    id: 'Node',
    params: 'NodeList',
    body: 'Node',
    typeParameters: 'Node',
    returnType: 'Node',
    predicate: 'Node',
  },
  AssignmentExpression: {
    left: 'Node',
    right: 'Node',
  },
  AssignmentPattern: {
    left: 'Node',
    right: 'Node',
  },
  AwaitExpression: {
    argument: 'Node',
  },
  BigIntLiteral: {},
  BigIntLiteralTypeAnnotation: {},
  BinaryExpression: {
    left: 'Node',
    right: 'Node',
  },
  BlockStatement: {
    body: 'NodeList',
  },
  BooleanLiteral: {},
  BooleanLiteralTypeAnnotation: {},
  BooleanTypeAnnotation: {},
  BreakStatement: {
    label: 'Node',
  },
  CallExpression: {
    callee: 'Node',
    typeArguments: 'Node',
    arguments: 'NodeList',
  },
  CatchClause: {
    param: 'Node',
    body: 'Node',
  },
  ChainExpression: {
    expression: 'Node',
  },
  ClassBody: {
    body: 'NodeList',
  },
  ClassDeclaration: {
    id: 'Node',
    typeParameters: 'Node',
    superClass: 'Node',
    superTypeParameters: 'Node',
    implements: 'NodeList',
    decorators: 'NodeList',
    body: 'Node',
  },
  ClassExpression: {
    id: 'Node',
    typeParameters: 'Node',
    superClass: 'Node',
    superTypeParameters: 'Node',
    implements: 'NodeList',
    decorators: 'NodeList',
    body: 'Node',
  },
  ClassImplements: {
    id: 'Node',
    typeParameters: 'Node',
  },
  ConditionalExpression: {
    test: 'Node',
    alternate: 'Node',
    consequent: 'Node',
  },
  ContinueStatement: {
    label: 'Node',
  },
  DebuggerStatement: {},
  DeclareClass: {
    id: 'Node',
    typeParameters: 'Node',
    extends: 'NodeList',
    implements: 'NodeList',
    mixins: 'NodeList',
    body: 'Node',
  },
  DeclaredPredicate: {
    value: 'Node',
  },
  DeclareExportAllDeclaration: {
    source: 'Node',
  },
  DeclareExportDeclaration: {
    declaration: 'Node',
    specifiers: 'NodeList',
    source: 'Node',
  },
  DeclareFunction: {
    id: 'Node',
    predicate: 'Node',
  },
  DeclareInterface: {
    id: 'Node',
    typeParameters: 'Node',
    extends: 'NodeList',
    body: 'Node',
  },
  DeclareModule: {
    id: 'Node',
    body: 'Node',
  },
  DeclareModuleExports: {
    typeAnnotation: 'Node',
  },
  DeclareOpaqueType: {
    id: 'Node',
    typeParameters: 'Node',
    impltype: 'Node',
    supertype: 'Node',
  },
  DeclareTypeAlias: {
    id: 'Node',
    typeParameters: 'Node',
    right: 'Node',
  },
  DeclareVariable: {
    id: 'Node',
  },
  DoWhileStatement: {
    body: 'Node',
    test: 'Node',
  },
  EmptyStatement: {},
  EmptyTypeAnnotation: {},
  EnumBooleanBody: {
    members: 'NodeList',
  },
  EnumBooleanMember: {
    id: 'Node',
    init: 'Node',
  },
  EnumDeclaration: {
    id: 'Node',
    body: 'Node',
  },
  EnumDefaultedMember: {
    id: 'Node',
  },
  EnumNumberBody: {
    members: 'NodeList',
  },
  EnumNumberMember: {
    id: 'Node',
    init: 'Node',
  },
  EnumStringBody: {
    members: 'NodeList',
  },
  EnumStringMember: {
    id: 'Node',
    init: 'Node',
  },
  EnumSymbolBody: {
    members: 'NodeList',
  },
  ExistsTypeAnnotation: {},
  ExportAllDeclaration: {
    exported: 'Node',
    source: 'Node',
  },
  ExportDefaultDeclaration: {
    declaration: 'Node',
  },
  ExportNamedDeclaration: {
    declaration: 'Node',
    specifiers: 'NodeList',
    source: 'Node',
  },
  ExportSpecifier: {
    exported: 'Node',
    local: 'Node',
  },
  ExpressionStatement: {
    expression: 'Node',
  },
  ForInStatement: {
    left: 'Node',
    right: 'Node',
    body: 'Node',
  },
  ForOfStatement: {
    left: 'Node',
    right: 'Node',
    body: 'Node',
  },
  ForStatement: {
    init: 'Node',
    test: 'Node',
    update: 'Node',
    body: 'Node',
  },
  FunctionDeclaration: {
    id: 'Node',
    params: 'NodeList',
    body: 'Node',
    typeParameters: 'Node',
    returnType: 'Node',
    predicate: 'Node',
  },
  FunctionExpression: {
    id: 'Node',
    params: 'NodeList',
    body: 'Node',
    typeParameters: 'Node',
    returnType: 'Node',
    predicate: 'Node',
  },
  FunctionTypeAnnotation: {
    params: 'NodeList',
    this: 'Node',
    returnType: 'Node',
    rest: 'Node',
    typeParameters: 'Node',
  },
  FunctionTypeParam: {
    name: 'Node',
    typeAnnotation: 'Node',
  },
  GenericTypeAnnotation: {
    id: 'Node',
    typeParameters: 'Node',
  },
  Identifier: {
    typeAnnotation: 'Node',
  },
  IfStatement: {
    test: 'Node',
    consequent: 'Node',
    alternate: 'Node',
  },
  ImportAttribute: {
    key: 'Node',
    value: 'Node',
  },
  ImportDeclaration: {
    specifiers: 'NodeList',
    source: 'Node',
    assertions: 'NodeList',
  },
  ImportDefaultSpecifier: {
    local: 'Node',
  },
  ImportExpression: {
    source: 'Node',
    attributes: 'Node',
  },
  ImportNamespaceSpecifier: {
    local: 'Node',
  },
  ImportSpecifier: {
    imported: 'Node',
    local: 'Node',
  },
  IndexedAccessType: {
    objectType: 'Node',
    indexType: 'Node',
  },
  InferredPredicate: {},
  InterfaceDeclaration: {
    id: 'Node',
    typeParameters: 'Node',
    extends: 'NodeList',
    body: 'Node',
  },
  InterfaceExtends: {
    id: 'Node',
    typeParameters: 'Node',
  },
  InterfaceTypeAnnotation: {
    extends: 'NodeList',
    body: 'Node',
  },
  IntersectionTypeAnnotation: {
    types: 'NodeList',
  },
  JSXAttribute: {
    name: 'Node',
    value: 'Node',
  },
  JSXClosingElement: {
    name: 'Node',
  },
  JSXClosingFragment: {},
  JSXElement: {
    openingElement: 'Node',
    children: 'NodeList',
    closingElement: 'Node',
  },
  JSXEmptyExpression: {},
  JSXExpressionContainer: {
    expression: 'Node',
  },
  JSXFragment: {
    openingFragment: 'Node',
    children: 'NodeList',
    closingFragment: 'Node',
  },
  JSXIdentifier: {},
  JSXMemberExpression: {
    object: 'Node',
    property: 'Node',
  },
  JSXNamespacedName: {
    namespace: 'Node',
    name: 'Node',
  },
  JSXOpeningElement: {
    name: 'Node',
    attributes: 'NodeList',
  },
  JSXOpeningFragment: {},
  JSXSpreadAttribute: {
    argument: 'Node',
  },
  JSXSpreadChild: {
    expression: 'Node',
  },
  JSXText: {},
  LabeledStatement: {
    label: 'Node',
    body: 'Node',
  },
  LogicalExpression: {
    left: 'Node',
    right: 'Node',
  },
  MemberExpression: {
    object: 'Node',
    property: 'Node',
  },
  MetaProperty: {
    meta: 'Node',
    property: 'Node',
  },
  MethodDefinition: {
    key: 'Node',
    value: 'Node',
  },
  MixedTypeAnnotation: {},
  NewExpression: {
    callee: 'Node',
    typeArguments: 'Node',
    arguments: 'NodeList',
  },
  NullableTypeAnnotation: {
    typeAnnotation: 'Node',
  },
  NullLiteral: {},
  NullLiteralTypeAnnotation: {},
  NumberLiteralTypeAnnotation: {},
  NumberTypeAnnotation: {},
  NumericLiteral: {},
  ObjectExpression: {
    properties: 'NodeList',
  },
  ObjectPattern: {
    properties: 'NodeList',
    typeAnnotation: 'Node',
  },
  ObjectTypeAnnotation: {
    properties: 'NodeList',
    indexers: 'NodeList',
    callProperties: 'NodeList',
    internalSlots: 'NodeList',
  },
  ObjectTypeCallProperty: {
    value: 'Node',
  },
  ObjectTypeIndexer: {
    id: 'Node',
    key: 'Node',
    value: 'Node',
    variance: 'Node',
  },
  ObjectTypeInternalSlot: {
    id: 'Node',
    value: 'Node',
  },
  ObjectTypeProperty: {
    key: 'Node',
    value: 'Node',
    variance: 'Node',
  },
  ObjectTypeSpreadProperty: {
    argument: 'Node',
  },
  OpaqueType: {
    id: 'Node',
    typeParameters: 'Node',
    impltype: 'Node',
    supertype: 'Node',
  },
  OptionalIndexedAccessType: {
    objectType: 'Node',
    indexType: 'Node',
  },
  PrivateIdentifier: {},
  Program: {
    body: 'NodeList',
  },
  Property: {
    key: 'Node',
    value: 'Node',
  },
  PropertyDefinition: {
    key: 'Node',
    value: 'Node',
    variance: 'Node',
    typeAnnotation: 'Node',
  },
  QualifiedTypeIdentifier: {
    qualification: 'Node',
    id: 'Node',
  },
  RegExpLiteral: {},
  RestElement: {
    argument: 'Node',
  },
  ReturnStatement: {
    argument: 'Node',
  },
  SequenceExpression: {
    expressions: 'NodeList',
  },
  SpreadElement: {
    argument: 'Node',
  },
  StringLiteral: {},
  StringLiteralTypeAnnotation: {},
  StringTypeAnnotation: {},
  Super: {},
  SwitchCase: {
    test: 'Node',
    consequent: 'NodeList',
  },
  SwitchStatement: {
    discriminant: 'Node',
    cases: 'NodeList',
  },
  SymbolTypeAnnotation: {},
  TaggedTemplateExpression: {
    tag: 'Node',
    quasi: 'Node',
  },
  TemplateElement: {},
  TemplateLiteral: {
    quasis: 'NodeList',
    expressions: 'NodeList',
  },
  ThisExpression: {},
  ThisTypeAnnotation: {},
  ThrowStatement: {
    argument: 'Node',
  },
  TryStatement: {
    block: 'Node',
    handler: 'Node',
    finalizer: 'Node',
  },
  TupleTypeAnnotation: {
    types: 'NodeList',
  },
  TypeAlias: {
    id: 'Node',
    typeParameters: 'Node',
    right: 'Node',
  },
  TypeAnnotation: {
    typeAnnotation: 'Node',
  },
  TypeCastExpression: {
    expression: 'Node',
    typeAnnotation: 'Node',
  },
  TypeofTypeAnnotation: {
    argument: 'Node',
  },
  TypeParameter: {
    bound: 'Node',
    variance: 'Node',
    default: 'Node',
  },
  TypeParameterDeclaration: {
    params: 'NodeList',
  },
  TypeParameterInstantiation: {
    params: 'NodeList',
  },
  UnaryExpression: {
    argument: 'Node',
  },
  UnionTypeAnnotation: {
    types: 'NodeList',
  },
  UpdateExpression: {
    argument: 'Node',
  },
  VariableDeclaration: {
    declarations: 'NodeList',
  },
  VariableDeclarator: {
    init: 'Node',
    id: 'Node',
  },
  Variance: {},
  VoidTypeAnnotation: {},
  WhileStatement: {
    body: 'Node',
    test: 'Node',
  },
  WithStatement: {
    object: 'Node',
    body: 'Node',
  },
  YieldExpression: {
    argument: 'Node',
  },
  File: {
    program: 'Node',
  },
  ObjectProperty: {
    key: 'Node',
    value: 'Node',
  },
  ObjectMethod: {
    key: 'Node',
    params: 'NodeList',
    body: 'Node',
    returnType: 'Node',
    typeParameters: 'NodeList',
  },
  ClassMethod: {
    key: 'Node',
    params: 'NodeList',
    body: 'Node',
    returnType: 'Node',
    typeParameters: 'NodeList',
  },
  Import: {},
  ClassProperty: {
    key: 'Node',
    value: 'Node',
    variance: 'Node',
    typeAnnotation: 'Node',
  },
  ClassPrivateProperty: {
    key: 'Node',
    value: 'Node',
    variance: 'Node',
    typeAnnotation: 'Node',
  },
  PrivateName: {
    id: 'Node',
  },
  OptionalCallExpression: {
    callee: 'Node',
    typeArguments: 'Node',
    arguments: 'NodeList',
  },
  OptionalMemberExpression: {
    object: 'Node',
    property: 'Node',
  },
};
