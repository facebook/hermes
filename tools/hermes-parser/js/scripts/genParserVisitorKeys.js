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
  LITERAL_TYPES,
} from './utils/scriptUtils';
import tempCustomASTDefs from './utils/tempCustomASTDefs';

const ALLOWED_ARG_TYPES = new Set(['NodePtr', 'NodeList']);

const visitorKeys: {[string]: {[string]: 'Node' | 'NodeList'}} =
  // $FlowExpectedError[incompatible-type]
  Object.create(null);
for (const node of HermesESTreeJSON) {
  const nodeVisitorKeys = {};
  for (const arg of node.arguments) {
    switch (arg.type) {
      case 'NodeList':
        nodeVisitorKeys[arg.name] = 'NodeList';
        break;

      case 'NodePtr':
        nodeVisitorKeys[arg.name] = 'Node';
        break;

      default:
        break;
    }
  }
  visitorKeys[node.name] = nodeVisitorKeys;
}

// custom temp defs
for (let typeName of Object.keys(tempCustomASTDefs)) {
  const visitors = tempCustomASTDefs[typeName].visitor;
  visitorKeys[typeName] = {...visitors};
}

const visitorKeysFileContents = `\
export const NODE_CHILD = 'Node';
export const NODE_LIST_CHILD = 'NodeList';
export const HERMES_AST_VISITOR_KEYS = ${JSON.stringify(visitorKeys, null, 2)};
`;

formatAndWriteDistArtifact({
  code: visitorKeysFileContents,
  package: 'hermes-parser',
  filename: 'visitor-keys.js',
  subdirSegments: ['generated'],
});
