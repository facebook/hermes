// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// C++ code generation helpers.
const commons = require("./generator-commons.js");
const FATAL = commons.FATAL;

// Utilities to convert primitives to C++ code.
class Utils {
  static toHex(val, pad) {
    if (val < 0)
      val = 0xFFFFFFFF + val + 1;

    let s = val.toString(16);
    if (pad != null && s.length < pad)
      s = "0".repeat(pad - s.length) + s;

    return "0x" + s.toUpperCase();
  }

  static capitalize(s) {
    s = String(s);
    return !s ? s : s[0].toUpperCase() + s.substr(1);
  }

  static camelCase(s) {
    if (s == null || s === "")
      return s;

    s = String(s);
    if (/^[A-Z]+$/.test(s))
      return s.toLowerCase();
    else
      return s[0].toLowerCase() + s.substr(1);
  }

  static normalizeSymbolName(s) {
    switch (s) {
      case "and":
      case "or":
      case "xor":
        return s + "_";
      default:
        return s;
    }
  }

  static indent(s, indentation) {
    if (typeof indentation === "number")
      indentation = " ".repeat(indentation);

    var lines = s.split(/\r?\n/g);
    if (indentation) {
      for (var i = 0; i < lines.length; i++) {
        var line = lines[i];
        if (line)
          lines[i] = indentation + line;
      }
    }

    return lines.join("\n");
  }
}
exports.Utils = Utils;

// A node that represents a C++ construct.
class Node {
  constructor(kind) {
    this.kind = kind;
  }
};
exports.Node = Node;

// A single line of C++ code that declares a variable with optional initialization.
class Var extends Node {
  constructor(type, name, init) {
    super("var");

    this.type = type;
    this.name = name;
    this.init = init || "";
  }

  toString() {
    let s = this.type + " " + this.name;
    if (this.init)
      s += " = " + this.init;
    return s + ";\n";
  }
};
exports.Var = Var;

// A single line of C++ code, which should not contain any branch or a variable declaration.
class Line extends Node {
  constructor(code) {
    super("line");

    this.code = code;
  }

  toString() {
    return String(this.code) + "\n";
  }
};
exports.Line = Line;

// A block containing an array of `Node` items (may contain nested blocks, etc...).
class Block extends Node {
  constructor(nodes) {
    super("block");

    this.nodes = nodes || [];
  }

  isEmpty() {
    return this.nodes.length === 0;
  }

  appendNode(node) {
    if (!(node instanceof Node))
      FATAL("Block.appendNode(): Node must be an instance of Node");

    this.nodes.push(node);
    return this;
  }

  prependNode(node) {
    if (!(node instanceof Node))
      FATAL("Block.prependNode(): Node must be an instance of Node");

    this.nodes.unshift(node);
    return this;
  }

  insertNode(index, node) {
    if (!(node instanceof Node))
      FATAL("Block.insertNode(): Node must be an instance of Node");

    if (index >= this.nodes.length)
      this.nodes.push(node);
    else
      this.nodes.splice(index, 0, node);

    return this;
  }

  addVarDecl(type, name, init) {
    let node = type;

    if (!(node instanceof Var))
      node = new Var(type, name, init);

    let i = 0;
    while (i < this.nodes.length) {
      const n = this.nodes[i];
      if (n.kind === "var" && n.name === node.name && n.init === node.init)
        return this;

      if (n.kind !== "var")
        break;

      i++;
    }

    this.insertNode(i, node);
    return this;
  }

  addLine(code) {
    if (typeof code !== "string")
      FATAL("Block.addLine(): Line must be string");

    this.nodes.push(new Line(code));
    return this;
  }

  prependEmptyLine() {
    if (!this.isEmpty())
      this.nodes.splice(0, 0, new Line(""));
    return this;
  }

  addEmptyLine() {
    if (!this.isEmpty())
      this.nodes.push(new Line(""));
    return this;
  }

  toString() {
    let s = "";
    for (let node of this.nodes)
      s += String(node);
    return s;
  }
}
exports.Block = Block;

// A C++ 'condition' (if statement) and its 'body' if it's taken.
class If extends Node {
  constructor(cond, body) {
    super("if");

    if (body == null)
      body = new Block();

    if (!(body instanceof Block))
      FATAL("If() - body must be a Block");

    this.cond = cond;
    this.body = body;
  }

  toString() {
    const cond = String(this.cond);
    const body = String(this.body);

    return `if (${cond}) {\n` + Utils.indent(body, 2) + `}\n`;
  }
}
exports.If = If;

//! A C++ switch statement.
class Case extends Node {
  constructor(cond, body) {
    super("case");

    this.cond = cond;
    this.body = body || new Block();
  }

  toString() {
    let s = "";
    for (let node of this.body.nodes)
      s += String(node)

    if (this.cond !== "default")
      return `case ${this.cond}: {\n` + Utils.indent(s, 2) + `}\n`;
    else
      return `default: {\n` + Utils.indent(s, 2) + `}\n`;
  }
};
exports.Case = Case;

class Switch extends Node {
  constructor(expression, cases) {
    super("switch");

    this.expression = expression;
    this.cases = cases || [];
  }

  addCase(cond, body) {
    this.cases.push(new Case(cond, body));
    return this;
  }

  toString() {
    let s = "";
    for (let c of this.cases) {
      if (s)
        s += "\n";
      s += String(c);
    }

    return `switch (${this.expression}) {\n` + Utils.indent(s, 2) + `}\n`;
  }
}
exports.Switch = Switch;
