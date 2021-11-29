/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {t, transform} from './test-utils';

function codemod(code: string) {
  return transform(code, context => ({
    CallExpression(node) {
      if (
        node.callee.type !== 'MemberExpression' ||
        node.callee.property.type !== 'Identifier' ||
        node.callee.property.name !== 'bind' ||
        node.callee.object.type !== 'FunctionExpression' ||
        node.arguments.length !== 1 ||
        node.arguments[0].type !== 'ThisExpression'
      ) {
        return;
      }

      const func = node.callee.object;

      if (func.generator) {
        // arrow functions cannot be generators
        return;
      }

      const funcVar = context.getDeclaredVariables(func);
      if (funcVar.length > 0 && funcVar[0].references.length > 0) {
        // this is a self referencing function expression:
        // ```
        // const x = function y() { y() }.bind(this);
        // ```
        // so we can't convert this into an arrow function
        return;
      }

      if (
        func.params.length > 0 &&
        func.params[0].type === 'Identifier' &&
        func.params[0].name === 'this'
      ) {
        // the user has manually typed the `this` context.
        // we don't know if their manual `this` type matches the implicit
        // `this` type - so we can't safely convert it
        return;
      }

      context.replaceNode(
        node,
        t.ArrowFunctionExpression({
          async: func.async,
          body: context.shallowCloneNode(func.body),
          params: context.shallowCloneArray(func.params),
          predicate: context.shallowCloneNode(func.predicate),
          typeParameters: context.shallowCloneNode(func.typeParameters),
          returnType: context.shallowCloneNode(func.returnType),
        }),
      );
    },
  }));
}

describe('React to react', () => {
  it('should transform valid cases correctly', () => {
    const result = codemod(`\
a = function y() {
  console.log(this.getMessage());
}.bind(this);

b = function y(arg1: string, arg2: number) {
  console.log(this.getMessage(), arg1, arg2);
}.bind(this);

c = function y<T>(arg1: T) {
  console.log(this.getMessage(), arg1, arg2);
}.bind(this);

d = function y(): void {}.bind(this);
`);

    expect(result).toBe(`\
a = () => {
  console.log(this.getMessage());
};

b = (arg1: string, arg2: number) => {
  console.log(this.getMessage(), arg1, arg2);
};

c = <T>(arg1: T) => {
  console.log(this.getMessage(), arg1, arg2);
};

d = (): void => {};
`);
  });

  it('should ignore invalid cases correctly', () => {
    const result = codemod(`\
a = function y() {
  y();
}.bind(this);

b = function *y() {
  yield true;
}.bind(this);

c = function y(this: Map<string, string>) {
  this.set('a', 'b');
}.bind(this);

d = function y(): void {}.call(this);
`);

    expect(result).toBe(`\
a = function y() {
  y();
}.bind(this);

b = function *y() {
  yield true;
}.bind(this);

c = function y(this: Map<string, string>) {
  this.set('a', 'b');
}.bind(this);

d = function y(): void {}.call(this);
`);
  });
});
