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
  HermesESTreeJSON,
  formatAndWriteDistArtifact,
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
  'Identifier',
  'NullLiteral',
  'NumericLiteral',
  'RegExpLiteral',
  'StringLiteral',
  'TemplateElement',
]);

for (const node of HermesESTreeJSON) {
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
export function ${node.name}({
  parent,
}: {
  +parent?: ESNode,
} = {}): DetachedNode<${node.name}Type> {
  return detachedProps<${node.name}Type>(parent, {
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
        type = `DetachedNode<${type}>`;
      } else if (arg.type === 'NodeList') {
        type = `$ReadOnlyArray<DetachedNode<${type}[number]>>`;
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
export function ${node.name}({parent, ...props}: {
  ...$ReadOnly<${node.name}Props>,
  +parent?: ESNode,
}): DetachedNode<${node.name}Type> {
  const node = detachedProps<${node.name}Type>(parent, {
    type: '${type}',
    ...props,
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
import type {DetachedNode} from '../detachedNode';

import {
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

${nodePropTypes.join('\n')}

${nodeTypeFunctions.join('\n')}

export * from './special-case-node-types';
`;

formatAndWriteDistArtifact({
  code: fileContents,
  package: 'hermes-transform',
  filename: 'node-types.js',
  subdirSegments: ['generated'],
});
formatAndWriteDistArtifact({
  code: fileContents,
  package: 'hermes-transform',
  filename: 'node-types.js.flow',
  flow: 'strict-local',
  subdirSegments: ['generated'],
});
