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

import type {Program} from 'hermes-estree';

import {parse, mutateESTreeASTForPrettier} from 'hermes-parser';

import {parsers as flowPluginParsers} from 'prettier/plugins/flow';

// copied from https://github.com/prettier/prettier/blob/20ab6d6f1c5bd774621230b493a3b71d39383a2c/src/language-js/parse/utils/replace-hashbang.js
function replaceHashbang(text: string): string {
  if (text.charAt(0) === '#' && text.charAt(1) === '!') {
    return '//' + text.slice(2);
  }

  return text;
}

export const parsers = {
  hermes: {
    ...flowPluginParsers.flow,
    parse(originalText: string): Program {
      const textToParse = replaceHashbang(originalText);

      const result = parse(textToParse, {
        allowReturnOutsideFunction: true,
        enableExperimentalComponentSyntax: true,
        flow: 'all',
        sourceType: 'module',
        tokens: true,
      });

      mutateESTreeASTForPrettier(result);

      return result;
    },
  },
};
