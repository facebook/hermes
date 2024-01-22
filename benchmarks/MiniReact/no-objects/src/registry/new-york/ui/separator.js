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

import * as React from 'react';
import * as SeparatorPrimitive from '@radix-ui/react-separator';

import {cn} from '@/lib/utils';

const Separator = React.forwardRef(
  /*<
  React.ElementRef<typeof SeparatorPrimitive.Root>,
  React.ComponentPropsWithoutRef<typeof SeparatorPrimitive.Root>,
>*/ (
    {className, orientation = 'horizontal', decorative = true, ...props}: Props,
    ref: any,
  ): React$MixedElement => (
    <SeparatorPrimitive.Root
      ref={ref}
      decorative={decorative}
      orientation={orientation}
      className={cn(
        'shrink-0 bg-border',
        orientation === 'horizontal' ? 'h-[1px] w-full' : 'h-full w-[1px]',
        className,
      )}
      {...props}
    />
  ),
);

export {Separator};
