/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {
  isIdentifier,
  isMemberExpression,
  isStringLiteral,
} from './HermesParserNodeTypes';

/**
 * Determines whether or not the input node `member` matches the
 * input `match`.
 *
 * For example, given the match `React.createClass` it would match the
 * parsed nodes of `React.createClass` and `React["createClass"]`.
 */
function matchesPattern(
  member: Object,
  match: string | Array<string>,
  allowPartial?: boolean
): boolean {
  // not a member expression
  if (!isMemberExpression(member)) return false;

  const parts = Array.isArray(match) ? match : match.split('.');
  const nodes = [];

  let node;
  for (node = member; isMemberExpression(node); node = node.object) {
    nodes.push(node.property);
  }
  nodes.push(node);

  if (nodes.length < parts.length) return false;
  if (!allowPartial && nodes.length > parts.length) return false;

  for (let i = 0, j = nodes.length - 1; i < parts.length; i++, j--) {
    const node = nodes[j];
    let value;
    if (isIdentifier(node)) {
      value = node.name;
    } else if (isStringLiteral(node)) {
      value = node.value;
    } else {
      return false;
    }

    if (parts[i] !== value) return false;
  }

  return true;
}

/**
 * Build a function that when called will return whether or not the
 * input `node` `MemberExpression` matches the input `match`.
 *
 * For example, given the match `React.createClass` it would match the
 * parsed nodes of `React.createClass` and `React["createClass"]`.
 */
export function buildMatchMemberExpression(
  match: string,
  allowPartial?: boolean
): Object => boolean {
  const parts = match.split('.');

  return (member: Object): boolean =>
    matchesPattern(member, parts, allowPartial);
}
