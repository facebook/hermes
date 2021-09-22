/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {isIdentifier, isImportDefaultSpecifier} from '../generated/node-types';

/**
 * Check if the input `specifier` is a `default` import or export.
 */
export default function isSpecifierDefault(
  specifier: t.ModuleSpecifier,
): boolean {
  return (
    isImportDefaultSpecifier(specifier) ||
    isIdentifier(specifier.imported || specifier.exported, {
      name: 'default',
    })
  );
}
