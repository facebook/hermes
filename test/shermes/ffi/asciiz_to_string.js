/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

const _malloc = $SHBuiltin.extern_c({include: "stdlib.h"},
    function malloc(size: c_size_t): c_ptr {
        throw 0;
    });
const _free = $SHBuiltin.extern_c({include: "stdlib.h"},
    function free(p: c_ptr): void {
    });
const _ptr_write_char = $SHBuiltin.extern_c({declared: true},
    function _sh_ptr_write_char(ptr: c_ptr, offset: c_int, v: c_char): void {
    });
const _sh_asciiz_to_string = $SHBuiltin.extern_c({declared: true, hv: true},
    function _sh_asciiz_to_string(runtime: c_ptr, str: c_ptr): string {
        throw 0;
    }
);

let buf = _malloc(10);
_ptr_write_char(buf, 0, 65);
_ptr_write_char(buf, 1, 98);
_ptr_write_char(buf, 2, 51);
_ptr_write_char(buf, 3, 0);
let str = _sh_asciiz_to_string($SHBuiltin.c_native_runtime(), buf);
_free(buf);
print(str);
// CHECK: Ab3
