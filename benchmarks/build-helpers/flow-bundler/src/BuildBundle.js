/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ModuleOverride} from './ModuleGraph';

import * as path from 'path';
import {createModuleGraph} from './ModuleGraph';
import {parseFile, hermesASTToBabel, writeBundle} from './utils';
import transformJSX from './transforms/transformJSX';
import {analyze} from 'hermes-eslint/dist/scope-manager/analyze';

// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import generate from '@babel/generator';
// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import {transformFromAstSync} from '@babel/core';

export type BundleOutOptions = {
  babelConfig?: ?{
    presets?: ?Array<string | [string, {...}]>,
    plugins?: ?Array<string | [string, {...}]>,
  },
};

export type BundleOptions = {
  entrypoints: Array<string>,
  root: string,
  outDir: string,
  simpleJsxTransform?: ?boolean,
  out: {[string]: ?BundleOutOptions},
  moduleOverrides?: ?Array<ModuleOverride>,
};

export async function buildBundles(
  bundleOptions: BundleOptions,
): Promise<void> {
  const rootPath = path.resolve(bundleOptions.root);
  const outPathDir = path.resolve(rootPath, bundleOptions.outDir);
  const simpleJSXTransform = bundleOptions.simpleJsxTransform === true;
  const entrypoints: Array<string> = bundleOptions.entrypoints.map(f =>
    path.resolve(rootPath, f),
  );
  const moduleOverrides: Array<ModuleOverride> = Array.isArray(
    bundleOptions.moduleOverrides,
  )
    ? bundleOptions.moduleOverrides.map(({target, override}) => ({
        target: path.resolve(rootPath, target),
        override: path.resolve(rootPath, override),
      }))
    : [];

  if (Object.keys(bundleOptions.out).length === 0) {
    throw new Error('No "output" specified');
  }

  const bundle = await createModuleGraph(
    rootPath,
    entrypoints,
    moduleOverrides,
  );
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

  for (const [outFilename, bundleOutOptions] of Object.entries(
    bundleOptions.out,
  )) {
    if (outFilename.includes('/')) {
      throw new Error(`Invalid output filename "${outFilename}"`);
    }

    const outPath = path.join(outPathDir, outFilename);
    const babelConfig = bundleOutOptions?.babelConfig;
    const babelPlugins =
      babelConfig != null &&
      Array.isArray(babelConfig.plugins) &&
      babelConfig.plugins.length > 0
        ? babelConfig.plugins
        : null;
    const babelPresets =
      babelConfig != null &&
      Array.isArray(babelConfig.presets) &&
      babelConfig.presets.length > 0
        ? babelConfig.presets
        : null;

    let transformedBundleAST = bundleAST;
    if (babelPlugins != null || babelPresets != null) {
      const transformedBundle = transformFromAstSync(bundleAST, fileMapping, {
        ast: true,
        code: false,
        filename: outPath,
        plugins: babelPlugins,
        presets: babelPresets,
      });
      transformedBundleAST = transformedBundle.ast;
    }

    // Generate bundle
    const bundleSource = generate(
      transformedBundleAST,
      {sourceMaps: true},
      fileMapping,
    );
    await writeBundle(outPath, bundleSource.code, bundleSource.map);
  }
}
