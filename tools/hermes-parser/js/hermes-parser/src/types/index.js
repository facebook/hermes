/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {FLIPPED_ALIAS_KEYS, VISITOR_KEYS} from './definitions';
import isCompatTag from './validators/react/isCompatTag';

export * from './traverse';
export * from './generated/node-types';
export * from './generated/asserts';

export const TYPES = Object.keys(VISITOR_KEYS).concat(
  Object.keys(FLIPPED_ALIAS_KEYS),
);
export * from './definitions';

/**
 * Comments
 */
export {default as addComment} from './comments/addComment';
export {default as addComments} from './comments/addComments';
export {default as inheritInnerComments} from './comments/inheritInnerComments';
export {default as inheritLeadingComments} from './comments/inheritLeadingComments';
export {default as inheritsComments} from './comments/inheritsComments';
export {default as inheritTrailingComments} from './comments/inheritTrailingComments';
export {default as removeComments} from './comments/removeComments';

// retrievers
export {default as getBindingIdentifiers} from './retrievers/getBindingIdentifiers';
export {default as getOuterBindingIdentifiers} from './retrievers/getOuterBindingIdentifiers';

/**
 * Validation
 */
export {default as is} from './validators/is';
export {default as isBinding} from './validators/isBinding';
export {default as isBlockScoped} from './validators/isBlockScoped';
export {default as isImmutable} from './validators/isImmutable';
export {default as isLet} from './validators/isLet';
export {default as isNode} from './validators/isNode';
export {default as isNodesEquivalent} from './validators/isNodesEquivalent';
export {default as isReferenced} from './validators/isReferenced';
export {default as isScope} from './validators/isScope';
export {default as isSpecifierDefault} from './validators/isSpecifierDefault';
export {default as isType} from './validators/isType';
export {default as isValidES3Identifier} from './validators/isValidES3Identifier';
export {default as isValidIdentifier} from './validators/isValidIdentifier';
export {default as isVar} from './validators/isVar';
export {default as matchesPattern} from './validators/matchesPattern';
export {default as validate} from './validators/validate';
export {default as buildMatchMemberExpression} from './validators/buildMatchMemberExpression';
export const react = {isCompatTag};
