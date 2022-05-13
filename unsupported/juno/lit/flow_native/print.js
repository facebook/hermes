/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

print("Hello world\n");
print(10 + 10);
print("\n");
print({});
print("\n");
print(function(){});
print("\n");
