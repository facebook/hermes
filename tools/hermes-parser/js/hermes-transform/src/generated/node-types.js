/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
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

import type {
  ESNode,
  AnyTypeAnnotation as AnyTypeAnnotationType,
  ArrayExpression as ArrayExpressionType,
  ArrayPattern as ArrayPatternType,
  ArrayTypeAnnotation as ArrayTypeAnnotationType,
  AssignmentExpression as AssignmentExpressionType,
  AssignmentPattern as AssignmentPatternType,
  AwaitExpression as AwaitExpressionType,
  BigIntLiteralTypeAnnotation as BigIntLiteralTypeAnnotationType,
  BinaryExpression as BinaryExpressionType,
  BlockStatement as BlockStatementType,
  BooleanLiteralTypeAnnotation as BooleanLiteralTypeAnnotationType,
  BooleanTypeAnnotation as BooleanTypeAnnotationType,
  BreakStatement as BreakStatementType,
  CallExpression as CallExpressionType,
  CatchClause as CatchClauseType,
  ChainExpression as ChainExpressionType,
  ClassBody as ClassBodyType,
  ClassExpression as ClassExpressionType,
  ClassImplements as ClassImplementsType,
  ConditionalExpression as ConditionalExpressionType,
  ContinueStatement as ContinueStatementType,
  DebuggerStatement as DebuggerStatementType,
  DeclareClass as DeclareClassType,
  DeclaredPredicate as DeclaredPredicateType,
  DeclareExportAllDeclaration as DeclareExportAllDeclarationType,
  DeclareInterface as DeclareInterfaceType,
  DeclareModule as DeclareModuleType,
  DeclareModuleExports as DeclareModuleExportsType,
  DeclareOpaqueType as DeclareOpaqueTypeType,
  DeclareTypeAlias as DeclareTypeAliasType,
  DeclareVariable as DeclareVariableType,
  DoWhileStatement as DoWhileStatementType,
  EmptyStatement as EmptyStatementType,
  EmptyTypeAnnotation as EmptyTypeAnnotationType,
  EnumBooleanBody as EnumBooleanBodyType,
  EnumBooleanMember as EnumBooleanMemberType,
  EnumDeclaration as EnumDeclarationType,
  EnumDefaultedMember as EnumDefaultedMemberType,
  EnumNumberBody as EnumNumberBodyType,
  EnumNumberMember as EnumNumberMemberType,
  EnumStringBody as EnumStringBodyType,
  EnumStringMember as EnumStringMemberType,
  EnumSymbolBody as EnumSymbolBodyType,
  ExistsTypeAnnotation as ExistsTypeAnnotationType,
  ExportAllDeclaration as ExportAllDeclarationType,
  ExportDefaultDeclaration as ExportDefaultDeclarationType,
  ExportSpecifier as ExportSpecifierType,
  ExpressionStatement as ExpressionStatementType,
  ForInStatement as ForInStatementType,
  ForOfStatement as ForOfStatementType,
  ForStatement as ForStatementType,
  FunctionDeclaration as FunctionDeclarationType,
  FunctionExpression as FunctionExpressionType,
  FunctionTypeAnnotation as FunctionTypeAnnotationType,
  FunctionTypeParam as FunctionTypeParamType,
  GenericTypeAnnotation as GenericTypeAnnotationType,
  IfStatement as IfStatementType,
  ImportAttribute as ImportAttributeType,
  ImportDeclaration as ImportDeclarationType,
  ImportDefaultSpecifier as ImportDefaultSpecifierType,
  ImportExpression as ImportExpressionType,
  ImportNamespaceSpecifier as ImportNamespaceSpecifierType,
  ImportSpecifier as ImportSpecifierType,
  IndexedAccessType as IndexedAccessTypeType,
  InferredPredicate as InferredPredicateType,
  InterfaceDeclaration as InterfaceDeclarationType,
  InterfaceExtends as InterfaceExtendsType,
  InterfaceTypeAnnotation as InterfaceTypeAnnotationType,
  IntersectionTypeAnnotation as IntersectionTypeAnnotationType,
  JSXAttribute as JSXAttributeType,
  JSXClosingElement as JSXClosingElementType,
  JSXClosingFragment as JSXClosingFragmentType,
  JSXElement as JSXElementType,
  JSXEmptyExpression as JSXEmptyExpressionType,
  JSXExpressionContainer as JSXExpressionContainerType,
  JSXFragment as JSXFragmentType,
  JSXIdentifier as JSXIdentifierType,
  JSXMemberExpression as JSXMemberExpressionType,
  JSXNamespacedName as JSXNamespacedNameType,
  JSXOpeningElement as JSXOpeningElementType,
  JSXOpeningFragment as JSXOpeningFragmentType,
  JSXSpreadAttribute as JSXSpreadAttributeType,
  JSXSpreadChild as JSXSpreadChildType,
  JSXText as JSXTextType,
  LabeledStatement as LabeledStatementType,
  LogicalExpression as LogicalExpressionType,
  MemberExpression as MemberExpressionType,
  MetaProperty as MetaPropertyType,
  MethodDefinition as MethodDefinitionType,
  MixedTypeAnnotation as MixedTypeAnnotationType,
  NewExpression as NewExpressionType,
  NullableTypeAnnotation as NullableTypeAnnotationType,
  NullLiteralTypeAnnotation as NullLiteralTypeAnnotationType,
  NumberLiteralTypeAnnotation as NumberLiteralTypeAnnotationType,
  NumberTypeAnnotation as NumberTypeAnnotationType,
  ObjectExpression as ObjectExpressionType,
  ObjectPattern as ObjectPatternType,
  ObjectTypeAnnotation as ObjectTypeAnnotationType,
  ObjectTypeCallProperty as ObjectTypeCallPropertyType,
  ObjectTypeIndexer as ObjectTypeIndexerType,
  ObjectTypeInternalSlot as ObjectTypeInternalSlotType,
  ObjectTypeSpreadProperty as ObjectTypeSpreadPropertyType,
  OpaqueType as OpaqueTypeType,
  OptionalIndexedAccessType as OptionalIndexedAccessTypeType,
  PrivateIdentifier as PrivateIdentifierType,
  Property as PropertyType,
  PropertyDefinition as PropertyDefinitionType,
  QualifiedTypeIdentifier as QualifiedTypeIdentifierType,
  RestElement as RestElementType,
  ReturnStatement as ReturnStatementType,
  SequenceExpression as SequenceExpressionType,
  SpreadElement as SpreadElementType,
  StringLiteralTypeAnnotation as StringLiteralTypeAnnotationType,
  StringTypeAnnotation as StringTypeAnnotationType,
  Super as SuperType,
  SwitchCase as SwitchCaseType,
  SwitchStatement as SwitchStatementType,
  SymbolTypeAnnotation as SymbolTypeAnnotationType,
  TaggedTemplateExpression as TaggedTemplateExpressionType,
  TemplateLiteral as TemplateLiteralType,
  ThisExpression as ThisExpressionType,
  ThisTypeAnnotation as ThisTypeAnnotationType,
  ThrowStatement as ThrowStatementType,
  TryStatement as TryStatementType,
  TupleTypeAnnotation as TupleTypeAnnotationType,
  TypeAlias as TypeAliasType,
  TypeAnnotation as TypeAnnotationType,
  TypeCastExpression as TypeCastExpressionType,
  TypeofTypeAnnotation as TypeofTypeAnnotationType,
  TypeParameter as TypeParameterType,
  TypeParameterDeclaration as TypeParameterDeclarationType,
  TypeParameterInstantiation as TypeParameterInstantiationType,
  UnaryExpression as UnaryExpressionType,
  UnionTypeAnnotation as UnionTypeAnnotationType,
  UpdateExpression as UpdateExpressionType,
  VariableDeclaration as VariableDeclarationType,
  VariableDeclarator as VariableDeclaratorType,
  Variance as VarianceType,
  VoidTypeAnnotation as VoidTypeAnnotationType,
  WhileStatement as WhileStatementType,
  WithStatement as WithStatementType,
  YieldExpression as YieldExpressionType,
} from 'hermes-estree';
import type {DetachedNode, MaybeDetachedNode} from '../detachedNode';

import {
  asDetachedNode,
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

export type AnyTypeAnnotationProps = {};

export type ArrayExpressionProps = {
  +elements: $ReadOnlyArray<
    MaybeDetachedNode<ArrayExpressionType['elements'][number]>,
  >,
  +trailingComma: ArrayExpressionType['trailingComma'],
};

export type ArrayPatternProps = {
  +elements: $ReadOnlyArray<
    MaybeDetachedNode<ArrayPatternType['elements'][number]>,
  >,
  +typeAnnotation?: ?MaybeDetachedNode<ArrayPatternType['typeAnnotation']>,
};

export type ArrayTypeAnnotationProps = {
  +elementType: MaybeDetachedNode<ArrayTypeAnnotationType['elementType']>,
};

export type AssignmentExpressionProps = {
  +operator: AssignmentExpressionType['operator'],
  +left: MaybeDetachedNode<AssignmentExpressionType['left']>,
  +right: MaybeDetachedNode<AssignmentExpressionType['right']>,
};

export type AssignmentPatternProps = {
  +left: MaybeDetachedNode<AssignmentPatternType['left']>,
  +right: MaybeDetachedNode<AssignmentPatternType['right']>,
};

export type AwaitExpressionProps = {
  +argument: MaybeDetachedNode<AwaitExpressionType['argument']>,
};

export type BigIntLiteralTypeAnnotationProps = {
  +raw: BigIntLiteralTypeAnnotationType['raw'],
};

export type BinaryExpressionProps = {
  +left: MaybeDetachedNode<BinaryExpressionType['left']>,
  +right: MaybeDetachedNode<BinaryExpressionType['right']>,
  +operator: BinaryExpressionType['operator'],
};

export type BlockStatementProps = {
  +body: $ReadOnlyArray<MaybeDetachedNode<BlockStatementType['body'][number]>>,
};

export type BooleanLiteralTypeAnnotationProps = {
  +value: BooleanLiteralTypeAnnotationType['value'],
  +raw: BooleanLiteralTypeAnnotationType['raw'],
};

export type BooleanTypeAnnotationProps = {};

export type BreakStatementProps = {
  +label?: ?MaybeDetachedNode<BreakStatementType['label']>,
};

export type CallExpressionProps = {
  +callee: MaybeDetachedNode<CallExpressionType['callee']>,
  +typeArguments?: ?MaybeDetachedNode<CallExpressionType['typeArguments']>,
  +arguments: $ReadOnlyArray<
    MaybeDetachedNode<CallExpressionType['arguments'][number]>,
  >,
};

export type CatchClauseProps = {
  +param?: ?MaybeDetachedNode<CatchClauseType['param']>,
  +body: MaybeDetachedNode<CatchClauseType['body']>,
};

export type ChainExpressionProps = {
  +expression: MaybeDetachedNode<ChainExpressionType['expression']>,
};

export type ClassBodyProps = {
  +body: $ReadOnlyArray<MaybeDetachedNode<ClassBodyType['body'][number]>>,
};

export type ClassExpressionProps = {
  +id?: ?MaybeDetachedNode<ClassExpressionType['id']>,
  +typeParameters?: ?MaybeDetachedNode<ClassExpressionType['typeParameters']>,
  +superClass?: ?MaybeDetachedNode<ClassExpressionType['superClass']>,
  +superTypeParameters?: ?MaybeDetachedNode<
    ClassExpressionType['superTypeParameters'],
  >,
  +implements: $ReadOnlyArray<
    MaybeDetachedNode<ClassExpressionType['implements'][number]>,
  >,
  +decorators: $ReadOnlyArray<
    MaybeDetachedNode<ClassExpressionType['decorators'][number]>,
  >,
  +body: MaybeDetachedNode<ClassExpressionType['body']>,
};

export type ClassImplementsProps = {
  +id: MaybeDetachedNode<ClassImplementsType['id']>,
  +typeParameters?: ?MaybeDetachedNode<ClassImplementsType['typeParameters']>,
};

export type ConditionalExpressionProps = {
  +test: MaybeDetachedNode<ConditionalExpressionType['test']>,
  +alternate: MaybeDetachedNode<ConditionalExpressionType['alternate']>,
  +consequent: MaybeDetachedNode<ConditionalExpressionType['consequent']>,
};

export type ContinueStatementProps = {
  +label?: ?MaybeDetachedNode<ContinueStatementType['label']>,
};

export type DebuggerStatementProps = {};

export type DeclareClassProps = {
  +id: MaybeDetachedNode<DeclareClassType['id']>,
  +typeParameters?: ?MaybeDetachedNode<DeclareClassType['typeParameters']>,
  +extends: $ReadOnlyArray<
    MaybeDetachedNode<DeclareClassType['extends'][number]>,
  >,
  +implements: $ReadOnlyArray<
    MaybeDetachedNode<DeclareClassType['implements'][number]>,
  >,
  +mixins: $ReadOnlyArray<
    MaybeDetachedNode<DeclareClassType['mixins'][number]>,
  >,
  +body: MaybeDetachedNode<DeclareClassType['body']>,
};

export type DeclaredPredicateProps = {
  +value: MaybeDetachedNode<DeclaredPredicateType['value']>,
};

export type DeclareExportAllDeclarationProps = {
  +source: MaybeDetachedNode<DeclareExportAllDeclarationType['source']>,
};

export type DeclareInterfaceProps = {
  +id: MaybeDetachedNode<DeclareInterfaceType['id']>,
  +typeParameters?: ?MaybeDetachedNode<DeclareInterfaceType['typeParameters']>,
  +extends: $ReadOnlyArray<
    MaybeDetachedNode<DeclareInterfaceType['extends'][number]>,
  >,
  +body: MaybeDetachedNode<DeclareInterfaceType['body']>,
};

export type DeclareModuleProps = {
  +id: MaybeDetachedNode<DeclareModuleType['id']>,
  +body: MaybeDetachedNode<DeclareModuleType['body']>,
  +kind: DeclareModuleType['kind'],
};

export type DeclareModuleExportsProps = {
  +typeAnnotation: MaybeDetachedNode<
    DeclareModuleExportsType['typeAnnotation'],
  >,
};

export type DeclareOpaqueTypeProps = {
  +id: MaybeDetachedNode<DeclareOpaqueTypeType['id']>,
  +typeParameters?: ?MaybeDetachedNode<DeclareOpaqueTypeType['typeParameters']>,
  +impltype?: ?MaybeDetachedNode<DeclareOpaqueTypeType['impltype']>,
  +supertype?: ?MaybeDetachedNode<DeclareOpaqueTypeType['supertype']>,
};

export type DeclareTypeAliasProps = {
  +id: MaybeDetachedNode<DeclareTypeAliasType['id']>,
  +typeParameters?: ?MaybeDetachedNode<DeclareTypeAliasType['typeParameters']>,
  +right: MaybeDetachedNode<DeclareTypeAliasType['right']>,
};

export type DeclareVariableProps = {
  +id: MaybeDetachedNode<DeclareVariableType['id']>,
};

export type DoWhileStatementProps = {
  +body: MaybeDetachedNode<DoWhileStatementType['body']>,
  +test: MaybeDetachedNode<DoWhileStatementType['test']>,
};

export type EmptyStatementProps = {};

export type EmptyTypeAnnotationProps = {};

export type EnumBooleanBodyProps = {
  +members: $ReadOnlyArray<
    MaybeDetachedNode<EnumBooleanBodyType['members'][number]>,
  >,
  +explicitType: EnumBooleanBodyType['explicitType'],
  +hasUnknownMembers: EnumBooleanBodyType['hasUnknownMembers'],
};

export type EnumBooleanMemberProps = {
  +id: MaybeDetachedNode<EnumBooleanMemberType['id']>,
  +init: MaybeDetachedNode<EnumBooleanMemberType['init']>,
};

export type EnumDeclarationProps = {
  +id: MaybeDetachedNode<EnumDeclarationType['id']>,
  +body: MaybeDetachedNode<EnumDeclarationType['body']>,
};

export type EnumDefaultedMemberProps = {
  +id: MaybeDetachedNode<EnumDefaultedMemberType['id']>,
};

export type EnumNumberBodyProps = {
  +members: $ReadOnlyArray<
    MaybeDetachedNode<EnumNumberBodyType['members'][number]>,
  >,
  +explicitType: EnumNumberBodyType['explicitType'],
  +hasUnknownMembers: EnumNumberBodyType['hasUnknownMembers'],
};

export type EnumNumberMemberProps = {
  +id: MaybeDetachedNode<EnumNumberMemberType['id']>,
  +init: MaybeDetachedNode<EnumNumberMemberType['init']>,
};

export type EnumStringBodyProps = {
  +members: $ReadOnlyArray<
    MaybeDetachedNode<EnumStringBodyType['members'][number]>,
  >,
  +explicitType: EnumStringBodyType['explicitType'],
  +hasUnknownMembers: EnumStringBodyType['hasUnknownMembers'],
};

export type EnumStringMemberProps = {
  +id: MaybeDetachedNode<EnumStringMemberType['id']>,
  +init: MaybeDetachedNode<EnumStringMemberType['init']>,
};

export type EnumSymbolBodyProps = {
  +members: $ReadOnlyArray<
    MaybeDetachedNode<EnumSymbolBodyType['members'][number]>,
  >,
  +hasUnknownMembers: EnumSymbolBodyType['hasUnknownMembers'],
};

export type ExistsTypeAnnotationProps = {};

export type ExportAllDeclarationProps = {
  +exported?: ?MaybeDetachedNode<ExportAllDeclarationType['exported']>,
  +source: MaybeDetachedNode<ExportAllDeclarationType['source']>,
  +exportKind: ExportAllDeclarationType['exportKind'],
};

export type ExportDefaultDeclarationProps = {
  +declaration: MaybeDetachedNode<ExportDefaultDeclarationType['declaration']>,
};

export type ExportSpecifierProps = {
  +exported: MaybeDetachedNode<ExportSpecifierType['exported']>,
  +local: MaybeDetachedNode<ExportSpecifierType['local']>,
};

export type ExpressionStatementProps = {
  +expression: MaybeDetachedNode<ExpressionStatementType['expression']>,
  +directive?: ?ExpressionStatementType['directive'],
};

export type ForInStatementProps = {
  +left: MaybeDetachedNode<ForInStatementType['left']>,
  +right: MaybeDetachedNode<ForInStatementType['right']>,
  +body: MaybeDetachedNode<ForInStatementType['body']>,
};

export type ForOfStatementProps = {
  +left: MaybeDetachedNode<ForOfStatementType['left']>,
  +right: MaybeDetachedNode<ForOfStatementType['right']>,
  +body: MaybeDetachedNode<ForOfStatementType['body']>,
  +await: ForOfStatementType['await'],
};

export type ForStatementProps = {
  +init?: ?MaybeDetachedNode<ForStatementType['init']>,
  +test?: ?MaybeDetachedNode<ForStatementType['test']>,
  +update?: ?MaybeDetachedNode<ForStatementType['update']>,
  +body: MaybeDetachedNode<ForStatementType['body']>,
};

export type FunctionDeclarationProps = {
  +id?: ?MaybeDetachedNode<FunctionDeclarationType['id']>,
  +params: $ReadOnlyArray<
    MaybeDetachedNode<FunctionDeclarationType['params'][number]>,
  >,
  +body: MaybeDetachedNode<FunctionDeclarationType['body']>,
  +typeParameters?: ?MaybeDetachedNode<
    FunctionDeclarationType['typeParameters'],
  >,
  +returnType?: ?MaybeDetachedNode<FunctionDeclarationType['returnType']>,
  +predicate?: ?MaybeDetachedNode<FunctionDeclarationType['predicate']>,
  +generator: FunctionDeclarationType['generator'],
  +async: FunctionDeclarationType['async'],
};

export type FunctionExpressionProps = {
  +id?: ?MaybeDetachedNode<FunctionExpressionType['id']>,
  +params: $ReadOnlyArray<
    MaybeDetachedNode<FunctionExpressionType['params'][number]>,
  >,
  +body: MaybeDetachedNode<FunctionExpressionType['body']>,
  +typeParameters?: ?MaybeDetachedNode<
    FunctionExpressionType['typeParameters'],
  >,
  +returnType?: ?MaybeDetachedNode<FunctionExpressionType['returnType']>,
  +predicate?: ?MaybeDetachedNode<FunctionExpressionType['predicate']>,
  +generator: FunctionExpressionType['generator'],
  +async: FunctionExpressionType['async'],
};

export type FunctionTypeAnnotationProps = {
  +params: $ReadOnlyArray<
    MaybeDetachedNode<FunctionTypeAnnotationType['params'][number]>,
  >,
  +this?: ?MaybeDetachedNode<FunctionTypeAnnotationType['this']>,
  +returnType: MaybeDetachedNode<FunctionTypeAnnotationType['returnType']>,
  +rest?: ?MaybeDetachedNode<FunctionTypeAnnotationType['rest']>,
  +typeParameters?: ?MaybeDetachedNode<
    FunctionTypeAnnotationType['typeParameters'],
  >,
};

export type FunctionTypeParamProps = {
  +name?: ?MaybeDetachedNode<FunctionTypeParamType['name']>,
  +typeAnnotation: MaybeDetachedNode<FunctionTypeParamType['typeAnnotation']>,
  +optional: FunctionTypeParamType['optional'],
};

export type GenericTypeAnnotationProps = {
  +id: MaybeDetachedNode<GenericTypeAnnotationType['id']>,
  +typeParameters?: ?MaybeDetachedNode<
    GenericTypeAnnotationType['typeParameters'],
  >,
};

export type IfStatementProps = {
  +test: MaybeDetachedNode<IfStatementType['test']>,
  +consequent: MaybeDetachedNode<IfStatementType['consequent']>,
  +alternate?: ?MaybeDetachedNode<IfStatementType['alternate']>,
};

export type ImportAttributeProps = {
  +key: MaybeDetachedNode<ImportAttributeType['key']>,
  +value: MaybeDetachedNode<ImportAttributeType['value']>,
};

export type ImportDeclarationProps = {
  +specifiers: $ReadOnlyArray<
    MaybeDetachedNode<ImportDeclarationType['specifiers'][number]>,
  >,
  +source: MaybeDetachedNode<ImportDeclarationType['source']>,
  +assertions?: ?$ReadOnlyArray<
    MaybeDetachedNode<ImportDeclarationType['assertions'][number]>,
  >,
  +importKind: ImportDeclarationType['importKind'],
};

export type ImportDefaultSpecifierProps = {
  +local: MaybeDetachedNode<ImportDefaultSpecifierType['local']>,
};

export type ImportExpressionProps = {
  +source: MaybeDetachedNode<ImportExpressionType['source']>,
  +attributes?: ?MaybeDetachedNode<ImportExpressionType['attributes']>,
};

export type ImportNamespaceSpecifierProps = {
  +local: MaybeDetachedNode<ImportNamespaceSpecifierType['local']>,
};

export type ImportSpecifierProps = {
  +imported: MaybeDetachedNode<ImportSpecifierType['imported']>,
  +local: MaybeDetachedNode<ImportSpecifierType['local']>,
  +importKind: ImportSpecifierType['importKind'],
};

export type IndexedAccessTypeProps = {
  +objectType: MaybeDetachedNode<IndexedAccessTypeType['objectType']>,
  +indexType: MaybeDetachedNode<IndexedAccessTypeType['indexType']>,
};

export type InferredPredicateProps = {};

export type InterfaceDeclarationProps = {
  +id: MaybeDetachedNode<InterfaceDeclarationType['id']>,
  +typeParameters?: ?MaybeDetachedNode<
    InterfaceDeclarationType['typeParameters'],
  >,
  +extends: $ReadOnlyArray<
    MaybeDetachedNode<InterfaceDeclarationType['extends'][number]>,
  >,
  +body: MaybeDetachedNode<InterfaceDeclarationType['body']>,
};

export type InterfaceExtendsProps = {
  +id: MaybeDetachedNode<InterfaceExtendsType['id']>,
  +typeParameters?: ?MaybeDetachedNode<InterfaceExtendsType['typeParameters']>,
};

export type InterfaceTypeAnnotationProps = {
  +extends: $ReadOnlyArray<
    MaybeDetachedNode<InterfaceTypeAnnotationType['extends'][number]>,
  >,
  +body?: ?MaybeDetachedNode<InterfaceTypeAnnotationType['body']>,
};

export type IntersectionTypeAnnotationProps = {
  +types: $ReadOnlyArray<
    MaybeDetachedNode<IntersectionTypeAnnotationType['types'][number]>,
  >,
};

export type JSXAttributeProps = {
  +name: MaybeDetachedNode<JSXAttributeType['name']>,
  +value?: ?MaybeDetachedNode<JSXAttributeType['value']>,
};

export type JSXClosingElementProps = {
  +name: MaybeDetachedNode<JSXClosingElementType['name']>,
};

export type JSXClosingFragmentProps = {};

export type JSXElementProps = {
  +openingElement: MaybeDetachedNode<JSXElementType['openingElement']>,
  +children: $ReadOnlyArray<
    MaybeDetachedNode<JSXElementType['children'][number]>,
  >,
  +closingElement?: ?MaybeDetachedNode<JSXElementType['closingElement']>,
};

export type JSXEmptyExpressionProps = {};

export type JSXExpressionContainerProps = {
  +expression: MaybeDetachedNode<JSXExpressionContainerType['expression']>,
};

export type JSXFragmentProps = {
  +openingFragment: MaybeDetachedNode<JSXFragmentType['openingFragment']>,
  +children: $ReadOnlyArray<
    MaybeDetachedNode<JSXFragmentType['children'][number]>,
  >,
  +closingFragment: MaybeDetachedNode<JSXFragmentType['closingFragment']>,
};

export type JSXIdentifierProps = {
  +name: JSXIdentifierType['name'],
};

export type JSXMemberExpressionProps = {
  +object: MaybeDetachedNode<JSXMemberExpressionType['object']>,
  +property: MaybeDetachedNode<JSXMemberExpressionType['property']>,
};

export type JSXNamespacedNameProps = {
  +namespace: MaybeDetachedNode<JSXNamespacedNameType['namespace']>,
  +name: MaybeDetachedNode<JSXNamespacedNameType['name']>,
};

export type JSXOpeningElementProps = {
  +name: MaybeDetachedNode<JSXOpeningElementType['name']>,
  +attributes: $ReadOnlyArray<
    MaybeDetachedNode<JSXOpeningElementType['attributes'][number]>,
  >,
  +selfClosing: JSXOpeningElementType['selfClosing'],
};

export type JSXOpeningFragmentProps = {};

export type JSXSpreadAttributeProps = {
  +argument: MaybeDetachedNode<JSXSpreadAttributeType['argument']>,
};

export type JSXSpreadChildProps = {
  +expression: MaybeDetachedNode<JSXSpreadChildType['expression']>,
};

export type JSXTextProps = {
  +value: JSXTextType['value'],
  +raw: JSXTextType['raw'],
};

export type LabeledStatementProps = {
  +label: MaybeDetachedNode<LabeledStatementType['label']>,
  +body: MaybeDetachedNode<LabeledStatementType['body']>,
};

export type LogicalExpressionProps = {
  +left: MaybeDetachedNode<LogicalExpressionType['left']>,
  +right: MaybeDetachedNode<LogicalExpressionType['right']>,
  +operator: LogicalExpressionType['operator'],
};

export type MemberExpressionProps = {
  +object: MaybeDetachedNode<MemberExpressionType['object']>,
  +property: MaybeDetachedNode<MemberExpressionType['property']>,
  +computed: MemberExpressionType['computed'],
};

export type MetaPropertyProps = {
  +meta: MaybeDetachedNode<MetaPropertyType['meta']>,
  +property: MaybeDetachedNode<MetaPropertyType['property']>,
};

export type MethodDefinitionProps = {
  +key: MaybeDetachedNode<MethodDefinitionType['key']>,
  +value: MaybeDetachedNode<MethodDefinitionType['value']>,
  +kind: MethodDefinitionType['kind'],
  +computed: MethodDefinitionType['computed'],
  +static: MethodDefinitionType['static'],
};

export type MixedTypeAnnotationProps = {};

export type NewExpressionProps = {
  +callee: MaybeDetachedNode<NewExpressionType['callee']>,
  +typeArguments?: ?MaybeDetachedNode<NewExpressionType['typeArguments']>,
  +arguments: $ReadOnlyArray<
    MaybeDetachedNode<NewExpressionType['arguments'][number]>,
  >,
};

export type NullableTypeAnnotationProps = {
  +typeAnnotation: MaybeDetachedNode<
    NullableTypeAnnotationType['typeAnnotation'],
  >,
};

export type NullLiteralTypeAnnotationProps = {};

export type NumberLiteralTypeAnnotationProps = {
  +value: NumberLiteralTypeAnnotationType['value'],
  +raw: NumberLiteralTypeAnnotationType['raw'],
};

export type NumberTypeAnnotationProps = {};

export type ObjectExpressionProps = {
  +properties: $ReadOnlyArray<
    MaybeDetachedNode<ObjectExpressionType['properties'][number]>,
  >,
};

export type ObjectPatternProps = {
  +properties: $ReadOnlyArray<
    MaybeDetachedNode<ObjectPatternType['properties'][number]>,
  >,
  +typeAnnotation?: ?MaybeDetachedNode<ObjectPatternType['typeAnnotation']>,
};

export type ObjectTypeAnnotationProps = {
  +properties: $ReadOnlyArray<
    MaybeDetachedNode<ObjectTypeAnnotationType['properties'][number]>,
  >,
  +indexers: $ReadOnlyArray<
    MaybeDetachedNode<ObjectTypeAnnotationType['indexers'][number]>,
  >,
  +callProperties: $ReadOnlyArray<
    MaybeDetachedNode<ObjectTypeAnnotationType['callProperties'][number]>,
  >,
  +internalSlots: $ReadOnlyArray<
    MaybeDetachedNode<ObjectTypeAnnotationType['internalSlots'][number]>,
  >,
  +inexact: ObjectTypeAnnotationType['inexact'],
  +exact: ObjectTypeAnnotationType['exact'],
};

export type ObjectTypeCallPropertyProps = {
  +value: MaybeDetachedNode<ObjectTypeCallPropertyType['value']>,
  +static: ObjectTypeCallPropertyType['static'],
};

export type ObjectTypeIndexerProps = {
  +id?: ?MaybeDetachedNode<ObjectTypeIndexerType['id']>,
  +key: MaybeDetachedNode<ObjectTypeIndexerType['key']>,
  +value: MaybeDetachedNode<ObjectTypeIndexerType['value']>,
  +static: ObjectTypeIndexerType['static'],
  +variance?: ?MaybeDetachedNode<ObjectTypeIndexerType['variance']>,
};

export type ObjectTypeInternalSlotProps = {
  +id: MaybeDetachedNode<ObjectTypeInternalSlotType['id']>,
  +value: MaybeDetachedNode<ObjectTypeInternalSlotType['value']>,
  +optional: ObjectTypeInternalSlotType['optional'],
  +static: ObjectTypeInternalSlotType['static'],
  +method: ObjectTypeInternalSlotType['method'],
};

export type ObjectTypeSpreadPropertyProps = {
  +argument: MaybeDetachedNode<ObjectTypeSpreadPropertyType['argument']>,
};

export type OpaqueTypeProps = {
  +id: MaybeDetachedNode<OpaqueTypeType['id']>,
  +typeParameters?: ?MaybeDetachedNode<OpaqueTypeType['typeParameters']>,
  +impltype: MaybeDetachedNode<OpaqueTypeType['impltype']>,
  +supertype?: ?MaybeDetachedNode<OpaqueTypeType['supertype']>,
};

export type OptionalIndexedAccessTypeProps = {
  +objectType: MaybeDetachedNode<OptionalIndexedAccessTypeType['objectType']>,
  +indexType: MaybeDetachedNode<OptionalIndexedAccessTypeType['indexType']>,
  +optional: OptionalIndexedAccessTypeType['optional'],
};

export type PrivateIdentifierProps = {
  +name: PrivateIdentifierType['name'],
};

export type PropertyProps = {
  +key: MaybeDetachedNode<PropertyType['key']>,
  +value: MaybeDetachedNode<PropertyType['value']>,
  +kind: PropertyType['kind'],
  +computed: PropertyType['computed'],
  +method: PropertyType['method'],
  +shorthand: PropertyType['shorthand'],
};

export type PropertyDefinitionProps = {
  +key: MaybeDetachedNode<PropertyDefinitionType['key']>,
  +value?: ?MaybeDetachedNode<PropertyDefinitionType['value']>,
  +computed: PropertyDefinitionType['computed'],
  +static: PropertyDefinitionType['static'],
  +declare: PropertyDefinitionType['declare'],
  +optional: PropertyDefinitionType['optional'],
  +variance?: ?MaybeDetachedNode<PropertyDefinitionType['variance']>,
  +typeAnnotation?: ?MaybeDetachedNode<
    PropertyDefinitionType['typeAnnotation'],
  >,
};

export type QualifiedTypeIdentifierProps = {
  +qualification: MaybeDetachedNode<
    QualifiedTypeIdentifierType['qualification'],
  >,
  +id: MaybeDetachedNode<QualifiedTypeIdentifierType['id']>,
};

export type RestElementProps = {
  +argument: MaybeDetachedNode<RestElementType['argument']>,
};

export type ReturnStatementProps = {
  +argument?: ?MaybeDetachedNode<ReturnStatementType['argument']>,
};

export type SequenceExpressionProps = {
  +expressions: $ReadOnlyArray<
    MaybeDetachedNode<SequenceExpressionType['expressions'][number]>,
  >,
};

export type SpreadElementProps = {
  +argument: MaybeDetachedNode<SpreadElementType['argument']>,
};

export type StringLiteralTypeAnnotationProps = {
  +value: StringLiteralTypeAnnotationType['value'],
  +raw: StringLiteralTypeAnnotationType['raw'],
};

export type StringTypeAnnotationProps = {};

export type SuperProps = {};

export type SwitchCaseProps = {
  +test?: ?MaybeDetachedNode<SwitchCaseType['test']>,
  +consequent: $ReadOnlyArray<
    MaybeDetachedNode<SwitchCaseType['consequent'][number]>,
  >,
};

export type SwitchStatementProps = {
  +discriminant: MaybeDetachedNode<SwitchStatementType['discriminant']>,
  +cases: $ReadOnlyArray<
    MaybeDetachedNode<SwitchStatementType['cases'][number]>,
  >,
};

export type SymbolTypeAnnotationProps = {};

export type TaggedTemplateExpressionProps = {
  +tag: MaybeDetachedNode<TaggedTemplateExpressionType['tag']>,
  +quasi: MaybeDetachedNode<TaggedTemplateExpressionType['quasi']>,
};

export type TemplateLiteralProps = {
  +quasis: $ReadOnlyArray<
    MaybeDetachedNode<TemplateLiteralType['quasis'][number]>,
  >,
  +expressions: $ReadOnlyArray<
    MaybeDetachedNode<TemplateLiteralType['expressions'][number]>,
  >,
};

export type ThisExpressionProps = {};

export type ThisTypeAnnotationProps = {};

export type ThrowStatementProps = {
  +argument: MaybeDetachedNode<ThrowStatementType['argument']>,
};

export type TryStatementProps = {
  +block: MaybeDetachedNode<TryStatementType['block']>,
  +handler?: ?MaybeDetachedNode<TryStatementType['handler']>,
  +finalizer?: ?MaybeDetachedNode<TryStatementType['finalizer']>,
};

export type TupleTypeAnnotationProps = {
  +types: $ReadOnlyArray<
    MaybeDetachedNode<TupleTypeAnnotationType['types'][number]>,
  >,
};

export type TypeAliasProps = {
  +id: MaybeDetachedNode<TypeAliasType['id']>,
  +typeParameters?: ?MaybeDetachedNode<TypeAliasType['typeParameters']>,
  +right: MaybeDetachedNode<TypeAliasType['right']>,
};

export type TypeAnnotationProps = {
  +typeAnnotation: MaybeDetachedNode<TypeAnnotationType['typeAnnotation']>,
};

export type TypeCastExpressionProps = {
  +expression: MaybeDetachedNode<TypeCastExpressionType['expression']>,
  +typeAnnotation: MaybeDetachedNode<TypeCastExpressionType['typeAnnotation']>,
};

export type TypeofTypeAnnotationProps = {
  +argument: MaybeDetachedNode<TypeofTypeAnnotationType['argument']>,
};

export type TypeParameterProps = {
  +name: TypeParameterType['name'],
  +bound?: ?MaybeDetachedNode<TypeParameterType['bound']>,
  +variance?: ?MaybeDetachedNode<TypeParameterType['variance']>,
  +default?: ?MaybeDetachedNode<TypeParameterType['default']>,
};

export type TypeParameterDeclarationProps = {
  +params: $ReadOnlyArray<
    MaybeDetachedNode<TypeParameterDeclarationType['params'][number]>,
  >,
};

export type TypeParameterInstantiationProps = {
  +params: $ReadOnlyArray<
    MaybeDetachedNode<TypeParameterInstantiationType['params'][number]>,
  >,
};

export type UnaryExpressionProps = {
  +operator: UnaryExpressionType['operator'],
  +argument: MaybeDetachedNode<UnaryExpressionType['argument']>,
  +prefix: UnaryExpressionType['prefix'],
};

export type UnionTypeAnnotationProps = {
  +types: $ReadOnlyArray<
    MaybeDetachedNode<UnionTypeAnnotationType['types'][number]>,
  >,
};

export type UpdateExpressionProps = {
  +operator: UpdateExpressionType['operator'],
  +argument: MaybeDetachedNode<UpdateExpressionType['argument']>,
  +prefix: UpdateExpressionType['prefix'],
};

export type VariableDeclarationProps = {
  +kind: VariableDeclarationType['kind'],
  +declarations: $ReadOnlyArray<
    MaybeDetachedNode<VariableDeclarationType['declarations'][number]>,
  >,
};

export type VariableDeclaratorProps = {
  +init?: ?MaybeDetachedNode<VariableDeclaratorType['init']>,
  +id: MaybeDetachedNode<VariableDeclaratorType['id']>,
};

export type VarianceProps = {
  +kind: VarianceType['kind'],
};

export type VoidTypeAnnotationProps = {};

export type WhileStatementProps = {
  +body: MaybeDetachedNode<WhileStatementType['body']>,
  +test: MaybeDetachedNode<WhileStatementType['test']>,
};

export type WithStatementProps = {
  +object: MaybeDetachedNode<WithStatementType['object']>,
  +body: MaybeDetachedNode<WithStatementType['body']>,
};

export type YieldExpressionProps = {
  +argument?: ?MaybeDetachedNode<YieldExpressionType['argument']>,
  +delegate?: ?YieldExpressionType['delegate'],
};

export function AnyTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<AnyTypeAnnotationType> {
  return detachedProps<AnyTypeAnnotationType>(props.parent, {
    type: 'AnyTypeAnnotation',
  });
}

export function ArrayExpression(props: {
  ...$ReadOnly<ArrayExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ArrayExpressionType> {
  const node = detachedProps<ArrayExpressionType>(props.parent, {
    type: 'ArrayExpression',
    elements: props.elements.map(n => asDetachedNode(n)),
    trailingComma: props.trailingComma,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ArrayPattern(props: {
  ...$ReadOnly<ArrayPatternProps>,
  +parent?: ESNode,
}): DetachedNode<ArrayPatternType> {
  const node = detachedProps<ArrayPatternType>(props.parent, {
    type: 'ArrayPattern',
    elements: props.elements.map(n => asDetachedNode(n)),
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ArrayTypeAnnotation(props: {
  ...$ReadOnly<ArrayTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<ArrayTypeAnnotationType> {
  const node = detachedProps<ArrayTypeAnnotationType>(props.parent, {
    type: 'ArrayTypeAnnotation',
    elementType: asDetachedNode(props.elementType),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function AssignmentExpression(props: {
  ...$ReadOnly<AssignmentExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<AssignmentExpressionType> {
  const node = detachedProps<AssignmentExpressionType>(props.parent, {
    type: 'AssignmentExpression',
    operator: props.operator,
    left: asDetachedNode(props.left),
    right: asDetachedNode(props.right),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function AssignmentPattern(props: {
  ...$ReadOnly<AssignmentPatternProps>,
  +parent?: ESNode,
}): DetachedNode<AssignmentPatternType> {
  const node = detachedProps<AssignmentPatternType>(props.parent, {
    type: 'AssignmentPattern',
    left: asDetachedNode(props.left),
    right: asDetachedNode(props.right),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function AwaitExpression(props: {
  ...$ReadOnly<AwaitExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<AwaitExpressionType> {
  const node = detachedProps<AwaitExpressionType>(props.parent, {
    type: 'AwaitExpression',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function BigIntLiteralTypeAnnotation(props: {
  ...$ReadOnly<BigIntLiteralTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<BigIntLiteralTypeAnnotationType> {
  const node = detachedProps<BigIntLiteralTypeAnnotationType>(props.parent, {
    type: 'BigIntLiteralTypeAnnotation',
    raw: props.raw,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function BinaryExpression(props: {
  ...$ReadOnly<BinaryExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<BinaryExpressionType> {
  const node = detachedProps<BinaryExpressionType>(props.parent, {
    type: 'BinaryExpression',
    left: asDetachedNode(props.left),
    right: asDetachedNode(props.right),
    operator: props.operator,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function BlockStatement(props: {
  ...$ReadOnly<BlockStatementProps>,
  +parent?: ESNode,
}): DetachedNode<BlockStatementType> {
  const node = detachedProps<BlockStatementType>(props.parent, {
    type: 'BlockStatement',
    body: props.body.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function BooleanLiteralTypeAnnotation(props: {
  ...$ReadOnly<BooleanLiteralTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<BooleanLiteralTypeAnnotationType> {
  const node = detachedProps<BooleanLiteralTypeAnnotationType>(props.parent, {
    type: 'BooleanLiteralTypeAnnotation',
    value: props.value,
    raw: props.raw,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function BooleanTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<BooleanTypeAnnotationType> {
  return detachedProps<BooleanTypeAnnotationType>(props.parent, {
    type: 'BooleanTypeAnnotation',
  });
}

export function BreakStatement(props: {
  ...$ReadOnly<BreakStatementProps>,
  +parent?: ESNode,
}): DetachedNode<BreakStatementType> {
  const node = detachedProps<BreakStatementType>(props.parent, {
    type: 'BreakStatement',
    label: asDetachedNode(props.label),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function CallExpression(props: {
  ...$ReadOnly<CallExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<CallExpressionType> {
  const node = detachedProps<CallExpressionType>(props.parent, {
    type: 'CallExpression',
    callee: asDetachedNode(props.callee),
    typeArguments: asDetachedNode(props.typeArguments),
    arguments: props.arguments.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function CatchClause(props: {
  ...$ReadOnly<CatchClauseProps>,
  +parent?: ESNode,
}): DetachedNode<CatchClauseType> {
  const node = detachedProps<CatchClauseType>(props.parent, {
    type: 'CatchClause',
    param: asDetachedNode(props.param),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ChainExpression(props: {
  ...$ReadOnly<ChainExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ChainExpressionType> {
  const node = detachedProps<ChainExpressionType>(props.parent, {
    type: 'ChainExpression',
    expression: asDetachedNode(props.expression),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ClassBody(props: {
  ...$ReadOnly<ClassBodyProps>,
  +parent?: ESNode,
}): DetachedNode<ClassBodyType> {
  const node = detachedProps<ClassBodyType>(props.parent, {
    type: 'ClassBody',
    body: props.body.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ClassExpression(props: {
  ...$ReadOnly<ClassExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ClassExpressionType> {
  const node = detachedProps<ClassExpressionType>(props.parent, {
    type: 'ClassExpression',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    superClass: asDetachedNode(props.superClass),
    superTypeParameters: asDetachedNode(props.superTypeParameters),
    implements: props.implements.map(n => asDetachedNode(n)),
    decorators: props.decorators.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ClassImplements(props: {
  ...$ReadOnly<ClassImplementsProps>,
  +parent?: ESNode,
}): DetachedNode<ClassImplementsType> {
  const node = detachedProps<ClassImplementsType>(props.parent, {
    type: 'ClassImplements',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ConditionalExpression(props: {
  ...$ReadOnly<ConditionalExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ConditionalExpressionType> {
  const node = detachedProps<ConditionalExpressionType>(props.parent, {
    type: 'ConditionalExpression',
    test: asDetachedNode(props.test),
    alternate: asDetachedNode(props.alternate),
    consequent: asDetachedNode(props.consequent),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ContinueStatement(props: {
  ...$ReadOnly<ContinueStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ContinueStatementType> {
  const node = detachedProps<ContinueStatementType>(props.parent, {
    type: 'ContinueStatement',
    label: asDetachedNode(props.label),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DebuggerStatement(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<DebuggerStatementType> {
  return detachedProps<DebuggerStatementType>(props.parent, {
    type: 'DebuggerStatement',
  });
}

export function DeclareClass(props: {
  ...$ReadOnly<DeclareClassProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareClassType> {
  const node = detachedProps<DeclareClassType>(props.parent, {
    type: 'DeclareClass',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    extends: props.extends.map(n => asDetachedNode(n)),
    implements: props.implements.map(n => asDetachedNode(n)),
    mixins: props.mixins.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclaredPredicate(props: {
  ...$ReadOnly<DeclaredPredicateProps>,
  +parent?: ESNode,
}): DetachedNode<DeclaredPredicateType> {
  const node = detachedProps<DeclaredPredicateType>(props.parent, {
    type: 'DeclaredPredicate',
    value: asDetachedNode(props.value),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareExportAllDeclaration(props: {
  ...$ReadOnly<DeclareExportAllDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareExportAllDeclarationType> {
  const node = detachedProps<DeclareExportAllDeclarationType>(props.parent, {
    type: 'DeclareExportAllDeclaration',
    source: asDetachedNode(props.source),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareInterface(props: {
  ...$ReadOnly<DeclareInterfaceProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareInterfaceType> {
  const node = detachedProps<DeclareInterfaceType>(props.parent, {
    type: 'DeclareInterface',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    extends: props.extends.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareModule(props: {
  ...$ReadOnly<DeclareModuleProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareModuleType> {
  const node = detachedProps<DeclareModuleType>(props.parent, {
    type: 'DeclareModule',
    id: asDetachedNode(props.id),
    body: asDetachedNode(props.body),
    kind: props.kind,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareModuleExports(props: {
  ...$ReadOnly<DeclareModuleExportsProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareModuleExportsType> {
  const node = detachedProps<DeclareModuleExportsType>(props.parent, {
    type: 'DeclareModuleExports',
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareOpaqueType(props: {
  ...$ReadOnly<DeclareOpaqueTypeProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareOpaqueTypeType> {
  const node = detachedProps<DeclareOpaqueTypeType>(props.parent, {
    type: 'DeclareOpaqueType',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    impltype: asDetachedNode(props.impltype),
    supertype: asDetachedNode(props.supertype),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareTypeAlias(props: {
  ...$ReadOnly<DeclareTypeAliasProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareTypeAliasType> {
  const node = detachedProps<DeclareTypeAliasType>(props.parent, {
    type: 'DeclareTypeAlias',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    right: asDetachedNode(props.right),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DeclareVariable(props: {
  ...$ReadOnly<DeclareVariableProps>,
  +parent?: ESNode,
}): DetachedNode<DeclareVariableType> {
  const node = detachedProps<DeclareVariableType>(props.parent, {
    type: 'DeclareVariable',
    id: asDetachedNode(props.id),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function DoWhileStatement(props: {
  ...$ReadOnly<DoWhileStatementProps>,
  +parent?: ESNode,
}): DetachedNode<DoWhileStatementType> {
  const node = detachedProps<DoWhileStatementType>(props.parent, {
    type: 'DoWhileStatement',
    body: asDetachedNode(props.body),
    test: asDetachedNode(props.test),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EmptyStatement(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<EmptyStatementType> {
  return detachedProps<EmptyStatementType>(props.parent, {
    type: 'EmptyStatement',
  });
}

export function EmptyTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<EmptyTypeAnnotationType> {
  return detachedProps<EmptyTypeAnnotationType>(props.parent, {
    type: 'EmptyTypeAnnotation',
  });
}

export function EnumBooleanBody(props: {
  ...$ReadOnly<EnumBooleanBodyProps>,
  +parent?: ESNode,
}): DetachedNode<EnumBooleanBodyType> {
  const node = detachedProps<EnumBooleanBodyType>(props.parent, {
    type: 'EnumBooleanBody',
    members: props.members.map(n => asDetachedNode(n)),
    explicitType: props.explicitType,
    hasUnknownMembers: props.hasUnknownMembers,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumBooleanMember(props: {
  ...$ReadOnly<EnumBooleanMemberProps>,
  +parent?: ESNode,
}): DetachedNode<EnumBooleanMemberType> {
  const node = detachedProps<EnumBooleanMemberType>(props.parent, {
    type: 'EnumBooleanMember',
    id: asDetachedNode(props.id),
    init: asDetachedNode(props.init),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumDeclaration(props: {
  ...$ReadOnly<EnumDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<EnumDeclarationType> {
  const node = detachedProps<EnumDeclarationType>(props.parent, {
    type: 'EnumDeclaration',
    id: asDetachedNode(props.id),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumDefaultedMember(props: {
  ...$ReadOnly<EnumDefaultedMemberProps>,
  +parent?: ESNode,
}): DetachedNode<EnumDefaultedMemberType> {
  const node = detachedProps<EnumDefaultedMemberType>(props.parent, {
    type: 'EnumDefaultedMember',
    id: asDetachedNode(props.id),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumNumberBody(props: {
  ...$ReadOnly<EnumNumberBodyProps>,
  +parent?: ESNode,
}): DetachedNode<EnumNumberBodyType> {
  const node = detachedProps<EnumNumberBodyType>(props.parent, {
    type: 'EnumNumberBody',
    members: props.members.map(n => asDetachedNode(n)),
    explicitType: props.explicitType,
    hasUnknownMembers: props.hasUnknownMembers,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumNumberMember(props: {
  ...$ReadOnly<EnumNumberMemberProps>,
  +parent?: ESNode,
}): DetachedNode<EnumNumberMemberType> {
  const node = detachedProps<EnumNumberMemberType>(props.parent, {
    type: 'EnumNumberMember',
    id: asDetachedNode(props.id),
    init: asDetachedNode(props.init),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumStringBody(props: {
  ...$ReadOnly<EnumStringBodyProps>,
  +parent?: ESNode,
}): DetachedNode<EnumStringBodyType> {
  const node = detachedProps<EnumStringBodyType>(props.parent, {
    type: 'EnumStringBody',
    members: props.members.map(n => asDetachedNode(n)),
    explicitType: props.explicitType,
    hasUnknownMembers: props.hasUnknownMembers,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumStringMember(props: {
  ...$ReadOnly<EnumStringMemberProps>,
  +parent?: ESNode,
}): DetachedNode<EnumStringMemberType> {
  const node = detachedProps<EnumStringMemberType>(props.parent, {
    type: 'EnumStringMember',
    id: asDetachedNode(props.id),
    init: asDetachedNode(props.init),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function EnumSymbolBody(props: {
  ...$ReadOnly<EnumSymbolBodyProps>,
  +parent?: ESNode,
}): DetachedNode<EnumSymbolBodyType> {
  const node = detachedProps<EnumSymbolBodyType>(props.parent, {
    type: 'EnumSymbolBody',
    members: props.members.map(n => asDetachedNode(n)),
    hasUnknownMembers: props.hasUnknownMembers,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ExistsTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<ExistsTypeAnnotationType> {
  return detachedProps<ExistsTypeAnnotationType>(props.parent, {
    type: 'ExistsTypeAnnotation',
  });
}

export function ExportAllDeclaration(props: {
  ...$ReadOnly<ExportAllDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<ExportAllDeclarationType> {
  const node = detachedProps<ExportAllDeclarationType>(props.parent, {
    type: 'ExportAllDeclaration',
    exported: asDetachedNode(props.exported),
    source: asDetachedNode(props.source),
    exportKind: props.exportKind,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ExportDefaultDeclaration(props: {
  ...$ReadOnly<ExportDefaultDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<ExportDefaultDeclarationType> {
  const node = detachedProps<ExportDefaultDeclarationType>(props.parent, {
    type: 'ExportDefaultDeclaration',
    declaration: asDetachedNode(props.declaration),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ExportSpecifier(props: {
  ...$ReadOnly<ExportSpecifierProps>,
  +parent?: ESNode,
}): DetachedNode<ExportSpecifierType> {
  const node = detachedProps<ExportSpecifierType>(props.parent, {
    type: 'ExportSpecifier',
    exported: asDetachedNode(props.exported),
    local: asDetachedNode(props.local),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ExpressionStatement(props: {
  ...$ReadOnly<ExpressionStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ExpressionStatementType> {
  const node = detachedProps<ExpressionStatementType>(props.parent, {
    type: 'ExpressionStatement',
    expression: asDetachedNode(props.expression),
    directive: props.directive,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ForInStatement(props: {
  ...$ReadOnly<ForInStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ForInStatementType> {
  const node = detachedProps<ForInStatementType>(props.parent, {
    type: 'ForInStatement',
    left: asDetachedNode(props.left),
    right: asDetachedNode(props.right),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ForOfStatement(props: {
  ...$ReadOnly<ForOfStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ForOfStatementType> {
  const node = detachedProps<ForOfStatementType>(props.parent, {
    type: 'ForOfStatement',
    left: asDetachedNode(props.left),
    right: asDetachedNode(props.right),
    body: asDetachedNode(props.body),
    await: props.await,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ForStatement(props: {
  ...$ReadOnly<ForStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ForStatementType> {
  const node = detachedProps<ForStatementType>(props.parent, {
    type: 'ForStatement',
    init: asDetachedNode(props.init),
    test: asDetachedNode(props.test),
    update: asDetachedNode(props.update),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function FunctionDeclaration(props: {
  ...$ReadOnly<FunctionDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<FunctionDeclarationType> {
  const node = detachedProps<FunctionDeclarationType>(props.parent, {
    type: 'FunctionDeclaration',
    id: asDetachedNode(props.id),
    params: props.params.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
    typeParameters: asDetachedNode(props.typeParameters),
    returnType: asDetachedNode(props.returnType),
    predicate: asDetachedNode(props.predicate),
    generator: props.generator,
    async: props.async,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function FunctionExpression(props: {
  ...$ReadOnly<FunctionExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<FunctionExpressionType> {
  const node = detachedProps<FunctionExpressionType>(props.parent, {
    type: 'FunctionExpression',
    id: asDetachedNode(props.id),
    params: props.params.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
    typeParameters: asDetachedNode(props.typeParameters),
    returnType: asDetachedNode(props.returnType),
    predicate: asDetachedNode(props.predicate),
    generator: props.generator,
    async: props.async,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function FunctionTypeAnnotation(props: {
  ...$ReadOnly<FunctionTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<FunctionTypeAnnotationType> {
  const node = detachedProps<FunctionTypeAnnotationType>(props.parent, {
    type: 'FunctionTypeAnnotation',
    params: props.params.map(n => asDetachedNode(n)),
    this: asDetachedNode(props.this),
    returnType: asDetachedNode(props.returnType),
    rest: asDetachedNode(props.rest),
    typeParameters: asDetachedNode(props.typeParameters),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function FunctionTypeParam(props: {
  ...$ReadOnly<FunctionTypeParamProps>,
  +parent?: ESNode,
}): DetachedNode<FunctionTypeParamType> {
  const node = detachedProps<FunctionTypeParamType>(props.parent, {
    type: 'FunctionTypeParam',
    name: asDetachedNode(props.name),
    typeAnnotation: asDetachedNode(props.typeAnnotation),
    optional: props.optional,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function GenericTypeAnnotation(props: {
  ...$ReadOnly<GenericTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<GenericTypeAnnotationType> {
  const node = detachedProps<GenericTypeAnnotationType>(props.parent, {
    type: 'GenericTypeAnnotation',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function IfStatement(props: {
  ...$ReadOnly<IfStatementProps>,
  +parent?: ESNode,
}): DetachedNode<IfStatementType> {
  const node = detachedProps<IfStatementType>(props.parent, {
    type: 'IfStatement',
    test: asDetachedNode(props.test),
    consequent: asDetachedNode(props.consequent),
    alternate: asDetachedNode(props.alternate),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ImportAttribute(props: {
  ...$ReadOnly<ImportAttributeProps>,
  +parent?: ESNode,
}): DetachedNode<ImportAttributeType> {
  const node = detachedProps<ImportAttributeType>(props.parent, {
    type: 'ImportAttribute',
    key: asDetachedNode(props.key),
    value: asDetachedNode(props.value),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ImportDeclaration(props: {
  ...$ReadOnly<ImportDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<ImportDeclarationType> {
  const node = detachedProps<ImportDeclarationType>(props.parent, {
    type: 'ImportDeclaration',
    specifiers: props.specifiers.map(n => asDetachedNode(n)),
    source: asDetachedNode(props.source),
    assertions: props.assertions?.map(n => asDetachedNode(n)),
    importKind: props.importKind,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ImportDefaultSpecifier(props: {
  ...$ReadOnly<ImportDefaultSpecifierProps>,
  +parent?: ESNode,
}): DetachedNode<ImportDefaultSpecifierType> {
  const node = detachedProps<ImportDefaultSpecifierType>(props.parent, {
    type: 'ImportDefaultSpecifier',
    local: asDetachedNode(props.local),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ImportExpression(props: {
  ...$ReadOnly<ImportExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ImportExpressionType> {
  const node = detachedProps<ImportExpressionType>(props.parent, {
    type: 'ImportExpression',
    source: asDetachedNode(props.source),
    attributes: asDetachedNode(props.attributes),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ImportNamespaceSpecifier(props: {
  ...$ReadOnly<ImportNamespaceSpecifierProps>,
  +parent?: ESNode,
}): DetachedNode<ImportNamespaceSpecifierType> {
  const node = detachedProps<ImportNamespaceSpecifierType>(props.parent, {
    type: 'ImportNamespaceSpecifier',
    local: asDetachedNode(props.local),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ImportSpecifier(props: {
  ...$ReadOnly<ImportSpecifierProps>,
  +parent?: ESNode,
}): DetachedNode<ImportSpecifierType> {
  const node = detachedProps<ImportSpecifierType>(props.parent, {
    type: 'ImportSpecifier',
    imported: asDetachedNode(props.imported),
    local: asDetachedNode(props.local),
    importKind: props.importKind,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function IndexedAccessType(props: {
  ...$ReadOnly<IndexedAccessTypeProps>,
  +parent?: ESNode,
}): DetachedNode<IndexedAccessTypeType> {
  const node = detachedProps<IndexedAccessTypeType>(props.parent, {
    type: 'IndexedAccessType',
    objectType: asDetachedNode(props.objectType),
    indexType: asDetachedNode(props.indexType),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function InferredPredicate(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<InferredPredicateType> {
  return detachedProps<InferredPredicateType>(props.parent, {
    type: 'InferredPredicate',
  });
}

export function InterfaceDeclaration(props: {
  ...$ReadOnly<InterfaceDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<InterfaceDeclarationType> {
  const node = detachedProps<InterfaceDeclarationType>(props.parent, {
    type: 'InterfaceDeclaration',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    extends: props.extends.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function InterfaceExtends(props: {
  ...$ReadOnly<InterfaceExtendsProps>,
  +parent?: ESNode,
}): DetachedNode<InterfaceExtendsType> {
  const node = detachedProps<InterfaceExtendsType>(props.parent, {
    type: 'InterfaceExtends',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function InterfaceTypeAnnotation(props: {
  ...$ReadOnly<InterfaceTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<InterfaceTypeAnnotationType> {
  const node = detachedProps<InterfaceTypeAnnotationType>(props.parent, {
    type: 'InterfaceTypeAnnotation',
    extends: props.extends.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function IntersectionTypeAnnotation(props: {
  ...$ReadOnly<IntersectionTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<IntersectionTypeAnnotationType> {
  const node = detachedProps<IntersectionTypeAnnotationType>(props.parent, {
    type: 'IntersectionTypeAnnotation',
    types: props.types.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXAttribute(props: {
  ...$ReadOnly<JSXAttributeProps>,
  +parent?: ESNode,
}): DetachedNode<JSXAttributeType> {
  const node = detachedProps<JSXAttributeType>(props.parent, {
    type: 'JSXAttribute',
    name: asDetachedNode(props.name),
    value: asDetachedNode(props.value),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXClosingElement(props: {
  ...$ReadOnly<JSXClosingElementProps>,
  +parent?: ESNode,
}): DetachedNode<JSXClosingElementType> {
  const node = detachedProps<JSXClosingElementType>(props.parent, {
    type: 'JSXClosingElement',
    name: asDetachedNode(props.name),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXClosingFragment(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<JSXClosingFragmentType> {
  return detachedProps<JSXClosingFragmentType>(props.parent, {
    type: 'JSXClosingFragment',
  });
}

export function JSXElement(props: {
  ...$ReadOnly<JSXElementProps>,
  +parent?: ESNode,
}): DetachedNode<JSXElementType> {
  const node = detachedProps<JSXElementType>(props.parent, {
    type: 'JSXElement',
    openingElement: asDetachedNode(props.openingElement),
    children: props.children.map(n => asDetachedNode(n)),
    closingElement: asDetachedNode(props.closingElement),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXEmptyExpression(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<JSXEmptyExpressionType> {
  return detachedProps<JSXEmptyExpressionType>(props.parent, {
    type: 'JSXEmptyExpression',
  });
}

export function JSXExpressionContainer(props: {
  ...$ReadOnly<JSXExpressionContainerProps>,
  +parent?: ESNode,
}): DetachedNode<JSXExpressionContainerType> {
  const node = detachedProps<JSXExpressionContainerType>(props.parent, {
    type: 'JSXExpressionContainer',
    expression: asDetachedNode(props.expression),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXFragment(props: {
  ...$ReadOnly<JSXFragmentProps>,
  +parent?: ESNode,
}): DetachedNode<JSXFragmentType> {
  const node = detachedProps<JSXFragmentType>(props.parent, {
    type: 'JSXFragment',
    openingFragment: asDetachedNode(props.openingFragment),
    children: props.children.map(n => asDetachedNode(n)),
    closingFragment: asDetachedNode(props.closingFragment),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXIdentifier(props: {
  ...$ReadOnly<JSXIdentifierProps>,
  +parent?: ESNode,
}): DetachedNode<JSXIdentifierType> {
  const node = detachedProps<JSXIdentifierType>(props.parent, {
    type: 'JSXIdentifier',
    name: props.name,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXMemberExpression(props: {
  ...$ReadOnly<JSXMemberExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<JSXMemberExpressionType> {
  const node = detachedProps<JSXMemberExpressionType>(props.parent, {
    type: 'JSXMemberExpression',
    object: asDetachedNode(props.object),
    property: asDetachedNode(props.property),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXNamespacedName(props: {
  ...$ReadOnly<JSXNamespacedNameProps>,
  +parent?: ESNode,
}): DetachedNode<JSXNamespacedNameType> {
  const node = detachedProps<JSXNamespacedNameType>(props.parent, {
    type: 'JSXNamespacedName',
    namespace: asDetachedNode(props.namespace),
    name: asDetachedNode(props.name),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXOpeningElement(props: {
  ...$ReadOnly<JSXOpeningElementProps>,
  +parent?: ESNode,
}): DetachedNode<JSXOpeningElementType> {
  const node = detachedProps<JSXOpeningElementType>(props.parent, {
    type: 'JSXOpeningElement',
    name: asDetachedNode(props.name),
    attributes: props.attributes.map(n => asDetachedNode(n)),
    selfClosing: props.selfClosing,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXOpeningFragment(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<JSXOpeningFragmentType> {
  return detachedProps<JSXOpeningFragmentType>(props.parent, {
    type: 'JSXOpeningFragment',
  });
}

export function JSXSpreadAttribute(props: {
  ...$ReadOnly<JSXSpreadAttributeProps>,
  +parent?: ESNode,
}): DetachedNode<JSXSpreadAttributeType> {
  const node = detachedProps<JSXSpreadAttributeType>(props.parent, {
    type: 'JSXSpreadAttribute',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXSpreadChild(props: {
  ...$ReadOnly<JSXSpreadChildProps>,
  +parent?: ESNode,
}): DetachedNode<JSXSpreadChildType> {
  const node = detachedProps<JSXSpreadChildType>(props.parent, {
    type: 'JSXSpreadChild',
    expression: asDetachedNode(props.expression),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function JSXText(props: {
  ...$ReadOnly<JSXTextProps>,
  +parent?: ESNode,
}): DetachedNode<JSXTextType> {
  const node = detachedProps<JSXTextType>(props.parent, {
    type: 'JSXText',
    value: props.value,
    raw: props.raw,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function LabeledStatement(props: {
  ...$ReadOnly<LabeledStatementProps>,
  +parent?: ESNode,
}): DetachedNode<LabeledStatementType> {
  const node = detachedProps<LabeledStatementType>(props.parent, {
    type: 'LabeledStatement',
    label: asDetachedNode(props.label),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function LogicalExpression(props: {
  ...$ReadOnly<LogicalExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<LogicalExpressionType> {
  const node = detachedProps<LogicalExpressionType>(props.parent, {
    type: 'LogicalExpression',
    left: asDetachedNode(props.left),
    right: asDetachedNode(props.right),
    operator: props.operator,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function MemberExpression(props: {
  ...$ReadOnly<MemberExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<MemberExpressionType> {
  const node = detachedProps<MemberExpressionType>(props.parent, {
    type: 'MemberExpression',
    object: asDetachedNode(props.object),
    property: asDetachedNode(props.property),
    computed: props.computed,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function MetaProperty(props: {
  ...$ReadOnly<MetaPropertyProps>,
  +parent?: ESNode,
}): DetachedNode<MetaPropertyType> {
  const node = detachedProps<MetaPropertyType>(props.parent, {
    type: 'MetaProperty',
    meta: asDetachedNode(props.meta),
    property: asDetachedNode(props.property),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function MethodDefinition(props: {
  ...$ReadOnly<MethodDefinitionProps>,
  +parent?: ESNode,
}): DetachedNode<MethodDefinitionType> {
  const node = detachedProps<MethodDefinitionType>(props.parent, {
    type: 'MethodDefinition',
    key: asDetachedNode(props.key),
    value: asDetachedNode(props.value),
    kind: props.kind,
    computed: props.computed,
    static: props.static,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function MixedTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<MixedTypeAnnotationType> {
  return detachedProps<MixedTypeAnnotationType>(props.parent, {
    type: 'MixedTypeAnnotation',
  });
}

export function NewExpression(props: {
  ...$ReadOnly<NewExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<NewExpressionType> {
  const node = detachedProps<NewExpressionType>(props.parent, {
    type: 'NewExpression',
    callee: asDetachedNode(props.callee),
    typeArguments: asDetachedNode(props.typeArguments),
    arguments: props.arguments.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function NullableTypeAnnotation(props: {
  ...$ReadOnly<NullableTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<NullableTypeAnnotationType> {
  const node = detachedProps<NullableTypeAnnotationType>(props.parent, {
    type: 'NullableTypeAnnotation',
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function NullLiteralTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<NullLiteralTypeAnnotationType> {
  return detachedProps<NullLiteralTypeAnnotationType>(props.parent, {
    type: 'NullLiteralTypeAnnotation',
  });
}

export function NumberLiteralTypeAnnotation(props: {
  ...$ReadOnly<NumberLiteralTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<NumberLiteralTypeAnnotationType> {
  const node = detachedProps<NumberLiteralTypeAnnotationType>(props.parent, {
    type: 'NumberLiteralTypeAnnotation',
    value: props.value,
    raw: props.raw,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function NumberTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<NumberTypeAnnotationType> {
  return detachedProps<NumberTypeAnnotationType>(props.parent, {
    type: 'NumberTypeAnnotation',
  });
}

export function ObjectExpression(props: {
  ...$ReadOnly<ObjectExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectExpressionType> {
  const node = detachedProps<ObjectExpressionType>(props.parent, {
    type: 'ObjectExpression',
    properties: props.properties.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ObjectPattern(props: {
  ...$ReadOnly<ObjectPatternProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectPatternType> {
  const node = detachedProps<ObjectPatternType>(props.parent, {
    type: 'ObjectPattern',
    properties: props.properties.map(n => asDetachedNode(n)),
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ObjectTypeAnnotation(props: {
  ...$ReadOnly<ObjectTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectTypeAnnotationType> {
  const node = detachedProps<ObjectTypeAnnotationType>(props.parent, {
    type: 'ObjectTypeAnnotation',
    properties: props.properties.map(n => asDetachedNode(n)),
    indexers: props.indexers.map(n => asDetachedNode(n)),
    callProperties: props.callProperties.map(n => asDetachedNode(n)),
    internalSlots: props.internalSlots.map(n => asDetachedNode(n)),
    inexact: props.inexact,
    exact: props.exact,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ObjectTypeCallProperty(props: {
  ...$ReadOnly<ObjectTypeCallPropertyProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectTypeCallPropertyType> {
  const node = detachedProps<ObjectTypeCallPropertyType>(props.parent, {
    type: 'ObjectTypeCallProperty',
    value: asDetachedNode(props.value),
    static: props.static,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ObjectTypeIndexer(props: {
  ...$ReadOnly<ObjectTypeIndexerProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectTypeIndexerType> {
  const node = detachedProps<ObjectTypeIndexerType>(props.parent, {
    type: 'ObjectTypeIndexer',
    id: asDetachedNode(props.id),
    key: asDetachedNode(props.key),
    value: asDetachedNode(props.value),
    static: props.static,
    variance: asDetachedNode(props.variance),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ObjectTypeInternalSlot(props: {
  ...$ReadOnly<ObjectTypeInternalSlotProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectTypeInternalSlotType> {
  const node = detachedProps<ObjectTypeInternalSlotType>(props.parent, {
    type: 'ObjectTypeInternalSlot',
    id: asDetachedNode(props.id),
    value: asDetachedNode(props.value),
    optional: props.optional,
    static: props.static,
    method: props.method,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ObjectTypeSpreadProperty(props: {
  ...$ReadOnly<ObjectTypeSpreadPropertyProps>,
  +parent?: ESNode,
}): DetachedNode<ObjectTypeSpreadPropertyType> {
  const node = detachedProps<ObjectTypeSpreadPropertyType>(props.parent, {
    type: 'ObjectTypeSpreadProperty',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function OpaqueType(props: {
  ...$ReadOnly<OpaqueTypeProps>,
  +parent?: ESNode,
}): DetachedNode<OpaqueTypeType> {
  const node = detachedProps<OpaqueTypeType>(props.parent, {
    type: 'OpaqueType',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    impltype: asDetachedNode(props.impltype),
    supertype: asDetachedNode(props.supertype),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function OptionalIndexedAccessType(props: {
  ...$ReadOnly<OptionalIndexedAccessTypeProps>,
  +parent?: ESNode,
}): DetachedNode<OptionalIndexedAccessTypeType> {
  const node = detachedProps<OptionalIndexedAccessTypeType>(props.parent, {
    type: 'OptionalIndexedAccessType',
    objectType: asDetachedNode(props.objectType),
    indexType: asDetachedNode(props.indexType),
    optional: props.optional,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function PrivateIdentifier(props: {
  ...$ReadOnly<PrivateIdentifierProps>,
  +parent?: ESNode,
}): DetachedNode<PrivateIdentifierType> {
  const node = detachedProps<PrivateIdentifierType>(props.parent, {
    type: 'PrivateIdentifier',
    name: props.name,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function Property(props: {
  ...$ReadOnly<PropertyProps>,
  +parent?: ESNode,
}): DetachedNode<PropertyType> {
  const node = detachedProps<PropertyType>(props.parent, {
    type: 'Property',
    key: asDetachedNode(props.key),
    value: asDetachedNode(props.value),
    kind: props.kind,
    computed: props.computed,
    method: props.method,
    shorthand: props.shorthand,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function PropertyDefinition(props: {
  ...$ReadOnly<PropertyDefinitionProps>,
  +parent?: ESNode,
}): DetachedNode<PropertyDefinitionType> {
  const node = detachedProps<PropertyDefinitionType>(props.parent, {
    type: 'PropertyDefinition',
    key: asDetachedNode(props.key),
    value: asDetachedNode(props.value),
    computed: props.computed,
    static: props.static,
    declare: props.declare,
    optional: props.optional,
    variance: asDetachedNode(props.variance),
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function QualifiedTypeIdentifier(props: {
  ...$ReadOnly<QualifiedTypeIdentifierProps>,
  +parent?: ESNode,
}): DetachedNode<QualifiedTypeIdentifierType> {
  const node = detachedProps<QualifiedTypeIdentifierType>(props.parent, {
    type: 'QualifiedTypeIdentifier',
    qualification: asDetachedNode(props.qualification),
    id: asDetachedNode(props.id),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function RestElement(props: {
  ...$ReadOnly<RestElementProps>,
  +parent?: ESNode,
}): DetachedNode<RestElementType> {
  const node = detachedProps<RestElementType>(props.parent, {
    type: 'RestElement',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ReturnStatement(props: {
  ...$ReadOnly<ReturnStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ReturnStatementType> {
  const node = detachedProps<ReturnStatementType>(props.parent, {
    type: 'ReturnStatement',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function SequenceExpression(props: {
  ...$ReadOnly<SequenceExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<SequenceExpressionType> {
  const node = detachedProps<SequenceExpressionType>(props.parent, {
    type: 'SequenceExpression',
    expressions: props.expressions.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function SpreadElement(props: {
  ...$ReadOnly<SpreadElementProps>,
  +parent?: ESNode,
}): DetachedNode<SpreadElementType> {
  const node = detachedProps<SpreadElementType>(props.parent, {
    type: 'SpreadElement',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function StringLiteralTypeAnnotation(props: {
  ...$ReadOnly<StringLiteralTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<StringLiteralTypeAnnotationType> {
  const node = detachedProps<StringLiteralTypeAnnotationType>(props.parent, {
    type: 'StringLiteralTypeAnnotation',
    value: props.value,
    raw: props.raw,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function StringTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<StringTypeAnnotationType> {
  return detachedProps<StringTypeAnnotationType>(props.parent, {
    type: 'StringTypeAnnotation',
  });
}

export function Super(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<SuperType> {
  return detachedProps<SuperType>(props.parent, {
    type: 'Super',
  });
}

export function SwitchCase(props: {
  ...$ReadOnly<SwitchCaseProps>,
  +parent?: ESNode,
}): DetachedNode<SwitchCaseType> {
  const node = detachedProps<SwitchCaseType>(props.parent, {
    type: 'SwitchCase',
    test: asDetachedNode(props.test),
    consequent: props.consequent.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function SwitchStatement(props: {
  ...$ReadOnly<SwitchStatementProps>,
  +parent?: ESNode,
}): DetachedNode<SwitchStatementType> {
  const node = detachedProps<SwitchStatementType>(props.parent, {
    type: 'SwitchStatement',
    discriminant: asDetachedNode(props.discriminant),
    cases: props.cases.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function SymbolTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<SymbolTypeAnnotationType> {
  return detachedProps<SymbolTypeAnnotationType>(props.parent, {
    type: 'SymbolTypeAnnotation',
  });
}

export function TaggedTemplateExpression(props: {
  ...$ReadOnly<TaggedTemplateExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<TaggedTemplateExpressionType> {
  const node = detachedProps<TaggedTemplateExpressionType>(props.parent, {
    type: 'TaggedTemplateExpression',
    tag: asDetachedNode(props.tag),
    quasi: asDetachedNode(props.quasi),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TemplateLiteral(props: {
  ...$ReadOnly<TemplateLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<TemplateLiteralType> {
  const node = detachedProps<TemplateLiteralType>(props.parent, {
    type: 'TemplateLiteral',
    quasis: props.quasis.map(n => asDetachedNode(n)),
    expressions: props.expressions.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ThisExpression(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<ThisExpressionType> {
  return detachedProps<ThisExpressionType>(props.parent, {
    type: 'ThisExpression',
  });
}

export function ThisTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<ThisTypeAnnotationType> {
  return detachedProps<ThisTypeAnnotationType>(props.parent, {
    type: 'ThisTypeAnnotation',
  });
}

export function ThrowStatement(props: {
  ...$ReadOnly<ThrowStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ThrowStatementType> {
  const node = detachedProps<ThrowStatementType>(props.parent, {
    type: 'ThrowStatement',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TryStatement(props: {
  ...$ReadOnly<TryStatementProps>,
  +parent?: ESNode,
}): DetachedNode<TryStatementType> {
  const node = detachedProps<TryStatementType>(props.parent, {
    type: 'TryStatement',
    block: asDetachedNode(props.block),
    handler: asDetachedNode(props.handler),
    finalizer: asDetachedNode(props.finalizer),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TupleTypeAnnotation(props: {
  ...$ReadOnly<TupleTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<TupleTypeAnnotationType> {
  const node = detachedProps<TupleTypeAnnotationType>(props.parent, {
    type: 'TupleTypeAnnotation',
    types: props.types.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeAlias(props: {
  ...$ReadOnly<TypeAliasProps>,
  +parent?: ESNode,
}): DetachedNode<TypeAliasType> {
  const node = detachedProps<TypeAliasType>(props.parent, {
    type: 'TypeAlias',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    right: asDetachedNode(props.right),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeAnnotation(props: {
  ...$ReadOnly<TypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<TypeAnnotationType> {
  const node = detachedProps<TypeAnnotationType>(props.parent, {
    type: 'TypeAnnotation',
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeCastExpression(props: {
  ...$ReadOnly<TypeCastExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<TypeCastExpressionType> {
  const node = detachedProps<TypeCastExpressionType>(props.parent, {
    type: 'TypeCastExpression',
    expression: asDetachedNode(props.expression),
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeofTypeAnnotation(props: {
  ...$ReadOnly<TypeofTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<TypeofTypeAnnotationType> {
  const node = detachedProps<TypeofTypeAnnotationType>(props.parent, {
    type: 'TypeofTypeAnnotation',
    argument: asDetachedNode(props.argument),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeParameter(props: {
  ...$ReadOnly<TypeParameterProps>,
  +parent?: ESNode,
}): DetachedNode<TypeParameterType> {
  const node = detachedProps<TypeParameterType>(props.parent, {
    type: 'TypeParameter',
    name: props.name,
    bound: asDetachedNode(props.bound),
    variance: asDetachedNode(props.variance),
    default: asDetachedNode(props.default),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeParameterDeclaration(props: {
  ...$ReadOnly<TypeParameterDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<TypeParameterDeclarationType> {
  const node = detachedProps<TypeParameterDeclarationType>(props.parent, {
    type: 'TypeParameterDeclaration',
    params: props.params.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function TypeParameterInstantiation(props: {
  ...$ReadOnly<TypeParameterInstantiationProps>,
  +parent?: ESNode,
}): DetachedNode<TypeParameterInstantiationType> {
  const node = detachedProps<TypeParameterInstantiationType>(props.parent, {
    type: 'TypeParameterInstantiation',
    params: props.params.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function UnaryExpression(props: {
  ...$ReadOnly<UnaryExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<UnaryExpressionType> {
  const node = detachedProps<UnaryExpressionType>(props.parent, {
    type: 'UnaryExpression',
    operator: props.operator,
    argument: asDetachedNode(props.argument),
    prefix: props.prefix,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function UnionTypeAnnotation(props: {
  ...$ReadOnly<UnionTypeAnnotationProps>,
  +parent?: ESNode,
}): DetachedNode<UnionTypeAnnotationType> {
  const node = detachedProps<UnionTypeAnnotationType>(props.parent, {
    type: 'UnionTypeAnnotation',
    types: props.types.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function UpdateExpression(props: {
  ...$ReadOnly<UpdateExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<UpdateExpressionType> {
  const node = detachedProps<UpdateExpressionType>(props.parent, {
    type: 'UpdateExpression',
    operator: props.operator,
    argument: asDetachedNode(props.argument),
    prefix: props.prefix,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function VariableDeclaration(props: {
  ...$ReadOnly<VariableDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<VariableDeclarationType> {
  const node = detachedProps<VariableDeclarationType>(props.parent, {
    type: 'VariableDeclaration',
    kind: props.kind,
    declarations: props.declarations.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function VariableDeclarator(props: {
  ...$ReadOnly<VariableDeclaratorProps>,
  +parent?: ESNode,
}): DetachedNode<VariableDeclaratorType> {
  const node = detachedProps<VariableDeclaratorType>(props.parent, {
    type: 'VariableDeclarator',
    init: asDetachedNode(props.init),
    id: asDetachedNode(props.id),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function Variance(props: {
  ...$ReadOnly<VarianceProps>,
  +parent?: ESNode,
}): DetachedNode<VarianceType> {
  const node = detachedProps<VarianceType>(props.parent, {
    type: 'Variance',
    kind: props.kind,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function VoidTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<VoidTypeAnnotationType> {
  return detachedProps<VoidTypeAnnotationType>(props.parent, {
    type: 'VoidTypeAnnotation',
  });
}

export function WhileStatement(props: {
  ...$ReadOnly<WhileStatementProps>,
  +parent?: ESNode,
}): DetachedNode<WhileStatementType> {
  const node = detachedProps<WhileStatementType>(props.parent, {
    type: 'WhileStatement',
    body: asDetachedNode(props.body),
    test: asDetachedNode(props.test),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function WithStatement(props: {
  ...$ReadOnly<WithStatementProps>,
  +parent?: ESNode,
}): DetachedNode<WithStatementType> {
  const node = detachedProps<WithStatementType>(props.parent, {
    type: 'WithStatement',
    object: asDetachedNode(props.object),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function YieldExpression(props: {
  ...$ReadOnly<YieldExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<YieldExpressionType> {
  const node = detachedProps<YieldExpressionType>(props.parent, {
    type: 'YieldExpression',
    argument: asDetachedNode(props.argument),
    delegate: props.delegate,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export * from './special-case-node-types';
