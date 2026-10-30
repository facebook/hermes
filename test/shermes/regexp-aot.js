/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -emit-c -o - %s | %FileCheck --check-prefixes=CHECK,NOOPT %s
// RUN: %shermes -O -emit-c -o - %s | %FileCheck --check-prefixes=CHECK,OPT %s
// RUN: %shermes -exec %s | %FileCheck --check-prefix=EXEC --match-full-lines %s

// SH compiles regexp literals ahead of time: each distinct literal gets one
// entry in s_regexp_table, and _sh_ljs_create_regexp takes its index.

function main() {
  function mk() { return /(?<y>\d{4})-(?<m>\d\d)/gy; }
  function same() { return /(?<y>\d{4})-(?<m>\d\d)/gy; }
  function uni() { return /é|\u{1F600}/u; }
  var prev = null;
  for (var i = 0; i < 3; ++i) {
    var r = i == 1 ? same() : mk();
    print(r === prev, r.flags, r.sticky, r.lastIndex);
    var m = r.exec("2024-10 rest");
    print(m.groups.y, m.groups.m, r.lastIndex);
    prev = r;
  }
  var u = uni();
  print(u.source, u.test("\u{1F600}"), u.test("é"), u.test("e"));
}
main();

// NOOPT: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 0);
// NOOPT: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 0);
// NOOPT: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 1);
// The optimizer merges mk and same, so -O emits one call for the two.
// OPT-DAG: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 0);
// OPT-DAG: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 1);
// CHECK: static unsigned char s_regexp_storage[{{[0-9]+}}] = {
// CHECK-NEXT: static const SHRegExpTableEntry s_regexp_table[] = {
// CHECK-NEXT:   { .offset = 0, .length = [[LEN:[0-9]+]] },
// CHECK-NEXT:   { .offset = [[LEN]], .length = {{[0-9]+}} },
// CHECK-NEXT: };
// CHECK: .regexp_storage = s_regexp_storage, .regexp_table = s_regexp_table, .num_regexps = 2,

// EXEC: false gy true 0
// EXEC-NEXT: 2024 10 7
// EXEC-NEXT: false gy true 0
// EXEC-NEXT: 2024 10 7
// EXEC-NEXT: false gy true 0
// EXEC-NEXT: 2024 10 7
// EXEC-NEXT: é|\u{1F600} true true false
