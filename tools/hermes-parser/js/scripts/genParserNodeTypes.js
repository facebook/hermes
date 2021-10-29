/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import {
  HermesESTreeJSON,
  formatAndWriteDistArtifact,
  FLIPPED_ALIAS_KEYS,
} from './scriptUtils';
import tempCustomASTDefs from '../hermes-parser/src/types/definitions/tempCustomASTDefs';

const nodeTypeFunctions: Array<string> = [];
for (const node of HermesESTreeJSON) {
  if (node.arguments.length === 0) {
    nodeTypeFunctions.push(
      `\
export function ${node.name}() {
  return {
    type: ${node.name},
    loc: null,
  };
}
`,
    );
  } else {
    const argNames = node.arguments.map(arg => arg.name);
    nodeTypeFunctions.push(
      `\
export function ${node.name}(
  ${argNames
    .map(
      name =>
        // we need to prefix them incase it is a reserved word
        `arg_${name}`,
    )
    .join(', ')}
) {
  return {
    type: '${node.name}',
    loc: null,
    ${argNames.map(name => `${name}: arg_${name}`).join(',\n')}
  };
}
  `,
    );
  }
}
// custom temp node type builders defs
for (let typeName of Object.keys(tempCustomASTDefs)) {
  const builders = tempCustomASTDefs[typeName].builder;
  nodeTypeFunctions.push(
    `

export function ${typeName}(${builders.map(name => `arg_${name}`).join(', ')}) {
  return {
    type: '${typeName}',
    loc: null,
    ${builders.map(name => `${name}: arg_${name}`).join(',\n')}
  };
}
export function is${typeName}(node, opts) {
  if (!node) return false;

  const nodeType = node.type;
  if (nodeType === '${typeName}') {
    if (typeof opts === "undefined") {
      return true;
    } else {
      return shallowEqual(node, opts);
    }
  }

  return false;
}`,
  );
}

const validatorFunctions: Array<string> = [];
for (const node of HermesESTreeJSON) {
  validatorFunctions.push(
    `\
export function is${node.name}(node, opts) {
  if (!node) return false;

  if (node.type === '${node.name}') {
    if (typeof opts === "undefined") {
      return true;
    } else {
      return shallowEqual(node, opts);
    }
  }

  return false;
}
`,
  );
}
// alias type validators
for (let aliasKey of Object.keys(FLIPPED_ALIAS_KEYS)) {
  const aliasSet = Array.from(FLIPPED_ALIAS_KEYS[aliasKey])
    .map(k => `'${k}'`)
    .join(', ');
  validatorFunctions.push(
    `\
const ${aliasKey}_ALIAS_SET = new Set([${aliasSet}]);
export function is${aliasKey}(node, opts) {
  if (!node) return false;

  const nodeType = node.type;
  if (${aliasKey}_ALIAS_SET.has(node.type)) {
    if (typeof opts === "undefined") {
      return true;
    } else {
      return shallowEqual(node, opts);
    }
  }

  return false;
}
`,
  );
}

const fileContents = `\
${nodeTypeFunctions.join('\n')}

${validatorFunctions.join('\n')}
`;

formatAndWriteDistArtifact({
  code: fileContents,
  package: 'hermes-parser',
  filename: 'node-types.js',
  subdirSegments: ['types', 'generated'],
});
