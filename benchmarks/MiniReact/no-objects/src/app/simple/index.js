/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import * as React from 'react';
import App from './App';
import {drainMicrotaskQueue} from 'sh/microtask';

function printIf1(i: number, str: string): void {
  if (i === 1) {
    print('===============================');
    print(str);
    print('===============================');
  }
}

function run(N: number): void {
  for (let i: number = 1; i <= N; ++i) {
    const root = React.createRoot();
    const rootElement = <App />;
    printIf1(i, root.render(rootElement));

    React.callOnClickOrChange('toggle-modal', null);
    React.callOnClickOrChange('update-text', {
      target: {value: '!!!!! some text !!!!!'},
    });
    drainMicrotaskQueue();
    printIf1(i, root.render(rootElement));
  }
}

run(1);
