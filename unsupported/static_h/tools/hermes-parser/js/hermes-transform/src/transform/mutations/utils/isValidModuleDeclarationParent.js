/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ESNode, ModuleDeclaration, Statement} from 'hermes-estree';
import type {DetachedNode} from '../../../detachedNode';

function isModuleDeclaration(node: ESNode): boolean %checks {
  return (
    node.type === 'ImportDeclaration' ||
    node.type === 'ExportNamedDeclaration' ||
    node.type === 'ExportDefaultDeclaration' ||
    node.type === 'ExportAllDeclaration'
  );
}

export function isValidModuleDeclarationParent(
  target: ESNode,
  nodesToInsertOrReplace: $ReadOnlyArray<
    DetachedNode<ModuleDeclaration | Statement>,
  >,
): boolean {
  if (
    target.type === 'Program' ||
    (target.type === 'BlockStatement' && target.parent.type === 'DeclareModule')
  ) {
    return true;
  }

  for (const node of nodesToInsertOrReplace) {
    if (
      !isModuleDeclaration(
        // $FlowExpectedError[incompatible-cast]
        (node: ESNode),
      )
    ) {
      continue;
    }

    return false;
  }

  return true;
}
