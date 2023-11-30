/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ScopeManager} from 'hermes-eslint';
import type {Program} from 'hermes-estree';

import {parse} from 'hermes-transform';
import {promises as fs} from 'fs';
import * as path from 'path';

export type ParseResult = {
  ast: Program,
  scopeManager: ScopeManager,
  code: string,
};

export async function parseFile(filePath: string): Promise<ParseResult> {
  const fileContents = await fs.readFile(filePath, 'utf8');
  return parse(fileContents);
}

export async function writeFile(
  filePath: string,
  contents: string,
): Promise<void> {
  await fs.mkdir(path.dirname(filePath), {recursive: true});
  await fs.writeFile(filePath, contents);
}
