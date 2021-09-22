/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

import traverse, {NodePath, Scope} from './traverse';
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
export function transformFromAstSync(ast, source, {plugins}) {
  if (ast.type !== 'File') {
    throw new Error('AST root must be a Program or File node');
  }

  const visitors = [];
  const visitorOptions = [];

  for (const plugin of plugins) {
    const [visitor, options] = getPluginVisitorAndOptions(ast, source, plugin);
    visitors.push(visitor);
    visitorOptions.push(options);
  }

  // Run all passes at once.
  const visitor = traverse.visitors.merge(visitors, visitorOptions);

  let scope = Scope;
  const hub = {
    getCode: () => source,
    getScope: () => scope,
    // TODO: Remove this code path
    addHelper: () => {
      throw new Error('Helpers are not supported.');
    },
    // TODO: Intergrate buildCodeFrameError (See: https://fburl.com/6czq4tll)
    buildError(node, msg, Error = TypeError): Error {
      return new Error(msg);
    },
  };

  const rootPath = NodePath.get({
    hub: hub,
    parentPath: null,
    parent: ast,
    container: ast,
    key: 'program',
  }).setContext();
  scope = rootPath.scope;

  traverse(ast, visitor, scope, {}, rootPath);

  return {ast};
}
