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
import {parseFile, hermesASTToBabel, writeBundle} from './utils';
import transformJSX from './transforms/transformJSX';
import {analyze} from 'hermes-eslint/dist/scope-manager/analyze';
import yargs from 'yargs';

// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import generate from '@babel/generator';
// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import {transformFromAstSync} from '@babel/core';

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
    .option('simple-jsx-transform', {
      describe: 'Basic JSX transform for use with MiniReact',
      default: false,
      type: 'boolean',
    })
    .option('es5', {
      describe:
        'Create ES5 syntax compatible bundle along side standard bundle',
      default: false,
      type: 'boolean',
    })
    .option('strip-types', {
      describe: 'Strip flow types without lowering',
      default: false,
      type: 'boolean',
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
    .option('babel-transforms', {
      alias: 't',
      describe:
        'Babel transforms to apply to the file before writing. Expects a path to a babel config file.',
      type: 'string',
      default: '',
    })
    .positional('entrypoint', {
      type: 'string',
      describe: 'Bundle entrypoint files',
    })
    .wrap(yargs.terminalWidth())
    .parse();

  const rootPath = path.resolve(cliYargs.root);
  const outPath = path.resolve(cliYargs.out);
  const createES5Bundle = cliYargs.es5;
  const stripTypes = cliYargs.stripTypes;
  const simpleJSXTransform = cliYargs.simpleJsxTransform;
  const babelTransforms: string = cliYargs.babelTransforms;
  const entrypoints: Array<string> = cliYargs._.map(f =>
    path.resolve(rootPath, f),
  );

  const bundle = await createModuleGraph(rootPath, entrypoints);
  const bundleHeader = `*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * ${'@'}flow
 * ${'@'}generated
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
      directives: [],
    },
  };

  const outPathDir = path.dirname(outPath);

  // Merge files into single bundle AST.
  for (const file of bundle) {
    let ast = file.ast;

    if (simpleJSXTransform) {
      const jsxTransformedOutput = await transformJSX({
        ast: file.ast,
        scopeManager: analyze(file.ast),
        code: file.code,
      });
      ast = jsxTransformedOutput.ast;
    }

    const relativeFilePath = path.relative(outPathDir, file.file);

    const babelAST = hermesASTToBabel(ast, relativeFilePath);

    bundleAST.program.body.push(...babelAST.program.body);
    fileMapping[relativeFilePath] = file.code;
  }

  // Add bundle docblock comment
  if (bundleAST.program.body.length > 0) {
    const firstStmt = bundleAST.program.body[0];
    firstStmt.leadingComments = [
      {type: 'CommentBlock', value: bundleHeader},
      ...(firstStmt.leadingComments ?? []),
    ];
  }

  let transformedBundleAST = bundleAST;
  if (babelTransforms !== '') {
    const transformedBundle = transformFromAstSync(bundleAST, fileMapping, {
      ast: true,
      code: false,
      // $FlowExpectedError[unsupported-syntax]
      plugins: require(babelTransforms),
    });
    transformedBundleAST = transformedBundle.ast;
  }

  // Generate bundle for Static Hermes
  const shBundleSource = generate(
    transformedBundleAST,
    {sourceMaps: true},
    fileMapping,
  );
  await writeBundle(outPath, shBundleSource.code, shBundleSource.map);

  if (createES5Bundle) {
    // Generate bundle for ES5 runtimes (aka normal Hermes)
    const es5Bundle = transformFromAstSync(transformedBundleAST, fileMapping, {
      ast: true,
      code: false,
      presets: [
        [
          require.resolve('metro-react-native-babel-preset'),
          {enableBabelRuntime: false},
        ],
      ],
    });
    const es5bundleSource = generate(
      es5Bundle.ast,
      {sourceMaps: true},
      fileMapping,
    );
    await writeBundle(
      outPath.replace('.js', '-es5.js'),
      es5bundleSource.code,
      es5bundleSource.map,
    );
  }

  if (stripTypes) {
    // Generate bundle without types.
    const strippedBundle = transformFromAstSync(
      transformedBundleAST,
      fileMapping,
      {
        ast: true,
        code: false,
        plugins: [require.resolve('@babel/plugin-transform-flow-strip-types')],
      },
    );
    const strippedSource = generate(
      strippedBundle.ast,
      {sourceMaps: true},
      fileMapping,
    );
    await writeBundle(
      outPath.replace('.js', '-stripped.js'),
      strippedSource.code,
      strippedSource.map,
    );
  }
}

main().catch((err: mixed) => {
  console.error(err);
  process.exitCode = 1;
});
