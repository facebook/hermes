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

import type {Plugin, AstPath, PrettierPrinterOptions} from 'prettier';
import type {Program} from 'hermes-estree';

type HermesPlugin = Plugin<Program>;

// copied from https://github.com/prettier/prettier/blob/20ab6d6f1c5bd774621230b493a3b71d39383a2c/src/language-js/parse/utils/replace-hashbang.js
function replaceHashbang(text: string): string {
  if (text.charAt(0) === '#' && text.charAt(1) === '!') {
    return '//' + text.slice(2);
  }

  return text;
}

function parse(originalText: string): Program {
  const {
    parse: hermesParserParse,
    mutateESTreeASTForPrettier,
  } = require('hermes-parser');

  const textToParse = replaceHashbang(originalText);

  const result = hermesParserParse(textToParse, {
    allowReturnOutsideFunction: true,
    enableExperimentalComponentSyntax: true,
    flow: 'all',
    sourceType: 'module',
    tokens: true,
  });

  mutateESTreeASTForPrettier(result);

  return result;
}

/**
 * Create a prettier plugin config for prettier v3 which adds
 * hermes parser support.
 */
function createPrettierV3HermesPlugin(): HermesPlugin {
  return {
    // $FlowExpectedError[unsafe-getters-setters]
    get parsers() {
      // Lazy require this module as its only available in prettier v3.
      const flowPlugin = require('prettier/plugins/flow');
      return {
        hermes: {
          ...flowPlugin.parsers.flow,
          parse,
        },
      };
    },
  };
}

/**
 * Create a prettier plugin config for prettier v2 which adds
 * hermes parser support and patches in prettier v3 printing logic.
 */
function createPrettierV2HermesPlugin(): HermesPlugin {
  // Lazy require custom prettier v3 internals
  const {
    getFlowPlugin,
    getESTreePlugin,
    getEmbeddedESTreePlugins,
    printAstToDoc,
  } = require('./third-party/internal-prettier-v3');

  return {
    // $FlowExpectedError[unsafe-getters-setters]
    get parsers() {
      return {
        hermes: {
          // $FlowExpectedError[incompatible-use] We know it exists
          ...getFlowPlugin().parsers.flow,

          // Switch to our own estree printer
          astFormat: 'estree-v3',

          parse(originalText: string): Program {
            const ast = parse(originalText);

            // We don't want prettier v2 to try attaching these comments
            // so we hide them under a different name.
            if (ast.comments != null) {
              // $FlowExpectedError[prop-missing]
              ast.__comments = ast.comments;
              // $FlowExpectedError[cannot-write]
              delete ast.comments;
            }

            return ast;
          },
        },
      };
    },

    // $FlowExpectedError[unsafe-getters-setters]
    get printers() {
      const estreePlugin = getESTreePlugin();
      return {
        'estree-v3': {
          // $FlowExpectedError[incompatible-use] We know it exists
          ...estreePlugin.printers.estree,

          // Skip prettier v2 trying to print comments for the top level node.
          willPrintOwnComments() {
            return true;
          },

          // Skip prettier v2 trying to handle prettier ignore comments.
          hasPrettierIgnore() {
            return false;
          },

          // Override printer and instead call the prettier v3 printing logic.
          print(
            path: AstPath<Program>,
            options: PrettierPrinterOptions<Program>,
          ) {
            // Get top level AST node from prettier v2.
            const ast: Program = path.getValue();

            // Reattach comments to AST
            // $FlowExpectedError[prop-missing]
            if (ast.__comments != null) {
              // $FlowExpectedError[cannot-write]
              ast.comments = ast.__comments;
              // $FlowExpectedError[prop-missing]
              delete ast.__comments;
            }

            // Collect existing V2 plugins that have not changed in V3.
            const existingPlugins = options.plugins.filter(plugin => {
              if (typeof plugin === 'string') {
                return false;
              }
              const printers = plugin.printers;
              if (printers == null) {
                return false;
              }
              return Object.keys(printers).some(
                printer =>
                  // Markdown plugin
                  printer === 'mdast' ||
                  // HTML plugin
                  printer === 'html',
              );
            });

            // Override prettier v2 options for prettier v3.
            const v3Options = {
              ...options,
              // Update plugins list to use prettier v3 plugins when processing
              // embedded syntax found by the prettier v3 estree printer.
              plugins: [...existingPlugins, ...getEmbeddedESTreePlugins()],

              printer: {
                ...options.printer,

                print: estreePlugin.printers?.estree.print,

                // Rename options overridden for prettier v2 above.
                willPrintOwnComments:
                  estreePlugin.printers?.estree.willPrintOwnComments,
                hasPrettierIgnore:
                  estreePlugin.printers?.estree.hasPrettierIgnore,
              },
            };

            // Call prettier v3 printing logic. This will recursively print the AST.
            return printAstToDoc(ast, v3Options);
          },
        },
      };
    },
  };
}

function getPrettierFlowPlugin(): HermesPlugin {
  const {version} = require('prettier');

  if (version.startsWith('3.')) {
    return createPrettierV3HermesPlugin();
  }

  if (version.startsWith('2.')) {
    return createPrettierV2HermesPlugin();
  }

  throw new Error(`Unsupported prettier version: ${version}`);
}

module.exports = (getPrettierFlowPlugin(): HermesPlugin);
