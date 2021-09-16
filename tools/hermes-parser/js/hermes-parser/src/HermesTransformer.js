/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

import traverse from './traverse';
import * as t from './types';

// Extract the visitor object from the given plugin.
function getPluginVisitorAndOptions(ast, code, pluginItem) {
  let plugin = pluginItem;
  const pluginOptions = {
    file: {ast, code},
    opts: {},
  };

  if (Array.isArray(pluginItem)) {
    plugin = pluginItem[0];
    pluginOptions.opts = pluginItem[1];
  }

  if (typeof plugin === 'function') {
    return [plugin({types: t}).visitor, pluginOptions];
  }
  return [plugin.visitor, pluginOptions];
}

// Run each of the plugins on the source and return the resultant AST.
export function transformFromAstSync(sourceAst, source, {plugins}) {
  const visitors = [];
  const visitorOptions = [];

  for (const plugin of plugins) {
    const [vistor, options] = getPluginVisitorAndOptions(
      sourceAst,
      source,
      plugin,
    );
    visitors.push(vistor);
    visitorOptions.push(options);
  }

  // Run all passes at once.
  const visitor = traverse.visitors.merge(visitors, visitorOptions);
  traverse(sourceAst, visitor);

  return {ast: sourceAst};
}
