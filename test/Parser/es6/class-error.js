// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

(class Foo {
  'constructor'() {}
  constructor() {}
});
// CHECK: {{.*}}:10:3: error: duplicate constructors in class
// CHECK:   constructor() {}
// CHECK:   ^~~~~~~~~~~~~~~~
// CHECK: {{.*}}:9:3: note: first constructor definition
// CHECK:   'constructor'() {}
// CHECK:   ^~~~~~~~~~~~~~~~~~

(class Foo {
  constructor() {
    super`asdf`;
  }
});
// CHECK: {{.*}}:21:5: error: invalid use of 'super' as a template literal tag
// CHECK:     super`asdf`;
// CHECK:     ^~~~~
