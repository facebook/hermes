/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {KEYWORDS_ES3_ONLY} from '../definitions/reservedWords';
import isValidIdentifier from './isValidIdentifier';

const RESERVED_WORDS_ES3_ONLY = new Set(KEYWORDS_ES3_ONLY);

/**
 * Check if the input `name` is a valid identifier name according to the ES3 specification.
 *
 * Additional ES3 reserved words are
 */
export default function isValidES3Identifier(name: string): boolean {
  return isValidIdentifier(name) && !RESERVED_WORDS_ES3_ONLY.has(name);
}
