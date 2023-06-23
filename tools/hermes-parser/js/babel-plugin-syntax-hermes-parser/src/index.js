/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {ParserOptions} from 'hermes-parser';

import * as HermesParser from 'hermes-parser';

export default function BabelPluginSyntaxHermesParser(
  // $FlowExpectedError[unclear-type] We don't have types for this.
  api: any,
): $ReadOnly<{...}> {
  api.assertVersion(7);

  let curParserOpts: ParserOptions = {};

  return {
    name: 'syntax-hermes-parser',

    manipulateOptions(opts: $ReadOnly<{parserOpts: ParserOptions}>) {
      curParserOpts = opts.parserOpts;
    },

    // API suggested via https://babeljs.io/docs/babel-parser#will-the-babel-parser-support-a-plugin-system
    parserOverride(code: string) {
      const opts: ParserOptions = {};
      for (const [key, value] of Object.entries(curParserOpts)) {
        if (HermesParser.ParserOptionsKeys.has(key)) {
          // $FlowExpectedError[incompatible-type]
          opts[key] = value;
        }
      }
      return HermesParser.parse(code, {...opts, babel: true});
    },

    pre() {
      curParserOpts = {};
    },
  };
}
