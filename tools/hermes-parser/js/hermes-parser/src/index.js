/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {Program as ESTreeProgram} from 'hermes-estree';
import type {ParserOptions} from './ParserOptions';

import * as HermesParser from './HermesParser';
import HermesToBabelAdapter from './HermesToBabelAdapter';
import HermesToESTreeAdapter from './HermesToESTreeAdapter';
import FlowVisitorKeys from './generated/ESTreeVisitorKeys';

const DEFAULTS = {
  flow: 'detect',
};

function getOptions(options?: ParserOptions = {...DEFAULTS}) {
  // Default to detecting whether to parse Flow syntax by the presence
  // of an @flow pragma.
  if (options.flow == null) {
    options.flow = DEFAULTS.flow;
  } else if (options.flow !== 'all' && options.flow !== 'detect') {
    throw new Error('flow option must be "all" or "detect"');
  }

  if (options.sourceType === 'unambiguous') {
    // Clear source type so that it will be detected from the contents of the file
    delete options.sourceType;
  } else if (
    options.sourceType != null &&
    options.sourceType !== 'script' &&
    options.sourceType !== 'module'
  ) {
    throw new Error(
      'sourceType option must be "script", "module", or "unambiguous" if set',
    );
  }

  options.tokens = options.tokens === true;
  options.allowReturnOutsideFunction =
    options.allowReturnOutsideFunction === true;

  return options;
}

function getAdapter(options: ParserOptions, code: string) {
  return options.babel === true
    ? new HermesToBabelAdapter(options)
    : new HermesToESTreeAdapter(options, code);
}

// $FlowExpectedError[unclear-type]
type BabelProgram = Object;
declare function parse(
  code: string,
  opts: {...ParserOptions, babel: true},
): BabelProgram;
// eslint-disable-next-line no-redeclare
declare function parse(
  code: string,
  opts?:
    | {...ParserOptions, babel?: false | void}
    | {...ParserOptions, babel: false},
): ESTreeProgram;

// eslint-disable-next-line no-redeclare
export function parse(
  code: string,
  opts?: ParserOptions,
): BabelProgram | ESTreeProgram {
  const options = getOptions(opts);
  const ast = HermesParser.parse(code, options);
  const adapter = getAdapter(options, code);

  return adapter.transform(ast);
}

export type {ParserOptions} from './ParserOptions';
export * from './traverse/SimpleTraverser';
export * from './transform/SimpleTransform';
export * from './traverse/getVisitorKeys';
export {FlowVisitorKeys};
export * as astArrayMutationHelpers from './transform/astArrayMutationHelpers';
export * as astNodeMutationHelpers from './transform/astNodeMutationHelpers';
export {default as mutateESTreeASTForPrettier} from './utils/mutateESTreeASTForPrettier';
