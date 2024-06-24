/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// User
type User = {
    // name
    name: string,
};

type Map = {
    [string]: Value,
};

type Actor = {
    lastName: string,
    ...User,
}


type Data = {
    "hello": string,
    123: ?string,
}


type IPerson = {
    name(): string
}
