/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-ir -ferror-limit=100 2>&1 ) | %FileCheck --match-full-lines --implicit-check-not error: %s

function testModuleFactoryErrors() {
  $SHBuiltin.moduleFactory();
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.moduleFactory requires exactly two arguments.
  $SHBuiltin.moduleFactory(0);
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.moduleFactory requires exactly two arguments.
  $SHBuiltin.moduleFactory(0, 1, function () { return 7; });
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.moduleFactory requires exactly two arguments.
  $SHBuiltin.moduleFactory('str', function () { return 7; });
  // CHECK: {{.*}}:[[@LINE-1]]:28: error: $SHBuiltin.moduleFactory requires first arg to be unsigned int numeric literal.
  $SHBuiltin.moduleFactory(1.5, function () { return 7; });
  // CHECK: {{.*}}:[[@LINE-1]]:28: error: $SHBuiltin.moduleFactory requires first arg to be unsigned int numeric literal.
  $SHBuiltin.moduleFactory(-7, function () { return 7; });
  // CHECK: {{.*}}:[[@LINE-1]]:28: error: $SHBuiltin.moduleFactory requires first arg to be unsigned int numeric literal.
  $SHBuiltin.moduleFactory(100, function (global, require) { return 7; });
  // ^^^This one is legal.
}

function testModuleExportErrors() {
  $SHBuiltin.export();
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.export requires exactly two arguments.
  $SHBuiltin.export('x');
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.export requires exactly two arguments.
  $SHBuiltin.export(17, x);
  // CHECK: {{.*}}:[[@LINE-1]]:21: error: $SHBuiltin.export requires first argument to be a string literal.
  $SHBuiltin.export('x', 17);
  // CHECK: {{.*}}:[[@LINE-1]]:26: error: Export x is neither an identifier nor a call.

  // These are the legal forms.
  $SHBuiltin.export('x', x);
  $SHBuiltin.export('x', $SHBuiltin.import(100, 'y'));
}

function testModuleImportErrors() {
  $SHBuiltin.import();
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.import requires either two or three arguments.
  $SHBuiltin.import(400);
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.import requires either two or three arguments.
  $SHBuiltin.import(400, 'x', undefined, undefined);
  // CHECK: {{.*}}:[[@LINE-1]]:3: error: $SHBuiltin.import requires either two or three arguments.
  $SHBuiltin.import('400', 'x', x);
  // CHECK: {{.*}}:[[@LINE-1]]:21: error: $SHBuiltin.import requires first arg to be unsigned int numeric literal.
  $SHBuiltin.import(400.5, 'x', x);
  // CHECK: {{.*}}:[[@LINE-1]]:21: error: $SHBuiltin.import requires first arg to be unsigned int numeric literal.
  $SHBuiltin.import(-400, 'x', x);
  // CHECK: {{.*}}:[[@LINE-1]]:21: error: $SHBuiltin.import requires first arg to be unsigned int numeric literal.
  $SHBuiltin.import(400, 10, x);
  // CHECK: {{.*}}:[[@LINE-1]]:26: error: $SHBuiltin.import requires second argument to be a string literal.
  var x2 = $SHBuiltin.import(400, 'x', x);
  // ^^^This one is legal.
}
