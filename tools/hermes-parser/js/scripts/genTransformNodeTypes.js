/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

import {
  GetHermesESTreeJSON,
  formatAndWriteSrcArtifact,
  LITERAL_TYPES,
  NODES_WITHOUT_TRANSFORM_NODE_TYPES,
} from './utils/scriptUtils';

const imports: Array<string> = [];
const nodeTypeFunctions: Array<string> = [];
const nodePropTypes: Array<string> = [];

// these nodes are listed in ./hermes-transform/src/generated/special-case-node-types.js
const NODES_WITH_SPECIAL_HANDLING = new Set([
  'ArrowFunctionExpression',
  'BigIntLiteral',
  'BooleanLiteral',
  'ClassDeclaration',
  'DeclareExportDeclaration',
  'DeclareFunction',
  'ExportNamedDeclaration',
  'Identifier',
  'NullLiteral',
  'NumericLiteral',
  'ObjectTypeProperty',
  'Program',
  'RegExpLiteral',
  'StringLiteral',
  'TemplateElement',
]);

for (const node of GetHermesESTreeJSON()) {
  if (
    NODES_WITH_SPECIAL_HANDLING.has(node.name) ||
    NODES_WITHOUT_TRANSFORM_NODE_TYPES.has(node.name)
  ) {
    continue;
  }

  imports.push(node.name);

  const type = LITERAL_TYPES.has(node.name) ? 'Literal' : node.name;

  if (node.arguments.length === 0) {
    nodePropTypes.push(`\
export type ${node.name}Props = {};
`);
    nodeTypeFunctions.push(
      `\
export function ${node.name}(props: {
  +parent?: ESNode,
} = {...null}): DetachedNode<${node.name}Type> {
  return detachedProps<${node.name}Type>(props.parent, {
    type: '${type}',
  });
}
`,
    );
  } else {
    nodePropTypes.push(
      `\
export type ${node.name}Props = {
  ${node.arguments
    .map(arg => {
      const baseType = `${node.name}Type['${arg.name}']`;
      let type = baseType;
      if (arg.type === 'NodePtr') {
        type = `MaybeDetachedNode<${type}>`;
      } else if (arg.type === 'NodeList') {
        type = `$ReadOnlyArray<MaybeDetachedNode<${type}[number]>>`;
      }

      if (arg.optional) {
        return `+${arg.name}?: ?${type}`;
      }
      return `+${arg.name}: ${type}`;
    })
    .join(',\n')},
};
`,
    );
    nodeTypeFunctions.push(
      `\
export function ${node.name}(props: {
  ...$ReadOnly<${node.name}Props>,
  +parent?: ESNode,
}): DetachedNode<${node.name}Type> {
  const node = detachedProps<${node.name}Type>(props.parent, {
    type: '${type}',
    ${node.arguments
      .map(arg => {
        switch (arg.type) {
          case 'NodePtr':
            return `${arg.name}: asDetachedNode(props.${arg.name})`;
          case 'NodeList':
            return `${arg.name}: props.${arg.name}${
              arg.optional ? '?.' : '.'
            }map(n => asDetachedNode(n))`;
          default:
            return `${arg.name}: props.${arg.name}`;
        }
      })
      .join(',\n')},
  });
  setParentPointersInDirectChildren(node);
  return node;
}
`,
    );
  }
}

const fileContents = `\
import type {
ESNode,
${imports.map(imp => `${imp} as ${imp}Type`).join(',\n')}
} from 'hermes-estree';
import type {DetachedNode, MaybeDetachedNode} from '../detachedNode';

import {
  asDetachedNode,
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

${nodePropTypes.join('\n')}

${nodeTypeFunctions.join('\n')}

export * from './special-case-node-types';
`;

formatAndWriteSrcArtifact({
  code: fileContents,
  package: 'hermes-transform',
  file: 'generated/node-types.js',
  flow: 'strict-local',
});
