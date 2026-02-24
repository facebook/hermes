/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -emit-c %s

"use strict";

// Emulate a module scope, since global scope is unsound.
(function (exports) {
    const c_null = $SHBuiltin.c_null();

// stdio.h
    const _fopen = $SHBuiltin.extern_c({include: "stdio.h"}, function fopen(path: c_ptr, mode: c_ptr): c_ptr {
        throw 0;
    });
    const _fclose = $SHBuiltin.extern_c({include: "stdio.h"}, function fclose(f: c_ptr): void {
    });
    const _fread = $SHBuiltin.extern_c({include: "stdio.h"}, function fread(ptr: c_ptr, size: c_size_t, nitems: c_size_t, stream: c_ptr): c_size_t {
        throw 0;
    });

// stdlib.h
    const _malloc = $SHBuiltin.extern_c({include: "stdlib.h"}, function malloc(size: c_size_t): c_ptr {
        throw 0;
    });
    const _free = $SHBuiltin.extern_c({include: "stdlib.h"}, function free(p: c_ptr): void {
    });

// string.h
    const _strerror_r = $SHBuiltin.extern_c({include: "string.h"}, function strerror_r(errnum: c_int, errbuf: c_ptr, buflen: c_size_t): c_int {
        throw 0;
    });

// Builtin provided by SH to wrap errno.
    const _sh_errno = $SHBuiltin.extern_c({declared: true}, function _sh_errno(): c_int {
        throw 0;
    });

// Pointer access builtins.
    const _ptr_write_char = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_write_char(ptr: c_ptr, offset: c_int, v: c_char): void {
    });
    const _ptr_read_uchar = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_read_uchar(ptr: c_ptr, offset: c_int): c_uchar {
        throw 0;
    });

    /// Allocate native memory using malloc() or throw an exception.
    function malloc(size: number): c_ptr {
        "inline";
        "use unsafe";

        let res = _malloc(size);
        if (res === 0) throw Error("OOM");
        return res;
    }

/// Convert a JS string to ASCIIZ.
    function stringToAsciiz(s: any): c_ptr {
        "use unsafe";

        if (typeof s !== "string") s = String(s);
        let buf = malloc(s.length + 1);
        try {
            let i = 0;
            for (let e = s.length; i < e; ++i) {
                let code: number = s.charCodeAt(i);
                if (code > 127) throw Error("String is not ASCII");
                _ptr_write_char(buf, i, code);
            }
            _ptr_write_char(buf, i, 0);
            return buf;
        } catch (e) {
            _free(buf);
            throw e;
        }
    }

/// Convert an ASCII string of certain size to a JS string.
    function asciiToString_unsafe(buf: c_ptr, size: number): string {
        let res = "";
        for (let i = 0; i < size; ++i) {
            let ch = _ptr_read_uchar(buf, i);
            if (ch > 127) throw Error("String is not ASCII");
            res += String.fromCharCode(ch);
        }
        return res;
    }

/// Convert an ASCIIZ string up to a maximum size to a JS string.
    function asciizToString_unsafe(buf: c_ptr, maxsize: number): string {
        let res = "";
        for (let i = 0; i < maxsize; ++i) {
            let ch = _ptr_read_uchar(buf, i);
            if (ch > 127) throw Error("String is not ASCII");
            if (ch === 0) break;
            res += String.fromCharCode(ch);
        }
        return res;
    }

    function strerror(errnum: number): string {
        "use unsafe";

        let errbuf = malloc(1024);
        try {
            _strerror_r(errnum, errbuf, 1024);
            return asciizToString_unsafe(errbuf, 1024);
        } finally {
            _free(errbuf);
        }
    }

    /// Very simple hack to ensure safety.
    let handles: c_ptr[] = [];
    // FIXME: fast array doesn't support .pop() yet.
    let closedHandles = Array();

    function fopen(path: string, mode: string): number {
        "use unsafe";

        let pathz: c_ptr = c_null;
        let modez: c_ptr = c_null;
        try {
            pathz = stringToAsciiz(path);
            modez = stringToAsciiz(mode);
            let filePtr = _fopen(pathz, modez);
            if (!filePtr) {
                let errnum = _sh_errno();
                throw Error(path + ": " + strerror(errnum));
            }
            // Allocate a handle.
            if (closedHandles.length > 0) {
                let f = closedHandles.pop();
                handles[f] = filePtr;
                return f;
            }
            handles.push(filePtr);
            return handles.length - 1;
        } finally {
            _free(pathz);
            _free(modez);
        }
    }

    function fclose(f: number): void {
        "use unsafe";

        if (f < 0 || f >= handles.length) throw Error("invalid file handle");
        if (handles[f]) {
            _fclose(handles[f]);
            handles[f] = c_null;
            closedHandles.push(f);
        }
    }

    function fread(size: number, f: number): string {
        "use unsafe";

        if (f < 0 || f >= handles.length) throw Error("invalid file handle");
        if (!handles[f]) throw Error("file is closed");

        if (size <= 0) throw Error("invalid size");
        let buf = malloc(size);
        try {
            let nr = _fread(buf, 1, size, handles[f]);
            return asciiToString_unsafe(buf, nr);
        } finally {
            _free(buf);
        }
    }

    function freadAll(f: number): string {
        let res = "";
        for (; ;) {
            let s = fread(1024, f);
            if (!s) break;
            res += s;
        }
        return res;
    }

    let f = fopen("file.txt", "r");
    try {
        print("====");
        print(freadAll(f));
        print("====");
    } finally {
        fclose(f);
    }

// Optionally force some methods to be emitted for debugging.
//exports.stringToAsciiz = stringToAsciiz;
//exports.asciiToString_unsafe = asciiToString_unsafe;
//exports.fopen = fopen;
//exports.fclose_unsafe = fclose_unsafe;
//exports.fread_unsafe = fread_unsafe;
//exports.freadAll_unsafe = freadAll_unsafe;
//exports.File = File;
})({});
