/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import CHECKED_CAST from 'sh/CHECKED_CAST';
import {fastArrayJoin} from 'sh/fastarray';

export function cva(
  base: string[] | string,
  variants: mixed,
): (opts: mixed) => string {
  const baseString: string =
    typeof base === 'string'
      ? CHECKED_CAST<string>(base)
      : fastArrayJoin(CHECKED_CAST<string[]>(base), ' ');
  return (opts: mixed): string => baseString;
}
