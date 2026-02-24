/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%shermes %s -dump-sema -ferror-limit=100 2>&1 ) | %FileCheck --match-full-lines --implicit-check-not "{{error|warning}}:" %s

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
