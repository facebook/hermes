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

import type {MaybeDetachedNode} from '../detachedNode';
import type {Program} from 'hermes-estree';

import {mutateESTreeASTForPrettier} from 'hermes-parser';
import * as prettier from 'prettier';
import {mutateESTreeASTCommentsForPrettier} from './comments/comments';
import type {VisitorKeysType} from 'hermes-parser';

let cache = 1;
const cacheBase = Math.random();

export async function print(
  ast: MaybeDetachedNode<Program>,
  originalCode: string,
  prettierOptions: {...} = {},
  visitorKeys?: ?VisitorKeysType,
): Promise<string> {
  // $FlowExpectedError[incompatible-type] This is now safe to access.
  const program: Program = ast;

  // If the AST body is empty, we can skip the cost of prettier by returning a static string of the contents.
  if (program.body.length === 0) {
    // If the program had a docblock comment, we need to create the string manually.
    const docblockComment = program.docblock?.comment;
    if (docblockComment != null) {
      return '/*' + docblockComment.value + '*/\n';
    }

    return '';
  }

  // Cleanup the comments from the AST and generate the "orginal" code needed for prettier.
  const codeForPrinting = mutateESTreeASTCommentsForPrettier(
    program,
    originalCode,
  );

  // Fix up the AST to match what prettier expects.
  mutateESTreeASTForPrettier(program, visitorKeys);

  switch (getPrettierMajorVersion()) {
    case '3': {
      // Lazy require this module as it only exists in prettier v3.
      const prettierFlowPlugin = require('prettier/plugins/flow');
      return prettier.format(
        codeForPrinting,
        // $FlowExpectedError[incompatible-exact] - we don't want to create a dependency on the prettier types
        {
          ...prettierOptions,
          parser: 'flow',
          requirePragma: false,
          plugins: [
            {
              parsers: {
                flow: {
                  ...prettierFlowPlugin.parsers.flow,
                  parse() {
                    return program;
                  },
                },
              },
            },
          ],
        },
      );
    }
    case '2': {
      const hermesPlugin = require('prettier-plugin-hermes-parser');
      const hermesParser = hermesPlugin.parsers?.hermes;
      if (hermesParser == null) {
        throw new Error('Hermes parser plugin not found');
      }

      return prettier.format(
        codeForPrinting,
        // $FlowExpectedError[incompatible-exact] - we don't want to create a dependency on the prettier types
        {
          ...prettierOptions,
          parser: 'hermes',
          requirePragma: false,
          plugins: [
            // $FlowExpectedError[incompatible-call] Cache value is not expected but needed in this case.
            {
              parsers: {
                hermes: {
                  ...hermesParser,

                  // Prettier caches the plugin, by making this key always unique we ensure the new `parse`
                  // function with the correct AST is always called.
                  cache: `${cacheBase}-${cache++}`,

                  // Provide the passed AST to prettier
                  parse() {
                    return program;
                  },
                },
              },
              printers: hermesPlugin.printers,
            },
          ],
        },
      );
    }
    case 'UNSUPPORTED':
    default: {
      throw new Error(
        `Unknown or unsupported prettier version of "${prettier.version}". Only major versions 3 or 2 of prettier are supported.`,
      );
    }
  }
}

function getPrettierMajorVersion(): '3' | '2' | 'UNSUPPORTED' {
  const {version} = prettier;

  if (version.startsWith('3.')) {
    return '3';
  }

  if (version.startsWith('2.')) {
    return '2';
  }

  return 'UNSUPPORTED';
}
