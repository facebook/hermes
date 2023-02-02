/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-bytecode -O %s | %FileCheck --check-prefixes ANON,CHECK %s
// RUN: %hermesc -fno-inline -dump-bytecode -O %s -Xgen-names-anon-functions | %FileCheck --check-prefixes SYNTH,CHECK %s

(() => () => print(10))()()()
(function() { print(20); })()

// CHECK: Function<global>(

// ANON: NCFunction<>(
// ANON: NCFunction<>(
// ANON: Function<>(

// SYNTH: NCFunction<[[OUTER:\?anon_[0-9]+_anonFunc@global]]>(
// SYNTH: NCFunction<?anon_{{[0-9]+}}_anonFunc@[[OUTER]]>(
// SYNTH: Function<?anon_{{[0-9]+}}_anonFunc@global>(
