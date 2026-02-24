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

import {includes} from 'sh/fastarray';
import * as React from 'react';
import {Primitive} from '@radix-ui/react-primitive';

/* -------------------------------------------------------------------------------------------------
 *  Separator
 * -----------------------------------------------------------------------------------------------*/

const DEFAULT_ORIENTATION = 'horizontal';
const ORIENTATIONS: string[] = ['horizontal', 'vertical'];

type SeparatorProps = any;

// type Orientation = (typeof ORIENTATIONS)[number];
// type SeparatorElement = React.ElementRef<typeof Primitive.div>;
// type PrimitiveDivProps = Radix.ComponentPropsWithoutRef<typeof Primitive.div>;
// interface SeparatorProps extends PrimitiveDivProps {
//   /**
//    * Either `vertical` or `horizontal`. Defaults to `horizontal`.
//    */
//   orientation?: Orientation;
//   /**
//    * Whether or not the component is purely decorative. When true, accessibility-related attributes
//    * are updated so that that the rendered element is removed from the accessibility tree.
//    */
//   decorative?: boolean;
// }

const Separator = React.forwardRef(
  /*<SeparatorElement, SeparatorProps>*/ (
    props: Props,
    forwardedRef: any,
  ): React$MixedElement => {
    const {
      decorative,
      orientation: orientationProp = DEFAULT_ORIENTATION,
      ...domProps
    } = props;
    const orientation = isValidOrientation(orientationProp)
      ? orientationProp
      : DEFAULT_ORIENTATION;
    // `aria-orientation` defaults to `horizontal` so we only need it if `orientation` is vertical
    const ariaOrientation =
      orientation === 'vertical' ? orientation : undefined;
    const semanticProps = decorative
      ? {role: 'none'}
      : {'aria-orientation': ariaOrientation, role: 'separator'};

    return (
      <Primitive.div
        data-orientation={orientation}
        {...semanticProps}
        {...domProps}
        ref={forwardedRef}
      />
    );
  },
);

function isValidOrientation(orientation: string): boolean {
  return includes<string>(ORIENTATIONS, orientation);
}

const Root = Separator;

export {
  Separator,
  //
  Root,
};
export type {SeparatorProps};
