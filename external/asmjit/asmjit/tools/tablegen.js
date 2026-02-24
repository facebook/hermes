// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// ============================================================================
// tablegen.js
//
// Provides core foundation for generating tables that AsmJit requires. This
// file should provide everything table generators need in general.
// ============================================================================

"use strict";

// ============================================================================
// [Imports]
// ============================================================================

const fs = require("fs");

const commons = require("./generator-commons.js");
const cxx = require("./generator-cxx.js");
const asmdb = require("../db");

exports.asmdb = asmdb;
exports.exp = asmdb.base.exp;

const hasOwn = Object.prototype.hasOwnProperty;

const FATAL = commons.FATAL;
const StringUtils = commons.StringUtils;

const kAsmJitRoot = "..";
exports.kAsmJitRoot = kAsmJitRoot;

// ============================================================================
// [InstructionNameData]
// ============================================================================

function charTo5Bit(c) {
  if (c >= 'a' && c <= 'z')
    return 1 + (c.charCodeAt(0) - 'a'.charCodeAt(0));
  else if (c >= '0' && c <= '4')
    return 1 + 26 + (c.charCodeAt(0) - '0'.charCodeAt(0));
  else
    FATAL(`Character '${c}' cannot be encoded into a 5-bit string`);
}

class InstructionNameData {
  constructor() {
    this.names = [];
    this.primaryTable = [];
    this.stringTable = "";
    this.size = 0;
    this.indexComment = [];
    this.maxNameLength = 0;
  }

  add(s) {
    // First try to encode the string with 5-bit characters that fit into a 32-bit int.
    if (/^[a-z0-4]{0,6}$/.test(s)) {
      let index = 0;
      for (let i = 0; i < s.length; i++)
        index |= charTo5Bit(s[i]) << (i * 5);

      this.names.push(s);
      this.primaryTable.push(index | (1 << 31));
      this.indexComment.push(`Small '${s}'.`);
    }
    else {
      // Put the string into a string table.
      this.names.push(s);
      this.primaryTable.push(-1);
      this.indexComment.push(``);
    }

    if (this.maxNameLength < s.length)
      this.maxNameLength = s.length;
  }

  index() {
    const kMaxPrefixSize = 15;
    const kMaxSuffixSize = 7;
    const names = [];

    for (let idx = 0; idx < this.primaryTable.length; idx++) {
      if (this.primaryTable[idx] === -1) {
        names.push({ name: this.names[idx], index: idx });
      }
    }

    names.sort(function(a, b) {
      if (a.name.length > b.name.length)
        return -1;
      if (a.name.length < b.name.length)
        return 1;
      return (a > b) ? 1 : (a < b) ? -1 : 0;
    });

    for (let z = 0; z < names.length; z++) {
      const idx = names[z].index;
      const name = names[z].name;

      let done = false;
      let longestPrefix = 0;
      let longestSuffix = 0;

      let prefix = "";
      let suffix = "";

      for (let i = Math.min(name.length, kMaxPrefixSize); i > 0; i--) {
        prefix = name.substring(0, i);
        suffix = name.substring(i);

        const prefixIndex = this.stringTable.indexOf(prefix);
        const suffixIndex = this.stringTable.indexOf(suffix);

        // Matched both parts?
        if (prefixIndex !== -1 && suffix === "") {
          done = true;
          break;
        }

        if (prefixIndex !== -1 && suffixIndex !== -1) {
          done = true;
          break;
        }

        if (prefixIndex !== -1 && longestPrefix === 0)
          longestPrefix = prefix.length;

        if (suffixIndex !== -1 && suffix.length > longestSuffix)
          longestSuffix = suffix.length;

        if (suffix.length === kMaxSuffixSize)
          break;
      }

      if (!done) {
        let minPrefixSize = name.length >= 8 ? name.length / 2 + 1 : name.length - 2;

        prefix = "";
        suffix = "";

        if (longestPrefix >= minPrefixSize) {
          prefix = name.substring(0, longestPrefix);
          suffix = name.substring(longestPrefix);
        }
        else if (longestSuffix) {
          const splitAt = Math.min(name.length - longestSuffix, kMaxPrefixSize);
          prefix = name.substring(0, splitAt);
          suffix = name.substring(splitAt);
        }
        else if (name.length > kMaxPrefixSize) {
          prefix = name.substring(0, kMaxPrefixSize);
          suffix = name.substring(kMaxPrefixSize);
        }
        else {
          prefix = name;
          suffix = "";
        }
      }

      if (suffix) {
        const prefixIndex = this.addOrReferenceString(prefix);
        const suffixIndex = this.addOrReferenceString(suffix);

        this.primaryTable[idx] = prefixIndex | (prefix.length << 12) | (suffixIndex << 16) | (suffix.length << 28);
        this.indexComment[idx] = `Large '${prefix}|${suffix}'.`;
      }
      else {
        const prefixIndex = this.addOrReferenceString(prefix);

        this.primaryTable[idx] = prefixIndex | (prefix.length << 12);
        this.indexComment[idx] = `Large '${prefix}'.`;
      }
    }
  }

  addOrReferenceString(s) {
    let index = this.stringTable.indexOf(s);
    if (index === -1) {
      index = this.stringTable.length;
      this.stringTable += s;
    }
    return index;
  }

  formatIndexTable(tableName) {
    if (this.size === -1)
      FATAL(`IndexedString.formatIndexTable(): Not indexed yet, call index()`);

    let s = "";
    for (let i = 0; i < this.primaryTable.length; i++) {
      s += cxx.Utils.toHex(this.primaryTable[i], 8);
      s += i !== this.primaryTable.length - 1 ? "," : " ";
      s += " // " + this.indexComment[i] + "\n";
    }

    return `const uint32_t ${tableName}[] = {\n${StringUtils.indent(s, "  ")}};\n`;
  }

  formatStringTable(tableName) {
    if (this.size === -1)
      FATAL(`IndexedString.formatStringTable(): Not indexed yet, call index()`);

    let s = "";
    for (let i = 0; i < this.stringTable.length; i += 80) {
      if (s)
        s += "\n"
      s += '"' + this.stringTable.substring(i, i + 80) + '"';
    }
    s += ";\n";

    return `const char ${tableName}[] =\n${StringUtils.indent(s, "  ")}\n`;
  }

  getSize() {
    if (this.size === -1)
      FATAL(`IndexedString.getSize(): Not indexed yet, call index()`);

    return this.primaryTable.length * 4 + this.stringTable.length;
  }

  getIndex(k) {
    if (this.size === -1)
      FATAL(`IndexedString.getIndex(): Not indexed yet, call index()`);

    if (!hasOwn.call(this.map, k))
      FATAL(`IndexedString.getIndex(): Key '${k}' not found.`);

    return this.map[k];
  }
}
exports.InstructionNameData = InstructionNameData;

// ============================================================================
// [Task]
// ============================================================================

// A base runnable task that can access the TableGen through `this.ctx`.
class Task {
  constructor(name, deps) {
    this.ctx = null;
    this.name = name || "";
    this.deps = deps || [];
  }

  inject(key, str, size) {
    this.ctx.inject(key, str, size);
    return this;
  }

  run() {
    FATAL("Task.run(): Must be reimplemented");
  }
}
exports.Task = Task;

// ============================================================================
// [TableGen]
// ============================================================================

class Injector {
  constructor() {
    this.files = Object.create(null);
    this.tableSizes = Object.create(null);
  }

  load(fileList) {
    for (var i = 0; i < fileList.length; i++) {
      const file = fileList[i];
      const path = kAsmJitRoot + "/" + file;
      const data = fs.readFileSync(path, "utf8").replace(/\r\n/g, "\n");

      this.files[file] = {
        prev: data,
        data: data
      };
    }
    return this;
  }

  save() {
    for (var file in this.files) {
      const obj = this.files[file];
      if (obj.data !== obj.prev) {
        const path = kAsmJitRoot + "/" + file;
        console.log(`MODIFIED '${file}'`);

        fs.writeFileSync(path + ".backup", obj.prev, "utf8");
        fs.writeFileSync(path, obj.data, "utf8");
      }
    }
  }

  dataOfFile(file) {
    const obj = this.files[file];
    if (!obj)
      FATAL(`TableGen.dataOfFile(): File '${file}' not loaded`);
    return obj.data;
  }

  inject(key, str, size) {
    const begin = "// ${" + key + ":Begin}\n";
    const end   = "// ${" + key + ":End}\n";

    var done = false;
    for (var file in this.files) {
      const obj = this.files[file];
      const data = obj.data;

      if (data.indexOf(begin) !== -1) {
        obj.data = StringUtils.inject(data, begin, end, str);
        done = true;
        break;
      }
    }

    if (!done)
      FATAL(`TableGen.inject(): Cannot find '${key}'`);

    if (size)
      this.tableSizes[key] = size;

    return this;
  }

  dumpTableSizes() {
    const sizes = this.tableSizes;

    var pad = 26;
    var total = 0;

    for (var name in sizes) {
      const size = sizes[name];
      total += size;
      console.log(("Size of " + name).padEnd(pad) + ": " + size);
    }

    console.log("Size of all tables".padEnd(pad) + ": " + total);
  }
}
exports.Injector = Injector;

// Main context used to load, generate, and store instruction tables. The idea
// is to be extensible, so it stores 'Task's to be executed with minimal deps
// management.
class TableGen extends Injector{
  constructor(arch) {
    super();

    this.arch = arch;

    this.tasks = [];
    this.taskMap = Object.create(null);

    this.insts = [];
    this.instMap = Object.create(null);

    this.aliases = [];
    this.aliasMem = Object.create(null);
  }

  // --------------------------------------------------------------------------
  // [Task Management]
  // --------------------------------------------------------------------------

  addTask(task) {
    if (!task.name)
      FATAL(`TableGen.addModule(): Module must have a name`);

    if (this.taskMap[task.name])
      FATAL(`TableGen.addModule(): Module '${task.name}' already added`);

    task.deps.forEach((dependency) => {
      if (!this.taskMap[dependency])
        FATAL(`TableGen.addModule(): Dependency '${dependency}' of module '${task.name}' doesn't exist`);
    });

    this.tasks.push(task);
    this.taskMap[task.name] = task;

    task.ctx = this;
    return this;
  }

  runTasks() {
    const tasks = this.tasks;
    const tasksDone = Object.create(null);

    var pending = tasks.length;
    while (pending) {
      const oldPending = pending;
      const arrPending = [];

      for (var i = 0; i < tasks.length; i++) {
        const task = tasks[i];
        if (tasksDone[task.name])
          continue;

        if (task.deps.every((dependency) => { return tasksDone[dependency] === true; })) {
          task.run();
          tasksDone[task.name] = true;
          pending--;
        }
        else {
          arrPending.push(task.name);
        }
      }

      if (oldPending === pending)
        throw Error(`TableGen.runModules(): Modules '${arrPending.join("|")}' stuck (cyclic dependency?)`);
    }
  }

  // --------------------------------------------------------------------------
  // [Instruction Management]
  // --------------------------------------------------------------------------

  addInst(inst) {
    if (this.instMap[inst.name])
      FATAL(`TableGen.addInst(): Instruction '${inst.name}' already added`);

    inst.id = this.insts.length;
    this.insts.push(inst);
    this.instMap[inst.name] = inst;

    return this;
  }

  addAlias(alias, name) {
    this.aliases.push(alias);
    this.aliasMap[alias] = name;

    return this;
  }

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  run() {
    this.onBeforeRun();
    this.runTasks();
    this.onAfterRun();
  }

  // --------------------------------------------------------------------------
  // [Hooks]
  // --------------------------------------------------------------------------

  onBeforeRun() {}
  onAfterRun() {}
}
exports.TableGen = TableGen;

// ============================================================================
// [IdEnum]
// ============================================================================

class IdEnum extends Task {
  constructor(name, deps) {
    super(name || "IdEnum", deps);
  }

  comment(name) {
    FATAL("IdEnum.comment(): Must be reimplemented");
  }

  run() {
    const insts = this.ctx.insts;

    var s = "";
    for (var i = 0; i < insts.length; i++) {
      const inst = insts[i];

      var line = "kId" + inst.enum + (i ? "" : " = 0") + ",";
      var text = this.comment(inst);

      if (text)
        line = line.padEnd(37) + "//!< " + text;

      s += line + "\n";
    }
    s += "_kIdCount\n";

    return this.ctx.inject("InstId", s);
  }
}
exports.IdEnum = IdEnum;

// ============================================================================
// [NameTable]
// ============================================================================

class Output {
  constructor() {
    this.content = Object.create(null);
    this.tableSize = Object.create(null);
  }

  add(id, content, tableSize) {
    this.content[id] = content;
    this.tableSize[id] = typeof tableSize === "number" ? tableSize : 0;
  }
};
exports.Output = Output;

function generateNameData(out, instructions) {
  const none = "Inst::kIdNone";

  const instFirst = new Array(26);
  const instLast  = new Array(26);
  const instNameData = new InstructionNameData();

  for (let i = 0; i < instructions.length; i++)
    instNameData.add(instructions[i].displayName);
  instNameData.index();

  for (let i = 0; i < instructions.length; i++) {
    const inst = instructions[i];
    const displayName = inst.displayName;
    const alphaIndex = displayName.charCodeAt(0) - 'a'.charCodeAt(0);

    if (alphaIndex < 0 || alphaIndex >= 26)
      FATAL(`generateNameData(): Invalid lookup character '${displayName[0]}' of '${displayName}'`);

    if (instFirst[alphaIndex] === undefined)
      instFirst[alphaIndex] = `Inst::kId${inst.enum}`;
    instLast[alphaIndex] = `Inst::kId${inst.enum}`;
  }

  var s = "";
  s += `const InstNameIndex InstDB::instNameIndex = {{\n`;
  for (var i = 0; i < instFirst.length; i++) {
    const firstId = instFirst[i] || none;
    const lastId = instLast[i] || none;

    s += `  { ${String(firstId).padEnd(22)}, ${String(lastId).padEnd(22)} + 1 }`;
    if (i !== 26 - 1)
      s += `,`;
    s += `\n`;
  }
  s += `}, uint16_t(${instNameData.maxNameLength})};\n`;
  s += `\n`;
  s += instNameData.formatStringTable("InstDB::_instNameStringTable");
  s += `\n`;
  s += instNameData.formatIndexTable("InstDB::_instNameIndexTable");

  const dataSize = instNameData.getSize() + 26 * 4;
  out.add("NameData", StringUtils.disclaimer(s), dataSize);
  return out;
}
exports.generateNameData = generateNameData;

class NameTable extends Task {
  constructor(name, deps) {
    super(name || "NameTable", deps);
  }

  run() {
    const output = new Output();
    generateNameData(output, this.ctx.insts);
    this.ctx.inject("NameData", output.content["NameData"], output.tableSize["NameData"]);
  }
}
exports.NameTable = NameTable;
