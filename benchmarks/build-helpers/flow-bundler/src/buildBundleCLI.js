/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import * as path from 'path';
import {createModuleGraph} from './ModuleGraph';
import {writeFile, parseFile} from './utils';
import yargs from 'yargs';

async function main() {
  const rawArgs = process.argv.slice(2);
  const cliYargs = yargs(rawArgs)
    .usage('Usage: $0 [entrypoint]')
    .scriptName('flow-bundler')
    .option('out', {
      alias: 'o',
      describe: 'Path to output the bundle',
      nargs: 1,
      required: true,
      requiresArg: true,
      type: 'string',
    })
    .option('root', {
      alias: 'r',
      describe:
        'Root path, paths will be shown relative to this location (default: cwd).',
      nargs: 1,
      requiresArg: true,
      type: 'string',
      default: process.cwd(),
    })
    .positional('entrypoint', {
      type: 'string',
      describe: 'Bundle entrypoint files',
    })
    .wrap(yargs.terminalWidth())
    .parse();

  const rootPath = cliYargs.root;
  const outPath = cliYargs.out;
  const entrypoints: Array<string> = cliYargs._;

  const bundleSource = await createModuleGraph(rootPath, entrypoints);
  const bundleHeader = `\
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 *
 * Entrypoints:
 *   ${entrypoints.map(f => path.relative(rootPath, f)).join('\n *   ')}
 */

'use strict';

// SH console Polyfill
const console =
  typeof globalThis.console === 'undefined'
    ? {
        log: globalThis.print,
        error: globalThis.print,
      }
    : globalThis.console;

`;

  await writeFile(outPath, bundleHeader + bundleSource);
}

main().catch((err: mixed) => {
  console.error(err);
  process.exitCode = 1;
});
