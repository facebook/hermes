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
/* global $NonMaybeType, Partial, $ReadOnly, $ReadOnlyArray */

'use strict';

import type {ESNode, Token} from 'hermes-estree';

export function isAnyTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'AnyTypeAnnotation';
}

export function isArrayExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ArrayExpression';
}

export function isArrayPattern(node: ESNode | Token): boolean %checks {
  return node.type === 'ArrayPattern';
}

export function isArrayTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'ArrayTypeAnnotation';
}

export function isArrowFunctionExpression(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ArrowFunctionExpression';
}

export function isAssignmentExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'AssignmentExpression';
}

export function isAssignmentPattern(node: ESNode | Token): boolean %checks {
  return node.type === 'AssignmentPattern';
}

export function isAwaitExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'AwaitExpression';
}

export function isBigIntLiteralTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'BigIntLiteralTypeAnnotation';
}

export function isBigIntTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'BigIntTypeAnnotation';
}

export function isBinaryExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'BinaryExpression';
}

export function isBlockStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'BlockStatement';
}

export function isBooleanLiteralTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'BooleanLiteralTypeAnnotation';
}

export function isBooleanTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'BooleanTypeAnnotation';
}

export function isBreakStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'BreakStatement';
}

export function isCallExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'CallExpression';
}

export function isCatchClause(node: ESNode | Token): boolean %checks {
  return node.type === 'CatchClause';
}

export function isChainExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ChainExpression';
}

export function isClassBody(node: ESNode | Token): boolean %checks {
  return node.type === 'ClassBody';
}

export function isClassDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'ClassDeclaration';
}

export function isClassExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ClassExpression';
}

export function isClassImplements(node: ESNode | Token): boolean %checks {
  return node.type === 'ClassImplements';
}

export function isComponentDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'ComponentDeclaration';
}

export function isComponentParameter(node: ESNode | Token): boolean %checks {
  return node.type === 'ComponentParameter';
}

export function isComponentTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ComponentTypeAnnotation';
}

export function isComponentTypeParameter(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ComponentTypeParameter';
}

export function isConditionalExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ConditionalExpression';
}

export function isConditionalTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ConditionalTypeAnnotation';
}

export function isContinueStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ContinueStatement';
}

export function isDebuggerStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'DebuggerStatement';
}

export function isDeclareClass(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareClass';
}

export function isDeclareComponent(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareComponent';
}

export function isDeclaredPredicate(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclaredPredicate';
}

export function isDeclareEnum(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareEnum';
}

export function isDeclareExportAllDeclaration(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'DeclareExportAllDeclaration';
}

export function isDeclareExportDeclaration(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'DeclareExportDeclaration';
}

export function isDeclareFunction(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareFunction';
}

export function isDeclareInterface(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareInterface';
}

export function isDeclareModule(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareModule';
}

export function isDeclareModuleExports(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareModuleExports';
}

export function isDeclareOpaqueType(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareOpaqueType';
}

export function isDeclareTypeAlias(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareTypeAlias';
}

export function isDeclareVariable(node: ESNode | Token): boolean %checks {
  return node.type === 'DeclareVariable';
}

export function isDoWhileStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'DoWhileStatement';
}

export function isEmptyStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'EmptyStatement';
}

export function isEmptyTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'EmptyTypeAnnotation';
}

export function isEnumBooleanBody(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumBooleanBody';
}

export function isEnumBooleanMember(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumBooleanMember';
}

export function isEnumDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumDeclaration';
}

export function isEnumDefaultedMember(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumDefaultedMember';
}

export function isEnumNumberBody(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumNumberBody';
}

export function isEnumNumberMember(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumNumberMember';
}

export function isEnumStringBody(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumStringBody';
}

export function isEnumStringMember(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumStringMember';
}

export function isEnumSymbolBody(node: ESNode | Token): boolean %checks {
  return node.type === 'EnumSymbolBody';
}

export function isExistsTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'ExistsTypeAnnotation';
}

export function isExportAllDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'ExportAllDeclaration';
}

export function isExportDefaultDeclaration(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ExportDefaultDeclaration';
}

export function isExportNamedDeclaration(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ExportNamedDeclaration';
}

export function isExportSpecifier(node: ESNode | Token): boolean %checks {
  return node.type === 'ExportSpecifier';
}

export function isExpressionStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ExpressionStatement';
}

export function isForInStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ForInStatement';
}

export function isForOfStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ForOfStatement';
}

export function isForStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ForStatement';
}

export function isFunctionDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'FunctionDeclaration';
}

export function isFunctionExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'FunctionExpression';
}

export function isFunctionTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'FunctionTypeAnnotation';
}

export function isFunctionTypeParam(node: ESNode | Token): boolean %checks {
  return node.type === 'FunctionTypeParam';
}

export function isGenericTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'GenericTypeAnnotation';
}

export function isIdentifier(node: ESNode | Token): boolean %checks {
  return node.type === 'Identifier';
}

export function isIfStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'IfStatement';
}

export function isImportAttribute(node: ESNode | Token): boolean %checks {
  return node.type === 'ImportAttribute';
}

export function isImportDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'ImportDeclaration';
}

export function isImportDefaultSpecifier(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ImportDefaultSpecifier';
}

export function isImportExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ImportExpression';
}

export function isImportNamespaceSpecifier(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ImportNamespaceSpecifier';
}

export function isImportSpecifier(node: ESNode | Token): boolean %checks {
  return node.type === 'ImportSpecifier';
}

export function isIndexedAccessType(node: ESNode | Token): boolean %checks {
  return node.type === 'IndexedAccessType';
}

export function isInferredPredicate(node: ESNode | Token): boolean %checks {
  return node.type === 'InferredPredicate';
}

export function isInferTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'InferTypeAnnotation';
}

export function isInterfaceDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'InterfaceDeclaration';
}

export function isInterfaceExtends(node: ESNode | Token): boolean %checks {
  return node.type === 'InterfaceExtends';
}

export function isInterfaceTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'InterfaceTypeAnnotation';
}

export function isIntersectionTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'IntersectionTypeAnnotation';
}

export function isJSXAttribute(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXAttribute';
}

export function isJSXClosingElement(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXClosingElement';
}

export function isJSXClosingFragment(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXClosingFragment';
}

export function isJSXElement(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXElement';
}

export function isJSXEmptyExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXEmptyExpression';
}

export function isJSXExpressionContainer(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'JSXExpressionContainer';
}

export function isJSXFragment(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXFragment';
}

export function isJSXIdentifier(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXIdentifier';
}

export function isJSXMemberExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXMemberExpression';
}

export function isJSXNamespacedName(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXNamespacedName';
}

export function isJSXOpeningElement(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXOpeningElement';
}

export function isJSXOpeningFragment(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXOpeningFragment';
}

export function isJSXSpreadAttribute(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXSpreadAttribute';
}

export function isJSXSpreadChild(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXSpreadChild';
}

export function isJSXText(node: ESNode | Token): boolean %checks {
  return node.type === 'JSXText';
}

export function isKeyofTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'KeyofTypeAnnotation';
}

export function isLabeledStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'LabeledStatement';
}

export function isLogicalExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'LogicalExpression';
}

export function isMemberExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'MemberExpression';
}

export function isMetaProperty(node: ESNode | Token): boolean %checks {
  return node.type === 'MetaProperty';
}

export function isMethodDefinition(node: ESNode | Token): boolean %checks {
  return node.type === 'MethodDefinition';
}

export function isMixedTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'MixedTypeAnnotation';
}

export function isNewExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'NewExpression';
}

export function isNullableTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'NullableTypeAnnotation';
}

export function isNullLiteralTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'NullLiteralTypeAnnotation';
}

export function isNumberLiteralTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'NumberLiteralTypeAnnotation';
}

export function isNumberTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'NumberTypeAnnotation';
}

export function isObjectExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ObjectExpression';
}

export function isObjectPattern(node: ESNode | Token): boolean %checks {
  return node.type === 'ObjectPattern';
}

export function isObjectTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'ObjectTypeAnnotation';
}

export function isObjectTypeCallProperty(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ObjectTypeCallProperty';
}

export function isObjectTypeIndexer(node: ESNode | Token): boolean %checks {
  return node.type === 'ObjectTypeIndexer';
}

export function isObjectTypeInternalSlot(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ObjectTypeInternalSlot';
}

export function isObjectTypeMappedTypeProperty(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ObjectTypeMappedTypeProperty';
}

export function isObjectTypeProperty(node: ESNode | Token): boolean %checks {
  return node.type === 'ObjectTypeProperty';
}

export function isObjectTypeSpreadProperty(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ObjectTypeSpreadProperty';
}

export function isOpaqueType(node: ESNode | Token): boolean %checks {
  return node.type === 'OpaqueType';
}

export function isOptionalIndexedAccessType(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'OptionalIndexedAccessType';
}

export function isPrivateIdentifier(node: ESNode | Token): boolean %checks {
  return node.type === 'PrivateIdentifier';
}

export function isProperty(node: ESNode | Token): boolean %checks {
  return node.type === 'Property';
}

export function isPropertyDefinition(node: ESNode | Token): boolean %checks {
  return node.type === 'PropertyDefinition';
}

export function isQualifiedTypeIdentifier(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'QualifiedTypeIdentifier';
}

export function isQualifiedTypeofIdentifier(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'QualifiedTypeofIdentifier';
}

export function isRestElement(node: ESNode | Token): boolean %checks {
  return node.type === 'RestElement';
}

export function isReturnStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ReturnStatement';
}

export function isSequenceExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'SequenceExpression';
}

export function isSpreadElement(node: ESNode | Token): boolean %checks {
  return node.type === 'SpreadElement';
}

export function isStringLiteralTypeAnnotation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'StringLiteralTypeAnnotation';
}

export function isStringTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'StringTypeAnnotation';
}

export function isSuper(node: ESNode | Token): boolean %checks {
  return node.type === 'Super';
}

export function isSwitchCase(node: ESNode | Token): boolean %checks {
  return node.type === 'SwitchCase';
}

export function isSwitchStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'SwitchStatement';
}

export function isSymbolTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'SymbolTypeAnnotation';
}

export function isTaggedTemplateExpression(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'TaggedTemplateExpression';
}

export function isTemplateElement(node: ESNode | Token): boolean %checks {
  return node.type === 'TemplateElement';
}

export function isTemplateLiteral(node: ESNode | Token): boolean %checks {
  return node.type === 'TemplateLiteral';
}

export function isThisExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ThisExpression';
}

export function isThisTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'ThisTypeAnnotation';
}

export function isThrowStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'ThrowStatement';
}

export function isTryStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'TryStatement';
}

export function isTupleTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'TupleTypeAnnotation';
}

export function isTupleTypeLabeledElement(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'TupleTypeLabeledElement';
}

export function isTupleTypeSpreadElement(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'TupleTypeSpreadElement';
}

export function isTypeAlias(node: ESNode | Token): boolean %checks {
  return node.type === 'TypeAlias';
}

export function isTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'TypeAnnotation';
}

export function isTypeCastExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'TypeCastExpression';
}

export function isTypeofTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'TypeofTypeAnnotation';
}

export function isTypeParameter(node: ESNode | Token): boolean %checks {
  return node.type === 'TypeParameter';
}

export function isTypeParameterDeclaration(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'TypeParameterDeclaration';
}

export function isTypeParameterInstantiation(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'TypeParameterInstantiation';
}

export function isTypePredicate(node: ESNode | Token): boolean %checks {
  return node.type === 'TypePredicate';
}

export function isUnaryExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'UnaryExpression';
}

export function isUnionTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'UnionTypeAnnotation';
}

export function isUpdateExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'UpdateExpression';
}

export function isVariableDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'VariableDeclaration';
}

export function isVariableDeclarator(node: ESNode | Token): boolean %checks {
  return node.type === 'VariableDeclarator';
}

export function isVariance(node: ESNode | Token): boolean %checks {
  return node.type === 'Variance';
}

export function isVoidTypeAnnotation(node: ESNode | Token): boolean %checks {
  return node.type === 'VoidTypeAnnotation';
}

export function isWhileStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'WhileStatement';
}

export function isWithStatement(node: ESNode | Token): boolean %checks {
  return node.type === 'WithStatement';
}

export function isYieldExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'YieldExpression';
}

export function isLiteral(node: ESNode | Token): boolean %checks {
  return node.type === 'Literal';
}

export function isLineComment(node: ESNode | Token): boolean %checks {
  return node.type === 'Line';
}

export function isBlockComment(node: ESNode | Token): boolean %checks {
  return node.type === 'Block';
}

export function isMinusToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '-';
}

export function isPlusToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '+';
}

export function isLogicalNotToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '!';
}

export function isUnaryNegationToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '~';
}

export function isTypeOfToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'typeof';
}

export function isVoidToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'void';
}

export function isDeleteToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'delete';
}

export function isLooseEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '==';
}

export function isLooseNotEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '!=';
}

export function isStrictEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '===';
}

export function isStrictNotEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '!==';
}

export function isLessThanToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '<';
}

export function isLessThanOrEqualToToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '<=';
}

export function isGreaterThanToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>';
}

export function isGreaterThanOrEqualToToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>=';
}

export function isBitwiseLeftShiftToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '<<';
}

export function isBitwiseRightShiftToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>>';
}

export function isBitwiseUnsignedRightShiftToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>>>';
}

export function isAsterixToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '*';
}

export function isForwardSlashToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '/';
}

export function isPercentToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '%';
}

export function isExponentiationToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '**';
}

export function isBitwiseORToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '|';
}

export function isBitwiseXORToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '^';
}

export function isBitwiseANDToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '&';
}

export function isInToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'in';
}

export function isInstanceOfToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'instanceof';
}

export function isLogicalORToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '||';
}

export function isLogicalANDToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '&&';
}

export function isNullishCoalesceToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '??';
}

export function isEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '=';
}

export function isPlusEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '+=';
}

export function isMinusEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '-=';
}

export function isMultiplyEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '*=';
}

export function isDivideEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '/=';
}

export function isRemainderEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '%=';
}

export function isExponentateEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '**=';
}

export function isBitwiseLeftShiftEqualToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '<<=';
}

export function isBitwiseRightShiftEqualToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>>=';
}

export function isBitwiseUnsignedRightShiftEqualToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>>>=';
}

export function isBitwiseOREqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '|=';
}

export function isBitwiseXOREqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '^=';
}

export function isBitwiseANDEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '&=';
}

export function isLogicalOREqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '||=';
}

export function isLogicalANDEqualToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '&&=';
}

export function isNullishCoalesceEqualToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '??=';
}

export function isIncrementToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '++';
}

export function isDecrementToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '--';
}

export function isUnionTypeToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '|';
}

export function isIntersectionTypeToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '&';
}

export function isBreakToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'break';
}

export function isCaseToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'case';
}

export function isCatchToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'catch';
}

export function isClassToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'class';
}

export function isConstToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'const';
}

export function isContinueToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'continue';
}

export function isDebuggerToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'debugger';
}

export function isDefaultToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'default';
}

export function isDoToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'do';
}

export function isElseToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'else';
}

export function isEnumToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'enum';
}

export function isExportToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'export';
}

export function isExtendsToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'extends';
}

export function isFinallyToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'finally';
}

export function isForToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'for';
}

export function isFunctionToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'function';
}

export function isIfToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'if';
}

export function isImplementsToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'implements';
}

export function isImportToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'import';
}

export function isInterfaceToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'interface';
}

export function isNewToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'new';
}

export function isReturnToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'return';
}

export function isStaticToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'static';
}

export function isSuperToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'super';
}

export function isSwitchToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'switch';
}

export function isThisToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'this';
}

export function isThrowToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'throw';
}

export function isTryToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'try';
}

export function isVarToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'var';
}

export function isWhileToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'while';
}

export function isWithToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'with';
}

export function isYieldToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Keyword' && node.value === 'yield';
}

export function isAsKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'as') ||
    (node.type === 'Keyword' && node.value === 'as')
  );
}

export function isAsyncKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'async') ||
    (node.type === 'Keyword' && node.value === 'async')
  );
}

export function isAwaitKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'await') ||
    (node.type === 'Keyword' && node.value === 'await')
  );
}

export function isDeclareKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'declare') ||
    (node.type === 'Keyword' && node.value === 'declare')
  );
}

export function isFromKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'from') ||
    (node.type === 'Keyword' && node.value === 'from')
  );
}

export function isGetKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'get') ||
    (node.type === 'Keyword' && node.value === 'get')
  );
}

export function isLetKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'let') ||
    (node.type === 'Keyword' && node.value === 'let')
  );
}

export function isModuleKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'module') ||
    (node.type === 'Keyword' && node.value === 'module')
  );
}

export function isOfKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'of') ||
    (node.type === 'Keyword' && node.value === 'of')
  );
}

export function isSetKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'set') ||
    (node.type === 'Keyword' && node.value === 'set')
  );
}

export function isTypeKeyword(node: ESNode | Token): boolean %checks {
  return (
    (node.type === 'Identifier' && node.name === 'type') ||
    (node.type === 'Keyword' && node.value === 'type')
  );
}

export function isCommaToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === ',';
}

export function isColonToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === ':';
}

export function isSemicolonToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === ';';
}

export function isDotToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '.';
}

export function isDotDotDotToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '...';
}

export function isOptionalChainToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '?.';
}

export function isQuestionMarkToken(node: ESNode | Token): boolean %checks {
  return node.type === 'Punctuator' && node.value === '?';
}

export function isOpeningParenthesisToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '(';
}

export function isClosingParenthesisToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === ')';
}

export function isOpeningCurlyBracketToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '{';
}

export function isClosingCurlyBracketToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '}';
}

export function isOpeningAngleBracketToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '<';
}

export function isClosingAngleBracketToken(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'Punctuator' && node.value === '>';
}
