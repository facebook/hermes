// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: (Zlib or Unlicense)

(function($scope, $as) {
"use strict";

function FAIL(msg) { throw new Error("[AArch64] " + msg); }

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

arm.dbName = "isa_aarch64.json";

// asmdb.aarch64.Utils
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
  "F"     : { "bits": 1 },
  "post"  : { "bits": 1 },
  "!post" : { "bits": 1 },
  "op"    : { "bits": 1 }, // TODO: This should be fixed.
  "s"     : { "bits": 1 },
  "sz"    : { "bits": 2 },
  "msz"   : { "bits": 2 },
  "sop"   : { "bits": 2 },
  "cond"  : { "bits": 4 },
  "nzcv"  : { "bits": 4 },
  "cmode" : { "bits": 4 },
  "CRn"   : { "bits": 4 },
  "CRm"   : { "bits": 4 },

  "Rx"    : { "bits": 5, "read": true , "write": true  },
  "Rx2"   : { "bits": 5, "read": true , "write": true  },
  "Rdn"   : { "bits": 5, "read": true , "write": true  },
  "Rd"    : { "bits": 5, "read": false, "write": true  },
  "Rd2"   : { "bits": 5, "read": false, "write": true  },
  "Rs"    : { "bits": 5, "read": true , "write": false },
  "Rs2"   : { "bits": 5, "read": true , "write": false },
  "Rn"    : { "bits": 5, "read": true , "write": false },
  "Rm"    : { "bits": 5, "read": true , "write": false },
  "Ra"    : { "bits": 5, "read": true , "write": false },
  "Rt"    : { "bits": 5, "read": true , "write": false },
  "Rt2"   : { "bits": 5, "read": true , "write": false },

  "Wx"    : { "bits": 5, "read": true , "write": true  },
  "Wx2"   : { "bits": 5, "read": true , "write": true  },
  "Wdn"   : { "bits": 5, "read": true , "write": true  },
  "Wd"    : { "bits": 5, "read": false, "write": true  },
  "Wd2"   : { "bits": 5, "read": false, "write": true  },
  "Ws"    : { "bits": 5, "read": true , "write": false },
  "Ws2"   : { "bits": 5, "read": true , "write": false },
  "Wn"    : { "bits": 5, "read": true , "write": false },
  "Wm"    : { "bits": 5, "read": true , "write": false },
  "Wa"    : { "bits": 5, "read": true , "write": false },
  "Wt"    : { "bits": 5, "read": true , "write": false },
  "Wt2"   : { "bits": 5, "read": true , "write": false },

  "Xx"    : { "bits": 5, "read": true , "write": true  },
  "Xx2"   : { "bits": 5, "read": true , "write": true  },
  "Xdn"   : { "bits": 5, "read": true , "write": true  },
  "Xd"    : { "bits": 5, "read": false, "write": true  },
  "Xd2"   : { "bits": 5, "read": false, "write": true  },
  "Xs"    : { "bits": 5, "read": true , "write": false },
  "Xs2"   : { "bits": 5, "read": true , "write": false },
  "Xn"    : { "bits": 5, "read": true , "write": false },
  "Xm"    : { "bits": 5, "read": true , "write": false },
  "Xa"    : { "bits": 5, "read": true , "write": false },
  "Xt"    : { "bits": 5, "read": true , "write": false },
  "Xt2"   : { "bits": 5, "read": true , "write": false },

  "Bx"    : { "bits": 5, "read": true , "write": true  },
  "Bx2"   : { "bits": 5, "read": true , "write": true  },
  "Bdn"   : { "bits": 5, "read": true , "write": true  },
  "Bd"    : { "bits": 5, "read": false, "write": true  },
  "Bd2"   : { "bits": 5, "read": false, "write": true  },
  "Bs"    : { "bits": 5, "read": true , "write": false },
  "Bs2"   : { "bits": 5, "read": true , "write": false },
  "Bn"    : { "bits": 5, "read": true , "write": false },
  "Bm"    : { "bits": 5, "read": true , "write": false },
  "Ba"    : { "bits": 5, "read": true , "write": false },

  "Hx"    : { "bits": 5, "read": true , "write": true  },
  "Hx2"   : { "bits": 5, "read": true , "write": true  },
  "Hdn"   : { "bits": 5, "read": true , "write": true  },
  "Hd"    : { "bits": 5, "read": false, "write": true  },
  "Hd2"   : { "bits": 5, "read": false, "write": true  },
  "Hs"    : { "bits": 5, "read": true , "write": false },
  "Hs2"   : { "bits": 5, "read": true , "write": false },
  "Hn"    : { "bits": 5, "read": true , "write": false },
  "Hm"    : { "bits": 5, "read": true , "write": false },
  "Ha"    : { "bits": 5, "read": true , "write": false },

  "Sx"    : { "bits": 5, "read": true , "write": true  },
  "Sx2"   : { "bits": 5, "read": true , "write": true  },
  "Sdn"   : { "bits": 5, "read": true , "write": true  },
  "Sd"    : { "bits": 5, "read": false, "write": true  },
  "Sd2"   : { "bits": 5, "read": false, "write": true  },
  "Ss"    : { "bits": 5, "read": true , "write": false },
  "Ss2"   : { "bits": 5, "read": true , "write": false },
  "Sn"    : { "bits": 5, "read": true , "write": false },
  "Sm"    : { "bits": 5, "read": true , "write": false },
  "Sa"    : { "bits": 5, "read": true , "write": false },

  "Dx"    : { "bits": 5, "read": true , "write": true  },
  "Dx2"   : { "bits": 5, "read": true , "write": true  },
  "Ddn"   : { "bits": 5, "read": true , "write": true  },
  "Dd"    : { "bits": 5, "read": false, "write": true  },
  "Dd2"   : { "bits": 5, "read": false, "write": true  },
  "Ds"    : { "bits": 5, "read": true , "write": false },
  "Ds2"   : { "bits": 5, "read": true , "write": false },
  "Dn"    : { "bits": 5, "read": true , "write": false },
  "Dn2"   : { "bits": 5, "read": true , "write": false },
  "Dm"    : { "bits": 5, "read": true , "write": false },
  "Da"    : { "bits": 5, "read": true , "write": false },

  "Qx"    : { "bits": 5, "read": true , "write": true  },
  "Qx2"   : { "bits": 5, "read": true , "write": true  },
  "Qdn"   : { "bits": 5, "read": true , "write": true  },
  "Qd"    : { "bits": 5, "read": false, "write": true  },
  "Qd2"   : { "bits": 5, "read": false, "write": true  },
  "Qs"    : { "bits": 5, "read": true , "write": false },
  "Qs2"   : { "bits": 5, "read": true , "write": false },
  "Qn"    : { "bits": 5, "read": true , "write": false },
  "Qn2"   : { "bits": 5, "read": true , "write": false },
  "Qm"    : { "bits": 5, "read": true , "write": false },
  "Qa"    : { "bits": 5, "read": true , "write": false },

  "Vx"    : { "bits": 5, "read": true , "write": true  },
  "Vx2"   : { "bits": 5, "read": true , "write": true  },
  "Vdn"   : { "bits": 5, "read": true , "write": true  },
  "Vd"    : { "bits": 5, "read": false, "write": true  },
  "Vd2"   : { "bits": 5, "read": false, "write": true  },
  "Vs"    : { "bits": 5, "read": true , "write": false },
  "Vs2"   : { "bits": 5, "read": true , "write": false },
  "Vn"    : { "bits": 5, "read": true , "write": false },
  "Vm"    : { "bits": 5, "read": true , "write": false },
  "Va"    : { "bits": 5, "read": true , "write": false },

  "Zx"    : { "bits": 5, "read": true , "write": true  },
  "Zx2"   : { "bits": 5, "read": true , "write": true  },
  "Zda"   : { "bits": 5, "read": true , "write": true  },
  "Zdn"   : { "bits": 5, "read": true , "write": true  },
  "Zdn2"  : { "bits": 5, "read": true , "write": true  },
  "Zd"    : { "bits": 5, "read": false, "write": true  },
  "Zd2"   : { "bits": 5, "read": false, "write": true  },
  "Zs"    : { "bits": 5, "read": true , "write": false },
  "Zs2"   : { "bits": 5, "read": true , "write": false },
  "Zn"    : { "bits": 5, "read": true , "write": false },
  "Zm"    : { "bits": 5, "read": true , "write": false },
  "Zk"    : { "bits": 5, "read": true , "write": false },
  "Za"    : { "bits": 5, "read": true , "write": false },

  "Pdn"   : { "bits": 4, "read": true , "write": true  },
  "Pdm"   : { "bits": 4, "read": true , "write": true  },
  "Pd"    : { "bits": 4, "read": false, "write": true  },
  "Ps"    : { "bits": 4, "read": true , "write": false },
  "Pn"    : { "bits": 4, "read": true , "write": false },
  "Pm"    : { "bits": 4, "read": true , "write": false },
  "Pg"    : { "bits": 4, "read": true , "write": false }
};

arm.FieldInfo = FieldInfo;

// AArch64 utilities.
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

  static parseShiftOrExtendOp(s) {
    const space = s.indexOf(" ");
    if (space === -1)
      return "";

    const ops = s.substring(0, space).trim();
    for (let op of ops.split("|"))
      if (!/^(sop|extend|lsl|lsr|asr|uxtw|sxtw|sxtx|mul)$/.test(op))
        return "";

    return ops;
  }
}
arm.Utils = Utils;

function normalizeNumber(n) {
  return n < 0 ? 0x100000000 + n : n;
}

function decomposeOperand(s) {
  let type = null;
  let element = null;
  let consecutive = 0;
  let maskType = "";

  const elementM = s.match(/\[#(\w+)\]$/);
  if (elementM) {
    element = elementM[1];
    s = s.substring(0, s.length - elementM[0].length);
  }

  const typeM = s.match(/\.(\w+)$/);
  if (typeM) {
    type = typeM[1];
    s = s.substring(0, s.length - typeM[0].length);
  }

  const maskM = s.match(/\/(M|Z|MZ)$/);
  if (maskM) {
    maskType = maskM[1];
    s = s.substring(0, s.length - maskM[0].length);
  }

  if (s.endsWith("++")) {
    consecutive = 2;
    s = s.substring(0, s.length - 2);
  }
  else if (s.endsWith("+")) {
    consecutive = 1;
    s = s.substring(0, s.length - 1);
  }

  let m = s.match(/==|\!=|>=|<=|\*/);
  let restrict = false;

  if (m) {
    restrict = s.substring(m.index);
    s = s.substring(0, m.index);
  }

  return {
    data       : s,
    maskType   : maskType,
    type       : type,
    element    : element,
    restrict   : restrict,
    consecutive: consecutive
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

  return out.map((field)=>{return field.trim(); });
}

// asmdb.aarch64.Operand
// =====================

// ARM operand.
class Operand extends base.Operand {
  constructor(def) {
    super(def);

    // Register.
    this.sp = "";   // GP register stack access: ["", "WSP" or "SP"].
    this.mask = ""; // Masking specifier.
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
      return parseInt(this.restrict.substring(1), 10);
    else
      return 0;
  }
}
arm.Operand = Operand;

// asmdb.aarch64.Instruction
// =========================

function patternFromOperand(key) { return key; }

// ARM instruction.
class Instruction extends base.Instruction {
  constructor(db, data) {
    super(db, data);

    this.name = data.name;
    this.it = dict();                // THUMB's 'it' flags.
    this.apsr = dict();
    this.fpcsr = dict();
    this.calc = dict();              // Calculations required to generate opcode.
    this.immCond = [];               // Immediate value conditions (array of conditions).

    this._assignOperands(data.operands);
    this._assignOpcode(data.op);

    for (let k in data) {
      if (k === "name" || k == "op" || k === "operands")
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
      let key = eq === -1 ? attr : attr.substring(0, eq);
      let val = eq === -1 ? true : attr.substring(eq + 1);

      // If the key contains "|" it's a definition of multiple attributes.
      if (key.indexOf("|") !== -1) {
        const dot = key.indexOf(".");

        const base = dot === -1 ? "" : key.substring(0, dot + 1);
        const keys = (dot === -1 ? key : key.substring(dot + 1)).split("|");

        for (let j = 0; j < keys.length; j++)
          this[name][base + keys[j]] = val;
      }
      else {
        this[name][key] = val;
      }
    }
  }

  _assignOperands(s) {
    if (!s) return;

    // Split into individual operands and push them to `operands`.
    const arr = base.Parsing.splitOperands(s);
    for (let i = 0; i < arr.length; i++) {
      let def = arr[i].trim();
      const op = new Operand(def);

      const sp = def.match(/^(\w+)\|(SP|WSP)$/);
      if (sp) {
        def = sp[1];
        op.sp = sp[2];
      }

      const consecutive = def.match(/(\d+)x\{(.*)\}([+]?[+]?)/);
      if (consecutive)
        def = consecutive[2];

      op.sign = false;
      op.element = null;
      op.shiftOp = "";
      op.shiftImm = null;
      op.shiftCond = "";

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
      let shiftOp = Utils.parseShiftOrExtendOp(def);
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

          const m = part.match(/^\{(([a-z]+)(\|[a-z]+)*)\s+#(\w+)\s*(\*\s*\d+\s*)?\}$/);
          if (m) {
            op.shiftOp = m[1];
            op.shiftImm = m[2];
            if (m[3])
              op.shiftCond = m[3]
            continue;
          }

          if (i === 0) {
            op.base = dict();
            op.base.field = part;
            op.base.exp = null;

            const m = part.match(/^([A-Za-z]\w*(?:\.\w+)?)/);
            if (m && m[1].length < part.length) {
              op.base.exp = exp.parse(part);
              op.base.field = m[1];
            }
          }
          else if (part.startsWith("#")) {
            let p = part.substring(1);
            let u = "1";

            let offExp = null;
            let offMul = 1;

            if (p.startsWith("+/-")) {
              u = "U";
              p = p.substring(3);
            }

            const expMatch = p.match(/^([A-Za-z]\w*)==/);
            if (expMatch) {
              offExp = exp.parse(p);
              p = p.substring(0, expMatch[1].length);
            }

            const mulMatch = p.match(/\s*\*\s*(\d+)$/);
            if (mulMatch) {
              offMul = parseInt(mulMatch[1]);
              p = p.substring(0, mulMatch.index);
            }

            op.offset = dict();
            op.offset.field = p;
            op.offset.u = u;
            op.offset.exp = offExp;
            op.offset.mul = offMul;
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

            const m = p.match(/^([A-Za-z\|]\w*(?:\.\w+)?)/);
            if (m && m[1].length < p.length) {
              op.index.exp = exp.parse(p);
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
        // Some instructions use Reg! to specify that the register increments.
        if (def.endsWith("!")) {
          def = def.substring(0, def.length - 1)
          op.regInc = true
        }

        const obj = decomposeOperand(def);
        const reg = obj.data;

        const type = reg.substring(0, 1).toLowerCase();
        const info = FieldInfo[reg];

        if (!info)
          FAIL(`Unknown register operand '${reg}' in '${def}'`);

        op.type        = info.list ? "reg-list" : "reg";
        op.reg         = reg;                // Register name (as specified in manual).
        op.regType     = type;               // Register type.
        op.regList     = !!info.list;        // Register list.
        op.maskType    = obj.maskType;       // Mask type.
        op.elementType = obj.type            // Element type or t, ta, tb.
        op.read        = info.read;          // Register access (read).
        op.write       = info.write;         // Register access (write).
        op.element     = obj.element;        // Register element[] access.
        op.restrict    = obj.restrict;       // Register condition.
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
          opN.artificial = true;
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
      let key = arr[i].trim();
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
          key = key.substring(0, m.index).trim();
        }
        else if ((m = key.match(/\[\s*(\d+)\s*\]$/))) {
          from = parseInt(m[1], 10);
          size = 1;
          mask = 1 << from;
          key = key.substring(0, m.index).trim();
        }
        else if ((m = key.match(/\:\s*(\d+)$/))) {
          size = parseInt(m[1], 10);
          bits = size;
          key = key.substring(0, m.index).trim();
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

    // Check if the opcode value has the correct number of bits.
    if (opcodeIndex !== 32)
      FAIL(`The number of bits '${opcodeIndex}' used by the opcode '${s}' doesn't match 32`);
    this.opcodeValue = normalizeNumber(opcodeValue);
  }

  _assignSpecificAttribute(key, value) {
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

  _postProcess() {}

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

// asmdb.aarch64.ISA
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
}
arm.ISA = ISA;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "aarch64"]);
