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

/// Extract the visitor object from the given plugin.
function getPluginVisitor(plugin) {
  // TODO: Provide a way to handle plugins which are functions and need 'types'.
  return plugin.visitor;
}

/// Run each of the plugins on the source and return the resultant AST.
function transformFromAstSync(sourceAst, source, {plugins}) {
  const visitors = [];
  const passes = [];

  for (const plugin of plugins) {
    visitors.push(getPluginVisitor(plugin));
  }

  // Run all passes at once.
  const visitor = traverse.visitors.merge(visitors, passes);
  traverse(sourceAst, visitor);

  return {ast: sourceAst};
}

module.exports = {transformFromAstSync};
