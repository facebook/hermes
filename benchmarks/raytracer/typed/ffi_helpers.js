/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 */

const _sh_ptr_read_c_char = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_char(ptr: c_ptr, offset: c_int): c_char { throw 0; });
const _sh_ptr_write_c_char = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_char(ptr: c_ptr, offset: c_int, v: c_char): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_schar = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_schar(ptr: c_ptr, offset: c_int): c_schar { throw 0; });
const _sh_ptr_write_c_schar = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_schar(ptr: c_ptr, offset: c_int, v: c_schar): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_uchar = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_uchar(ptr: c_ptr, offset: c_int): c_uchar { throw 0; });
const _sh_ptr_write_c_uchar = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_uchar(ptr: c_ptr, offset: c_int, v: c_uchar): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_short = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_short(ptr: c_ptr, offset: c_int): c_short { throw 0; });
const _sh_ptr_write_c_short = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_short(ptr: c_ptr, offset: c_int, v: c_short): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_ushort = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_ushort(ptr: c_ptr, offset: c_int): c_ushort { throw 0; });
const _sh_ptr_write_c_ushort = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_ushort(ptr: c_ptr, offset: c_int, v: c_ushort): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_int = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_int(ptr: c_ptr, offset: c_int): c_int { throw 0; });
const _sh_ptr_write_c_int = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_int(ptr: c_ptr, offset: c_int, v: c_int): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_uint = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_uint(ptr: c_ptr, offset: c_int): c_uint { throw 0; });
const _sh_ptr_write_c_uint = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_uint(ptr: c_ptr, offset: c_int, v: c_uint): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_long = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_long(ptr: c_ptr, offset: c_int): c_long { throw 0; });
const _sh_ptr_write_c_long = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_long(ptr: c_ptr, offset: c_int, v: c_long): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_ulong = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_ulong(ptr: c_ptr, offset: c_int): c_ulong { throw 0; });
const _sh_ptr_write_c_ulong = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_ulong(ptr: c_ptr, offset: c_int, v: c_ulong): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_longlong = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_longlong(ptr: c_ptr, offset: c_int): c_longlong { throw 0; });
const _sh_ptr_write_c_longlong = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_longlong(ptr: c_ptr, offset: c_int, v: c_longlong): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_ulonglong = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_ulonglong(ptr: c_ptr, offset: c_int): c_ulonglong { throw 0; });
const _sh_ptr_write_c_ulonglong = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_ulonglong(ptr: c_ptr, offset: c_int, v: c_ulonglong): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_bool = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_bool(ptr: c_ptr, offset: c_int): c_bool { throw 0; });
const _sh_ptr_write_c_bool = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_bool(ptr: c_ptr, offset: c_int, v: c_bool): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_float = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_float(ptr: c_ptr, offset: c_int): c_float { throw 0; });
const _sh_ptr_write_c_float = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_float(ptr: c_ptr, offset: c_int, v: c_float): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_double = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_double(ptr: c_ptr, offset: c_int): c_double { throw 0; });
const _sh_ptr_write_c_double = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_double(ptr: c_ptr, offset: c_int, v: c_double): void {include: "ffi_helpers.h"});
const _sh_ptr_read_c_ptr = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_read_c_ptr(ptr: c_ptr, offset: c_int): c_ptr { throw 0; });
const _sh_ptr_write_c_ptr = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_write_c_ptr(ptr: c_ptr, offset: c_int, v: c_ptr): void {include: "ffi_helpers.h"});
const _sh_ptr_add = $SHBuiltin.extern_c({include: "ffi_helpers.h"}, function _sh_ptr_add(ptr: c_ptr, offset: c_int): c_ptr { throw 0; });

const _malloc = $SHBuiltin.extern_c({include: "stdlib.h"}, function malloc(size: c_size_t): c_ptr {
    throw 0;
});
const _calloc = $SHBuiltin.extern_c({include: "stdlib.h"}, function calloc(count: c_size_t, size: c_size_t): c_ptr {
    throw 0;
});
const _free = $SHBuiltin.extern_c({include: "stdlib.h"}, function free(p: c_ptr): void {
});

const c_null = $SHBuiltin.c_null();
