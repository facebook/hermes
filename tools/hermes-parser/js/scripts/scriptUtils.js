/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

// @lint-ignore-every LICENSELINT

'use strict';

import {execSync} from 'child_process';
import fs from 'fs';
import path from 'path';

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
export const HermesESTreeJSON: ESTreeJSON = require('./dist/HermesESTreeJSON.json');

type FlowStyle = false | 'loose' | 'strict' | 'strict-local';
function HEADER(flow: FlowStyle): string {
  let flowDirective = '';
  if (flow !== false) {
    flowDirective = ` * ${'@'}flow`;
    if (flow !== 'loose') {
      flowDirective += flow;
    }
    flowDirective += '\n';
  }

  return (
    `\
/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
` +
    flowDirective +
    `\
 * @format
 */

'use strict';

`
  );
}

type Package =
  | 'hermes-eslint'
  | 'hermes-estree'
  | 'hermes-parser'
  | 'hermes-transform';
export function formatAndWriteDistArtifact({
  code: code_,
  flow = false,
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

  // Format then sign file and write to disk
  const formattedContents = execSync('prettier --parser=flow', {
    input: code,
  }).toString();
  fs.writeFileSync(
    path.resolve(__dirname, '..', pkg, 'dist', ...subdirSegments, filename),
    formattedContents,
  );
}
