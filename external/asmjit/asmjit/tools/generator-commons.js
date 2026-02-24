// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

const hasOwn = Object.prototype.hasOwnProperty;
function nop(x) { return x; }

// Generator - Constants
// ---------------------

const kIndent = "  ";
exports.kIndent = kIndent;

const kLineWidth = 120;

// Generator - Logging
// -------------------

let VERBOSE = false;

function setDebugVerbosity(value) {
  VERBOSE = value;
}
exports.setDebugVerbosity = setDebugVerbosity;

function DEBUG(msg) {
  if (VERBOSE)
    console.log(msg);
}
exports.DEBUG = DEBUG;

function WARN(msg) {
  console.log(msg);
}
exports.WARN = WARN;

function FATAL(msg) {
  console.log(`FATAL: ${msg}`);
  throw new Error(msg);
}
exports.FATAL = FATAL;

// Generator - Object Utilities
// ----------------------------

class ObjectUtils {
  static clone(map) {
    return Object.assign(Object.create(null), map);
  }

  static merge(a, b) {
    if (a === b)
      return a;

    for (let k in b) {
      let av = a[k];
      let bv = b[k];

      if (typeof av === "object" && typeof bv === "object")
        ObjectUtils.merge(av, bv);
      else
        a[k] = bv;
    }

    return a;
  }

  static equals(a, b) {
    if (a === b)
      return true;

    if (typeof a !== typeof b)
      return false;

    if (typeof a !== "object")
      return a === b;

    if (Array.isArray(a) || Array.isArray(b)) {
      if (Array.isArray(a) !== Array.isArray(b))
        return false;

      const len = a.length;
      if (b.length !== len)
        return false;

      for (let i = 0; i < len; i++)
        if (!ObjectUtils.equals(a[i], b[i]))
          return false;
    }
    else {
      if (a === null || b === null)
        return a === b;

      for (let k in a)
        if (!hasOwn.call(b, k) || !ObjectUtils.equals(a[k], b[k]))
          return false;

      for (let k in b)
        if (!hasOwn.call(a, k))
          return false;
    }

    return true;
  }

  static equalsExcept(a, b, except) {
    if (a === b)
      return true;

    if (typeof a !== "object" || typeof b !== "object" || Array.isArray(a) || Array.isArray(b))
      return ObjectUtils.equals(a, b);

    for (let k in a)
      if (!hasOwn.call(except, k) && (!hasOwn.call(b, k) || !ObjectUtils.equals(a[k], b[k])))
        return false;

    for (let k in b)
      if (!hasOwn.call(except, k) && !hasOwn.call(a, k))
        return false;

    return true;
  }

  static findKey(map, keys) {
    for (let key in keys)
      if (hasOwn.call(map, key))
        return key;
    return undefined;
  }

  static hasAny(map, keys) {
    for (let key in keys)
      if (hasOwn.call(map, key))
        return true;
    return false;
  }

  static and(a, b) {
    const out = Object.create(null);
    for (let k in a)
      if (hasOwn.call(b, k))
        out[k] = true;
    return out;
  }

  static xor(a, b) {
    const out = Object.create(null);
    for (let k in a) if (!hasOwn.call(b, k)) out[k] = true;
    for (let k in b) if (!hasOwn.call(a, k)) out[k] = true;
    return out;
  }
}
exports.ObjectUtils = ObjectUtils;

// Generator - Array Utilities
// ---------------------------

class ArrayUtils {
  static min(arr, fn) {
    if (!arr.length)
      return null;

    if (!fn)
      fn = nop;

    let v = fn(arr[0]);
    for (let i = 1; i < arr.length; i++)
      v = Math.min(v, fn(arr[i]));
    return v;
  }

  static max(arr, fn) {
    if (!arr.length)
      return null;

    if (!fn)
      fn = nop;

    let v = fn(arr[0]);
    for (let i = 1; i < arr.length; i++)
      v = Math.max(v, fn(arr[i]));
    return v;
  }

  static sorted(obj, cmp) {
    const out = Array.isArray(obj) ? obj.slice() : Object.getOwnPropertyNames(obj);
    out.sort(cmp);
    return out;
  }

  static deepIndexOf(arr, what) {
    for (let i = 0; i < arr.length; i++)
      if (ObjectUtils.equals(arr[i], what))
        return i;
    return -1;
  }

  static toDict(arr, value) {
    if (value === undefined)
      value = true;

    const out = Object.create(null);
    for (let i = 0; i < arr.length; i++)
      out[arr[i]] = value;
    return out;
  }
}
exports.ArrayUtils = ArrayUtils;


// Generator - String Utilities
// ----------------------------

class StringUtils {
  static asString(x) { return String(x); }

  static countOf(s, pattern) {
    if (!pattern)
      FATAL(`Pattern cannot be empty`);

    let n = 0;
    let pos = 0;

    while ((pos = s.indexOf(pattern, pos)) >= 0) {
      n++;
      pos += pattern.length;
    }

    return n;
  }

  static trimLeft(s) { return s.replace(/^\s+/, ""); }
  static trimRight(s) { return s.replace(/\s+$/, ""); }

  static upFirst(s) {
    if (!s) return "";
    return s[0].toUpperCase() + s.substr(1);
  }

  static decToHex(n, nPad) {
    let hex = Number(n < 0 ? 0x100000000 + n : n).toString(16);
    while (nPad > hex.length)
      hex = "0" + hex;
    return "0x" + hex.toUpperCase();
  }

  static format(array, indent, showIndex, mapFn) {
    if (!mapFn)
      mapFn = StringUtils.asString;

    let s = "";
    let threshold = 80;

    if (showIndex === -1)
      s += indent;

    for (let i = 0; i < array.length; i++) {
      const item = array[i];
      const last = i === array.length - 1;

      if (showIndex !== -1)
        s += indent;

      s += mapFn(item);
      if (showIndex > 0) {
        s += `${last ? " " : ","} // #${i}`;
        if (typeof array.refCountOf === "function")
          s += ` [ref=${array.refCountOf(item)}x]`;
      }
      else if (!last) {
        s += ",";
      }

      if (showIndex === -1) {
        if (s.length >= threshold - 1 && !last) {
          s += "\n" + indent;
          threshold += 80;
        }
        else {
          if (!last) s += " ";
        }
      }
      else {
        if (!last) s += "\n";
      }
    }

    return s;
  }

  static makeCxxArray(array, code, indent) {
    if (typeof indent !== "string")
      indent = kIndent;

    return `${code} = {\n${indent}` + array.join(`,\n${indent}`) + `\n};\n`;
  }

  static makeCxxArrayWithComment(array, code, indent) {
    if (typeof indent !== "string")
      indent = kIndent;

    let s = "";
    for (let i = 0; i < array.length; i++) {
      const last = i === array.length - 1;
      s += indent + array[i].data +
           (last ? "  // " : ", // ") + (array[i].refs ? "#" + String(i) : "").padEnd(5) + array[i].comment + "\n";
    }
    return `${code} = {\n${s}};\n`;
  }

  static formatCppStruct(...args) {
    return "{ " + args.join(", ") + " }";
  }

  static formatCppFlags(obj, fn, none) {
    if (none == null)
      none = "0";

    if (!fn)
      fn = nop;

    let out = "";
    for (let k in obj) {
      if (obj[k])
        out += (out ? " | " : "") + fn(k);
    }
    return out ? out : none;
  }

  static formatRecords(array, indent, fn) {
    if (typeof indent !== "string")
      indent = kIndent;

    if (!fn)
      fn = nop;

    let s = "";
    let line = "";
    for (let i = 0; i < array.length; i++) {
      const item = fn(array[i]);
      const combined = line ? line + ", " + item : item;

      if (combined.length >= kLineWidth) {
        s = s ? s + ",\n" + line : line;
        line = item;
      }
      else {
        line = combined;
      }
    }

    if (line) {
      s = s ? s + ",\n" + line : line;
    }

    return StringUtils.indent(s, indent);
  }

  static disclaimer(s) {
    return "// ------------------- Automatically generated, do not edit -------------------\n" +
           s +
           "// ----------------------------------------------------------------------------\n";
  }

  static indent(s, indentation) {
    if (typeof indentation === "number")
      indentation = " ".repeat(indentation);

    let lines = s.split(/\r?\n/g);
    if (indentation) {
      for (let i = 0; i < lines.length; i++) {
        let line = lines[i];
        if (line)
          lines[i] = indentation + line;
      }
    }

    return lines.join("\n");
  }

  static extract(s, start, end) {
    const iStart = s.indexOf(start);
    const iEnd   = s.indexOf(end);

    if (iStart === -1)
      FATAL(`StringUtils.extract(): Couldn't locate start mark '${start}'`);

    if (iEnd === -1)
      FATAL(`StringUtils.extract(): Couldn't locate end mark '${end}'`);

    return s.substring(iStart + start.length, iEnd).trim();
  }

  static inject(s, start, end, code) {
    let iStart = s.indexOf(start);
    let iEnd   = s.indexOf(end);

    if (iStart === -1)
      FATAL(`StringUtils.inject(): Couldn't locate start mark '${start}'`);

    if (iEnd === -1)
      FATAL(`StringUtils.inject(): Couldn't locate end mark '${end}'`);

    let nIndent = 0;
    while (iStart > 0 && s[iStart-1] === " ") {
      iStart--;
      nIndent++;
    }

    if (nIndent) {
      const indentation = " ".repeat(nIndent);
      code = StringUtils.indent(code, indentation) + indentation;
    }

    return s.substr(0, iStart + start.length + nIndent) + code + s.substr(iEnd);
  }

  static makePriorityCompare(priorityArray) {
    const map = Object.create(null);
    priorityArray.forEach((str, index) => { map[str] = index; });

    return function(a, b) {
      const ax = hasOwn.call(map, a) ? map[a] : Infinity;
      const bx = hasOwn.call(map, b) ? map[b] : Infinity;
      return ax != bx ? ax - bx : a < b ? -1 : a > b ? 1 : 0;
    }
  }
}
exports.StringUtils = StringUtils;

// Generator - Indexed Array
// =========================

// IndexedArray is an Array replacement that allows to index each item inserted to it. Its main purpose
// is to avoid data duplication, if an item passed to `addIndexed()` is already within the Array then
// it's not inserted and the existing index is returned instead.
function IndexedArray_keyOf(item) {
  return typeof item === "string" ? item : JSON.stringify(item);
}

class IndexedArray extends Array {
  constructor() {
    super();
    this._index = Object.create(null);
  }

  refCountOf(item) {
    const key = IndexedArray_keyOf(item);
    const idx = this._index[key];

    return idx !== undefined ? idx.refCount : 0;
  }

  addIndexed(item) {
    const key = IndexedArray_keyOf(item);
    let idx = this._index[key];

    if (idx !== undefined) {
      idx.refCount++;
      return idx.data;
    }

    idx = this.length;
    this._index[key] = {
      data: idx,
      refCount: 1
    };
    this.push(item);
    return idx;
  }
}
exports.IndexedArray = IndexedArray;

// Generator - Indexed String
// ==========================

// IndexedString is mostly used to merge all instruction names into a single string with external
// index. It's designed mostly for generating C++ tables. Consider the following cases in C++:
//
//   a) static const char* const* instNames = { "add", "mov", "vpunpcklbw" };
//
//   b) static const char instNames[] = { "add\0" "mov\0" "vpunpcklbw\0" };
//      static const uint16_t instNameIndex[] = { 0, 4, 8 };
//
// The latter (b) has an advantage that it doesn't have to be relocated by the linker, which saves
// a lot of space in the resulting binary and a lot of CPU cycles (and memory) when the linker loads
// it. AsmJit supports thousands of instructions so each optimization like this makes it smaller and
// faster to load.
class IndexedString {
  constructor() {
    this.map = Object.create(null);
    this.array = [];
    this.size = -1;
  }

  add(s) {
    this.map[s] = -1;
  }

  index() {
    const map = this.map;
    const array = this.array;
    const partialMap = Object.create(null);

    let k, kp;
    let i, len;

    // Create a map that will contain all keys and partial keys.
    for (k in map) {
      if (!k) {
        partialMap[k] = k;
      }
      else {
        for (i = 0, len = k.length; i < len; i++) {
          kp = k.substr(i);
          if (!hasOwn.call(partialMap, kp) || partialMap[kp].length < len)
            partialMap[kp] = k;
        }
      }
    }

    // Create an array that will only contain keys that are needed.
    for (k in map)
      if (partialMap[k] === k)
        array.push(k);
    array.sort();

    // Create valid offsets to the `array`.
    let offMap = Object.create(null);
    let offset = 0;

    for (i = 0, len = array.length; i < len; i++) {
      k = array[i];

      offMap[k] = offset;
      offset += k.length + 1;
    }
    this.size = offset;

    // Assign valid offsets to `map`.
    for (kp in map) {
      k = partialMap[kp];
      map[kp] = offMap[k] + k.length - kp.length;
    }
  }

  format(indent, justify) {
    if (this.size === -1)
      FATAL(`IndexedString.format(): not indexed yet, call index()`);

    const array = this.array;
    if (!justify) justify = 0;

    let i;
    let s = "";
    let line = "";

    for (i = 0; i < array.length; i++) {
      const item = "\"" + array[i] + ((i !== array.length - 1) ? "\\0\"" : "\";");
      const newl = line + (line ? " " : indent) + item;

      if (newl.length <= justify) {
        line = newl;
        continue;
      }
      else {
        s += line + "\n";
        line = indent + item;
      }
    }

    return s + line;
  }

  getSize() {
    if (this.size === -1)
      FATAL(`IndexedString.getSize(): Not indexed yet, call index()`);
    return this.size;
  }

  getIndex(k) {
    if (this.size === -1)
      FATAL(`IndexedString.getIndex(): Not indexed yet, call index()`);

    if (!hasOwn.call(this.map, k))
      FATAL(`IndexedString.getIndex(): Key '${k}' not found.`);

    return this.map[k];
  }
}
exports.IndexedString = IndexedString;
