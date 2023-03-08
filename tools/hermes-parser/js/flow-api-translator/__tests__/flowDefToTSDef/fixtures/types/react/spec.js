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

type T1 = React.Node; // React.ReactNode
type T2 = React.MixedElement; // JSX.Element
type T3 = React.Element<typeof Component>; // React.ReactElement<typeof Component>
type T4 = React.AbstractComponent<Props>; // React.ForwardRefExoticComponent<Props>
type T4 = React.AbstractComponent<Props, HTMLElement>; // React.ForwardRefExoticComponent<Props & React.RefAttributes<HTMLElement>>

type Props = {A: string};
declare function Component(props: Props): React.Node;
