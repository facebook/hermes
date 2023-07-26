/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import type {Plugin} from 'prettier';
import type {Program} from 'hermes-estree';

export function getFlowPlugin(): Plugin<Program> {
  return require('./plugins/flow');
}

export function getESTreePlugin(): Plugin<Program> {
  return require('./plugins/estree');
}

export function getEmbeddedESTreePlugins(): $ReadOnlyArray<Plugin<>> {
  return [require('./plugins/graphql'), require('./plugins/postcss')];
}

type Doc = mixed;
export function printAstToDoc(program: Program, options: mixed): Doc {
  return require('./ast-to-doc').printAstToDoc(program, options);
}
