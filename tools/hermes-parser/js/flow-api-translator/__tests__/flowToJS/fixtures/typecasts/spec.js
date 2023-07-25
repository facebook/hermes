/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

const xxx = 0;
const yyy = '';

(xxx: number);
({xxx: 0, yyy: 'hey'}: {xxx: number, yyy: string});
(xxx => xxx + 1: (xxx: number) => number);
(xxx: number), (yyy: string);
((xxx: number): number);
