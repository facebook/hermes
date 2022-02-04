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

import type {ESNode, Token} from './types';

import {
  isArrowFunctionExpression,
  isBlockComment,
  isClassDeclaration,
  isClassExpression,
  isClassPrivateProperty,
  isClassProperty,
  isFunctionDeclaration,
  isFunctionExpression,
  isLineComment,
  isLiteral,
  isMemberExpression,
  isMethodDefinition,
  isProperty,
} from './generated/predicates';

export * from './generated/predicates';

export function isClass(node: ESNode): boolean %checks {
  return isClassDeclaration(node) || isClassExpression(node);
}

export function isClassPropertyWithNonComputedName(
  node: ESNode,
): boolean %checks {
  return isClassProperty(node) && node.computed === false;
}

export function isClassMember(node: ESNode): boolean %checks {
  return (
    isClassProperty(node) ||
    isClassPrivateProperty(node) ||
    isMethodDefinition(node)
  );
}

export function isClassMemberWithNonComputedName(
  node: ESNode,
): boolean %checks {
  return isClassMember(node) && node.computed === false;
}

export function isComment(node: ESNode | Token): boolean %checks {
  return isBlockComment(node) || isLineComment(node);
}

export function isFunction(node: ESNode): boolean %checks {
  return (
    isArrowFunctionExpression(node) ||
    isFunctionDeclaration(node) ||
    isFunctionExpression(node)
  );
}

export function isMethodDefinitionWithNonComputedName(
  node: ESNode,
): boolean %checks {
  return isMethodDefinition(node) && node.computed === false;
}

export function isMemberExpressionWithNonComputedProperty(
  node: ESNode,
): boolean %checks {
  return isMemberExpression(node) && node.computed === false;
}

export function isOptionalMemberExpressionWithNonComputedProperty(
  node: ESNode,
): boolean %checks {
  return isMemberExpression(node) && node.computed === false;
}

export function isObjectPropertyWithShorthand(node: ESNode): boolean %checks {
  return isProperty(node) && node.shorthand === true;
}

export function isObjectPropertyWithNonComputedName(
  node: ESNode,
): boolean %checks {
  return isProperty(node) && node.computed === false;
}

export function isBigIntLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'bigint';
}

export function isBooleanLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'boolean';
}

export function isNullLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'null';
}

export function isNumericLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'numeric';
}

export function isRegExpLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'regexp';
}

export function isStringLiteral(node: ESNode): boolean %checks {
  return isLiteral(node) && node.literalType === 'string';
}
