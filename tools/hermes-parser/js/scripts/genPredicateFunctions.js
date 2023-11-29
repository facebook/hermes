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

const nodeTypesToImport: Array<string> = [];
const predicateFunctions: Array<string> = [];

const NODES_WITH_SPECIAL_HANDLING = new Set<string>([
  'Identifier',
  'JSXIdentifier',
  'JSXText',
]);

nodeTypesToImport.push('Identifier', 'JSXIdentifier', 'JSXText');
predicateFunctions.push(
  `
export function isIdentifier(node /*: ESNode | Token */) /*: node is (Identifier | MostTokens) */ {
  return node.type === 'Identifier';
}
  `,
  `
export function isJSXIdentifier(node /*: ESNode | Token */) /*: node is (JSXIdentifier | MostTokens) */ {
  return node.type === 'JSXIdentifier';
}
  `,
  `
export function isJSXText(node /*: ESNode | Token */) /*: node is (JSXText | MostTokens) */ {
  return node.type === 'JSXText';
}
  `,
);

const nodes = GetHermesESTreeJSON()
  .map(n => n.name)
  .concat('Literal');
for (const node of nodes) {
  if (
    NODES_WITH_SPECIAL_HANDLING.has(node) ||
    NODES_WITHOUT_TRANSFORM_NODE_TYPES.has(node) ||
    LITERAL_TYPES.has(node)
  ) {
    continue;
  }

  nodeTypesToImport.push(node);
  predicateFunctions.push(
    `
export function is${node}(node /*: ESNode | Token */) /*: node is ${node} */ {
  return node.type === '${node}';
}
    `,
  );
}

const COMMENTS = ['Line', 'Block'];
for (const comment of COMMENTS) {
  nodeTypesToImport.push(`${comment}Comment`);
  predicateFunctions.push(
    `
export function is${comment}Comment(node /*: ESNode | Token */) /*: node is (MostTokens | ${comment}Comment) */ {
  return node.type === '${comment}';
}
    `,
  );
}

// TODO(bradzacher) generate this list from the types maybe?
const TOKENS: $ReadOnlyArray<[string, string, string]> = [
  // Both BinaryOperator and UnaryOperator
  ['Minus', '-', 'Punctuator'],
  ['Plus', '+', 'Punctuator'],

  // UnaryOperator
  ['LogicalNot', '!', 'Punctuator'],
  ['UnaryNegation', '~', 'Punctuator'],
  ['TypeOf', 'typeof', 'Keyword'],
  ['Void', 'void', 'Keyword'],
  ['Delete', 'delete', 'Keyword'],

  // BinaryOperator
  ['LooseEqual', '==', 'Punctuator'],
  ['LooseNotEqual', '!=', 'Punctuator'],
  ['StrictEqual', '===', 'Punctuator'],
  ['StrictNotEqual', '!==', 'Punctuator'],
  ['LessThan', '<', 'Punctuator'],
  ['LessThanOrEqualTo', '<=', 'Punctuator'],
  ['GreaterThan', '>', 'Punctuator'],
  ['GreaterThanOrEqualTo', '>=', 'Punctuator'],
  ['BitwiseLeftShift', '<<', 'Punctuator'],
  ['BitwiseRightShift', '>>', 'Punctuator'],
  ['BitwiseUnsignedRightShift', '>>>', 'Punctuator'],
  ['Asterix', '*', 'Punctuator'],
  ['ForwardSlash', '/', 'Punctuator'],
  ['Percent', '%', 'Punctuator'],
  ['Exponentiation', '**', 'Punctuator'],
  ['BitwiseOR', '|', 'Punctuator'],
  ['BitwiseXOR', '^', 'Punctuator'],
  ['BitwiseAND', '&', 'Punctuator'],
  ['In', 'in', 'Keyword'],
  ['InstanceOf', 'instanceof', 'Keyword'],

  // LogicalOperator
  ['LogicalOR', '||', 'Punctuator'],
  ['LogicalAND', '&&', 'Punctuator'],
  ['NullishCoalesce', '??', 'Punctuator'],

  // AssignmentOperator
  ['Equal', '=', 'Punctuator'],
  ['PlusEqual', '+=', 'Punctuator'],
  ['MinusEqual', '-=', 'Punctuator'],
  ['MultiplyEqual', '*=', 'Punctuator'],
  ['DivideEqual', '/=', 'Punctuator'],
  ['RemainderEqual', '%=', 'Punctuator'],
  ['ExponentateEqual', '**=', 'Punctuator'],
  ['BitwiseLeftShiftEqual', '<<=', 'Punctuator'],
  ['BitwiseRightShiftEqual', '>>=', 'Punctuator'],
  ['BitwiseUnsignedRightShiftEqual', '>>>=', 'Punctuator'],
  ['BitwiseOREqual', '|=', 'Punctuator'],
  ['BitwiseXOREqual', '^=', 'Punctuator'],
  ['BitwiseANDEqual', '&=', 'Punctuator'],
  ['LogicalOREqual', '||=', 'Punctuator'],
  ['LogicalANDEqual', '&&=', 'Punctuator'],
  ['NullishCoalesceEqual', '??=', 'Punctuator'],

  // UpdateOperator
  ['Increment', '++', 'Punctuator'],
  ['Decrement', '--', 'Punctuator'],

  // Type Operators
  // These are duplicates of BinaryOperators, but here for convenience and clarity
  ['UnionType', '|', 'Punctuator'],
  ['IntersectionType', '&', 'Punctuator'],

  // Misc other keywords
  ['Break', 'break', 'Keyword'],
  ['Case', 'case', 'Keyword'],
  ['Catch', 'catch', 'Keyword'],
  ['Class', 'class', 'Keyword'],
  ['Const', 'const', 'Keyword'],
  ['Continue', 'continue', 'Keyword'],
  ['Debugger', 'debugger', 'Keyword'],
  ['Default', 'default', 'Keyword'],
  ['Do', 'do', 'Keyword'],
  ['Else', 'else', 'Keyword'],
  ['Enum', 'enum', 'Keyword'],
  ['Export', 'export', 'Keyword'],
  ['Extends', 'extends', 'Keyword'],
  ['Finally', 'finally', 'Keyword'],
  ['For', 'for', 'Keyword'],
  ['Function', 'function', 'Keyword'],
  ['If', 'if', 'Keyword'],
  ['Implements', 'implements', 'Keyword'],
  ['Import', 'import', 'Keyword'],
  ['Interface', 'interface', 'Keyword'],
  ['New', 'new', 'Keyword'],
  ['Return', 'return', 'Keyword'],
  ['Static', 'static', 'Keyword'],
  ['Super', 'super', 'Keyword'],
  ['Switch', 'switch', 'Keyword'],
  ['This', 'this', 'Keyword'],
  ['Throw', 'throw', 'Keyword'],
  ['Try', 'try', 'Keyword'],
  ['Var', 'var', 'Keyword'],
  ['While', 'while', 'Keyword'],
  ['With', 'with', 'Keyword'],
  ['Yield', 'yield', 'Keyword'],

  // these keywords aren't reported as Keyword tokens by the parser either
  // due to syntax ambiguity, or as they are flow-specific keywords
  ['As', 'as', 'Identifier'],
  ['Async', 'async', 'Identifier'],
  ['Await', 'await', 'Identifier'],
  ['Declare', 'declare', 'Identifier'],
  ['From', 'from', 'Identifier'],
  ['Get', 'get', 'Identifier'],
  ['Let', 'let', 'Identifier'],
  ['Module', 'module', 'Identifier'],
  ['Of', 'of', 'Identifier'],
  ['Set', 'set', 'Identifier'],
  ['Type', 'type', 'Identifier'],

  // Misc other punctuators
  ['Comma', ',', 'Punctuator'],
  ['Colon', ':', 'Punctuator'],
  ['Semicolon', ';', 'Punctuator'],
  ['Dot', '.', 'Punctuator'],
  ['DotDotDot', '...', 'Punctuator'],
  ['OptionalChain', '?.', 'Punctuator'],
  ['QuestionMark', '?', 'Punctuator'],
  // could call these "round brackets" for consistency, but I think more people call them parentheses
  ['OpeningParenthesis', '(', 'Punctuator'],
  ['ClosingParenthesis', ')', 'Punctuator'],
  // could call these "Braces", but I think more people call them curlies
  ['OpeningCurlyBracket', '{', 'Punctuator'],
  ['ClosingCurlyBracket', '}', 'Punctuator'],
  // could call these "Chevrons", but I think more people call them angle brackets
  ['OpeningAngleBracket', '<', 'Punctuator'],
  ['ClosingAngleBracket', '>', 'Punctuator'],
];

nodeTypesToImport.push('MostTokens');
for (const [name, token, type] of TOKENS) {
  if (type === 'Identifier') {
    // certain keywords may be reported as Identifier depending on the context
    predicateFunctions.push(
      `
export function is${name}Keyword(node /*: ESNode | Token */) /*: node is (Identifier | MostTokens) */ {
  return (
    (node.type === 'Identifier' && node.name === '${token}') ||
    (node.type === 'Keyword' && node.value === '${token}')
  );
}
      `,
    );
  } else {
    predicateFunctions.push(
      `
export function is${name}Token(node /*: ESNode | Token */) /*: node is MostTokens */ {
  return node.type === '${type}' && node.value === '${token}';
}
      `,
    );
  }
}

const fileContents = `\
/*::
import type {
  ESNode,
  Token,
${nodeTypesToImport.map(n => `  ${n}`).join(',\n')},
} from 'hermes-estree';
*/

${predicateFunctions.join('\n')}
`;

formatAndWriteSrcArtifact({
  code: fileContents,
  package: 'hermes-estree',
  file: 'generated/predicates.js',
  flow: 'strict-local',
  skipFormat: true,
});
