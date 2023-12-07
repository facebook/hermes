/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Props, React$MixedElement} from './React';

import * as React from './React';

function Title(props: Props): React$MixedElement {
  return <h1>{props.children}</h1>;
}

function MyComponent(_props: Props): React$MixedElement {
  const [str, setStr] = React.useState<string>('');

  return (
    <div>
      <Title>Hello</Title> world!
      {str}
    </div>
  );
}

function run(): void {
  var N = 1;
  for (var i = 0; i < N; ++i) {
    var root = React.createRoot();
    var rendered = root.render(<MyComponent />);
  }
  print(rendered);
}

run();
