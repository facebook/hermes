/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s

// Regression test: a scope containing both a `using` declaration and a
// block-scoped function declaration used to abort in
// ScopedFunctionPromoter::extractDeclaredIdents(), which asserted that a
// variable declaration's kind is var/let/const. The promoter runs before
// the resolver rejects `using`, so `using` declarations reach it. Now the
// promoter treats them as lexically scoped (like `const`) and the explicit
// rejection is reached cleanly.

using x = 1;
{
  function f() {}
}
// CHECK: {{.*}}using-scoped-fn-promotion.js:[[@LINE-4]]:1: error: using declarations are not yet supported

async function g() {
  await using y = 1;
  {
    function h() {}
  }
}
// CHECK: {{.*}}using-scoped-fn-promotion.js:[[@LINE-5]]:3: error: using declarations are not yet supported
