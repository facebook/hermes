/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

export * from './traverse';

import {HERMES_AST_VISITOR_KEYS} from '../HermesParserVisitorKeys';

export const VISITOR_KEYS = {};
for (const key of Object.keys(HERMES_AST_VISITOR_KEYS)) {
  VISITOR_KEYS[key] = Object.keys(HERMES_AST_VISITOR_KEYS[key]);
}

import {nodeBases} from './HermesParserNodeTypes';

export * from './HermesParserNodeTypes';

export const TYPES = Object.keys(HERMES_AST_VISITOR_KEYS);
export const ALIAS_KEYS = {};
export const FLIPPED_ALIAS_KEYS = {};
export const DEPRECATED_KEYS = {};

/**
 * Test if a `nodeType` is a `targetType`.
 * This would ordinarily invoke alias logic, but there is none in Hermes yet.
 */
export function isType(nodeType: string, targetType: string) {
  return nodeType === targetType;
}

// comments
export {default as addComment} from './comments/addComment';
export {default as addComments} from './comments/addComments';
export {default as inheritInnerComments} from './comments/inheritInnerComments';
export {default as inheritLeadingComments} from './comments/inheritLeadingComments';
export {default as inheritsComments} from './comments/inheritsComments';
export {default as inheritTrailingComments} from './comments/inheritTrailingComments';
export {default as removeComments} from './comments/removeComments';

/**
 * Validation unimplemented for now.
 */
export function validate(node?: Object, key: string, val: any): void {}

export {buildMatchMemberExpression} from './buildMatchMemberExpression';

export {isScope} from './isScope';
