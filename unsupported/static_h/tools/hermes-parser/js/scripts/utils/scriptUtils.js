/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

// @licenselint-loose-mode

'use strict';

import fs from 'fs';
import mkdirp from 'mkdirp';
import path from 'path';
import * as prettier from 'prettier';
import aliasDefs from './aliases';
// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../.prettierrc.json';

export type ESTreeJSON = $ReadOnlyArray<
  $ReadOnly<{
    name: string,
    base: string,
    arguments: $ReadOnlyArray<
      $ReadOnly<{
        type:
          | 'NodeLabel'
          | 'NodeString'
          | 'NodeBoolean'
          | 'NodeNumber'
          | 'NodePtr'
          | 'NodeList',
        name: string,
        optional: boolean,
      }>,
    >,
  }>,
>;

// $FlowExpectedError[cannot-resolve-module]
export const HermesESTreeJSON: ESTreeJSON = require('../dist/HermesESTreeJSON.json');

type FlowStyle = false | 'loose' | 'strict' | 'strict-local';
function HEADER(flow: FlowStyle): string {
  let flowDirective = '';
  if (flow !== false) {
    flowDirective = `* ${'@'}flow`;
    if (flow !== 'loose') {
      flowDirective += ` ${flow}`;
    }
    flowDirective += '\n';
  }

  return `\
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
${flowDirective}\
 * @format
 */

// lint directives to let us do some basic validation of generated files
/* eslint no-undef: 'error', no-unused-vars: ['error', {vars: "local"}], no-redeclare: 'error' */
/* global $NonMaybeType, $Partial, $ReadOnly, $ReadOnlyArray */

'use strict';

`;
}

type Package =
  | 'hermes-eslint'
  | 'hermes-estree'
  | 'hermes-parser'
  | 'hermes-transform';
export function formatAndWriteDistArtifact({
  code: code_,
  flow = 'loose',
  package: pkg,
  filename,
  subdirSegments = [],
}: $ReadOnly<{
  code: string,
  flow?: FlowStyle,
  // will write to ../<package>/dist/<...subdirSegments>/<filename>
  package: Package,
  filename: string,
  subdirSegments?: $ReadOnlyArray<string>,
}>): void {
  // make sure the code has a header
  const code = code_.slice(0, 3) === '/**' ? code_ : HEADER(flow) + code_;

  // Format the file
  const formattedContents = prettier.format(code, {
    ...prettierConfig,
    parser: 'flow',
  });

  // make sure the folder exists first
  const folder = path.resolve(
    __dirname,
    '..',
    '..',
    pkg,
    'dist',
    ...subdirSegments,
  );
  mkdirp.sync(folder);
  // write to disk
  const artifactPath = path.resolve(folder, filename);
  fs.writeFileSync(artifactPath, formattedContents);
}

export const LITERAL_TYPES: $ReadOnlySet<string> = new Set([
  'BigIntLiteral',
  'BooleanLiteral',
  'NullLiteral',
  'NumericLiteral',
  'RegExpLiteral',
  'StringLiteral',
]);

export const FLIPPED_ALIAS_KEYS: $ReadOnly<{
  [string]: $ReadOnlySet<string>,
}> = (() => {
  // $FlowExpectedError[incompatible-type]
  const flippedAliasKeys: {[string]: Set<string>} = Object.create(null);

  for (let typeName of Object.keys(aliasDefs)) {
    for (let aliasName of aliasDefs[typeName]) {
      if (flippedAliasKeys[aliasName]) {
        flippedAliasKeys[aliasName].add(typeName);
      } else {
        flippedAliasKeys[aliasName] = new Set([typeName]);
      }
    }
  }

  return flippedAliasKeys;
})();

export const NODES_WITHOUT_TRANSFORM_NODE_TYPES: $ReadOnlySet<string> = new Set(
  [
    // a lot of additional properties are set on this, but nobody should ever "create" one so
    // we purposely don't define a creation function
    'Program',
  ],
);
