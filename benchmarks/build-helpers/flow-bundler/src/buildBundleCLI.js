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
import {writeFile, parseFile, hermesASTToBabel} from './utils';
import yargs from 'yargs';

// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import generate from '@babel/generator';

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

  const bundle = await createModuleGraph(rootPath, entrypoints);
  const bundleHeader = `*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 *
 * Entrypoints:
 *   ${entrypoints.map(f => path.relative(rootPath, f)).join('\n *   ')}
 `;

  const fileMapping: {[string]: string} = {};
  const bundleAST = {
    type: 'File',
    program: {
      type: 'Program',
      // $FlowExpectedError[unclear-type]
      body: [] as Array<any>,
    },
  };

  // Merge files into single bundle AST.
  for (const file of bundle) {
    const babelAST = hermesASTToBabel(file.ast, file.file);

    bundleAST.program.body.push(...babelAST.program.body);
    fileMapping[file.file] = file.code;
  }

  // Add bundle docblock comment
  if (bundleAST.program.body.length > 0) {
    const firstStmt = bundleAST.program.body[0];
    firstStmt.leadingComments = [
      {type: 'CommentBlock', value: bundleHeader},
      ...firstStmt.leadingComments,
    ];
  }

  const bundleSource = generate(bundleAST, {sourceMaps: true}, fileMapping);

  await writeFile(
    outPath,
    bundleSource.code +
      '\n//# sourceMappingURL=' +
      path.basename(outPath) +
      '.map\n',
  );
  await writeFile(outPath + '.map', JSON.stringify(bundleSource.map));
}

main().catch((err: mixed) => {
  console.error(err);
  process.exitCode = 1;
});
