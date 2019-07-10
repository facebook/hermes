// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// Verify that hermes and hermesc produce the same output.
// RUN: diff <(%hermes -target=HBC -dump-bytecode %s) <(%hermesc -target=HBC -dump-bytecode %s)
print(42);
