/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const c_null = $SHBuiltin.c_null();

// stdlib.h
const _malloc = $SHBuiltin.extern_c(
  { include: "stdlib.h" },
  function malloc(size: c_size_t): c_ptr {
    throw 0;
  }
);
const _free = $SHBuiltin.extern_c(
  { include: "stdlib.h" },
  function free(p: c_ptr): void {}
);

// Pointer access builtins.
const _ptr_write_char = $SHBuiltin.extern_c(
  { declared: true },
  function _sh_ptr_write_char(ptr: c_ptr, offset: c_int, v: c_char): void {}
);
const _ptr_read_uchar = $SHBuiltin.extern_c(
  { declared: true },
  function _sh_ptr_read_uchar(ptr: c_ptr, offset: c_int): c_uchar {
    throw 0;
  }
);
const _sh_ptr_read_ptr = $SHBuiltin.extern_c(
  { declared: true },
  function _sh_ptr_read_ptr(buf: c_ptr, offset: c_int): c_ptr {
    throw 0;
  }
);
const _sh_asciiz_to_string = $SHBuiltin.extern_c(
  { declared: true, hv: true },
  function _sh_asciiz_to_string(runtime: c_ptr, str: c_ptr, len: c_ptrdiff_t): string { throw 0; }
);


// SQLite3.
const _sqlite3_open = $SHBuiltin.extern_c(
  {},
  function sqlite3_open(filename: c_ptr, ppDb: c_ptr): c_int {
    throw 0;
  }
);
const _sqlite3_prepare_v2 = $SHBuiltin.extern_c(
  {},
  function sqlite3_prepare_v2(
    db: c_ptr,
    zSql: c_ptr,
    nByte: c_int,
    ppStmt: c_ptr,
    pzTail: c_ptr
  ): c_int {
    throw 0;
  }
);
const _sqlite3_step = $SHBuiltin.extern_c(
  {},
  function sqlite3_step(pStmt: c_ptr): c_int {
    throw 0;
  }
);
const _sqlite3_column_int = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_int(pStmt: c_ptr, iCol: c_int): c_int {
    throw 0;
  }
);
const _sqlite3_column_float = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_double(pStmt: c_ptr, iCol: c_int): c_float {
    throw 0;
  }
);
const _sqlite3_column_text = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_text(pStmt: c_ptr, iCol: c_int): c_ptr {
    throw 0;
  }
);
const _sqlite3_column_bytes = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_bytes(pStmt: c_ptr, iCol: c_int): c_int {
    throw 0;
  }
);
const _sqlite3_column_type = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_type(pStmt: c_ptr, iCol: c_int): c_int {
    throw 0;
  }
);
const _sqlite3_column_name = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_name(pStmt: c_ptr, iCol: c_int): c_ptr {
    throw 0;
  }
);
const _sqlite3_column_count = $SHBuiltin.extern_c(
  {},
  function sqlite3_column_count(pStmt: c_ptr): c_int {
    throw 0;
  }
);
const _sqlite3_finalize = $SHBuiltin.extern_c(
  {},
  function sqlite3_finalize(pStmt: c_ptr): c_int {
    throw 0;
  }
);
const _sqlite3_close = $SHBuiltin.extern_c(
  {},
  function sqlite3_close(db: c_ptr): c_int {
    throw 0;
  }
);

const SQLITE_OK = 0;
const SQLITE_ROW = 100;
const SQLITE_INTEGER = 1;
const SQLITE_FLOAT = 2;
const SQLITE_BLOB = 4;
const SQLITE_NULL = 5;
const SQLITE_TEXT = 3;

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
    return (function(buf: c_ptr) : c_ptr {
      let i = 0;
      for (let e = s.length; i < e; ++i) {
        let code: number = s.charCodeAt(i);
        if (code > 127) throw Error("String is not ASCII");
        _ptr_write_char(buf, i, code);
      }
      _ptr_write_char(buf, i, 0);
      return buf;
    })(buf);
  } catch (e) {
    _free(buf);
    throw e;
  }
}

/// Convert an ASCIIZ string up to a maximum size to a JS string.
function asciizToString_unsafe(buf: c_ptr, len: number): string {
  "inline";
  return _sh_asciiz_to_string($SHBuiltin.c_native_runtime(), buf, len);
}

function main() {
  let dbPtrBuffer: c_ptr = malloc(8);
  let stmt: c_ptr = c_null;
  let db: c_ptr = c_null;
  let sqlPtr: c_ptr = c_null;
  let res = 0;

  try {
    // Open the database
    // print("mk1");
    res = _sqlite3_open(stringToAsciiz("largeDB.sqlite"), dbPtrBuffer);
    // print("mk2");
    if (res !== SQLITE_OK) throw Error(res);
    db = _sh_ptr_read_ptr(dbPtrBuffer, 0);

    const start = Date.now();
    // Prepare the SQL statement
    sqlPtr = stringToAsciiz("SELECT * FROM Test;");
    // print("mk3");
    res = _sqlite3_prepare_v2(db, sqlPtr, -1, dbPtrBuffer, c_null);
    // print("mk4");
    if (res !== SQLITE_OK) throw Error(res);
    // print("mk5");
    stmt = _sh_ptr_read_ptr(dbPtrBuffer, 0);

    // Loop through all rows in the table
    let results : any[] = (function(): any[] {
      const mapConst = Map;
      const mapPrototypeGet: any = Map.prototype.get;
      const mapPrototypeSet: any = Map.prototype.set;

      let results = [];

      // Use a JS object.
      while (_sqlite3_step(stmt) === SQLITE_ROW) {
       const row = {};
        let count = _sqlite3_column_count(stmt);
        for (let i = 0; i < count; i++) {
          const name = asciizToString_unsafe(_sqlite3_column_name(stmt, i), -1);
          const type = _sqlite3_column_type(stmt, i);
          if (type === SQLITE_INTEGER) {
            row[name] = _sqlite3_column_int(stmt, i);
          } else if (type === SQLITE_FLOAT) {
            row[name] = _sqlite3_column_float(stmt, i);
          } else if (type === SQLITE_TEXT) {
            const textPtr = _sqlite3_column_text(stmt, i);
            const len = _sqlite3_column_bytes(stmt, i);
            row[name] = asciizToString_unsafe(textPtr, len);
          }
        }

/*
       // Use an array.
       const row = [];
       let count = _sqlite3_column_count(stmt);
        for (let i = 0; i < count; i++) {
          const name = asciizToString_unsafe(_sqlite3_column_name(stmt, i), -1);
          row.push(name);
          const type = _sqlite3_column_type(stmt, i);
          if (type === SQLITE_INTEGER) {
            row.push(_sqlite3_column_int(stmt, i));
          } else if (type === SQLITE_FLOAT) {
            row.push(_sqlite3_column_float(stmt, i));
          } else if (type === SQLITE_TEXT) {
            const textPtr = _sqlite3_column_text(stmt, i);
            const len = _sqlite3_column_bytes(stmt, i);
            row.push(asciizToString_unsafe(textPtr, len));
          }
        }
*/

/*
        // Use a Map.
        const row = new mapConst();
        let count = _sqlite3_column_count(stmt);
        for (let i = 0; i < count; i++) {
          const name = asciizToString_unsafe(_sqlite3_column_name(stmt, i), -1);
          const type = _sqlite3_column_type(stmt, i);
          if (type === SQLITE_INTEGER) {
            $SHBuiltin.call(mapPrototypeSet, row, name, _sqlite3_column_int(stmt, i));
          } else if (type === SQLITE_FLOAT) {
            $SHBuiltin.call(mapPrototypeSet, row, name, _sqlite3_column_float(stmt, i));
          } else if (type === SQLITE_TEXT) {
            const textPtr = _sqlite3_column_text(stmt, i);
            const len = _sqlite3_column_bytes(stmt, i);
            $SHBuiltin.call(mapPrototypeSet, row, name, asciizToString_unsafe(textPtr, len));
          }
        }
*/
        results.push(row);
        if (results.length > 500)
          results = []
      }
      return results;
    })();
    const end = Date.now();

    print(results.length);
    print("Time:", end - start, "ms");
    print(JSON.stringify(HermesInternal.getInstrumentedStats(), null, 2));
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
