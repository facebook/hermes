/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import aliasDefs from './aliases';
import {HERMES_AST_VISITOR_KEYS} from '../generated/visitor-keys';

/**
 * Alias keys
 */
export const ALIAS_KEYS = aliasDefs;
export const FLIPPED_ALIAS_KEYS = Object.create(null);

for (let typeName of Object.keys(ALIAS_KEYS)) {
  for (let aliasName of ALIAS_KEYS[typeName]) {
    FLIPPED_ALIAS_KEYS[aliasName] = FLIPPED_ALIAS_KEYS[aliasName] || [];
    FLIPPED_ALIAS_KEYS[aliasName].push(typeName);
  }
}

/**
 * Visitor keys
 */
export const VISITOR_KEYS = {};
// TODO: Should this acutally be different?
export const NODE_FIELDS = {};

for (const key of Object.keys(HERMES_AST_VISITOR_KEYS)) {
  VISITOR_KEYS[key] = NODE_FIELDS[key] = Object.keys(
    HERMES_AST_VISITOR_KEYS[key],
  );
}

/**
 * Constant keys
 */
export * from './constants';

/**
 * Other keys
 *
 * TODO: REMOVE THESE!
 */
export const BUILDER_KEYS = {};
export const DEPRECATED_KEYS = {};
export const NODE_PARENT_VALIDATIONS = {};
