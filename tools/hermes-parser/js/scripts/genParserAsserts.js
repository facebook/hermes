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

import {HermesESTreeJSON, formatAndWriteDistArtifact} from './scriptUtils';
import aliasDefs from '../hermes-parser/src/types/definitions/aliases';

const assertionFunctions: Array<string> = [];

for (const node of HermesESTreeJSON) {
  assertionFunctions.push(
    `\
export function assert${node.name}(node, opts) {
  if (
    node &&
    node.type === '${node.name}' &&
    (typeof opts === "undefined" ||
    shallowEqual(node, opts))
  ) {
    return;
  }

  throw new Error(throwMessage(${node.name}, node, opts));
}
`,
  );
}

/**
 * Generate alias node types
 */
// $FlowExpectedError[incompatible-type]
const FLIPPED_ALIAS_KEYS: {[string]: Set<string>} = Object.create(null);
for (const typeName of Object.keys(aliasDefs)) {
  for (const aliasName of aliasDefs[typeName]) {
    if (FLIPPED_ALIAS_KEYS[aliasName]) {
      FLIPPED_ALIAS_KEYS[aliasName].add(typeName);
    } else {
      FLIPPED_ALIAS_KEYS[aliasName] = new Set([typeName]);
    }
  }
}
for (const aliasKey of Object.keys(FLIPPED_ALIAS_KEYS)) {
  const aliasSet = Array.from(FLIPPED_ALIAS_KEYS[aliasKey])
    .map(k => `'${k}'`)
    .join(', ');
  assertionFunctions.push(
    `\
const ${aliasKey}_ALIAS_SET = new Set([${aliasSet}]);
export function assert${aliasKey}(node, opts) {
  if (
    node &&
    ${aliasKey}_ALIAS_SET.has(node.type) &&
    (typeof opts === "undefined" ||
    shallowEqual(node, opts))
  ) {
    return;
  }

  throw new Error(throwMessage("${aliasKey}", node, opts));
}
`,
  );
}

const fileContents = `\
import shallowEqual from '../utils/shallowEqual';

function throwMessage(type, node, opts = {}) {
  return \`Expected type "\${type}" with option \${JSON.stringify(opts)}, but instead got "\${node.type}".\`;
}

${assertionFunctions.join('\n')}
`;

formatAndWriteDistArtifact({
  code: fileContents,
  package: 'hermes-parser',
  filename: 'asserts.js',
  subdirSegments: ['types', 'generated'],
});
