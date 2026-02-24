/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {SetState} from 'MiniReact';

import * as MiniReact from 'MiniReact';

import invariant from 'invariant';

test('it renders host element trees', () => {
  const root = MiniReact.createRoot();
  const rendered = root.render(
    <div>
      <h1>Hello</h1> world!
    </div>,
  );
  expect(rendered).toMatchSnapshot();
});

test('it renders custom components', () => {
  const root = MiniReact.createRoot();
  function Foo(props: $ReadOnly<{greeting: string}>) {
    return (
      <Bar>
        <div>
          <Baz greeting={props.greeting} />
        </div>
      </Bar>
    );
  }
  function Bar(
    props: $ReadOnly<{
      children: React$MixedElement | Array<React$MixedElement>,
    }>,
  ) {
    return <article>{props.children}</article>;
  }
  function Baz(props: $ReadOnly<{greeting: string}>) {
    return <h1>{props.greeting}</h1>;
  }
  const rendered = root.render(<Foo greeting="hello" />);
  expect(rendered).toMatchSnapshot();
});

test('it updates custom components', () => {
  const root = MiniReact.createRoot();
  let setToggle: SetState<boolean> | null = null;
  function Foo(props: $ReadOnly<{greeting: string}>) {
    const [toggle, _setToggle] = MiniReact.useState(true);
    setToggle = _setToggle;
    return toggle ? (
      <Bar greeting={props.greeting} />
    ) : (
      <Baz greeting={props.greeting} />
    );
  }
  function Bar(props: $ReadOnly<{greeting: string}>) {
    return <article>{props.greeting.toUpperCase()}</article>;
  }
  function Baz(props: $ReadOnly<{greeting: string}>) {
    return <h1>{props.greeting}</h1>;
  }
  const rendered = root.render(<Foo greeting="hello" />);
  expect(rendered).toMatchSnapshot();
  invariant(
    setToggle != null,
    'Expected setToggle to be initialized from initial render',
  );
  setToggle(false);
  const rendered2 = root.render(<Foo greeting="hello" />);
  expect(rendered2).toMatchSnapshot();
});
