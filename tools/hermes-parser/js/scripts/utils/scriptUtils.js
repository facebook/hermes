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

export const HermesESTreeJSONFile: string = path.resolve(
  __dirname,
  '../dist/HermesESTreeJSON.json',
);
// $FlowExpectedError[unsupported-syntax]
export const GetHermesESTreeJSON: () => ESTreeJSON = () =>
  // $FlowExpectedError[unsupported-syntax]
  require(HermesESTreeJSONFile);

type FlowStyle = false | 'loose' | 'strict' | 'strict-local';
function HEADER(flow: FlowStyle): string {
  let flowDirective = ``;
  if (flow === false) {
    flowDirective += `${'@'}noflow`;
  } else {
    flowDirective += `${'@'}flow`;
    if (flow !== 'loose') {
      flowDirective += ` ${flow}`;
    }
  }

  return `\
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * ${flowDirective}
 * @format
 * ${'@'}generated
 */

/*
 * !!! GENERATED FILE !!!
 *
 * Any manual changes to this file will be overwritten. To regenerate run \`yarn build\`.
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

type ArtifactOptions = $ReadOnly<{
  code: string,
  flow?: FlowStyle,
  // will write to ../<package>/<file>
  package: Package,
  file: string,
}>;

export function formatAndWriteDistArtifact(opts: ArtifactOptions): void {
  formatAndWriteArtifact({
    ...opts,
    file: path.join('dist', opts.file),
  });
}
export function formatAndWriteSrcArtifact(opts: ArtifactOptions): void {
  formatAndWriteArtifact({
    ...opts,
    file: path.join('src', opts.file),
  });
}

function formatAndWriteArtifact({
  code: code_,
  flow = 'loose',
  package: pkg,
  file,
}: ArtifactOptions): void {
  // make sure the code has a header
  const code = code_.slice(0, 3) === '/**' ? code_ : HEADER(flow) + code_;

  // Format the file
  const formattedContents = prettier.format(code, {
    ...prettierConfig,
    parser: 'flow',
  });

  // make sure the folder exists first
  const folder = path.resolve(__dirname, '..', '..', pkg, path.dirname(file));
  mkdirp.sync(folder);
  // write to disk
  const artifactPath = path.resolve(folder, path.basename(file));
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
