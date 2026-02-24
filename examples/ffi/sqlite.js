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
    const _sh_ptr_read_ptr = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_read_ptr(buf: c_ptr, offset: c_int): c_ptr {
        throw 0;
    });

// SQLite3.
    const _sqlite3_open = $SHBuiltin.extern_c({}, function sqlite3_open(filename: c_ptr, ppDb: c_ptr): c_int {
        throw 0;
    });
    const _sqlite3_prepare_v2 = $SHBuiltin.extern_c({}, function sqlite3_prepare_v2(db: c_ptr, zSql: c_ptr, nByte: c_int, ppStmt: c_ptr, pzTail: c_ptr): c_int {
        throw 0;
    });
    const _sqlite3_step = $SHBuiltin.extern_c({}, function sqlite3_step(pStmt: c_ptr): c_int {
        throw 0;
    });
    const _sqlite3_column_int = $SHBuiltin.extern_c({}, function sqlite3_column_int(pStmt: c_ptr, iCol: c_int): c_int {
        throw 0;
    });
    const _sqlite3_column_text = $SHBuiltin.extern_c({}, function sqlite3_column_text(pStmt: c_ptr, iCol: c_int): c_ptr {
        throw 0;
    });
    const _sqlite3_finalize = $SHBuiltin.extern_c({}, function sqlite3_finalize(pStmt: c_ptr): c_int {
        throw 0;
    });
    const _sqlite3_close = $SHBuiltin.extern_c({}, function sqlite3_close(db: c_ptr): c_int {
        throw 0;
    });

    const SQLITE_OK = 0;
    const SQLITE_ROW = 100;

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

    function main() {
        let dbPtrBuffer: c_ptr = malloc(8);
        let stmt: c_ptr = c_null
        let db: c_ptr = c_null
        let sqlPtr: c_ptr = c_null;
        let res = 0;

        try {
            // Open the database
            res = _sqlite3_open(stringToAsciiz("my_database.db"), dbPtrBuffer);
            if (res !== SQLITE_OK)
                throw Error(res);
            db = _sh_ptr_read_ptr(dbPtrBuffer, 0);

            // Prepare the SQL statement
            sqlPtr = stringToAsciiz("SELECT id, name FROM my_table ORDER BY id");
            res = _sqlite3_prepare_v2(db, sqlPtr, -1, dbPtrBuffer, c_null);
            if (res !== SQLITE_OK)
                throw Error(res);
            stmt = _sh_ptr_read_ptr(dbPtrBuffer, 0);

            // Loop through all rows in the table
            let row = 1;
            while (_sqlite3_step(stmt) === SQLITE_ROW) {
                const id = _sqlite3_column_int(stmt, 0);
                const namePtr = _sqlite3_column_text(stmt, 1);
                const name = asciizToString_unsafe(namePtr, 1024);
                print(`row${row++}:`, id, name)
            }
        } finally {
            // Finalize the statement and close the database
            if (stmt) _sqlite3_finalize(stmt);
            if (db) _sqlite3_close(db);

            // Free the allocated memory
            if (dbPtrBuffer) _free(dbPtrBuffer);
            if (sqlPtr) _free(sqlPtr);
        }
    }

    main();

// Optionally force some methods to be emitted for debugging.
// exports.foo = foo;
})({});
