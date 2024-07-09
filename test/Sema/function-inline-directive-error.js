/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Restore the run line to this form once D58649571 has landed (and eliminated a
// stray warning.
// RAN: (%shermes %s -ferror-limit=100 2>&1 ) | %FileCheck --match-full-lines --implicit-check-not "{{error|warning}}:" %s
// RUN: (%shermes %s -ferror-limit=100 2>&1 ) | %FileCheck --match-full-lines --implicit-check-not "error:" %s

function doubleDirective() {
    'inline'
    'noinline'
    //CHECK: {{.*}}:[[@LINE-1]]:5: warning: Should not declare both 'inline' and 'noinline'.
    return 7;
}

function doubleDirectiveReverseOrder() {
    'noinline'
    'inline'
    //CHECK: {{.*}}:[[@LINE-1]]:5: warning: Should not declare both 'inline' and 'noinline'.
    return 7;
}
