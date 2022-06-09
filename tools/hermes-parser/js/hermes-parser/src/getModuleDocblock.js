/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {HermesNode} from './HermesAST';
import type {DocblockDirectives, Program} from 'hermes-estree';

const DIRECTIVE_REGEX = /^\s*@([a-zA-Z0-9_-]+)( +.+)?$/;

export function parseDocblockString(docblock: string): DocblockDirectives {
  const directiveLines = docblock
    .split('\n')
    // remove the leading " *" from each line
    .map(line => line.trimStart().replace(/^\* ?/, '').trim())
    .filter(line => line.startsWith('@'));

  const directives: {
    [string]: Array<string>,
  } = {};

  for (const line of directiveLines) {
    const match = DIRECTIVE_REGEX.exec(line);
    if (match == null) {
      continue;
    }
    const name = match[1];
    // explicitly use an empty string if there's no value
    // this way the array length tracks how many instances of the directive there was
    const value = (match[2] ?? '').trim();
    if (directives[name]) {
      directives[name].push(value);
    } else {
      directives[name] = [value];
    }
  }

  return directives;
}

export function getModuleDocblock(
  hermesProgram: HermesNode,
): Program['docblock'] {
  const docblockNode = (() => {
    if (hermesProgram.type !== 'Program') {
      return null;
    }

    // $FlowExpectedError[incompatible-type] - escape out of the unsafe hermes types
    const program: Program = hermesProgram;

    if (program.comments.length === 0) {
      return null;
    }

    const firstComment = program.comments[0];
    if (firstComment.type !== 'Block') {
      return null;
    }

    /*
    Handle cases like this where the comment isn't actually the first thing in the code:
    ```
    const x = 1; /* docblock *./
    ```
    */
    if (
      program.body.length > 0 &&
      program.body[0].range[0] < firstComment.range[0]
    ) {
      return null;
    }

    return firstComment;
  })();

  if (docblockNode == null) {
    return null;
  }

  return {
    directives: parseDocblockString(docblockNode.value),
    comment: docblockNode,
  };
}
