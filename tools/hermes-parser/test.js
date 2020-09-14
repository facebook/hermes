/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const parser = require('./HermesParser.js');
const assert = require('assert');

// Successfully parsed
let ast = parser.parse(`const x = 1`);
assert.strictEqual(ast.body[0].type, 'VariableDeclaration');
assert.strictEqual(ast.body[0].kind, 'const');
assert.strictEqual(ast.body[0].declarations[0].id.name, 'x');
assert.strictEqual(ast.body[0].declarations[0].init.value, 1);

// Parse error
assert.throws(
  () => parser.parse(`const = 1`),
  new Error(
    `1:6: 'identifier' expected in declaration
const = 1
~~~~~~^
`,
  ),
);

// Parse error does not include caret line if non-ASCII characters are used
assert.throws(
  () => parser.parse(`/*\u0176*/ const = 1`),
  new Error(
    `1:13: 'identifier' expected in declaration
/*\u0176*/ const = 1
`,
  ),
);

// Comments parsed
ast = parser.parse(`/*comment*/ 1;`);
assert.strictEqual(ast.comments.length, 1);
assert.strictEqual(ast.comments[0].type, 'Block');
assert.strictEqual(ast.comments[0].value, 'comment');

console.log('Tests passed!');
