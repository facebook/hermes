/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

import type {Expression} from 'hermes-estree';

export type ParserOptions = {
  allowReturnOutsideFunction?: boolean,
  babel?: boolean,
  flow?: 'all' | 'detect',
  enableExperimentalComponentSyntax?: boolean,
  enableExperimentalFlowMatchSyntax?: boolean,
  enableExperimentalFlowRecordSyntax?: boolean,
  reactRuntimeTarget?: '18' | '19',
  sourceFilename?: string,
  sourceType?: 'module' | 'script' | 'unambiguous',
  tokens?: boolean,
  transformOptions?: {
    +TransformEnumSyntax?: {
      +enable: boolean,
      +getRuntime?: () => Expression,
    },
  },
};

export const ParserOptionsKeys: $ReadOnlySet<$Keys<ParserOptions>> = new Set([
  'allowReturnOutsideFunction',
  'babel',
  'flow',
  'enableExperimentalComponentSyntax',
  'enableExperimentalFlowMatchSyntax',
  'enableExperimentalFlowRecordSyntax',
  'reactRuntimeTarget',
  'sourceFilename',
  'sourceType',
  'tokens',
  'transformOptions',
]);
