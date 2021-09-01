/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

'use strict';

import * as HermesParser from './HermesParser';
import HermesToBabelAdapter from './HermesToBabelAdapter';
import HermesToESTreeAdapter from './HermesToESTreeAdapter';
import {transformFromAstSync} from './HermesTransformer';

type Options = {
  allowReturnOutsideFunction?: boolean,
  babel?: boolean,
  flow?: 'all' | 'detect',
  sourceFilename?: string,
  sourceType?: 'module' | 'script' | 'unambiguous',
  tokens?: boolean,
};

type Program = Object;

function getOptions(options: Options) {
  // Default to detecting whether to parse Flow syntax by the presence
  // of an @flow pragma.
  if (options.flow == null) {
    options.flow = 'detect';
  } else if (options.flow != 'all' && options.flow != 'detect') {
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
      'sourceType option must be "script", "module", or "unambiguous" if set'
    );
  }

  options.tokens = options.tokens === true;
  options.allowReturnOutsideFunction =
    options.allowReturnOutsideFunction === true;

  return options;
}

function getAdapter(options: Options, code: string) {
  return options.babel === true
    ? new HermesToBabelAdapter(options)
    : new HermesToESTreeAdapter(options, code);
}

export function parse(code: string, opts: Options = {}): Program {
  const options = getOptions(opts);
  const ast = HermesParser.parse(code, options);
  const adapter = getAdapter(options, code);

  return adapter.transform(ast);
}

export {transformFromAstSync} from './HermesTransformer';
