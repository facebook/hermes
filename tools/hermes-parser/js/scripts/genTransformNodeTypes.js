/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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
} from './utils/scriptUtils';

const imports: Array<string> = [];
const nodeTypeFunctions: Array<string> = [];

const NODES_WITH_SPECIAL_HANDLING = new Set([
  'ArrowFunctionExpression',
  'RegExpLiteral',
  'TemplateElement',
  'Identifier',
  'BooleanLiteral',
  'NumericLiteral',
  'NullLiteral',
  'StringLiteral',

  // TODO: BigIntLiteral is not supported by flow/hermes yet - so it has no function at all
  'BigIntLiteral',
  // a lot of additional properties are set on this, but nobody should ever "create" one so
  // we purposely don't define a creation function
  'Program',
]);

for (const node of HermesESTreeJSON) {
  if (NODES_WITH_SPECIAL_HANDLING.has(node.name)) {
    continue;
  }

  imports.push(node.name);

  const type = LITERAL_TYPES.has(node.name) ? 'Literal' : node.name;

  if (node.arguments.length === 0) {
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
    nodeTypeFunctions.push(
      `\
export function ${node.name}({parent, ...props}: {
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
