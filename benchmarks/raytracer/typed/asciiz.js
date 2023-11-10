/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 */

// Pointer access builtins.
const _ptr_write_char = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_write_char(ptr: c_ptr, offset: c_int, v: c_char): void {
});
const _ptr_read_uchar = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_read_uchar(ptr: c_ptr, offset: c_int): c_uchar {
    throw 0;
});

/// Allocate native memory using calloc() or throw an exception.
function calloc(size: number): c_ptr {
    "inline";
    "use unsafe";

    let res = _calloc(1, size);
    if (res === 0) throw Error("OOM");
    return res;
}

/// Allocate native memory using malloc() or throw an exception.
function malloc(size: number): c_ptr {
    "inline";
    "use unsafe";

    let res = _malloc(size);
    if (res === 0) throw Error("OOM");
    return res;
}

function copyToAsciiz(s: any, buf: c_ptr, size: number): void {
    if (s.length >= size) throw Error("String too long");
    let i = 0;
    for (let e = s.length; i < e; ++i) {
        let code: number = s.charCodeAt(i);
        if (code > 127) throw Error("String is not ASCII");
        _ptr_write_char(buf, i, code);
    }
    _ptr_write_char(buf, i, 0);
}

/// Convert a JS string to ASCIIZ.
function stringToAsciiz(s: any): c_ptr {
    "use unsafe";

    if (typeof s !== "string") s = String(s);
    let buf = malloc(s.length + 1);
    try {
        copyToAsciiz(s, buf, s.length + 1);
        return buf;
    } catch (e) {
        _free(buf);
        throw e;
    }
}

/// Convert a JS string to ASCIIZ.
function tmpAsciiz(s: any): c_ptr {
    "use unsafe";

    if (typeof s !== "string") s = String(s);
    let buf = allocTmp(s.length + 1);
    copyToAsciiz(s, buf, s.length + 1);
    return buf;
}


let _allocas: c_ptr[] = [];

function allocTmp(size: number): c_ptr {
    let res = calloc(size);
    _allocas.push(res);
    return res;
}

function flushAllocTmp(): void {
    for (let i = 0; i < _allocas.length; ++i) {
        _free(_allocas[i]);
    }
    let empty: c_ptr[] = [];
    _allocas = empty;
}
