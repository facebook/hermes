/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno --gen-js -O %s | %FileCheck %s --match-full-lines

function foo(x, y) {
  return true ? x + 1 : y + 1
}

// CHECK-LABEL: function foo(x, y) {
// CHECK-NEXT:   return x + 1;
// CHECK-NEXT: }

function bar(x, y) {
  return false ? x + 1 : y + 1
}

// CHECK-LABEL: function bar(x, y) {
// CHECK-NEXT:   return y + 1;
// CHECK-NEXT: }

function if_false(x, y) {
  if (false) {
    return x;
  }
  return y;
}

// CHECK-LABEL: function if_false(x, y) {
// CHECK-NEXT:   return y;
// CHECK-NEXT: }

function if_true(x, y) {
  if (true) {
    return x;
  }
  return y;
}

// CHECK-LABEL: function if_true(x, y) {
// CHECK-NEXT:   {
// CHECK-NEXT:     return x;
// CHECK-NEXT:   }
// CHECK-NEXT:   return y;
// CHECK-NEXT: }

function if_false_else(x, y) {
  if (false) {
    return x;
  } else {
    return y;
  }
}

// CHECK-LABEL: function if_false_else(x, y) {
// CHECK-NEXT:   {
// CHECK-NEXT:     return y;
// CHECK-NEXT:   }
// CHECK-NEXT: }

function if_true_else(x, y) {
  if (true) {
    return x;
  } else {
    return y;
  }
}

// CHECK-LABEL: function if_true_else(x, y) {
// CHECK-NEXT:   {
// CHECK-NEXT:     return x;
// CHECK-NEXT:   }
// CHECK-NEXT: }

function for_if_false(x, y) {
  for (;;) if (false) {}
}

// CHECK-LABEL: function for_if_false(x, y) {
// CHECK-NEXT:   for(;;)
// CHECK-NEXT:     ;
// CHECK-NEXT: }
