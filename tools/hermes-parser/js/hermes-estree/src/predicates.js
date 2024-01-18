/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {ESNode, Token} from './types';

import {
  isArrayExpression,
  isArrowFunctionExpression,
  isAssignmentExpression,
  isAwaitExpression,
  isBinaryExpression,
  isBlockComment,
  isBlockStatement,
  isBreakStatement,
  isCallExpression,
  isChainExpression,
  isClassDeclaration,
  isClassExpression,
  isConditionalExpression,
  isContinueStatement,
  isDebuggerStatement,
  isDeclareClass,
  isDeclareFunction,
  isDeclareInterface,
  isDeclareModule,
  isDeclareOpaqueType,
  isDeclareTypeAlias,
  isDeclareVariable,
  isDoWhileStatement,
  isEmptyStatement,
  isEnumDeclaration,
  isExpressionStatement,
  isForInStatement,
  isForOfStatement,
  isForStatement,
  isFunctionDeclaration,
  isFunctionExpression,
  isIdentifier,
  isIfStatement,
  isImportExpression,
  isInterfaceDeclaration,
  isJSXElement,
  isJSXFragment,
  isLabeledStatement,
  isLineComment,
  isLiteral,
  isLogicalExpression,
  isMemberExpression,
  isMetaProperty,
  isMethodDefinition,
  isNewExpression,
  isObjectExpression,
  isOpaqueType,
  isProperty,
  isPropertyDefinition,
  isReturnStatement,
  isSequenceExpression,
  isSwitchStatement,
  isTaggedTemplateExpression,
  isTemplateLiteral,
  isThisExpression,
  isThrowStatement,
  isTryStatement,
  isTypeAlias,
  isTypeCastExpression,
  isUnaryExpression,
  isUpdateExpression,
  isVariableDeclaration,
  isWhileStatement,
  isWithStatement,
  isYieldExpression,
} from './generated/predicates';

export * from './generated/predicates';

// $FlowFixMe[deprecated-type]
export function isClass(node: ESNode): boolean %checks {
  return isClassDeclaration(node) || isClassExpression(node);
}

export function isPropertyDefinitionWithNonComputedName(
  node: ESNode,
  // $FlowFixMe[deprecated-type]
): boolean %checks {
  return isPropertyDefinition(node) && node.computed === false;
}

// $FlowFixMe[deprecated-type]
export function isClassMember(node: ESNode): boolean %checks {
  return isPropertyDefinition(node) || isMethodDefinition(node);
}

export function isClassMemberWithNonComputedName(
  node: ESNode,
  // $FlowFixMe[deprecated-type]
): boolean %checks {
  return isClassMember(node) && node.computed === false;
}

// $FlowFixMe[deprecated-type]
export function isComment(node: ESNode | Token): boolean %checks {
  return isBlockComment(node) || isLineComment(node);
}

// $FlowFixMe[deprecated-type]
export function isFunction(node: ESNode): boolean %checks {
  return (
    isArrowFunctionExpression(node) ||
    isFunctionDeclaration(node) ||
    isFunctionExpression(node)
  );
}

export function isMethodDefinitionWithNonComputedName(
  node: ESNode,
  // $FlowFixMe[deprecated-type]
): boolean %checks {
  return isMethodDefinition(node) && node.computed === false;
}

export function isMemberExpressionWithNonComputedProperty(
  node: ESNode,
  // $FlowFixMe[deprecated-type]
): boolean %checks {
  return isMemberExpression(node) && node.computed === false;
}

export function isOptionalMemberExpressionWithNonComputedProperty(
  node: ESNode,
  // $FlowFixMe[deprecated-type]
): boolean %checks {
  return isMemberExpression(node) && node.computed === false;
}

// $FlowFixMe[deprecated-type]
export function isObjectPropertyWithShorthand(node: ESNode): boolean %checks {
  return isProperty(node) && node.shorthand === true;
}

export function isObjectPropertyWithNonComputedName(
  node: ESNode,
  // $FlowFixMe[deprecated-type]
): boolean %checks {
  return isProperty(node) && node.computed === false;
}

// $FlowFixMe[deprecated-type]
export function isBigIntLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'bigint';
}

// $FlowFixMe[deprecated-type]
export function isBooleanLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'boolean';
}

// $FlowFixMe[deprecated-type]
export function isNullLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'null';
}

// $FlowFixMe[deprecated-type]
export function isNumericLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'numeric';
}

// $FlowFixMe[deprecated-type]
export function isRegExpLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'regexp';
}

// $FlowFixMe[deprecated-type]
export function isStringLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'string';
}

// $FlowFixMe[deprecated-type]
export function isExpression(node: ESNode): boolean %checks {
  return (
    isThisExpression(node) ||
    isArrayExpression(node) ||
    isObjectExpression(node) ||
    isFunctionExpression(node) ||
    isArrowFunctionExpression(node) ||
    isYieldExpression(node) ||
    isLiteral(node) ||
    isUnaryExpression(node) ||
    isUpdateExpression(node) ||
    isBinaryExpression(node) ||
    isAssignmentExpression(node) ||
    isLogicalExpression(node) ||
    isMemberExpression(node) ||
    isConditionalExpression(node) ||
    isCallExpression(node) ||
    isNewExpression(node) ||
    isSequenceExpression(node) ||
    isTemplateLiteral(node) ||
    isTaggedTemplateExpression(node) ||
    isClassExpression(node) ||
    isMetaProperty(node) ||
    isIdentifier(node) ||
    isAwaitExpression(node) ||
    isImportExpression(node) ||
    isChainExpression(node) ||
    isTypeCastExpression(node) ||
    isJSXFragment(node) ||
    isJSXElement(node)
  );
}

// $FlowFixMe[deprecated-type]
export function isStatement(node: ESNode): boolean %checks {
  return (
    isBlockStatement(node) ||
    isBreakStatement(node) ||
    isClassDeclaration(node) ||
    isContinueStatement(node) ||
    isDebuggerStatement(node) ||
    isDeclareClass(node) ||
    isDeclareVariable(node) ||
    isDeclareFunction(node) ||
    isDeclareInterface(node) ||
    isDeclareModule(node) ||
    isDeclareOpaqueType(node) ||
    isDeclareTypeAlias(node) ||
    isDoWhileStatement(node) ||
    isEmptyStatement(node) ||
    isEnumDeclaration(node) ||
    isExpressionStatement(node) ||
    isForInStatement(node) ||
    isForOfStatement(node) ||
    isForStatement(node) ||
    isFunctionDeclaration(node) ||
    isIfStatement(node) ||
    isInterfaceDeclaration(node) ||
    isLabeledStatement(node) ||
    isOpaqueType(node) ||
    isReturnStatement(node) ||
    isSwitchStatement(node) ||
    isThrowStatement(node) ||
    isTryStatement(node) ||
    isTypeAlias(node) ||
    isVariableDeclaration(node) ||
    isWhileStatement(node) ||
    isWithStatement(node)
  );
}
