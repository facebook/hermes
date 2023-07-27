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
import {
  addCommentsToNode,
  getLeadingCommentsForNode,
} from './comments/comments';
import type {VisitorKeysType} from 'hermes-parser';

let cache = 1;

export async function print(
  ast: MaybeDetachedNode<Program>,
  originalCode: string,
  prettierOptions: {...} = {},
  visitorKeys?: ?VisitorKeysType,
): Promise<string> {
  // $FlowExpectedError[incompatible-type] This is now safe to access.
  const program: Program = ast;

  // The docblock comment is never attached to any AST nodes, since its technically
  // attached to the program. However this is specific to our AST and in order for
  // prettier to correctly print it we need to attach it to the first node in the
  // program body.
  if (program.docblock != null && program.body.length > 0) {
    const firstNode = program.body[0];
    const docblockComment = program.docblock.comment;
    const leadingComments = getLeadingCommentsForNode(firstNode);
    if (!leadingComments.includes(docblockComment)) {
      addCommentsToNode(firstNode, [docblockComment], 'leading');
    }
  }

  // Fix up the AST to match what prettier expects.
  mutateESTreeASTForPrettier(program, visitorKeys);

  // we need to delete the comments prop or else prettier will do
  // its own attachment pass after the mutation and duplicate the
  // comments on each node, borking the output
  // $FlowExpectedError[cannot-write]
  delete program.comments;

  switch (getPrettierMajorVersion()) {
    case '3': {
      // Lazy require this module as it only exists in prettier v3.
      const prettierFlowPlugin = require('prettier/plugins/flow');
      return prettier.format(
        originalCode,
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
        originalCode,
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
                  cache: cache++,

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
