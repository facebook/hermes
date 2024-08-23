// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

"use strict";

const fs = require("fs");
const path = require("path");

const commons = require("./generator-commons.js");
const cxx = require("./generator-cxx.js");
const core = require("./tablegen.js");

const asmdb = core.asmdb;

const DEBUG = commons.DEBUG;
const FATAL = commons.FATAL;
const kIndent = commons.kIndent;
const ArrayUtils = commons.ArrayUtils;
const IndexedArray = commons.IndexedArray;
const ObjectUtils = commons.ObjectUtils;
const StringUtils = commons.StringUtils;

const hasOwn = Object.prototype.hasOwnProperty;
const disclaimer = StringUtils.disclaimer;

const decToHex = StringUtils.decToHex;

function readJSON(fileName) {
  const content = fs.readFileSync(fileName);
  return JSON.parse(content);
}

const x86data = readJSON(path.join(__dirname, "..", "db", asmdb.x86.dbName));

// ============================================================================
// [tablegen.x86.x86isa]
// ============================================================================

// Create the X86 database and add some special cases recognized by AsmJit.
const x86isa = new asmdb.x86.ISA(x86data);

// ============================================================================
// [tablegen.x86.Filter]
// ============================================================================

class Filter {
  static unique(instArray) {
    const result = [];
    const known = {};

    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      if (inst.altForm)
        continue;

      const s = inst.operands.map((op) => { return op.isImm() ? "imm" : op.toString(); }).join(", ");
      if (known[s] === true)
        continue;

      known[s] = true;
      result.push(inst);
    }

    return result;
  }

  static noAltForm(instArray) {
    const result = [];
    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      if (inst.altForm)
        continue;
      result.push(inst);
    }
    return result;
  }

  static byArch(instArray, arch) {
    return instArray.filter(function(inst) {
      return inst.arch === "ANY" || inst.arch === arch;
    });
  }
}

// ============================================================================
// [tablegen.x86.GenUtils]
// ============================================================================

const VexToEvexMap = {
  "vbroadcastf128": "vbroadcastf32x4",
  "vbroadcasti128": "vbroadcasti32x4",
  "vextractf128": "vextractf32x4",
  "vextracti128": "vextracti32x4",
  "vinsertf128": "vinsertf32x4",
  "vinserti128": "vinserti32x4",
  "vmovdqa": "vmovdqa32",
  "vmovdqu": "vmovdqu32",
  "vpand": "vpandd",
  "vpandn": "vpandnd",
  "vpor": "vpord",
  "vpxor": "vpxord",
  "vroundpd": "vrndscalepd",
  "vroundps": "vrndscaleps",
  "vroundsd": "vrndscalesd",
  "vroundss": "vrndscaless"
};

class GenUtils {
  static cpuArchOf(dbInsts) {
    var anyArch = false;
    var x86Arch = false;
    var x64Arch = false;

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      if (dbInst.arch === "ANY") anyArch = true;
      if (dbInst.arch === "X86") x86Arch = true;
      if (dbInst.arch === "X64") x64Arch = true;
    }

    return anyArch || (x86Arch && x64Arch) ? "" : x86Arch ? "(X86)" : "(X64)";
  }

  static cpuFeaturesOf(dbInsts) {
    function cmp(a, b) {
      if (a.startsWith("AVX512") && !b.startsWith("AVX512"))
        return 1;
      if (b.startsWith("AVX512") && !a.startsWith("AVX512"))
        return -1;

      if (a.startsWith("AVX") && !b.startsWith("AVX"))
        return 1;
      if (b.startsWith("AVX") && !a.startsWith("AVX"))
        return -1;

      if (a === "FPU" && b !== "FPU")
        return 1;
      if (b === "FPU" && a !== "FPU")
        return -1;

      return a < b ? -1 : a === b ? 0 : 1;
    }

    const features = Object.getOwnPropertyNames(dbInsts.unionCpuFeatures());
    features.sort(cmp);
    return features;
  }

  static assignVexEvexCompatibilityFlags(f, dbInsts) {
    const vexInsts = dbInsts.filter((inst) => { return inst.prefix === "VEX"; });
    const evexInsts = dbInsts.filter((inst) => { return inst.prefix === "EVEX"; });

    function isCompatible(vexInst, evexInst) {
      if (vexInst.operands.length !== evexInst.operands.length)
        return false;

      for (let i = 0; i < vexInst.operands.length; i++) {
        const vexOp = vexInst.operands[i];
        const evexOp = evexInst.operands[i];

        if (vexOp.data === evexOp.data)
          continue;

        if (vexOp.reg && vexOp.reg === evexOp.reg)
          continue;
        if (vexOp.mem && vexOp.mem === evexOp.mem)
          continue;

        return false;
      }
      return true;
    }

    let compatible = 0;
    for (const vexInst of vexInsts) {
      for (const evexInst of evexInsts) {
        if (isCompatible(vexInst, evexInst)) {
          compatible++;
          break;
        }
      }
    }

    if (compatible == vexInsts.length) {
      f.EvexCompat = true;
      return true;
    }

    if (evexInsts[0].operands[0].reg === "k") {
      f.EvexKReg = true;
      return true;
    }

    if (evexInsts[0].operands.length == 2 && vexInsts[0].operands.length === 3) {
      f.EvexTwoOp = true;
      return true;
    }

    return false;
  }

  static flagsOf(dbInsts) {
    const f = Object.create(null);
    var i, j;

    var mib = dbInsts.length > 0 && /^(?:bndldx|bndstx)$/.test(dbInsts[0].name);
    if (mib)
      f.Mib = true;

    var mmx = false;
    var vec = false;

    for (i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const operands = dbInst.operands;

      if (dbInst.name === "emms")
        mmx = true;

      if (dbInst.name === "vzeroall" || dbInst.name === "vzeroupper")
        vec = true;

      for (j = 0; j < operands.length; j++) {
        const op = operands[j];
        if (op.reg === "mm")
          mmx = true;
        else if (/^(xmm|ymm|zmm)$/.test(op.reg)) {
          vec = true;
        }
      }
    }

    if (mmx) f.Mmx = true;
    if (vec) f.Vec = true;

    for (i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const operands = dbInst.operands;

      if (dbInst.prefixes.lock           ) f.Lock            = true;
      if (dbInst.prefixes.xacquire       ) f.XAcquire        = true;
      if (dbInst.prefixes.xrelease       ) f.XRelease        = true;
      if (dbInst.prefixes.bnd            ) f.Rep             = true;
      if (dbInst.prefixes.rep            ) f.Rep             = true;
      if (dbInst.prefixes.repne          ) f.Rep             = true;
      if (dbInst.prefixes.repIgnore      ) f.RepIgnored      = true;
      if (dbInst.k === "zeroing"         ) f.Avx512ImplicitZ = true;

      if (dbInst.category.FPU) {
        for (var j = 0; j < operands.length; j++) {
          const op = operands[j];
          if (op.memSize === 16) f.FpuM16 = true;
          if (op.memSize === 32) f.FpuM32 = true;
          if (op.memSize === 64) f.FpuM64 = true;
          if (op.memSize === 80) f.FpuM80 = true;
        }
      }

      if (dbInst.tsib)
        f.Tsib = true;

      if (dbInst.vsibReg)
        f.Vsib = true;

      if (dbInst.prefix === "VEX" || dbInst.prefix === "XOP")
        f.Vex = true;

      if (dbInst.encodingPreference === "EVEX")
        f.PreferEvex = true;

      if (dbInst.prefix === "EVEX") {
        f.Evex = true;
        if (dbInst.kmask) f.Avx512K = true;
        if (dbInst.zmask) f.Avx512Z = true;

        if (dbInst.er) f.Avx512ER = true;
        if (dbInst.sae) f.Avx512SAE = true;

        if (dbInst.broadcast) f["Avx512B" + String(dbInst.elementSize)] = true;
        if (dbInst.tupleType === "T1_4X") f.Avx512T4X = true;
      }

      if (VexToEvexMap[dbInst.name])
        f.EvexTransformable = true;
    }

    if (f.Vex && f.Evex) {
      GenUtils.assignVexEvexCompatibilityFlags(f, dbInsts)
    }

    const result = Object.getOwnPropertyNames(f);
    result.sort();
    return result;
  }

  static eqOps(aOps, aFrom, bOps, bFrom) {
    var x = 0;
    for (;;) {
      const aIndex = x + aFrom;
      const bIndex = x + bFrom;

      const aOut = aIndex >= aOps.length;
      const bOut = bIndex >= bOps.length;

      if (aOut || bOut)
        return !!(aOut && bOut);

      const aOp = aOps[aIndex];
      const bOp = bOps[bIndex];

      if (aOp.data !== bOp.data)
        return false;

      x++;
    }
  }

  // Prevent some instructions from having implicit memory size if that would
  // make them ambiguous. There are some instructions where the ambiguity is
  // okay, but some like 'push' and 'pop' where it isn't.
  static canUseImplicitMemSize(name) {
    switch (name) {
      case "pop":
      case "push":
        return false;

      default:
        return true;
    }
  }

  static singleRegCase(name) {
    switch (name) {
      case "xchg"    :

      case "and"     :
      case "pand"    : case "vpand"  : case "vpandd"  : case "vpandq"   :
      case "andpd"   : case "vandpd" :
      case "andps"   : case "vandps" :

      case "or"      :
      case "por"     : case "vpor"   : case "vpord"   : case "vporq"    :
      case "orpd"    : case "vorpd"  :
      case "orps"    : case "vorps"  :

      case "pminsb"  : case "vpminsb": case "pmaxsb"  : case "vpmaxsb"  :
      case "pminsw"  : case "vpminsw": case "pmaxsw"  : case "vpmaxsw"  :
      case "pminsd"  : case "vpminsd": case "pmaxsd"  : case "vpmaxsd"  :
      case "pminub"  : case "vpminub": case "pmaxub"  : case "vpmaxub"  :
      case "pminuw"  : case "vpminuw": case "pmaxuw"  : case "vpmaxuw"  :
      case "pminud"  : case "vpminud": case "pmaxud"  : case "vpmaxud"  :
        return "RO";

      case "pandn"   : case "vpandn" : case "vpandnd" : case "vpandnq"  :

      case "xor"     :
      case "pxor"    : case "vpxor"  : case "vpxord"  : case "vpxorq"   :
      case "xorpd"   : case "vxorpd" :
      case "xorps"   : case "vxorps" :

      case "kxnorb":
      case "kxnord":
      case "kxnorw":
      case "kxnorq":

      case "kxorb":
      case "kxord":
      case "kxorw":
      case "kxorq":

      case "sub"     :
      case "sbb"     :
      case "psubb"   : case "vpsubb" :
      case "psubw"   : case "vpsubw" :
      case "psubd"   : case "vpsubd" :
      case "psubq"   : case "vpsubq" :
      case "psubsb"  : case "vpsubsb": case "psubusb" : case "vpsubusb" :
      case "psubsw"  : case "vpsubsw": case "psubusw" : case "vpsubusw" :

      case "vpcmpeqb": case "pcmpeqb": case "vpcmpgtb": case "pcmpgtb"  :
      case "vpcmpeqw": case "pcmpeqw": case "vpcmpgtw": case "pcmpgtw"  :
      case "vpcmpeqd": case "pcmpeqd": case "vpcmpgtd": case "pcmpgtd"  :
      case "vpcmpeqq": case "pcmpeqq": case "vpcmpgtq": case "pcmpgtq"  :

      case "vpcmpb"  : case "vpcmpub":
      case "vpcmpd"  : case "vpcmpud":
      case "vpcmpw"  : case "vpcmpuw":
      case "vpcmpq"  : case "vpcmpuq":
        return "WO";

      default:
        return "None";
    }
  }

  static fixedRegOfRegName(reg) {
    switch (reg) {
      case "es"  : return 1;
      case "cs"  : return 2;
      case "ss"  : return 3;
      case "ds"  : return 4;
      case "fs"  : return 5;
      case "gs"  : return 6;
      case "ah"  : return 0;
      case "ch"  : return 1;
      case "dh"  : return 2;
      case "bh"  : return 3;
      case "al"  : case "ax": case "eax": case "rax": case "zax": return 0;
      case "cl"  : case "cx": case "ecx": case "rcx": case "zcx": return 1;
      case "dl"  : case "dx": case "edx": case "rdx": case "zdx": return 2;
      case "bl"  : case "bx": case "ebx": case "rbx": case "zbx": return 3;
      case "spl" : case "sp": case "esp": case "rsp": case "zsp": return 4;
      case "bpl" : case "bp": case "ebp": case "rbp": case "zbp": return 5;
      case "sil" : case "si": case "esi": case "rsi": case "zsi": return 6;
      case "dil" : case "di": case "edi": case "rdi": case "zdi": return 7;
      case "st0" : return 0;
      case "xmm0": return 0;
      case "ymm0": return 0;
      case "zmm0": return 0;
      default:
        return -1;
    }
  }

  static fixedRegOf(op) {
    if (op.isReg()) {
      return GenUtils.fixedRegOfRegName(op.reg);
    }
    else if (op.isMem() && op.memRegOnly) {
      return GenUtils.fixedRegOfRegName(op.memRegOnly);
    }
    else {
      return -1;
    }
  }

  static controlFlow(dbInsts) {
    if (dbInsts.checkAttribute("control", "jump")) return "Jump";
    if (dbInsts.checkAttribute("control", "call")) return "Call";
    if (dbInsts.checkAttribute("control", "branch")) return "Branch";
    if (dbInsts.checkAttribute("control", "return")) return "Return";
    return "Regular";
  }
}

// ============================================================================
// [tablegen.x86.X86TableGen]
// ============================================================================

class X86TableGen extends core.TableGen {
  constructor() {
    super("X86");

    this.emitMissingString = "";
  }

  // --------------------------------------------------------------------------
  // [Query]
  // --------------------------------------------------------------------------

  // Get instructions (dbInsts) having the same name as understood by AsmJit.
  query(name) {
    return x86isa.query(name);
  }

  // --------------------------------------------------------------------------
  // [Parse / Merge]
  // --------------------------------------------------------------------------

  parse() {
    const data = this.dataOfFile("src/asmjit/x86/x86instdb.cpp");
    const re = new RegExp(
      "INST\\(" +
        "([A-Za-z0-9_]+)\\s*"              + "," +  // [01] Instruction.
        "([^,]+)"                          + "," +  // [02] Encoding.
        "(.{26}[^,]*)"                     + "," +  // [03] Opcode[0].
        "(.{26}[^,]*)"                     + "," +  // [04] Opcode[1].
        // --- autogenerated fields ---
        "([^\\)]+)"                        + "," +  // [05] MainOpcodeIndex.
        "([^\\)]+)"                        + "," +  // [06] AltOpcodeIndex.
        "([^\\)]+)"                        + "," +  // [07] CommonDataIndex.
        "([^\\)]+)"                        + "\\)", // [08] OperationDataIndex.
      "g");

    var m;
    while ((m = re.exec(data)) !== null) {
      var enum_       = m[1];
      var name        = enum_ === "None" ? "" : enum_.toLowerCase();
      var encoding    = m[2].trim();
      var opcode0     = m[3].trim();
      var opcode1     = m[4].trim();

      const dbInsts = this.query(name);
      if (name && !dbInsts.length)
        FATAL(`Instruction '${name}' not found in asmdb`);

      const flags         = GenUtils.flagsOf(dbInsts);
      const controlFlow   = GenUtils.controlFlow(dbInsts);
      const singleRegCase = GenUtils.singleRegCase(name);

      this.addInst({
        id                 : 0,             // Instruction id (numeric value).
        name               : name,          // Instruction name.
        displayName        : name,          // Instruction name to display.
        enum               : enum_,         // Instruction enum without `kId` prefix.
        dbInsts            : dbInsts,       // All dbInsts returned from asmdb query.
        encoding           : encoding,      // Instruction encoding.
        opcode0            : opcode0,       // Primary opcode.
        opcode1            : opcode1,       // Secondary opcode.
        flags              : flags,
        signatures         : null,          // Instruction signatures.
        controlFlow        : controlFlow,
        singleRegCase      : singleRegCase,

        mainOpcodeValue    : -1,            // Main opcode value (0.255 hex).
        mainOpcodeIndex    : -1,            // Index to InstDB::_mainOpcodeTable.
        altOpcodeIndex     : -1,            // Index to InstDB::_altOpcodeTable.
        nameIndex          : -1,            // Index to InstDB::_nameData.
        commonInfoIndex    : -1,
        additionalInfoIndex: -1,

        signatureIndex     : -1,
        signatureCount     : -1
      });
    }

    if (this.insts.length === 0)
      FATAL("X86TableGen.parse(): Invalid parsing regexp (no data parsed)");

    console.log("Number of Instructions: " + this.insts.length);
  }

  merge() {
    var s = StringUtils.format(this.insts, "", true, function(inst) {
      return "INST(" +
        String(inst.enum               ).padEnd(17) + ", " +
        String(inst.encoding           ).padEnd(19) + ", " +
        String(inst.opcode0            ).padEnd(26) + ", " +
        String(inst.opcode1            ).padEnd(26) + ", " +
        String(inst.mainOpcodeIndex    ).padEnd( 3) + ", " +
        String(inst.altOpcodeIndex     ).padEnd( 3) + ", " +
        String(inst.commonInfoIndex    ).padEnd( 3) + ", " +
        String(inst.additionalInfoIndex).padEnd( 3) + ")";
    }) + "\n";
    this.inject("InstInfo", s, this.insts.length * 8);
  }

  // --------------------------------------------------------------------------
  // [Other]
  // --------------------------------------------------------------------------

  printMissing() {
    const ignored = ArrayUtils.toDict([
      "cmpsb", "cmpsw", "cmpsd", "cmpsq",
      "lodsb", "lodsw", "lodsd", "lodsq",
      "movsb", "movsw", "movsd", "movsq",
      "scasb", "scasw", "scasd", "scasq",
      "stosb", "stosw", "stosd", "stosq",
      "insb" , "insw" , "insd" ,
      "outsb", "outsw", "outsd",
      "wait" // Maps to `fwait`, which AsmJit uses instead.
    ]);

    var out = "";
    x86isa.instructionNames.forEach(function(name) {
      var dbInsts = x86isa.query(name);
      if (!this.instMap[name] && ignored[name] !== true) {
        console.log(`MISSING INSTRUCTION '${name}'`);
        var inst = this.newInstFromGroup(dbInsts);
        if (inst) {
          out += "  INST(" +
            String(inst.enum      ).padEnd(17) + ", " +
            String(inst.encoding  ).padEnd(19) + ", " +
            String(inst.opcode0   ).padEnd(26) + ", " +
            String(inst.opcode1   ).padEnd(26) + ", " +
            String("0"            ).padEnd( 3) + ", " +
            String("0"            ).padEnd( 3) + ", " +
            String("0"            ).padEnd( 5) + ", " +
            String("0"            ).padEnd( 3) + ", " +
            String("0"            ).padEnd( 3) + "),\n";
        }
      }
    }, this);
    console.log(out);
    console.log(this.emitMissingString);
  }

  newInstFromGroup(dbInsts) {
    function composeOpCode(obj) {
      return `${obj.type}(${obj.prefix},${obj.opcode},${obj.o},${obj.l},${obj.w},${obj.ew},${obj.en},${obj.tt})`;
    }

    function GetAccess(dbInst) {
      var operands = dbInst.operands;
      if (!operands.length) return "";

      var op = operands[0];
      if (op.read && op.write)
        return "RW";
      else if (op.read)
        return "RO";
      else
        return "WO";
    }

    function isVecPrefix(s) {
      return s === "VEX" || s === "EVEX" || s === "XOP";
    }

    function formatEmit(dbi) {
      const results = [];
      const nameUp = dbi.name[0].toUpperCase() + dbi.name.substr(1);

      for (let choice = 0; choice < 2; choice++) {
        let s = `ASMJIT_INST_${dbi.operands.length}x(${dbi.name}, ${nameUp}`;
        for (let j = 0; j < dbi.operands.length; j++) {
          s += ", ";
          const op = dbi.operands[j];
          var reg = op.reg;
          var mem = op.mem;

          if (op.isReg() && op.isMem()) {
            if (choice == 0) mem = null;
            if (choice == 1) reg = null;
          }

          if (reg) {
            if (reg === "xmm" || reg === "ymm" || reg === "zmm")
              s += "Vec";
            else if (reg === "k")
              s += "KReg";
            else if (reg === "r32" || reg === "r64" || reg === "r16" || reg === "r8")
              s += "Gp";
            else
              s += reg;
          }
          else if (mem) {
            s += "Mem";
          }
          else if (op.isImm()) {
            s += "Imm";
          }
          else {
            s += "Unknown";
          }
        }
        s += `)`;
        results.push(s);
      }

      return results;
    }

    var dbi = dbInsts[0];

    var id = this.insts.length;
    var name = dbi.name;
    var enum_ = name[0].toUpperCase() + name.substr(1);

    var opcode = dbi.opcodeHex;
    var modR = dbi.modR;
    var mm = dbi.mm;
    var pp = dbi.pp;
    var encoding = dbi.encoding;
    var isVec = isVecPrefix(dbi.prefix);
    var evexCount = 0;

    var access = GetAccess(dbi);

    var vexL = undefined;
    var vexW = undefined;
    var evexW = undefined;
    var cdshl = "_";
    var tupleType = "_";

    const tupleTypeToCDSHL = {
      "FVM": "4",
      "FV": "4",
      "HVM": "3",
      "HV": "3",
      "QVM": "2",
      "QV": "2",
      "T1S": "?"
    }

    const emitMap = {};

    for (var i = 0; i < dbInsts.length; i++) {
      dbi = dbInsts[i];

      if (dbi.prefix === "VEX" || dbi.prefix === "XOP") {
        var newVexL = String(dbi.l === "128" ? 0 : dbi.l === "256" ? 1 : dbi.l === "512" ? 2 : "_");
        var newVexW = String(dbi.w === "W0" ? 0 : dbi.w === "W1" ? 1 : "_");

        if (vexL !== undefined && vexL !== newVexL)
          vexL = "x";
        else
          vexL = newVexL;
        if (vexW !== undefined && vexW !== newVexW)
          vexW = "x";
        else
          vexW = newVexW;
      }

      if (dbi.prefix === "EVEX") {
        evexCount++;
        var newEvexW = String(dbi.w === "W0" ? 0 : dbi.w === "W1" ? 1 : "_");
        if (evexW !== undefined && evexW !== newEvexW)
          evexW = "x";
        else
          evexW = newEvexW;

        if (dbi.tupleType) {
          if (tupleType !== "_" && tupleType !== dbi.tupleType) {
            console.log(`${dbi.name}: WARNING: TupleType ${tupleType} != ${dbi.tupleType}`);
          }

          tupleType = dbi.tupleType;
        }
      }

      if (opcode   !== dbi.opcodeHex ) { console.log(`${dbi.name}: ISSUE: Opcode ${opcode} != ${dbi.opcodeHex}`); return null; }
      if (modR     !== dbi.modR      ) { console.log(`${dbi.name}: ISSUE: ModR ${modR} != ${dbi.modR}`); return null; }
      if (mm       !== dbi.mm        ) { console.log(`${dbi.name}: ISSUE: MM ${mm} != ${dbi.mm}`); return null; }
      if (pp       !== dbi.pp        ) { console.log(`${dbi.name}: ISSUE: PP ${pp} != ${dbi.pp}`); return null; }
      if (encoding !== dbi.encoding  ) { console.log(`${dbi.name}: ISSUE: Enc ${encoding} != ${dbi.encoding}`); return null; }
      if (access   !== GetAccess(dbi)) { console.log(`${dbi.name}: ISSUE: Access ${access} != ${GetAccess(dbi)}`); return null; }
      if (isVec    != isVecPrefix(dbi.prefix)) { console.log(`${dbi.name}: ISSUE: Vex/Non-Vex mismatch`); return null; }

      formatEmit(dbi).forEach((emit) => {
        if (!emitMap[emit]) {
          emitMap[emit] = true;
          this.emitMissingString += emit + "\n";
        }
      });
    }

    if (tupleType !== "_")
      cdshl = tupleTypeToCDSHL[tupleType] || "?";

    var ppmm = pp.padEnd(2).replace(/ /g, "0") +
               mm.padEnd(4).replace(/ /g, "0") ;

    var composed = composeOpCode({
      type  : evexCount == dbInsts.length ? "E" : isVec ? "V" : "O",
      prefix: ppmm,
      opcode: opcode,
      o     : modR === "r" ? "_" : (modR ? modR : "_"),
      l     : vexL !== undefined ? vexL : "_",
      w     : vexW !== undefined ? vexW : "_",
      ew    : evexW !== undefined ? evexW : "_",
      en    : cdshl,
      tt    : dbi.modRM ? dbi.modRM + "  " : tupleType.padEnd(3)
    });

    return {
      id                 : id,
      name               : name,
      enum               : enum_,
      encoding           : encoding,
      opcode0            : composed,
      opcode1            : "0",
      nameIndex          : -1,
      commonInfoIndex    : -1,
      additionalInfoIndex: -1
    };
  }

  // --------------------------------------------------------------------------
  // [Hooks]
  // --------------------------------------------------------------------------

  onBeforeRun() {
    this.load([
      "src/asmjit/x86/x86globals.h",
      "src/asmjit/x86/x86instdb.cpp",
      "src/asmjit/x86/x86instdb.h",
      "src/asmjit/x86/x86instdb_p.h"
    ]);
    this.parse();
  }

  onAfterRun() {
    this.merge();
    this.save();
    this.dumpTableSizes();
    this.printMissing();
  }
}

// ============================================================================
// [tablegen.x86.IdEnum]
// ============================================================================

class IdEnum extends core.IdEnum {
  constructor() {
    super("IdEnum");
  }

  comment(inst) {
    function filterAVX(features, avx) {
      return features.filter(function(item) { return /^(AVX|FMA)/.test(item) === avx; });
    }

    var dbInsts = inst.dbInsts;
    if (!dbInsts.length) return "Invalid instruction id.";

    var text = "";
    var features = GenUtils.cpuFeaturesOf(dbInsts);

    const priorityFeatures = ["AVX_VNNI", "AVX_VNNI_INT8", "AVX_IFMA", "AVX_NE_CONVERT"];

    if (features.length) {
      text += "{";
      const avxFeatures = filterAVX(features, true);
      const otherFeatures = filterAVX(features, false);

      for (const pf of priorityFeatures) {
        const index = avxFeatures.indexOf(pf);
        if (index != -1) {
          avxFeatures.splice(index, 1);
          avxFeatures.unshift(pf);
        }
      }

      const vl = avxFeatures.indexOf("AVX512_VL");
      if (vl !== -1) avxFeatures.splice(vl, 1);

      const fma = avxFeatures.indexOf("FMA");
      if (fma !== -1) { avxFeatures.splice(fma, 1); avxFeatures.splice(0, 0, "FMA"); }

      text += avxFeatures.join("|");
      if (vl !== -1) text += "+VL";

      if (otherFeatures.length)
        text += (avxFeatures.length ? " & " : "") + otherFeatures.join("|");

      text += "}";
    }

    var arch = GenUtils.cpuArchOf(dbInsts);
    if (arch)
      text += (text ? " " : "") + arch;

    return `Instruction '${inst.name}'${(text ? " " + text : "")}.`;
  }
}

// ============================================================================
// [tablegen.x86.NameTable]
// ============================================================================

class NameTable extends core.NameTable {
  constructor() {
    super("NameTable");
  }
}

// ============================================================================
// [tablegen.x86.AltOpcodeTable]
// ============================================================================

class AltOpcodeTable extends core.Task {
  constructor() {
    super("AltOpcodeTable");
  }

  run() {
    const insts = this.ctx.insts;

    const mainOpcodeTable = new IndexedArray();
    const altOpcodeTable = new IndexedArray();

    const cdttSimplification = {
      "0"    : "None",
      "_"    : "None",
      "FV"   : "ByLL",
      "HV"   : "ByLL",
      "QV"   : "ByLL",
      "FVM"  : "ByLL",
      "T1S"  : "None",
      "T1F"  : "None",
      "T1_4X": "None",
      "T2"   : "None",
      "T4"   : "None",
      "T8"   : "None",
      "HVM"  : "ByLL",
      "QVM"  : "ByLL",
      "OVM"  : "ByLL",
      "128"  : "None",
      "T4X"  : "None"
    }

    const noOp = "O(000000,00,0,0,0,0,0,0   )";

    mainOpcodeTable.addIndexed(noOp);

    function splitOpcodeToComponents(opcode) {
      const i = opcode.indexOf("(");
      const prefix = opcode.substr(0, i);
      return [prefix].concat(opcode.substring(i + 1, opcode.length - 1).split(","));
    }

    function normalizeOpcodeComponents(components) {
      for (let i = 1; i < components.length; i++) {
        components[i] = components[i].trim();
        // These all are zeros that only have some contextual meaning in the table, but the assembler doesn't care.
        if (components[i] === "_" || components[i] === "I" || components[i] === "x")
          components[i] = "0";
      }

      // Simplify CDTT (compressed displacement TupleType).
      if (components.length >= 9) {
        if (components[0] === "V" || components[0] === "E") {
          const cdtt = components[8];
          if (cdttSimplification[cdtt] !== undefined)
            components[8] = cdttSimplification[cdtt];
        }
      }
      return components;
    }

    function joinOpcodeComponents(components) {
      const prefix = components[0];
      const values = components.slice(1);
      if (values.length >= 8)
        values[7] = values[7].padEnd(4);
      return prefix + "(" + values.join(",") + ")";
    }

    function indexMainOpcode(opcode) {
      if (opcode === "0")
        return ["00", 0];

      var opcodeByte = "";
      const components = normalizeOpcodeComponents(splitOpcodeToComponents(opcode));

      if (components[0] === "O_FPU") {
        // Reset opcode byte, this is stored in the instruction data itself.
        opcodeByte = components[2].substr(2, 2);
        components[2] = components[2].substr(0, 2) + "00";
      }
      else if (components[0] === "O" || components[0] === "V" || components[0] === "E") {
        // Reset opcode byte, this is stored in the instruction data itself.
        opcodeByte = components[2];
        components[2] = "00";
      }
      else {
        FATAL(`Failed to process opcode '${opcode}'`);
      }

      const newOpcode = joinOpcodeComponents(components);
      return [opcodeByte, mainOpcodeTable.addIndexed(newOpcode.padEnd(27))];
    }

    function indexAltOpcode(opcode) {
      if (opcode === "0")
        opcode = noOp;
      else
        opcode = joinOpcodeComponents(normalizeOpcodeComponents(splitOpcodeToComponents(opcode)));
      return altOpcodeTable.addIndexed(opcode.padEnd(27));
    }

    insts.map((inst) => {
      const [value, index] = indexMainOpcode(inst.opcode0);
      inst.mainOpcodeValue = value;
      inst.mainOpcodeIndex = index;
      inst.altOpcodeIndex = indexAltOpcode(inst.opcode1);
    });

    // console.log(mainOpcodeTable.length);
    // console.log(StringUtils.format(mainOpcodeTable, kIndent, true));

    this.inject("MainOpcodeTable",
                disclaimer(`const uint32_t InstDB::_mainOpcodeTable[] = {\n${StringUtils.format(mainOpcodeTable, kIndent, true)}\n};\n`),
                mainOpcodeTable.length * 4);

    this.inject("AltOpcodeTable",
                disclaimer(`const uint32_t InstDB::_altOpcodeTable[] = {\n${StringUtils.format(altOpcodeTable, kIndent, true)}\n};\n`),
                altOpcodeTable.length * 4);
  }
}

// ============================================================================
// [tablegen.x86.InstSignatureTable]
// ============================================================================

const RegOp = ArrayUtils.toDict(["al", "ah", "ax", "eax", "rax", "cl", "r8lo", "r8hi", "r16", "r32", "r64", "xmm", "ymm", "zmm", "mm", "k", "sreg", "creg", "dreg", "st", "bnd"]);
const MemOp = ArrayUtils.toDict(["m8", "m16", "m32", "m48", "m64", "m80", "m128", "m256", "m512", "m1024"]);

const cmpOp = StringUtils.makePriorityCompare([
  "RegGpbLo", "RegGpbHi", "RegGpw", "RegGpd", "RegGpq", "RegXmm", "RegYmm", "RegZmm", "RegMm", "RegKReg", "RegSReg", "RegCReg", "RegDReg", "RegSt", "RegBnd", "RegTmm",
  "MemUnspecified", "Mem8", "Mem16", "Mem32", "Mem48", "Mem64", "Mem80", "Mem128", "Mem256", "Mem512", "Mem1024",
  "Vm32x", "Vm32y", "Vm32z", "Vm64x", "Vm64y", "Vm64z",
  "ImmI4", "ImmU4", "ImmI8", "ImmU8", "ImmI16", "ImmU16", "ImmI32", "ImmU32", "ImmI64", "ImmU64",
  "Rel8", "Rel32",
  "FlagMemBase",
  "FlagMemDs",
  "FlagMemEs",
  "FlagMib",
  "FlagTMem",
  "FlagConsecutive",
  "FlagImplicit"
]);

function StringifyOpArray(a, map) {
  var s = "";
  for (var i = 0; i < a.length; i++) {
    const op = a[i];
    var mapped = null;
    if (typeof map === "function")
      mapped = map(op);
    else if (hasOwn.call(map, op))
      mapped = map[op];
    else
      FATAL(`UNHANDLED OPERAND '${op}'`);
    s += (s ? " | " : "") + mapped;
  }
  return s ? s : "0";
}

class OSignature {
  constructor() {
    this.flags = Object.create(null);
  }

  equals(other) {
    return ObjectUtils.equals(this.flags, other.flags);
  }

  xor(other) {
    const result = ObjectUtils.xor(this.flags, other.flags);
    return Object.getOwnPropertyNames(result).length === 0 ? null : result;
  }

  mergeWith(other) {
    const af = this.flags;
    const bf = other.flags;

    var k;
    var indexKind = "";
    var hasReg = false;

    for (k in af) {
      const index = asmdb.x86.Utils.regIndexOf(k);
      const kind = asmdb.x86.Utils.regKindOf(k);

      if (kind)
        hasReg = true;

      if (index !== null && index !== -1)
        indexKind = kind;
    }

    if (hasReg) {
      for (k in bf) {
        const index = asmdb.x86.Utils.regIndexOf(k);
        if (index !== null && index !== -1) {
          const kind = asmdb.x86.Utils.regKindOf(k);
          if (indexKind !== kind)
            return false;
        }
      }
    }

    // Can merge...
    for (k in bf)
      af[k] = true;
    return true;
  }

  toString() {
    var s = "";
    var flags = this.flags;

    for (var k in flags) {
      if (k === "read" || k === "write" || k === "implicit" || k === "memDS" || k === "memES")
        continue;

      var x = k;
      if (x === "memZAX") x = "zax";
      if (x === "memZDI") x = "zdi";
      if (x === "memZSI") x = "zsi";
      s += (s ? "|" : "") + x;
    }

    if (flags.memDS) s = "ds:[" + s + "]";
    if (flags.memES) s = "es:[" + s + "]";

    if (flags.implicit)
      s = "<" + s + ">";

    return s;
  }

  toAsmJitOpData() {
    var opFlags = Object.create(null);
    var regMask = 0;

    for (var k in this.flags) {
      switch (k) {
        case "r8lo"    : opFlags.RegGpbLo = true; break;
        case "r8hi"    : opFlags.RegGpbHi = true; break;
        case "r16"     : opFlags.RegGpw = true; break;
        case "r32"     : opFlags.RegGpd = true; break;
        case "r64"     : opFlags.RegGpq = true; break;
        case "creg"    : opFlags.RegCReg = true; break;
        case "dreg"    : opFlags.RegDReg = true; break;
        case "sreg"    : opFlags.RegSReg = true; break;
        case "bnd"     : opFlags.RegBnd = true; break;
        case "st"      : opFlags.RegSt = true; break;
        case "k"       : opFlags.RegKReg = true; break;
        case "mm"      : opFlags.RegMm = true; break;
        case "xmm"     : opFlags.RegXmm = true; break;
        case "ymm"     : opFlags.RegYmm = true; break;
        case "zmm"     : opFlags.RegZmm = true; break;
        case "tmm"     : opFlags.RegTmm = true; break;

        case "m8"      : opFlags.Mem8 = true; break;
        case "m16"     : opFlags.Mem16 = true; break;
        case "m32"     : opFlags.Mem32 = true; break;
        case "m48"     : opFlags.Mem48 = true; break;
        case "m64"     : opFlags.Mem64 = true; break;
        case "m80"     : opFlags.Mem80 = true; break;
        case "m128"    : opFlags.Mem128 = true; break;
        case "m256"    : opFlags.Mem256 = true; break;
        case "m512"    : opFlags.Mem512 = true; break;
        case "m1024"   : opFlags.Mem1024 = true; break;

        case "mem"     : opFlags.MemUnspecified = true; break;
        case "mib"     : opFlags.MemUnspecified = true; opFlags.FlagMib = true; break;
        case "tmem"    : opFlags.MemUnspecified = true; opFlags.FlagTMem = true; break;

        case "memBase" : opFlags.FlagMemBase = true; break;
        case "memDS"   : opFlags.FlagMemDs = true; break;
        case "memES"   : opFlags.FlagMemEs = true; break;
        case "memZAX"  : regMask |= 1 << 0; break;
        case "memZSI"  : regMask |= 1 << 6; break;
        case "memZDI"  : regMask |= 1 << 7; break;

        case "vm32x"   : opFlags.Vm32x = true; break;
        case "vm32y"   : opFlags.Vm32y = true; break;
        case "vm32z"   : opFlags.Vm32z = true; break;
        case "vm64x"   : opFlags.Vm64x = true; break;
        case "vm64y"   : opFlags.Vm64y = true; break;
        case "vm64z"   : opFlags.Vm64z = true; break;

        case "i4"      : opFlags.ImmI4 = true; break;
        case "u4"      : opFlags.ImmU4 = true; break;
        case "i8"      : opFlags.ImmI8 = true; break;
        case "u8"      : opFlags.ImmU8 = true; break;
        case "i16"     : opFlags.ImmI16 = true; break;
        case "u16"     : opFlags.ImmU16 = true; break;
        case "i32"     : opFlags.ImmI32 = true; break;
        case "u32"     : opFlags.ImmU32 = true; break;
        case "i64"     : opFlags.ImmI64 = true; break;
        case "u64"     : opFlags.ImmU64 = true; break;

        case "rel8"    : opFlags.ImmI32 = true; opFlags.ImmI64 = true; opFlags.Rel8  = true; break;
        case "rel16"   : opFlags.ImmI32 = true; opFlags.ImmI64 = true; opFlags.Rel32 = true; break;
        case "rel32"   : opFlags.ImmI32 = true; opFlags.ImmI64 = true; opFlags.Rel32 = true; break;

        case "es"      : opFlags.RegSReg  = true; regMask |= 1 << 1; break;
        case "cs"      : opFlags.RegSReg  = true; regMask |= 1 << 2; break;
        case "ss"      : opFlags.RegSReg  = true; regMask |= 1 << 3; break;
        case "ds"      : opFlags.RegSReg  = true; regMask |= 1 << 4; break;
        case "fs"      : opFlags.RegSReg  = true; regMask |= 1 << 5; break;
        case "gs"      : opFlags.RegSReg  = true; regMask |= 1 << 6; break;
        case "al"      : opFlags.RegGpbLo = true; regMask |= 1 << 0; break;
        case "ah"      : opFlags.RegGpbHi = true; regMask |= 1 << 0; break;
        case "ax"      : opFlags.RegGpw   = true; regMask |= 1 << 0; break;
        case "eax"     : opFlags.RegGpd   = true; regMask |= 1 << 0; break;
        case "rax"     : opFlags.RegGpq   = true; regMask |= 1 << 0; break;
        case "cl"      : opFlags.RegGpbLo = true; regMask |= 1 << 1; break;
        case "ch"      : opFlags.RegGpbHi = true; regMask |= 1 << 1; break;
        case "cx"      : opFlags.RegGpw   = true; regMask |= 1 << 1; break;
        case "ecx"     : opFlags.RegGpd   = true; regMask |= 1 << 1; break;
        case "rcx"     : opFlags.RegGpq   = true; regMask |= 1 << 1; break;
        case "dl"      : opFlags.RegGpbLo = true; regMask |= 1 << 2; break;
        case "dh"      : opFlags.RegGpbHi = true; regMask |= 1 << 2; break;
        case "dx"      : opFlags.RegGpw   = true; regMask |= 1 << 2; break;
        case "edx"     : opFlags.RegGpd   = true; regMask |= 1 << 2; break;
        case "rdx"     : opFlags.RegGpq   = true; regMask |= 1 << 2; break;
        case "bl"      : opFlags.RegGpbLo = true; regMask |= 1 << 3; break;
        case "bh"      : opFlags.RegGpbHi = true; regMask |= 1 << 3; break;
        case "bx"      : opFlags.RegGpw   = true; regMask |= 1 << 3; break;
        case "ebx"     : opFlags.RegGpd   = true; regMask |= 1 << 3; break;
        case "rbx"     : opFlags.RegGpq   = true; regMask |= 1 << 3; break;
        case "si"      : opFlags.RegGpw   = true; regMask |= 1 << 6; break;
        case "esi"     : opFlags.RegGpd   = true; regMask |= 1 << 6; break;
        case "rsi"     : opFlags.RegGpq   = true; regMask |= 1 << 6; break;
        case "di"      : opFlags.RegGpw   = true; regMask |= 1 << 7; break;
        case "edi"     : opFlags.RegGpd   = true; regMask |= 1 << 7; break;
        case "rdi"     : opFlags.RegGpq   = true; regMask |= 1 << 7; break;
        case "st0"     : opFlags.RegSt    = true; regMask |= 1 << 0; break;
        case "xmm0"    : opFlags.RegXmm   = true; regMask |= 1 << 0; break;
        case "ymm0"    : opFlags.RegYmm   = true; regMask |= 1 << 0; break;

        case "implicit": opFlags.FlagImplicit = true; break;

        default:
          console.log(`UNKNOWN OPERAND '${k}'`);
      }
    }

    const outputFlags = StringifyOpArray(ArrayUtils.sorted(opFlags, cmpOp), function(k) { return `F(${k})`; });
    return `ROW(${outputFlags || 0}, ${decToHex(regMask, 2)})`;
  }
}

class ISignature extends Array {
  constructor(name) {
    super();
    this.name = name;
    this.x86 = false;
    this.x64 = false;
    this.implicit = 0; // Number of implicit operands.
  }

  opEquals(other) {
    const len = this.length;
    if (len !== other.length) return false;

    for (var i = 0; i < len; i++)
      if (!this[i].equals(other[i]))
        return false;

    return true;
  }

  mergeWith(other) {
    // If both architectures are the same, it's fine to merge.
    const sameArch = this.x86 === other.x86 && this.x64 === other.x64;

    // If the first arch is [X86|X64] and the second [X64] it's also fine.
    // if (!ok && this.x86 && this.x64 && !other.x86 && other.x64)
    //   ok = true;

    // It's not ok if both signatures have different number of implicit operands.
    if (!sameArch || this.implicit !== other.implicit)
      return false;

    // It's not ok if both signatures have different number of operands.
    const len = this.length;
    if (len !== other.length)
      return false;

    let xorIndex = -1;
    for (let i = 0; i < len; i++) {
      const xor = this[i].xor(other[i]);
      if (xor === null) continue;

      if (xorIndex === -1)
        xorIndex = i;
      else
        return false;
    }

    // Bail if mergeWidth at operand-level failed.
    if (xorIndex === -1 || !this[xorIndex].mergeWith(other[xorIndex]))
      return false;

    return true;
  }

  toString() {
    return "{" + this.join(", ") + "}";
  }
}

class SignatureArray extends Array {
  // Iterate over all signatures and check which operands don't need explicit memory size.
  calcImplicitMemSize(instName) {
    // Calculates a hash-value (aka key) of all register operands specified by `regOps` in `inst`.
    function keyOf(inst, regOps) {
      var s = "";
      for (var i = 0; i < inst.length; i++) {
        const op = inst[i];
        if (regOps & (1 << i))
          s += "{" + ArrayUtils.sorted(ObjectUtils.and(op.flags, RegOp)).join("|") + "}";
      }
      return s || "?";
    }

    var i;
    var aIndex, bIndex;

    for (aIndex = 0; aIndex < this.length; aIndex++) {
      const aInst = this[aIndex];
      const len = aInst.length;

      var memOp = "";
      var memPos = -1;
      var regOps = 0;

      // Check if this instruction signature has a memory operand of explicit size.
      for (i = 0; i < len; i++) {
        const aOp = aInst[i];
        const mem = ObjectUtils.findKey(aOp.flags, MemOp);

        if (mem) {
          // Stop if the memory operand has implicit-size or if there is more than one.
          if (aOp.flags.mem || memPos >= 0) {
            memPos = -1;
            break;
          }
          else {
            memOp = mem;
            memPos = i;
          }
        }
        else if (ObjectUtils.hasAny(aOp.flags, RegOp)) {
          // Doesn't consider 'r/m' as we already checked 'm'.
          regOps |= (1 << i);
        }
      }

      if (memPos < 0)
        continue;

      // Create a `sameSizeSet` set of all instructions having the exact
      // explicit memory operand at the same position and registers at
      // positions matching `regOps` bits and `diffSizeSet` having memory
      // operand of different size, but registers at the same positions.
      const sameSizeSet = [aInst];
      const diffSizeSet = [];
      const diffSizeHash = Object.create(null);

      for (bIndex = 0; bIndex < this.length; bIndex++) {
        const bInst = this[bIndex];
        if (aIndex === bIndex || len !== bInst.length) continue;

        var hasMatch = 1;
        for (i = 0; i < len; i++) {
          if (i === memPos) continue;

          const reg = ObjectUtils.hasAny(bInst[i].flags, RegOp);
          if (regOps & (1 << i))
            hasMatch &= reg;
          else if (reg)
            hasMatch = 0;
        }

        if (hasMatch) {
          const bOp = bInst[memPos];
          if (bOp.flags.mem) continue;

          const mem = ObjectUtils.findKey(bOp.flags, MemOp);
          if (mem === memOp) {
            sameSizeSet.push(bInst);
          }
          else if (mem) {
            const key = keyOf(bInst, regOps);
            diffSizeSet.push(bInst);
            if (!diffSizeHash[key])
              diffSizeHash[key] = [bInst];
            else
              diffSizeHash[key].push(bInst);
          }
        }
      }

      // Two cases.
      //   A) The memory operand has implicit-size if `diffSizeSet` is empty. That
      //      means that the instruction only uses one size for all reg combinations.
      //
      //   B) The memory operand has implicit-size if `diffSizeSet` contains different
      //      register signatures than `sameSizeSet`.
      var implicit = true;

      if (!diffSizeSet.length) {
        // Case A:
      }
      else {
        // Case B: Find collisions in `sameSizeSet` and `diffSizeSet`.
        for (bIndex = 0; bIndex < sameSizeSet.length; bIndex++) {
          const bInst = sameSizeSet[bIndex];
          const key = keyOf(bInst, regOps);

          const diff = diffSizeHash[key];
          if (diff) {
            diff.forEach((diffInst) => {
              if ((bInst.x86 && !diffInst.x86) || (!bInst.x86 && diffInst.x86)) {
                // If this is X86|ANY instruction and the other is X64, or vice-versa,
                // then keep this implicit as it won't do any harm. These instructions
                // cannot be mixed and it will make implicit the 32-bit one in cases
                // where X64 introduced 64-bit ones like `cvtsi2ss`.
                if (!/^(bndcl|bndcn|bndcu|ptwrite|(v)?cvtsi2ss|(v)?cvtsi2sd|vcvtusi2ss|vcvtusi2sd)$/.test(instName))
                  implicit = false;
              }
              else {
                implicit = false;
              }
            });
          }
        }
      }

      // Patch all instructions to accept implicit-size memory operand.
      for (bIndex = 0; bIndex < sameSizeSet.length; bIndex++) {
        const bInst = sameSizeSet[bIndex];
        if (implicit) {
          bInst[memPos].flags.mem = true;
        }

        if (!implicit)
          DEBUG(`${this.name}: Explicit: ${bInst}`);
      }
    }
  }

  compact() {
    var didSomething = true;
    while (didSomething) {
      didSomething = false;
      for (var i = 0; i < this.length; i++) {
        var row = this[i];
        var j = i + 1;
        while (j < this.length) {
          if (row.mergeWith(this[j])) {
            this.splice(j, 1);
            didSomething = true;
            continue;
          }
          j++;
        }
      }
    }
  }

  toString() {
    return `[${this.join(", ")}]`;
  }
}

class InstSignatureTable extends core.Task {
  constructor() {
    super("InstSignatureTable");
    this.maxOpRows = 0;
  }

  run() {
    const insts = this.ctx.insts;

    insts.forEach((inst) => {
      inst.signatures = this.makeSignatures(Filter.noAltForm(inst.dbInsts));
      this.maxOpRows = Math.max(this.maxOpRows, inst.signatures.length);
    });

    const iSignatureMap = Object.create(null);
    const iSignatureArr = [];

    const oSignatureMap = Object.create(null);
    const oSignatureArr = [];

    // Must be first to be assigned to zero.
    const oSignatureNone = "ROW(0, 0xFF)";
    oSignatureMap[oSignatureNone] = [0];
    oSignatureArr.push(oSignatureNone);

    function findSignaturesIndex(rows) {
      const len = rows.length;
      if (!len) return 0;

      const indexes = iSignatureMap[rows[0].data];
      if (indexes === undefined) return -1;

      for (var i = 0; i < indexes.length; i++) {
        const index = indexes[i];
        if (index + len > iSignatureArr.length) continue;

        var ok = true;
        for (var j = 0; j < len; j++) {
          if (iSignatureArr[index + j].data !== rows[j].data) {
            ok = false;
            break;
          }
        }

        if (ok)
          return index;
      }

      return -1;
    }

    function indexSignatures(signatures) {
      const result = iSignatureArr.length;

      for (var i = 0; i < signatures.length; i++) {
        const signature = signatures[i];
        const idx = iSignatureArr.length;

        if (!hasOwn.call(iSignatureMap, signature.data))
          iSignatureMap[signature.data] = [];

        iSignatureMap[signature.data].push(idx);
        iSignatureArr.push(signature);
      }

      return result;
    }

    for (var len = this.maxOpRows; len >= 0; len--) {
      insts.forEach((inst) => {
        const signatures = inst.signatures;
        if (signatures.length === len) {
          const signatureEntries = [];
          for (var j = 0; j < len; j++) {
            const signature = signatures[j];

            var signatureEntry = `ROW(${signature.length}, ${signature.x86 ? 1 : 0}, ${signature.x64 ? 1 : 0}, ${signature.implicit}`;
            var signatureComment = signature.toString();

            var x = 0;
            while (x < signature.length) {
              const h = signature[x].toAsmJitOpData();
              var index = -1;
              if (!hasOwn.call(oSignatureMap, h)) {
                index = oSignatureArr.length;
                oSignatureMap[h] = index;
                oSignatureArr.push(h);
              }
              else {
                index = oSignatureMap[h];
              }

              signatureEntry += `, ${String(index).padEnd(3)}`;
              x++;
            }

            while (x < 6) {
              signatureEntry += `, ${String(0).padEnd(3)}`;
              x++;
            }

            signatureEntry += `)`;
            signatureEntries.push({ data: signatureEntry, comment: signatureComment, refs: 0 });
          }

          var count = signatureEntries.length;
          var index = findSignaturesIndex(signatureEntries);

          if (index === -1)
            index = indexSignatures(signatureEntries);

          iSignatureArr[index].refs++;
          inst.signatureIndex = index;
          inst.signatureCount = count;
        }
      });
    }

    var s = `#define ROW(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5)       \\\n` +
            `  { count, uint8_t(x86 ? uint8_t(InstDB::Mode::kX86) : uint8_t(0)) | \\\n` +
            `                  (x64 ? uint8_t(InstDB::Mode::kX64) : uint8_t(0)) , \\\n` +
            `    implicit,                                                        \\\n` +
            `    0,                                                               \\\n` +
            `    { o0, o1, o2, o3, o4, o5 }                                       \\\n` +
            `  }\n` +
            StringUtils.makeCxxArrayWithComment(iSignatureArr, "const InstDB::InstSignature InstDB::_instSignatureTable[]") +
            `#undef ROW\n` +
            `\n` +
            `#define ROW(opFlags, regId) { opFlags, uint8_t(regId) }\n` +
            `#define F(VAL) uint64_t(InstDB::OpFlags::k##VAL)\n` +
            StringUtils.makeCxxArray(oSignatureArr, "const InstDB::OpSignature InstDB::_opSignatureTable[]") +
            `#undef F\n` +
            `#undef ROW\n`;
    this.inject("InstSignatureTable", disclaimer(s), oSignatureArr.length * 8 + iSignatureArr.length * 8);
  }

  makeSignatures(dbInsts) {
    const instName = dbInsts.length ? dbInsts[0].name : "";
    const signatures = new SignatureArray();

    for (var i = 0; i < dbInsts.length; i++) {
      const inst = dbInsts[i];
      const ops = inst.operands;

      // NOTE: This changed from having reg|mem merged into creating two signatures
      // instead. Imagine two instructions in one `dbInsts` array:
      //
      //   1. mov reg, reg/mem
      //   2. mov reg/mem, reg
      //
      // If we merge them and then unmerge, we will have 4 signatures, when iterated:
      //
      //   1a. mov reg, reg
      //   1b. mov reg, mem
      //   2a. mov reg, reg
      //   2b. mov mem, reg
      //
      // So, instead of merging them here, we insert separated signatures and let
      // the tool merge them in a way that can be easily unmerged at runtime into:
      //
      //   1a. mov reg, reg
      //   1b. mov reg, mem
      //   2b. mov mem, reg
      var modrmCount = 1;
      for (var modrm = 0; modrm < modrmCount; modrm++) {
        var row = new ISignature(inst.name);
        row.x86 = (inst.arch === "ANY" || inst.arch === "X86");
        row.x64 = (inst.arch === "ANY" || inst.arch === "X64");

        for (var j = 0; j < ops.length; j++) {
          var iop = ops[j];

          var reg = iop.reg;
          var mem = iop.mem;
          var imm = iop.imm;
          var rel = iop.rel;

          // Skip all instructions having implicit `imm` operand of `1`.
          if (iop.immValue !== null)
            break;

          // Shorten the number of signatures of 'mov' instruction.
          if (inst.name === "mov" && mem.startsWith("moff"))
            break;

          if (reg === "seg") reg = "sreg";
          if (reg === "st(i)") reg = "st";
          if (reg === "st(0)") reg = "st0";

          if (mem === "moff8") mem = "m8";
          if (mem === "moff16") mem = "m16";
          if (mem === "moff32") mem = "m32";
          if (mem === "moff64") mem = "m64";

          if (mem === "m32fp") mem = "m32";
          if (mem === "m64fp") mem = "m64";
          if (mem === "m80fp") mem = "m80";
          if (mem === "m80bcd") mem = "m80";
          if (mem === "m80dec") mem = "m80";
          if (mem === "m16int") mem = "m16";
          if (mem === "m32int") mem = "m32";
          if (mem === "m64int") mem = "m64";

          if (mem === "m16_16") mem = "m32";
          if (mem === "m16_32") mem = "m48";
          if (mem === "m16_64") mem = "m80";

          if (reg && mem) {
            if (modrmCount === 1) {
              mem = null;
              modrmCount++;
            }
            else {
              reg = null;
            }
          }

          const op = new OSignature();
          if (iop.implicit) {
            row.implicit++;
            op.flags.implicit = true;
          }

          const seg = iop.memSegment;
          if (seg) {
            switch (inst.name) {
              case "insb": op.flags.m8 = true; break;
              case "insw": op.flags.m16 = true; break;
              case "insd": op.flags.m32 = true; break;
              case "outsb": op.flags.m8 = true; break;
              case "outsw": op.flags.m16 = true; break;
              case "outsd": op.flags.m32 = true; break;
              case "clzero": op.flags.mem = true; op.flags.m512 = true; break;
              case "enqcmd": op.flags.mem = true; op.flags.m512 = true; break;
              case "enqcmds": op.flags.mem = true; op.flags.m512 = true; break;
              case "movdir64b": op.flags.mem = true; op.flags.m512 = true; break;
              case "maskmovq": op.flags.mem = true; op.flags.m64 = true; break;
              case "maskmovdqu": op.flags.mem = true; op.flags.m128 = true; break;
              case "vmaskmovdqu": op.flags.mem = true; op.flags.m128 = true; break;
              case "monitor": op.flags.mem = true; break;
              case "monitorx": op.flags.mem = true; break;
              case "umonitor": op.flags.mem = true; break;
              default: console.log(`UNKNOWN MEM IN INSTRUCTION '${inst.name}'`); break;
            }

            if (iop.memRegOnly)
              reg = iop.memRegOnly;

            if (seg === "ds") op.flags.memDS = true;
            if (seg === "es") op.flags.memES = true;
            if (reg === "reg") { op.flags.memBase = true; }
            if (reg === "r32") { op.flags.memBase = true; }
            if (reg === "r64") { op.flags.memBase = true; }
            if (reg === "zax") { op.flags.memBase = true; op.flags.memZAX = true; }
            if (reg === "zsi") { op.flags.memBase = true; op.flags.memZSI = true; }
            if (reg === "zdi") { op.flags.memBase = true; op.flags.memZDI = true; }
          }
          else if (reg) {
            if (reg == "r8") {
              op.flags["r8lo"] = true;

              if (!inst.w || inst.w === "W0")
                op.flags["r8hi"] = true;
            }
            else {
              op.flags[reg] = true;
            }
          }

          if (mem) {
            op.flags[mem] = true;
            // HACK: Allow LEA to use any memory size.
            if (/^(lea)$/.test(inst.name)) {
              op.flags.mem = true;
              Object.assign(op.flags, MemOp);
            }

            // HACK: These instructions specify explicit memory size, but it's just informational.
            if (/^(call|enqcmd|enqcmds|lcall|ljmp|movdir64b)$/.test(inst.name)) {
              op.flags.mem = true;
            }
          }

          if (imm) {
            if (iop.immSign === "any" || iop.immSign === "signed"  ) op.flags["i" + imm] = true;
            if (iop.immSign === "any" || iop.immSign === "unsigned") op.flags["u" + imm] = true;
          }

          if (rel) {
            op.flags["rel" + rel] = true;
          }

          row.push(op);
        }

        // Not equal if we terminated the loop.
        if (j === ops.length)
          signatures.push(row);
      }
    }

    if (signatures.length && GenUtils.canUseImplicitMemSize(instName))
      signatures.calcImplicitMemSize(instName);

    signatures.compact();
    return signatures;
  }
}

// ============================================================================
// [tablegen.x86.AdditionalInfoTable]
// ============================================================================

class AdditionalInfoTable extends core.Task {
  constructor() {
    super("AdditionalInfoTable");
  }

  run() {
    const insts = this.ctx.insts;
    const rwInfoTable = new IndexedArray();
    const instFlagsTable = new IndexedArray();
    const additionaInfoTable = new IndexedArray();

    // If the instruction doesn't read any flags it should point to the first index.
    rwInfoTable.addIndexed(`{ 0, 0 }`);

    insts.forEach((inst) => {
      const dbInsts = inst.dbInsts;

      var features = GenUtils.cpuFeaturesOf(dbInsts).map(function(f) { return `EXT(${f})`; }).join(", ");
      if (!features) features = "0";

      var [r, w] = this.rwFlagsOf(dbInsts);
      const rData = r.map(function(flag) { return `FLAG(${flag})`; }).join(" | ") || "0";
      const wData = w.map(function(flag) { return `FLAG(${flag})`; }).join(" | ") || "0";
      const instFlags = Object.create(null);

      switch (inst.name) {
        case "kmovb":
        case "kmovd":
        case "kmovq":
        case "kmovw":
        case "mov":
        case "movq":
        case "movsd":
        case "movss":
        case "movapd":
        case "movaps":
        case "movdqa":
        case "movdqu":
        case "movupd":
        case "movups":
        case "vmovapd":
        case "vmovaps":
        case "vmovdqa":
        case "vmovdqa8":
        case "vmovdqa16":
        case "vmovdqa32":
        case "vmovdqa64":
        case "vmovdqu":
        case "vmovdqu8":
        case "vmovdqu16":
        case "vmovdqu32":
        case "vmovdqu64":
        case "vmovq":
        case "vmovsd":
        case "vmovss":
        case "vmovupd":
        case "vmovups":
          instFlags["MovOp"] = true;
          break;
      }

      const instFlagsIndex = instFlagsTable.addIndexed("InstRWFlags(" + StringUtils.formatCppFlags(instFlags, (f) => { return `FLAG(${f})`; }, "FLAG(None)") + ")");
      const rwInfoIndex = rwInfoTable.addIndexed(`{ ${rData}, ${wData} }`);

      inst.additionalInfoIndex = additionaInfoTable.addIndexed(`{ ${instFlagsIndex}, ${rwInfoIndex}, { ${features} } }`);
    });

    var s = `#define EXT(VAL) uint32_t(CpuFeatures::X86::k##VAL)\n` +
            `const InstDB::AdditionalInfo InstDB::_additionalInfoTable[] = {\n${StringUtils.format(additionaInfoTable, kIndent, true)}\n};\n` +
            `#undef EXT\n` +
            `\n` +
            `#define FLAG(VAL) uint32_t(CpuRWFlags::kX86_##VAL)\n` +
            `const InstDB::RWFlagsInfoTable InstDB::_rwFlagsInfoTable[] = {\n${StringUtils.format(rwInfoTable, kIndent, true)}\n};\n` +
            `#undef FLAG\n` +
            `\n` +
            `#define FLAG(VAL) uint32_t(InstRWFlags::k##VAL)\n` +
            `const InstRWFlags InstDB::_instFlagsTable[] = {\n${StringUtils.format(instFlagsTable, kIndent, true)}\n};\n` +
            `#undef FLAG\n`;
    this.inject("AdditionalInfoTable", disclaimer(s), additionaInfoTable.length * 8 + rwInfoTable.length * 8 + instFlagsTable.length * 4);
  }

  rwFlagsOf(dbInsts) {
    const r = Object.create(null);
    const w = Object.create(null);

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];

      // Omit special cases, this is handled well in C++ code.
      if (dbInst.name === "mov")
        continue;

      const regs = dbInst.io;

      // Mov is a special case, moving to/from control regs makes flags undefined,
      // which we don't want to have in `X86InstDB::operationData`. This is, thus,
      // a special case instruction analyzer must deal with.
      if (dbInst.name === "mov")
        continue;

      for (var reg in regs) {
        var flag = "";
        switch (reg) {
          case "CF": flag = "CF"; break;
          case "OF": flag = "OF"; break;
          case "SF": flag = "SF"; break;
          case "ZF": flag = "ZF"; break;
          case "AF": flag = "AF"; break;
          case "PF": flag = "PF"; break;
          case "DF": flag = "DF"; break;
          case "IF": flag = "IF"; break;
        //case "TF": flag = "TF"; break;
          case "AC": flag = "AC"; break;
          case "C0": flag = "C0"; break;
          case "C1": flag = "C1"; break;
          case "C2": flag = "C2"; break;
          case "C3": flag = "C3"; break;
          default:
            continue;
        }

        switch (regs[reg]) {
          case "R":
            r[flag] = true;
            break;
          case "X":
            r[flag] = true;
            // ... fallthrough ...
          case "W":
          case "U":
          case "0":
          case "1":
            w[flag] = true;
            break;
        }
      }
    }

    return [ArrayUtils.sorted(r), ArrayUtils.sorted(w)];
  }
}

// ============================================================================
// [tablegen.x86.InstRWInfoTable]
// ============================================================================

const NOT_MEM_AMBIGUOUS = ArrayUtils.toDict([
  "call", "movq"
]);

class InstRWInfoTable extends core.Task {
  constructor() {
    super("InstRWInfoTable");

    this.rwInfoIndexA = [];
    this.rwInfoIndexB = [];
    this.rwInfoTableA = new IndexedArray();
    this.rwInfoTableB = new IndexedArray();

    this.rmInfoTable = new IndexedArray();
    this.opInfoTable = new IndexedArray();

    this.rwCategoryByName = {
      "imul"      : "Imul",
      "mov"       : "Mov",
      "movabs"    : "Movabs",
      "movhpd"    : "Movh64",
      "movhps"    : "Movh64",
      "punpcklbw" : "Punpcklxx",
      "punpckldq" : "Punpcklxx",
      "punpcklwd" : "Punpcklxx",
      "vmaskmovpd": "Vmaskmov",
      "vmaskmovps": "Vmaskmov",
      "vmovddup"  : "Vmovddup",
      "vmovmskpd" : "Vmovmskpd",
      "vmovmskps" : "Vmovmskps",
      "vpmaskmovd": "Vmaskmov",
      "vpmaskmovq": "Vmaskmov"
    };

    const _ = null;
    this.rwCategoryByData = {
      Vmov1_8: [
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width:  8}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 64},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 16}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:128},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 32}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:256},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 64}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:512},_,_,_,_]
      ],
      Vmov1_4: [
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 32}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:128},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 64}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:256},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width:128}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:512},_,_,_,_]
      ],
      Vmov1_2: [
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 64}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:128},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width:128}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:256},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width:256}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:512},_,_,_,_]
      ],
      Vmov2_1: [
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 128}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 64},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 256}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:128},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 512}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:256},_,_,_,_]
      ],
      Vmov4_1: [
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 128}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 32},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 256}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 64},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 512}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width:128},_,_,_,_]
      ],
      Vmov8_1: [
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 128}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 16},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 256}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 32},_,_,_,_],
        [{access: "W", clc: 0, flags: {}, fixed: -1, index: 0, width: 512}, {access: "R", clc: 0, flags: {}, fixed: -1, index: 0, width: 64},_,_,_,_]
      ]
    };
  }

  run() {
    const insts = this.ctx.insts;

    const noRmInfo = StringUtils.formatCppStruct(
      "InstDB::RWInfoRm::kCategory" + "None".padEnd(10),
      StringUtils.decToHex(0, 2),
      String(0).padEnd(2),
      StringUtils.formatCppFlags({}),
      "0"
    );

    const noOpInfo = StringUtils.formatCppStruct(
      "0x0000000000000000u",
      "0x0000000000000000u",
      "0xFF",
      "0",
      StringUtils.formatCppStruct(0),
      "OpRWFlags::kNone"
    );

    this.rmInfoTable.addIndexed(noRmInfo);
    this.opInfoTable.addIndexed(noOpInfo);

    insts.forEach((inst) => {
      // Alternate forms would only mess this up, so filter them out.
      const dbInsts = Filter.noAltForm(inst.dbInsts);

      // The best we can do is to divide instructions that have 2 operands and others.
      // This gives us the highest chance of preventing special cases (which were not
      // entirely avoided).
      const o2Insts = dbInsts.filter((inst) => { return inst.operands.length === 2; });
      const oxInsts = dbInsts.filter((inst) => { return inst.operands.length !== 2; });

      const rwInfoArray = [this.rwInfo(inst, o2Insts), this.rwInfo(inst, oxInsts)];
      const rmInfoArray = [this.rmInfo(inst, o2Insts), this.rmInfo(inst, oxInsts)];

      for (var i = 0; i < 2; i++) {
        const rwInfo = rwInfoArray[i];
        const rmInfo = rmInfoArray[i];

        const rwOps = rwInfo.rwOps;
        const rwOpsIndex = [];
        for (var j = 0; j < rwOps.length; j++) {
          const op = rwOps[j];
          if (!op) {
            rwOpsIndex.push(this.opInfoTable.addIndexed(noOpInfo));
            continue;
          }

          const flags = {};
          const opAcc = op.access;

          if (opAcc === "R") flags.Read = true;
          if (opAcc === "W") flags.Write = true;
          if (opAcc === "X") flags.RW = true;
          ObjectUtils.merge(flags, op.flags);

          const rIndex = opAcc === "X" || opAcc === "R" ? op.index : -1;
          const rWidth = opAcc === "X" || opAcc === "R" ? op.width : -1;
          const wIndex = opAcc === "X" || opAcc === "W" ? op.index : -1;
          const wWidth = opAcc === "X" || opAcc === "W" ? op.width : -1;

          const consecutiveLeadCount = op.clc;

          const opData = StringUtils.formatCppStruct(
            this.byteMaskFromBitRanges([{ start: rIndex, end: rIndex + rWidth - 1 }]) + "u",
            this.byteMaskFromBitRanges([{ start: wIndex, end: wIndex + wWidth - 1 }]) + "u",
            StringUtils.decToHex(op.fixed === -1 ? 0xFF : op.fixed, 2),
            String(consecutiveLeadCount),
            StringUtils.formatCppStruct(0),
            StringUtils.formatCppFlags(flags, function(flag) { return "OpRWFlags::k" + flag; }, "OpRWFlags::kNone")
          );

          rwOpsIndex.push(this.opInfoTable.addIndexed(opData));
        }

        const rmData = StringUtils.formatCppStruct(
          "InstDB::RWInfoRm::kCategory" + rmInfo.category.padEnd(10),
          StringUtils.decToHex(rmInfo.rmIndexes, 2),
          String(Math.max(rmInfo.memFixed, 0)).padEnd(2),
          StringUtils.formatCppFlags({
            "InstDB::RWInfoRm::kFlagAmbiguous": Boolean(rmInfo.memAmbiguous),
            "InstDB::RWInfoRm::kFlagMovssMovsd": Boolean(inst.name === "movss" || inst.name === "movsd"),
            "InstDB::RWInfoRm::kFlagPextrw": Boolean(inst.name === "pextrw"),
            "InstDB::RWInfoRm::kFlagFeatureIfRMI": Boolean(rmInfo.memExtensionIfRMI)
          }),
          rmInfo.memExtension === "None" ? "0" : "uint32_t(CpuFeatures::X86::k" + rmInfo.memExtension + ")"
        );

        const rwData = StringUtils.formatCppStruct(
          "InstDB::RWInfo::kCategory" + rwInfo.category.padEnd(10),
          String(this.rmInfoTable.addIndexed(rmData)).padEnd(2),
          StringUtils.formatCppStruct(...(rwOpsIndex.map(function(item) { return String(item).padEnd(2); })))
        );

        if (i == 0)
          this.rwInfoIndexA.push(this.rwInfoTableA.addIndexed(rwData));
        else
          this.rwInfoIndexB.push(this.rwInfoTableB.addIndexed(rwData));
      }
    });

    var s = "";
    s += "const uint8_t InstDB::rwInfoIndexA[Inst::_kIdCount] = {\n" + StringUtils.format(this.rwInfoIndexA, kIndent, -1) + "\n};\n";
    s += "\n";
    s += "const uint8_t InstDB::rwInfoIndexB[Inst::_kIdCount] = {\n" + StringUtils.format(this.rwInfoIndexB, kIndent, -1) + "\n};\n";
    s += "\n";
    s += "const InstDB::RWInfo InstDB::rwInfoA[] = {\n" + StringUtils.format(this.rwInfoTableA, kIndent, true) + "\n};\n";
    s += "\n";
    s += "const InstDB::RWInfo InstDB::rwInfoB[] = {\n" + StringUtils.format(this.rwInfoTableB, kIndent, true) + "\n};\n";
    s += "\n";
    s += "const InstDB::RWInfoOp InstDB::rwInfoOp[] = {\n" + StringUtils.format(this.opInfoTable, kIndent, true) + "\n};\n";
    s += "\n";
    s += "const InstDB::RWInfoRm InstDB::rwInfoRm[] = {\n" + StringUtils.format(this.rmInfoTable, kIndent, true) + "\n};\n";

    const size = this.rwInfoIndexA.length +
                 this.rwInfoIndexB.length +
                 this.rwInfoTableA.length * 8 +
                 this.rwInfoTableB.length * 8 +
                 this.rmInfoTable.length * 4 +
                 this.opInfoTable.length * 24;

    this.inject("InstRWInfoTable", disclaimer(s), size);
  }

  byteMaskFromBitRanges(ranges) {
    const arr = [];
    for (var i = 0; i < 64; i++)
      arr.push(0);

    for (var i = 0; i < ranges.length; i++) {
      const start = ranges[i].start;
      const end = ranges[i].end;

      if (start < 0)
        continue;

      for (var j = start; j <= end; j++) {
        const bytePos = j >> 3;
        if (bytePos < 0 || bytePos >= arr.length)
          FATAL(`Range ${start}:${end} cannot be used to create a byte-mask`);
        arr[bytePos] = 1;
      }
    }

    var s = "0x";
    for (var i = arr.length - 4; i >= 0; i -= 4) {
      const value = (arr[i + 3] << 3) | (arr[i + 2] << 2) | (arr[i + 1] << 1) | arr[i];
      s += value.toString(16).toUpperCase();
    }
    return s;
  }

  // Read/Write Info
  // ---------------

  rwInfo(asmInst, dbInsts) {
    const self = this;

    function nullOps() {
      return [null, null, null, null, null, null];
    }

    function makeRwFromOp(op) {
      if (!op.isRegOrMem())
        return null;

      return {
        access: op.read && op.write ? "X" : op.read ? "R" : op.write ? "W" : "?",
        clc: 0,
        flags: {},
        fixed: GenUtils.fixedRegOf(op),
        index: op.rwxIndex,
        width: op.rwxWidth
      };
    }

    function queryRwGeneric(dbInsts, step) {
      var rwOps = nullOps();
      for (var i = 0; i < dbInsts.length; i++) {
        const dbInst = dbInsts[i];
        const operands = dbInst.operands;

        for (var j = 0; j < operands.length; j++) {
          const op = operands[j];
          if (!op.isRegOrMem())
            continue;

          const opSize = op.isReg() ? op.regSize : op.memSize;
          var d = {
            access: op.read && op.write ? "X" : op.read ? "R" : op.write ? "W" : "?",
            clc: 0,
            flags: {},
            fixed: -1,
            index: -1,
            width: -1
          };

          if (op.consecutiveLeadCount)
            d.clc = op.consecutiveLeadCount;

          const instName = dbInst.name;
          // NOTE: Avoid push/pop here as PUSH/POP has many variations for segment registers,
          // which would set 'd.fixed' field even for GP variation of the instuction.
          if (instName !== "push" && instName !== "pop") {
            d.fixed = GenUtils.fixedRegOf(op);
          }

          switch (instName) {
            case "vfcmaddcph":
            case "vfmaddcph":
            case "vfcmaddcsh":
            case "vfmaddcsh":
            case "vfcmulcsh":
            case "vfmulcsh":
            case "vfcmulcph":
            case "vfmulcph":
              if (j === 0)
                d.flags.Unique = true;
              break;
          }

          if (op.zext)
            d.flags.ZExt = true;

          if (op.regIndexRel)
            d.flags.Consecutive = true;

          for (var k in self.rwOpFlagsForInstruction(asmInst.name, j))
            d.flags[k] = true;

          if ((step === -1 || step === j) || op.rwxIndex !== 0 || op.rwxWidth !== opSize) {
            d.index = op.rwxIndex;
            d.width = op.rwxWidth;
          }

          if (d.fixed !== -1) {
            if (op.memSegment)
              d.flags.MemPhysId = true;
            else
              d.flags.RegPhysId = true;
          }

          if (rwOps[j] === null) {
            rwOps[j] = d;
          }
          else {
            if (!ObjectUtils.equalsExcept(rwOps[j], d, { "fixed": true, "flags": true }))
              return null;

            if (rwOps[j].fixed === -1)
              rwOps[j].fixed = d.fixed;
            ObjectUtils.merge(rwOps[j].flags, d.flags);
          }
        }
      }

      const name = dbInsts.length ? dbInsts[0].name : "";

      switch (name) {
        case "vpternlogd":
        case "vpternlogq":
          return { category: "GenericEx", rwOps };

        default:
          return { category: "Generic", rwOps };
      }
    }

    function queryRwByData(dbInsts, rwOpsArray) {
      for (var i = 0; i < dbInsts.length; i++) {
        const dbInst = dbInsts[i];
        const operands = dbInst.operands;
        const rwOps = nullOps();

        for (var j = 0; j < operands.length; j++) {
          rwOps[j] = makeRwFromOp(operands[j])
        }

        var match = 0;
        for (var j = 0; j < rwOpsArray.length; j++)
          match |= ObjectUtils.equals(rwOps, rwOpsArray[j]);

        if (!match)
          return false;
      }

      return true;
    }

    function dumpRwToData(dbInsts) {
      const out = [];
      for (var i = 0; i < dbInsts.length; i++) {
        const dbInst = dbInsts[i];
        const operands = dbInst.operands;
        const rwOps = nullOps();

        for (var j = 0; j < operands.length; j++)
          rwOps[j] = makeRwFromOp(operands[j])

        if (ArrayUtils.deepIndexOf(out, rwOps) !== -1)
          continue;

        out.push(rwOps);
      }
      return out;
    }

    // Some instructions are just special...
    const name = dbInsts.length ? dbInsts[0].name : "";
    if (name in this.rwCategoryByName)
      return { category: this.rwCategoryByName[name], rwOps: nullOps() };

    // Generic rules.
    for (var i = -1; i <= 6; i++) {
      const rwInfo = queryRwGeneric(dbInsts, i);
      if (rwInfo)
        return rwInfo;
    }

    // Specific rules.
    for (var k in this.rwCategoryByData)
      if (queryRwByData(dbInsts, this.rwCategoryByData[k]))
        return { category: k, rwOps: nullOps() };

    // FATALURE: Missing data to categorize this instruction.
    if (name) {
      const items = dumpRwToData(dbInsts)
      console.log(`RW: ${dbInsts.length ? dbInsts[0].name : ""}:`);
      items.forEach((item) => {
        console.log("  " + JSON.stringify(item));
      });
    }

    return null;
  }

  rwOpFlagsForInstruction(instName, opIndex) {
    const toMap = ArrayUtils.toDict;

    // TODO: We should be able to get this information from asmdb.
    switch (instName + "@" + opIndex) {
      case "cmps@0": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "cmps@1": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "movs@0": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "movs@1": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "lods@1": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "stos@0": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "scas@1": return toMap(['MemBaseRW', 'MemBasePostModify']);
      case "bndstx@0": return toMap(['MemBaseWrite', 'MemIndexWrite']);

      default:
        return {};
    }
  }

  // Reg/Mem Info
  // ------------

  rmInfo(asmInst, dbInsts) {
    const info = {
      category: "None",
      rmIndexes: this.rmReplaceableIndexes(dbInsts),
      memFixed: this.rmFixedSize(dbInsts),
      memAmbiguous: this.rmIsAmbiguous(dbInsts),
      memConsistent: this.rmIsConsistent(dbInsts),
      memExtension: this.rmExtension(dbInsts),
      memExtensionIfRMI: this.rmExtensionIfRMI(dbInsts)
    };

    if (info.memFixed !== -1)
      info.category = "Fixed";
    else if (info.memConsistent)
      info.category = "Consistent";
    else if (info.rmIndexes)
      info.category = this.rmReplaceableCategory(dbInsts);

    return info;
  }

  rmReplaceableCategory(dbInsts) {
    var category = null;

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const operands = dbInst.operands;

      var rs = -1;
      var ms = -1;

      for (var j = 0; j < operands.length; j++) {
        const op = operands[j];
        if (op.isMem())
          ms = op.memSize;
        else if (op.isReg())
          rs = Math.max(rs, op.regSize);
      }

      var c = (rs === -1    ) ? "None"    :
              (ms === -1    ) ? "None"    :
              (ms === rs    ) ? "Fixed"   :
              (ms === rs / 2) ? "Half"    :
              (ms === rs / 4) ? "Quarter" :
              (ms === rs / 8) ? "Eighth"  : "Unknown";

      if (category === null)
        category = c;
      else if (category !== c) {
        // Special cases.
        if (dbInst.name === "mov" || dbInst.name === "vmovddup")
          return "None";

        if (/^(punpcklbw|punpckldq|punpcklwd)$/.test(dbInst.name))
          return "None";

        return cxx.Utils.capitalize(dbInst.name);
      }
    }

    if (category === "Unknown")
      console.log(`Instruction '${dbInsts[0].name}' has no RMInfo category.`);

    return category || "Unknown";
  }

  rmReplaceableIndexes(dbInsts) {
    function maskOf(inst, fn) {
      var m = 0;
      var operands = inst.operands;
      for (var i = 0; i < operands.length; i++)
        if (fn(operands[i]))
          m |= (1 << i);
      return m;
    }

    function getRegIndexes(inst) { return maskOf(inst, function(op) { return op.isReg(); }); };
    function getMemIndexes(inst) { return maskOf(inst, function(op) { return op.isMem(); }); };

    var mask = 0;

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];

      var mi = getMemIndexes(dbInst);
      var ri = getRegIndexes(dbInst) & ~mi;

      if (!mi)
        continue;

      const match = dbInsts.some((inst) => {
        var ti = getRegIndexes(inst);
        return ((ri & ti) === ri && (mi & ti) === mi);
      });

      if (!match)
        return 0;
      mask |= mi;
    }

    return mask;
  }

  rmFixedSize(insts) {
    var savedOp = null;

    for (var i = 0; i < insts.length; i++) {
      const inst = insts[i];
      const operands = inst.operands;

      for (var j = 0; j < operands.length; j++) {
        const op = operands[j];
        if (op.mem) {
          if (savedOp && savedOp.mem !== op.mem)
            return -1;
          savedOp = op;
        }
      }
    }

    return savedOp ? Math.max(savedOp.memSize, 0) / 8 : -1;
  }

  rmIsConsistent(insts) {
    var hasMem = 0;
    for (var i = 0; i < insts.length; i++) {
      const inst = insts[i];
      const operands = inst.operands;
      for (var j = 0; j < operands.length; j++) {
        const op = operands[j];
        if (op.mem) {
          hasMem = 1;
          if (!op.reg)
            return 0;
          if (asmdb.x86.Utils.regSize(op.reg) !== op.memSize)
            return 0;
        }
      }
    }
    return hasMem;
  }

  rmIsAmbiguous(dbInsts) {
    function isAmbiguous(dbInsts) {
      const memMap = {};
      const immMap = {};

      for (var i = 0; i < dbInsts.length; i++) {
        const dbInst = dbInsts[i];
        const operands = dbInst.operands;

        var memStr = "";
        var immStr = "";
        var hasMem = false;
        var hasImm = false;

        for (var j = 0; j < operands.length; j++) {
          const op = operands[j];
          if (j) {
            memStr += ", ";
            immStr += ", ";
          }

          if (op.isImm()) {
            immStr += "imm";
            hasImm = true;
          }
          else {
            immStr += op.toString();
          }

          if (op.mem) {
            memStr += "m";
            hasMem = true;
          }
          else {
            memStr += op.isImm() ? "imm" : op.toString();
          }
        }

        if (hasImm) {
          if (immMap[immStr] === true)
            continue;
          immMap[immStr] = true;
        }

        if (hasMem) {
          if (memMap[memStr] === true)
            return 1;
          memMap[memStr] = true;
        }
      }
      return 0;
    }

    const uniqueInsts = Filter.unique(dbInsts);

    // Special cases.
    if (!dbInsts.length)
      return 0;

    if (NOT_MEM_AMBIGUOUS[dbInsts[0].name])
      return 0;

    return (isAmbiguous(Filter.byArch(uniqueInsts, "X86")) << 0) |
           (isAmbiguous(Filter.byArch(uniqueInsts, "X64")) << 1) ;
  }

  rmExtension(dbInsts) {
    if (!dbInsts.length)
      return "None";

    const name = dbInsts[0].name;
    switch (name) {
      case "pextrw":
        return "SSE4_1";

      case "vpslld":
      case "vpsllq":
      case "vpsrad":
      case "vpsrld":
      case "vpsrlq":
        return "AVX512_F";

      case "vpslldq":
      case "vpsllw":
      case "vpsraw":
      case "vpsrldq":
      case "vpsrlw":
        return "AVX512_BW";

      default:
        return "None";
    }
  }

  rmExtensionIfRMI(dbInsts) {
    if (!dbInsts.length)
      return 0;

    const name = dbInsts[0].name;
    return /^(vpslld|vpsllq|vpsrad|vpsrld|vpsrlq|vpslldq|vpsllw|vpsraw|vpsrldq|vpsrlw)$/.test(name);
  }
}

// ============================================================================
// [tablegen.x86.InstCommonTable]
// ============================================================================

class InstCommonTable extends core.Task {
  constructor() {
    super("InstCommonTable", [
      "IdEnum",
      "NameTable",
      "InstSignatureTable",
      "AdditionalInfoTable",
      "InstRWInfoTable"
    ]);
  }

  run() {
    const insts = this.ctx.insts;
    const table = new IndexedArray();

    insts.forEach((inst) => {
      const commonFlagsArray = inst.flags.filter((flag) => { return !flag.startsWith("Avx512"); });
      const avx512FlagsArray = inst.flags.filter((flag) => { return  flag.startsWith("Avx512"); });

      const commonFlags = commonFlagsArray.map(function(flag) { return `F(${flag          })`; }).join("|") || "0";
      const avx512Flags = avx512FlagsArray.map(function(flag) { return `X(${flag.substr(6)})`; }).join("|") || "0";

      const controlFlow = `CONTROL_FLOW(${inst.controlFlow})`;
      const singleRegCase = `SAME_REG_HINT(${inst.singleRegCase})`;

      const row = "{ " +
        String(commonFlags        ).padEnd(50) + ", " +
        String(avx512Flags        ).padEnd(30) + ", " +
        String(inst.signatureIndex).padEnd( 3) + ", " +
        String(inst.signatureCount).padEnd( 2) + ", " +
        String(controlFlow        ).padEnd(16) + ", " +
        String(singleRegCase      ).padEnd(16) + "}";
      inst.commonInfoIndex = table.addIndexed(row);
    });

    var s = `#define F(VAL) uint32_t(InstDB::InstFlags::k##VAL)\n` +
            `#define X(VAL) uint32_t(InstDB::Avx512Flags::k##VAL)\n` +
            `#define CONTROL_FLOW(VAL) uint8_t(InstControlFlow::k##VAL)\n` +
            `#define SAME_REG_HINT(VAL) uint8_t(InstSameRegHint::k##VAL)\n` +
            `const InstDB::CommonInfo InstDB::_commonInfoTable[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef SAME_REG_HINT\n` +
            `#undef CONTROL_FLOW\n` +
            `#undef X\n` +
            `#undef F\n`;
    this.inject("InstCommonTable", disclaimer(s), table.length * 8);
  }
}

// ============================================================================
// [Main]
// ============================================================================

new X86TableGen()
  .addTask(new IdEnum())
  .addTask(new NameTable())
  .addTask(new AltOpcodeTable())
  .addTask(new InstSignatureTable())
  .addTask(new AdditionalInfoTable())
  .addTask(new InstRWInfoTable())
  .addTask(new InstCommonTable())
  .run();
