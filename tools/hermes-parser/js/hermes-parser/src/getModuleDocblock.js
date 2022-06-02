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
import type {Program} from 'hermes-estree';

const commentStartRe = /^\/\*\*?/;
const commentEndRe = /\*+\/$/;
const wsRe = /[\t ]+/g;
const stringStartRe = /(\r?\n|^) *\*/g;
const multilineRe =
  /(?:^|\r?\n) *(@[^\r\n]*?) *\r?\n *([^@\r\n\s][^@\r\n]+?) *\r?\n/g;
const propertyRe = /(?:^|\r?\n) *@(\S+) *([^\r\n]*)/g;

/**
 * Returns an object of `{[prop]: value}`
 * If a property appers more than once the last one will be returned
 * unless coalesceRepeatedValues is true, in which case it will return an array
 */
function parse(docblockIn: string) {
  let docblock = docblockIn
    .replace(commentStartRe, '')
    .replace(commentEndRe, '')
    .replace(wsRe, ' ')
    .replace(stringStartRe, '$1');

  // Normalize multi-line directives
  let prev = '';
  while (prev !== docblock) {
    prev = docblock;
    docblock = docblock.replace(multilineRe, '\n$1 $2\n');
  }
  docblock = docblock.trim();

  const pairs = [];
  let match;
  while ((match = propertyRe.exec(docblock))) {
    pairs.push([match[1], match[2] ?? '']);
  }

  const result: {
    [string]: Array<string>,
  } = {};
  for (const [k, v] of pairs) {
    if (result[k]) {
      result[k].push(v);
    } else {
      result[k] = [v];
    }
  }
  return result;
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
    directives: parse(docblockNode.value),
    comment: docblockNode,
  };
}
