/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

/*
Unfortunately flow does not strictly enforce generic constraints for a type like
type T = <T>(target: T, nodeToReplaceWith: T) => void;

When called, it will set T to the union of argument types instead of enforcing they
are the same type.
So in order to have a strict type guarantee that both arguments are the same type -
we need codegen!
*/

import {
  HermesESTreeJSON,
  formatAndWriteDistArtifact,
  LITERAL_TYPES,
} from './utils/scriptUtils';

const imports: Array<string> = [];
const replaceSignatures: Array<string> = [];

const nodes = HermesESTreeJSON.concat({name: 'Literal', arguments: []});
const signatureNames: Array<string> = [];
for (const node of nodes) {
  if (LITERAL_TYPES.has(node.name)) {
    continue;
  }

  imports.push(node.name);
  const signatureName = `${node.name}ReplaceSignature`;
  signatureNames.push(signatureName);

  replaceSignatures.push(
    `type ${signatureName} = (
      target: ${node.name},
      nodeToReplaceWith: DetachedNode<${node.name}>,
      options?: $ReadOnly<{keepComments?: boolean}>,
    ) => void`,
  );
}

const fileContents = `\
import type {
${imports.join(',\n')}
} from 'hermes-estree';
import type {DetachedNode} from '../detachedNode';

${replaceSignatures.join(';\n')};
export type TransformReplaceSignatures = ${signatureNames.join(' & ')};
`;

formatAndWriteDistArtifact({
  code: fileContents,
  package: 'hermes-transform',
  filename: 'TransformReplaceSignatures.js.flow',
  subdirSegments: ['generated'],
  flow: 'strict',
});
