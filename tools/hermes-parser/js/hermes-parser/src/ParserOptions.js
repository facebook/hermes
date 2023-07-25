/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

export type ParserOptions = {
  allowReturnOutsideFunction?: boolean,
  babel?: boolean,
  flow?: 'all' | 'detect',
  enableExperimentalComponentSyntax?: boolean,
  sourceFilename?: string,
  sourceType?: 'module' | 'script' | 'unambiguous',
  tokens?: boolean,
};

export const ParserOptionsKeys: $ReadOnlySet<$Keys<ParserOptions>> = new Set([
  'allowReturnOutsideFunction',
  'babel',
  'flow',
  'enableExperimentalComponentSyntax',
  'sourceFilename',
  'sourceType',
  'tokens',
]);
