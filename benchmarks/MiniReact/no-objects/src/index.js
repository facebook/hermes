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
import App from './App';

function run(): void {
  var N = 1;
  for (var i = 0; i < N; ++i) {
    var root = React.createRoot();
    var rendered = root.render(<App />);
  }
  print(rendered);
}

run();
