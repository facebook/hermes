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

type T2 = React.Node; // React.ReactNode
type T1 = React.MixedElement; // JSX.Element
type T2 = React.Element<typeof Component>; // React.ReactElement<typeof Component>

type Props = {A: string};
declare function Component(props: Props): React.Node;
