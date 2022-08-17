/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

(class Foo {
  'constructor'() {}
  constructor() {}
});
// CHECK: {{.*}}:12:3: error: duplicate constructors in class
// CHECK:   constructor() {}
// CHECK:   ^~~~~~~~~~~~~~~~
// CHECK: {{.*}}:11:3: note: first constructor definition
// CHECK:   'constructor'() {}
// CHECK:   ^~~~~~~~~~~~~~~~~~

(class Foo {
  constructor() {
    super`asdf`;
  }
});
// CHECK: {{.*}}:23:10: error: '(', '[' or '.' expected after 'super' keyword
// CHECK:     super`asdf`;
// CHECK:     ~~~~~^
