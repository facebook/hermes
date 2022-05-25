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

/*
 * !!! THIS FILE WILL BE OVERWRITTEN BY CODEGEN !!!
 *
 * Statically it should only contain the minimal set of
 * definitions required to typecheck the local code.
 */

import type {ESNode, Token} from '../types';

export function isArrowFunctionExpression(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'ArrowFunctionExpression';
}

export function isClassDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'ClassDeclaration';
}

export function isClassExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'ClassExpression';
}

export function isPropertyDefinition(node: ESNode | Token): boolean %checks {
  return node.type === 'PropertyDefinition';
}

export function isFunctionDeclaration(node: ESNode | Token): boolean %checks {
  return node.type === 'FunctionDeclaration';
}

export function isFunctionExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'FunctionExpression';
}

export function isIdentifier(node: ESNode | Token): boolean %checks {
  return node.type === 'Identifier';
}

export function isLiteral(node: ESNode | Token): boolean %checks {
  return node.type === 'Literal';
}

export function isMemberExpression(node: ESNode | Token): boolean %checks {
  return node.type === 'MemberExpression';
}

export function isMethodDefinition(node: ESNode | Token): boolean %checks {
  return node.type === 'MethodDefinition';
}

export function isOptionalMemberExpression(
  node: ESNode | Token,
): boolean %checks {
  return node.type === 'OptionalMemberExpression';
}

export function isProperty(node: ESNode | Token): boolean %checks {
  return node.type === 'Property';
}

export function isBlockComment(node: ESNode | Token): boolean %checks {
  return node.type === 'Block';
}

export function isLineComment(node: ESNode | Token): boolean %checks {
  return node.type === 'Line';
}
