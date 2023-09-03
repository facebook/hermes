/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

"use strict";

// Emulate a module scope, since global scope is unsound.
(function (exports) {
    const c_null = $SHBuiltin.c_null();

// stdio.h
    const _getenv = $SHBuiltin.extern_c({include: "stdlib.h"}, function getenv(name: c_ptr): c_ptr {
        throw 0;
    });

// stdlib.h
    const _malloc = $SHBuiltin.extern_c({include: "stdlib.h"}, function malloc(size: c_size_t): c_ptr {
        throw 0;
    });
    const _free = $SHBuiltin.extern_c({include: "stdlib.h"}, function free(p: c_ptr): void {
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

    function getenv(name: string): string {
        "use unsafe";

        let name_z = stringToAsciiz(name);
        try {
            let val_z = _getenv(name_z);
            return asciizToString_unsafe(val_z, 2048);
        } finally {
            _free(name_z);
        }
    }

    let path = getenv("PATH");
    print(path);


// Optionally force some methods to be emitted for debugging.
// exports.foo = foo;
})({});
