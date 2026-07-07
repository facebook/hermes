/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import hermesPlugin from './index.generated.mjs';

const HERMES_AST_FORMAT = 'estree-hermes';

const hermesPrinter = hermesPlugin.printers[HERMES_AST_FORMAT];

const printers = {
  ...hermesPlugin.printers,
  [HERMES_AST_FORMAT]: {
    ...hermesPrinter,
    experimentalFeatures: {
      ...hermesPrinter.experimentalFeatures,
      avoidAstMutation: true,
    },
    features: {
      ...hermesPrinter.features,
      experimental_avoidAstMutation: true,
    },
  },
};

export const languages = hermesPlugin.languages;
export const options = hermesPlugin.options;
export const parsers = hermesPlugin.parsers;
export {printers};

export default {
  languages,
  options,
  parsers,
  printers,
};
