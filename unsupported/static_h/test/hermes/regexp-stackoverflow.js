/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes %s | %FileCheck --match-full-lines %s

try {
  var r = /(?:(?:\\2|\\3*)*|(?=\\B)+|.(?:$)){68719476736}/g;
  var s = "";
  print(r.test(s));
} catch (e) { print('caught', e.message); }
// CHECK: caught Maximum regex stack depth reached
