/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

export function isScopable(node: ?Object, opts?: Object): boolean {
  if (!node) return false;

  const nodeType = node.type;
  if (
    nodeType === "Scopable" ||
    "BlockStatement" === nodeType ||
    "CatchClause" === nodeType ||
    "DoWhileStatement" === nodeType ||
    "ForInStatement" === nodeType ||
    "ForStatement" === nodeType ||
    "FunctionDeclaration" === nodeType ||
    "FunctionExpression" === nodeType ||
    "Program" === nodeType ||
    "ObjectMethod" === nodeType ||
    "SwitchStatement" === nodeType ||
    "WhileStatement" === nodeType ||
    "ArrowFunctionExpression" === nodeType ||
    "ClassExpression" === nodeType ||
    "ClassDeclaration" === nodeType ||
    "ForOfStatement" === nodeType ||
    "ClassMethod" === nodeType ||
    "ClassPrivateMethod" === nodeType ||
    "StaticBlock" === nodeType ||
    "TSModuleBlock" === nodeType ||
    (nodeType === "Placeholder" && "BlockStatement" === node.expectedNode)
  ) {
    if (typeof opts === "undefined") {
      return true;
    } else {
      return shallowEqual(node, opts);
    }
  }

  return false;
}
