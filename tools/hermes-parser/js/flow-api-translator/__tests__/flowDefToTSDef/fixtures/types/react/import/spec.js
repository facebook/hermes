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
type T4 = React.ElementRef<typeof Component>; // React.ElementRef<typeof Component>
type T5 = React.AbstractComponent<Props>; // React.ForwardRefExoticComponent<Props>
type T6 = React.AbstractComponent<Props, HTMLElement>; // React.ForwardRefExoticComponent<Props & React.RefAttributes<HTMLElement>>
type T7 = React.Context<Foo>; // React.Context<Foo>
type T8 = React.ComponentType<Props>; // React.ComponentType<Props>
type T9 = React.Fragment; // React.Fragment
type T10 = React.ElementConfig<typeof Component>; // JSX.LibraryManagedAttributes<typeof Component, React.ComponentProps<typeof Component>>
type T11 = React.Ref<typeof Component>; // NonNullable<React.Ref<typeof Component> | string | number>
type T12 = React.Key; // React.Key
type T13 = React.Component<Props, {}>; // React.Component<Props, {}>
type T14 = React.ElementType; // React.ElementType
type T15 = React.ChildrenArray<T>; // T | ReadonlyArray<T>
type T15 = React.ElementProps<T>; // React.ComponentProps<T>

type Props = {A: string};
declare function Component(props: Props): React.Node;
