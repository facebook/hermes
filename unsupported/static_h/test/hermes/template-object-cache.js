/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

var previousObj = null;
function checkTemplateObject(templateObj) {
  print(templateObj instanceof Object, templateObj == previousObj);
  previousObj = templateObj;
}

checkTemplateObject``;
// CHECK: true false
checkTemplateObject`hello${1 + 1}world`;
// CHECK: true false
checkTemplateObject`hello${1 + 2}world`;
// CHECK: true true
checkTemplateObject`hello${1 + 1}world`;
// CHECK: true true
checkTemplateObject`hellowo${6}rld`;
// CHECK: true false
checkTemplateObject`hello\n${2}`;
// CHECK: true false
checkTemplateObject`hello\n${2}`;
// CHECK: true true
checkTemplateObject`hello\n${1}`;
// CHECK: true true
