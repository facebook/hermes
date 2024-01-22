/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Props, React$MixedElement} from 'react';

import {Button} from '@/registry/new-york/ui/button';
import {useState} from 'react';

export default function MusicPage(props: Props): React$MixedElement {
  const [toggle, setToggle] = useState<boolean>(true);
  return (
    <>
      <Button id="click-me" onClick={() => setToggle(!toggle)}>
        Click me: {String(toggle)}
      </Button>
      <span>Other</span>
    </>
  );
}
