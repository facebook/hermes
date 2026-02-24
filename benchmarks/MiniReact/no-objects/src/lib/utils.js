/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

// TODO switch from legacy function when SH supports rest args.
export function cn(...rest) {
  return rest.join(' ');
}
