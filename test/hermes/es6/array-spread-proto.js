/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Ensure that overriding slots in the array prototype doesn't break spread,
// because array spreading uses DefineOwnProperty.
Object.defineProperty(Array.prototype, 0, {});

print([..."asd"])
// CHECK: a,s,d

print([...['a', 's', 'd']])
// CHECK: a,s,d
