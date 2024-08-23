// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: (Zlib or Unlicense)

(function($scope, $as) {
"use strict";

function FAIL(msg) { throw new Error("[AArch32] " + msg); }

// Import
// ======

const base = $scope.base ? $scope.base : require("./base.js");
const exp = $scope.exp ? $scope.exp : require("./exp.js")

const hasOwn = Object.prototype.hasOwnProperty;
const dict = base.dict;
const NONE = base.NONE;
const Parsing = base.Parsing;
const MapUtils = base.MapUtils;

// Export
// ======

const arm = $scope[$as] = dict();

// Database
// ========

arm.dbName = "isa_aarch32.json";

// asmdb.aarch32.Utils
// ===================

// Can be used to assign the number of bits each part of the opcode occupies.
// NOTE: THUMB instructions that use halfword must always specify the width
// of all registers as many instructions accept only LO (r0..r7) registers.
const FieldInfo = {
  "P"     : { "bits": 1 },
  "U"     : { "bits": 1 },
  "W"     : { "bits": 1 },
  "S"     : { "bits": 1 },
  "R"     : { "bits": 1 },
  "H"     : { "bits": 1 },
  "isFp32": { "bits": 1 },
  "F"     : { "bits": 1 },
  "align" : { "bits": 2 },
  "ja"    : { "bits": 1 },
  "jb"    : { "bits": 1 },
  "op"    : { "bits": 1 }, // TODO: This should be fixed.
  "sz"    : { "bits": 2 },
  "sop"   : { "bits": 2 },
  "cond"  : { "bits": 4 },
  "cmode" : { "bits": 4 },
  "Cn"    : { "bits": 4 },
  "Cm"    : { "bits": 4 },

  "Rd"    : { "bits": 4, "read": false, "write": true  },
  "Rd2"   : { "bits": 4, "read": false, "write": true  },
  "RdLo"  : { "bits": 4, "read": false, "write": true  },
  "RdHi"  : { "bits": 4, "read": false, "write": true  },
  "RdList": { "bits": 4, "read": false, "write": true  , "list": true },
  "Rx"    : { "bits": 4, "read": true , "write": true  },
  "RxLo"  : { "bits": 4, "read": true , "write": true  },
  "RxHi"  : { "bits": 4, "read": true , "write": true  },
  "Rn"    : { "bits": 4, "read": true , "write": false },
  "Rm"    : { "bits": 4, "read": true , "write": false },
  "Ra"    : { "bits": 4, "read": true , "write": false },
  "Rs"    : { "bits": 4, "read": true , "write": false },
  "Rs2"   : { "bits": 4, "read": true , "write": false },
  "RsList": { "bits": 4, "read": true , "write": false , "list": true },

  "Sd"    : { "bits": 4, "read": false, "write": true  },
  "Sd2"   : { "bits": 4, "read": false, "write": true  },
  "SdList": { "bits": 4, "read": false, "write": true  , "list": true },
  "Sx"    : { "bits": 4, "read": true , "write": true  },
  "Sn"    : { "bits": 4, "read": true , "write": false },
  "Sm"    : { "bits": 4, "read": true , "write": false },
  "Ss"    : { "bits": 4, "read": true , "write": false },
  "Ss2"   : { "bits": 4, "read": true , "write": false },
  "SsList": { "bits": 4, "read": true , "write": false , "list": true },

  "Dd"    : { "bits": 4, "read": false, "write": true  },
  "Dd2"   : { "bits": 4, "read": false, "write": true  },
  "Dd3"   : { "bits": 4, "read": false, "write": true  },
  "Dd4"   : { "bits": 4, "read": false, "write": true  },
  "DdList": { "bits": 4, "read": false, "write": true  , "list": true },
  "Dx"    : { "bits": 4, "read": true , "write": true  },
  "Dx2"   : { "bits": 4, "read": true , "write": true  },
  "Dn"    : { "bits": 4, "read": true , "write": false },
  "Dn2"   : { "bits": 4, "read": true , "write": false },
  "Dn3"   : { "bits": 4, "read": true , "write": false },
  "Dn4"   : { "bits": 4, "read": true , "write": false },
  "Dm"    : { "bits": 4, "read": true , "write": false },
  "Ds"    : { "bits": 4, "read": true , "write": false },
  "Ds2"   : { "bits": 4, "read": true , "write": false },
  "Ds3"   : { "bits": 4, "read": true , "write": false },
  "Ds4"   : { "bits": 4, "read": true , "write": false },
  "DsList": { "bits": 4, "read": true , "write": false , "list": true },

  "Vd"    : { "bits": 4, "read": false, "write": true  },
  "Vd2"   : { "bits": 4, "read": false, "write": true  },
  "Vd3"   : { "bits": 4, "read": false, "write": true  },
  "Vd4"   : { "bits": 4, "read": false, "write": true  },
  "Vx"    : { "bits": 4, "read": true , "write": true  },
  "Vx2"   : { "bits": 4, "read": true , "write": true  },
  "Vn"    : { "bits": 4, "read": true , "write": false },
  "Vm"    : { "bits": 4, "read": true , "write": false },
  "Vs"    : { "bits": 4, "read": true , "write": false },
  "Vs2"   : { "bits": 4, "read": true , "write": false },
};

arm.FieldInfo = FieldInfo;

// ARM utilities.
class Utils {
  static splitInstructionSignature(s) {
    const names = s.match(/^[\w\|]+/)[0];
    s = s.substring(names.length);

    const opOffset = s.indexOf(" ")
    const suffix = s.substring(0, opOffset).trim();
    const operands = opOffset === -1 ? "" : s.substring(opOffset + 1).trim();

    return {
      names: names.split("|").map((base)=>{ return base + suffix}),
      operands: operands
    }
  }

  static parseShiftOp(s) {
    const m = s.match(/^(sop|lsl_or_asr|lsl|lsr|asr|ror|rrx) /);
    return m ? m[1] : "";
  }

  static parseDtArray(s) {
    const out = [];
    if (!s) return out;

    const arr = s.split("|");
    let i;

    // First expand anything between X-Y, for example s8-32 would be expanded to [s8, s16, s32].
    for (i = 0; i < arr.length; i++) {
      const v = arr[i];

      if (v.indexOf("-") !== -1) {
        const m = /^([A-Za-z]+)?(\d+)-(\d+)$/.exec(v);
        if (!m)
          FAIL(`Couldn't parse '${s}' data-type`);

        let type = m[1] || "";
        let size = parseInt(m[2], 10);
        let last = parseInt(m[3], 10);

        if (!Utils.checkDtSize(size) || !Utils.checkDtSize(last))
          FAIL(`Invalid dt width in '${s}'`);

        do {
          out.push(type + String(size));
          size <<= 1;
        } while (size <= last);
      }
      else {
        out.push(v);
      }
    }

    // Now expand 'x' to 's' and 'u'.
    i = 0;
    while (i < out.length) {
      const v = out[i];
      if (v.startsWith("x")) {
        out.splice(i, 1, "s" + v.substr(1), "u" + v.substr(1));
        i += 2;
      }
      else {
        i++;
      }
    }

    return out;
  }

  static checkDtSize(x) {
    return x === 8 || x === 16 || x === 32 || x === 64;
  }
}
arm.Utils = Utils;

function normalizeNumber(n) {
  return n < 0 ? 0x100000000 + n : n;
}

function decomposeOperand(s) {
  const elementSuffix = "[#i]";
  let element = null;
  let consecutive = 0;
  let userRegList = false;

  if (s.endsWith("^")) {
    userRegList = true;
    s = s.substring(0, s.length - 1);
  }

  if (s.endsWith(elementSuffix)) {
    element = "#i";
    s = s.substring(0, s.length - elementSuffix.length);
  }

  if (s.endsWith("++")) {
    consecutive = 2;
    s = s.substr(0, s.length - 2);
  }
  else if (s.endsWith("+")) {
    consecutive = 1;
    s = s.substr(0, s.length - 1);
  }

  let m = s.match(/==|\!=|>=|<=|\*/);
  let restrict = false;

  if (m) {
    restrict = s.substr(m.index);
    s = s.substr(0, m.index);
  }

  return {
    data    : s,
    element : element,
    restrict: restrict,
    consecutive: consecutive,
    userRegList: true
  };
}

function splitOpcodeFields(s) {
  const arr = s.split("|");
  const out = [];

  for (let i = 0; i < arr.length; i++) {
    const val = arr[i];
    if (/^[0-1A-Z]{2,}$/.test(val))
      out.push.apply(out, val.match(/([0-1]+)|[A-Z]/g));
    else
      out.push(val);
  }

  return out.map((field) => { return field.trim(); });
}

// asmdb.aarch32.Operand
// =====================

// ARM operand.
class Operand extends base.Operand {
  constructor(def) {
    super(def);
  }

  hasMemModes() {
    return Object.keys(this.memModes).length !== 0;
  }

  get name() {
    switch (this.type) {
      case "reg": return this.reg;
      case "mem": return this.mem;
      case "imm": return this.imm;
      case "rel": return this.rel;
      default   : return "";
    }
  }

  get scale() {
    if (this.restrict && this.restrict.startsWith("*"))
      return parseInt(this.restrict.substr(1), 10);
    else
      return 0;
  }

  isRelative() {
    if (this.type === "imm")
      return this.name === "relA" || this.name === "relS" || this.name === "relZ";
    else
      return false;
  }
}
arm.Operand = Operand;

// asmdb.aarch32.Instruction
// =========================

function patternFromOperand(key) {
  return key;
  // return key.replace(/\b(?:[RVDS](?:d|s|n|m|x|x2))\b/, "R");
}

// Rewrite a memory operand expression (either base or index) to a simplified one, which is okay
// to be generated as C++ expression. In general, we want to simplify != to a more favorable code.
function simplifyMemoryExpression(e) {
  if (e.type === "binary" && e.op === "!=" && e.right.type === "var") {
    // Rewrite A != PC to A < PC
    if (e.right.name === "PC") { e.op = "<"; }

    // Rewrite A != HI to A < 8
    if (e.right.name === "HI") { e.op = "<"; e.right = exp.Imm(8); }

    // Rewrite A != XX to A < SP || A == LR
    if (e.right.name === "XX") {
      return exp.Or(exp.Lt(e.left, exp.Var("SP")),
                    exp.Eq(e.left.clone(), exp.Var("LR")));
    }
  }

  return e;
}

// ARM instruction.
class Instruction extends base.Instruction {
  constructor(db, data) {
    super(db, data);
    // name, operands, encoding, opcode, metadata

    const encoding = hasOwn.call(data, "a32") ? "a32" :
                     hasOwn.call(data, "t32") ? "t32" :
                     hasOwn.call(data, "t16") ? "t16" : "";

    this.name = data.name;
    this.it = dict();                // THUMB's 'it' flags.
    this.apsr = dict();
    this.fpcsr = dict();
    this.calc = dict();              // Calculations required to generate opcode.
    this.immCond = [];               // Immediate value conditions (array of conditions).

    this.s = null;                   // Instruction S flag (null, true, or false).
    this.dt = [];                    // Instruction <dt> field (first data-type).
    this.dt2 = [];                   // Instruction <dt2> field (second data-type).

    this.availableFrom  = "";        // Instruction supported by from  ARMv???.
    this.availableUntil = "";        // Instruction supported by until ARMv???.

    this._assignOperands(data.operands);
    this._assignEncoding(encoding.toUpperCase());
    this._assignOpcode(data[encoding]);

    for (let k in data) {
      if (k === "name" || k == encoding || k === "operands")
        continue;
      this._assignAttribute(k, data[k]);
    }

    this._updateOperandsInfo();
    this._postProcess();
  }

  _assignAttribute(key, value) {
    switch (key) {
      case "it":
        for (let it of value.split(" "))
          this.it[it.trim()] = true;
        break;

      case "apsr":
      case "fpcsr":
        this._assignAttributeKeyValue(key, value);
        break;

      case "imm":
        this.imm = exp.parse(value);
        break;

      case "calc":
        for (let calcKey in value)
          this.calc[calcKey] = exp.parse(value[calcKey]);
        break;

      default:
        super._assignAttribute(key, value);
    }
  }

  _assignAttributeKeyValue(name, content) {
    const attributes = content.trim().split(/[ ]+/);

    for (let i = 0; i < attributes.length; i++) {
      const attr = attributes[i].trim();
      if (!attr)
        continue;

      const eq = attr.indexOf("=");
      let key = eq === -1 ? attr : attr.substr(0, eq);
      let val = eq === -1 ? true : attr.substr(eq + 1);

      // If the key contains "|" it's a definition of multiple attributes.
      if (key.indexOf("|") !== -1) {
        const dot = key.indexOf(".");

        const base = dot === -1 ? "" : key.substr(0, dot + 1);
        const keys = (dot === -1 ? key : key.substr(dot + 1)).split("|");

        for (let j = 0; j < keys.length; j++)
          this[name][base + keys[j]] = val;
      }
      else {
        this[name][key] = val;
      }
    }
  }

  _assignEncoding(s) {
    this.arch = s === "T16" || s === "T32" ? "THUMB" : "ARM";
    this.encoding = s;
  }

  _assignOperands(s) {
    if (!s) return;

    // Split into individual operands and push them to `operands`.
    const arr = base.Parsing.splitOperands(s);
    for (let i = 0; i < arr.length; i++) {
      let def = arr[i];
      const op = new Operand(def);

      const consecutive = def.match(/(\d+)x\{(.*)\}([+][+]?)/);
      if (consecutive)
        def = consecutive[2];

      op.sign = false;
      op.element = null;
      op.shiftOp = "";
      op.shiftImm = null;

      // Handle {optional} attribute.
      if (Parsing.isOptional(def)) {
        op.optional = true;
        def = Parsing.clearOptional(def);
      }

      // Handle commutativity <-> symbol.
      if (Parsing.isCommutative(def)) {
        op.commutative = true;
        def = Parsing.clearCommutative(def);
      }

      // Handle shift operation.
      let shiftOp = Utils.parseShiftOp(def);
      if (shiftOp) {
        op.shiftOp = shiftOp;
        def = def.substring(shiftOp.length + 1);
      }

      if (def.startsWith("[")) {
        op.type = "mem";
        op.memModes = dict();

        op.base = null;
        op.index = null;
        op.offset = null;

        let mem = def;
        let didHaveMemMode = false;

        for (;;) {
          if (mem.endsWith("!")) {
            op.memModes.preIndex = true;
            mem = mem.substring(0, mem.length - 1);

            didHaveMemMode = true;
            break;
          }

          if (mem.endsWith("@")) {
            op.memModes.postIndex = true;
            mem = mem.substring(0, mem.length - 1);

            didHaveMemMode = true;
            break;
          }

          if (mem.endsWith("{!}")) {
            op.memModes.offset = true;
            op.memModes.preIndex = true;
            mem = mem.substring(0, mem.length - 3);

            didHaveMemMode = true;
            continue;
          }

          if (mem.endsWith("{@}")) {
            op.memModes.offset = true;
            op.memModes.postIndex = true;
            mem = mem.substring(0, mem.length - 3);

            didHaveMemMode = true;
            continue;
          }

          break;
        }

        if (!mem.endsWith("]"))
          FAIL(`Unknown memory operand '${mem}' in '${def}'`);

        let parts = mem.substring(1, mem.length - 1).split(",").map(function(s) { return s.trim() });
        for (let i = 0; i < parts.length; i++) {
          const part = parts[i];

          const m = part.match(/^\{(lsl|sop)\s+#(\w+)\}$/);
          if (m) {
            op.shiftOp = m[1];
            op.shiftImm = m[2];
            continue;
          }

          if (i === 0) {
            op.base = dict();
            op.base.field = part;
            op.base.exp = null;

            const m = part.match(/^([A-Za-z]\w*)/);
            if (m.length < part.length) {
              op.base.exp = simplifyMemoryExpression(exp.parse(part));
              op.base.field = m[1];
            }
          }
          else if (part.startsWith("#")) {
            let p = part.substring(1);
            let u = "1";
            let alwaysNegative = false;

            let offExp = null;
            let offMul = 1;

            if (p.startsWith("+/-")) {
              u = "U";
              p = p.substring(3);
            }

            if (p.startsWith("-")) {
              alwaysNegative = false;
              p = p.substring(1);
            }

            const expMatch = p.match(/^([A-Za-z]\w*)==/);
            if (expMatch) {
              offExp = exp.parse(p);
              p = p.substr(0, expMatch[1].length);
            }

            const mulMatch = p.match(/\s*\*\s*(\d+)$/);
            if (mulMatch) {
              offMul = parseInt(mulMatch[1]);
              p = p.substr(0, mulMatch.index);
            }

            op.offset = dict();
            op.offset.field = p;
            op.offset.u = u;
            op.offset.exp = offExp;
            op.offset.mul = offMul;
            op.offset.negative = alwaysNegative;
          }
          else {
            let p = part;
            let u = "1";

            if (p.startsWith("+/-")) {
              u = "U";
              p = p.substring(3);
            }

            op.index = dict();
            op.index.field = p;
            op.index.u = u;

            const m = p.match(/^([A-Za-z]\w*)/);
            if (m.length < p.length) {
              op.index.exp = simplifyMemoryExpression(exp.parse(p));
              op.index.field = m[1];
            }
          }
        }

        if (!op.hasMemModes() && (op.offset || op.index))
          op.memModes.offset = true;

        op.mem = mem;
      }
      else if (def.startsWith("#")) {
        const obj = decomposeOperand(def);
        const imm = obj.data;

        op.type = "imm";
        op.imm = imm.substring(1);        // Immediate operand name.
        op.immSize = 0;                   // Immediate size in bits.
        op.restrict = obj.restrict;       // Immediate condition.
      }
      else {
        const obj = decomposeOperand(def);
        const reg = obj.data;

        const type = reg.substr(0, 1).toLowerCase();
        const info = FieldInfo[reg];

        if (!info)
          FAIL(`Unknown register operand '${reg}' in '${def}'`);

        op.type     = info.list ? "reg-list" : "reg";
        op.reg      = reg;                // Register name (as specified in manual).
        op.regType  = type;               // Register type.
        op.regList  = !!info.list;        // Register list.
        op.read     = info.read;          // Register access (read).
        op.write    = info.write;         // Register access (write).
        op.element  = obj.element;        // Register element[] access.
        op.restrict = obj.restrict;       // Register condition.
        op.consecutive = obj.consecutive;
      }

      this.operands.push(op);

      if (consecutive) {
        const count = parseInt(consecutive[1]);
        for (let n = 2; n <= count; n++) {
          const def = consecutive[3].replace(op.reg, op.reg + n);
          const opN = new Operand(def);
          opN.type = "reg";
          opN.reg = op.reg + n;
          opN.regType = op.regType;
          opN.read = op.read;
          opN.write = op.write;
          opN.element = op.element;
          opN.consecutive = consecutive[3].length;
          this.operands.push(opN);
        }
      }
    }
  }

  _assignOpcode(s) {
    this.opcodeString = s;

    let opcodeIndex = 0;
    let opcodeValue = 0;

    let patternMap = {};

    // Split opcode into its fields.
    const arr = splitOpcodeFields(s);
    const dup = dict();

    const fields = this.fields;
    const pattern = [];

    const fieldMap = Object.create(null);
    for (let field of arr) {
      fieldMap[field] = true;
    }

    for (let i = arr.length - 1; i >= 0; i--) {
      let key = arr[i];
      let m;

      if (/^[0-1]+$/.test(key)) {
        // This part of the opcode is RAW bits, they contribute to the `opcodeValue`.
        opcodeValue |= parseInt(key, 2) << opcodeIndex;
        opcodeIndex += key.length;
        pattern.unshift("_".repeat(key.length));
      }
      else {
        pattern.unshift(patternFromOperand(key));
        patternMap[patternFromOperand(key)] = true;

        let size = 0;
        let mask = 0;
        let bits = 0;
        let from = -1;

        let lbit = key.startsWith("'");
        let hbit = key.endsWith("'");

        if ((m = key.match(/\[\s*(\d+)\s*\:\s*(\d+)\s*\]$/))) {
          const a = parseInt(m[1], 10);
          const b = parseInt(m[2], 10);
          if (a < b)
            FAIL(`Invalid bit range '${key}' in opcode '${s}'`);
          from = b;
          size = a - b + 1;
          mask = ((1 << size) - 1) << b;
          key = key.substr(0, m.index).trim();
        }
        else if ((m = key.match(/\[\s*(\d+)\s*\]$/))) {
          from = parseInt(m[1], 10);
          size = 1;
          mask = 1 << from;
          key = key.substr(0, m.index).trim();
        }
        else if ((m = key.match(/\:\s*(\d+)$/))) {
          size = parseInt(m[1], 10);
          bits = size;
          key = key.substr(0, m.index).trim();
        }
        else {
          const key_ = key;

          if (lbit || hbit) {
            from = 0;

            if (lbit && hbit)
              FAIL(`Couldn't recognize the format of '${key}' in opcode '${s}'`);

            if (lbit) {
              key = key.substring(1);
            }

            if (hbit) {
              key = key.substring(0, key.length - 1);
              from = 4;
            }

            size = 1;
          }
          else if (FieldInfo[key]) {
            // Sizes of some standard fields can be assigned automatically.
            size = FieldInfo[key].bits;
            bits = size;

            if (fieldMap["'" + key])
              from = 1;
          }
          else if (key.length === 1) {
            // Sizes of one-letter fields (like 'U', 'F', etc...) is 1 if not specified.
            size = 1;
            bits = 1;
          }
          else {
            FAIL(`Couldn't recognize the size of '${key}' in opcode '${s}'`);
          }

          if (dup[key_] === true) {
            bits = 0;
            lbit = 0;
            hbit = 0;
          }
          else {
            dup[key_] = true;
          }
        }

        let field = fields[key];
        if (!field) {
          field = {
            index: opcodeIndex,
            values: [],
            bits: 0,
            mask: 0,
            lbit: 0,
            hbit: 0 // Only 1 if a single quote (') was used.
          }
          fields[key] = field;
        }

        if (from === -1)
          from = field.bits;

        field.mask |= mask;
        field.bits += bits;
        field.lbit += lbit;
        field.hbit += hbit;
        field.values.push({
          index: opcodeIndex,
          from: from,
          size: size
        });

        opcodeIndex += size;
      }
    }

    for (let i = 0; i < pattern.length; i++)
      if (pattern[i] === 'U')
        pattern[i] = "_";

    // Normalize all fields.
    for (let key in fields) {
      const field = fields[key];

      // There should be either number of bits or mask, there shouldn't be both.
      if (!field.bits && !field.mask)
        FAIL(`Part '${key}' of opcode '${s}' contains neither size nor mask`);

      if (field.bits && field.mask)
        FAIL(`Part '${key}' of opcode '${s}' contains both size and mask`);

      if (field.bits)
        field.mask = ((1 << field.bits) - 1);
      else if (field.mask)
        field.bits = 32 - Math.clz32(field.mask);

      // Handle field that used single-quote.
      if (field.lbit) {
        field.mask = (field.mask << 1) | 0x1;
        field.bits++;
      }

      if (field.hbit) {
        field.mask |= 1 << field.bits;
        field.bits++;
      }

      const op = this.operandByName(key);
      if (op && op.isImm())
        op.immSize = field.bits;
    }

    // Check if the opcode value has the correct number of bits (either 16 or 32).
    if (opcodeIndex !== 16 && opcodeIndex !== 32)
      FAIL(`The number of bits '${opcodeIndex}' used by the opcode '${s}' doesn't match 16 or 32`);
    this.opcodeValue = normalizeNumber(opcodeValue);
  }

  _assignSpecificAttribute(key, value) {
    // Support ARMv?+ and ARMv?- attributes.
    if (/^ARM\w+[+-]$/.test(key)) {
      const armv = key.substr(0, key.length - 1);
      const sign = key.substr(key.length - 1);

      if (sign === "+")
        this.availableFrom = armv;
      else
        this.availableUntil = armv;
      return true;
    }

    switch (key) {
      case "it": {
        const values = String(value).split("|");
        for (let i = 0; i < values.length; i++) {
          const value = values[i];
          switch (value) {
            case "in"  : this.it.IN   = true; break;
            case "out" : this.it.OUT  = true; break;
            case "any" : this.it.IN   = true;
                         this.it.OUT  = true; break;
            case "last": this.it.LAST = true; break;
            case "def" : this.it.DEF  = true; break;
            default:
              this.report(`${this.name}: Unhandled IT value '${value}'`);
          }
        }
        return true;
      }
    }

    return false;
  }

  // ARM instruction name could consist of name and optional type information
  // specified as <dt> and <dt2> in ARM manuals. We parse this information and
  // store it to `dt` and `dt2` fields. In addition, we also recognize the `S`
  // suffix (uppercase) of the instruction and mark it as `S` instruction. After
  // that the name is normalized to be lowercased.
  //
  // This functionality requires all the instruction data to be already set-up.
  _postProcess() {
    let s = this.name;

    // Parse <dt> and <dt2> fields.
    if (s.indexOf(".") !== -1) {
      const parts = s.split(".");
      this.name = parts[0];

      if (parts.length > 3)
        FAIL(`Couldn't recognize name attributes of '${s}'`);

      for (let i = 1; i < parts.length; i++) {
        const dt = Utils.parseDtArray(parts[i]);
        if (i === 1)
          this.dt = dt;
        else
          this.dt2 = dt;
      }
    }

    // Recognize "S" suffix.
    if (this.name.endsWith("S")) {
      this.name = this.name.substr(0, this.name.length - 1) + "s";
      this.s = true;
    }

    this.dt.sort();
  }

  operandByName(name) {
    const operands = this.operands;
    for (let i = 0; i < operands.length; i++) {
      const op = operands[i];
      if (op.name === name)
        return op;
    }
    return null;
  }
}
arm.Instruction = Instruction;

// asmdb.aarch32.ISA
// =================

function mergeGroupData(data, group) {
  for (let k in group) {
    switch (k) {
      case "group":
      case "data":
        break;

      case "ext":
        data[k] = (data[k] ? data[k] + " " : "") + group[k];
        break;

      default:
        if (data[k] === undefined)
          data[k] = group[k]
        break;
    }
  }
}

class ISA extends base.ISA {
  constructor(data) {
    super(data);
    this.addData(data || NONE);
  }

  _addInstructions(groups) {
    for (let group of groups) {
      for (let inst of group.data) {
        const sgn = Utils.splitInstructionSignature(inst.inst);
        const data = MapUtils.cloneExcept(inst, { "inst": true });

        mergeGroupData(data, group)

        for (let j = 0; j < sgn.names.length; j++) {
          data.name = sgn.names[j];
          data.operands = sgn.operands;
          if (j > 0)
            data.aliasOf = sgn.names[0];
          this._addInstruction(new Instruction(this, data));
        }
      }
    }

    return this;
  }
/*
  _addInstructions(instructions) {
    for (let i = 0; i < instructions.length; i++) {
      const obj = instructions[i];
      const sgn = obj.inst;
      const sep = sgn.indexOf(" ");

      const names = (sep !== -1 ? sgn.substring(0, sep) : sgn).trim().split("/");
      const operands = sep !== -1 ? sgn.substring(sep + 1) : "";

      const encoding = hasOwn.call(obj, "a32") ? "a32" :
                       hasOwn.call(obj, "t32") ? "t32" :
                       hasOwn.call(obj, "t16") ? "t16" : "";

      if (!encoding)
        FAIL(`Instruction ${names.join("/")} doesn't encoding, it must provide either a32, t32, or t16 field`);

      for (let j = 0; j < names.length; j++) {
        const inst = new Instruction(this, names[j], operands, encoding.toUpperCase(), obj[encoding], obj);
        if (j > 0)
          inst.aliasOf = names[0];
        this._addInstruction(inst);
      }
    }

    return this;
  }
*/
}
arm.ISA = ISA;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "aarch32"]);
