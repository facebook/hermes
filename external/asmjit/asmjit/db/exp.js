// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: (Zlib or Unlicense)

(function($scope, $as) {
"use strict";

const hasOwn = Object.prototype.hasOwnProperty;

// Supported Operators
// -------------------

const kUnaryOperators = {
  "-": {prec: 3, rtl : 1, emit: "-@1" },
  "~": {prec: 3, rtl : 1, emit: "~@1" },
  "!": {prec: 3, rtl : 1, emit: "!@1" }
};

const kBinaryOperators = {
  "*" : { prec: 5, rtl : 0, emit: "@1 * @2"  },
  "/" : { prec: 5, rtl : 0, emit: "@1 / @2"  },
  "%" : { prec: 5, rtl : 0, emit: "@1 % @2"  },
  "+" : { prec: 6, rtl : 0, emit: "@1 + @2"  },
  "-" : { prec: 6, rtl : 0, emit: "@1 - @2"  },
  ">>": { prec: 7, rtl : 0, emit: "@1 >> @2" },
  "<<": { prec: 7, rtl : 0, emit: "@1 << @2" },
  "<" : { prec: 9, rtl : 0, emit: "@1 < @2"  },
  ">" : { prec: 9, rtl : 0, emit: "@1 > @2"  },
  "<=": { prec: 9, rtl : 0, emit: "@1 <= @2" },
  ">=": { prec: 9, rtl : 0, emit: "@1 >= @2" },
  "==": { prec:10, rtl : 0, emit: "@1 == @2" },
  "!=": { prec:10, rtl : 0, emit: "@1 != @2" },
  "&" : { prec:11, rtl : 0, emit: "@1 & @2"  },
  "^" : { prec:12, rtl : 0, emit: "@1 ^ @2"  },
  "|" : { prec:13, rtl : 0, emit: "@1 | @2"  },
  "&&": { prec:14, rtl : 0, emit: "@1 && @2" },
  "||": { prec:15, rtl : 0, emit: "@1 || @2" },
  "?" : { prec:16, rtl : 0, emit: "@1 ? @2"  },
  ":" : { prec:16, rtl : 0, emit: "@1 : @2"  }
};

const kMaxOperatorLen = 4;

function rightAssociate(info, bPrec) {
  return info.prec > bPrec || (info.prec === bPrec && info.rtl);
}

// Expression Error
// ----------------

// Contains `message` and `position` members. If the `position` is not `-1` then it is
// a zero-based index, which points to a first character of the token near the error.
class ExpressionError extends Error {
  constructor(message, position) {
    super(message);
    this.name = "ExpressionError";
    this.message = message;
    this.position = position != null ? position : -1;
  }
}

function throwTokenizerError(token) {
  throw new ExpressionError(`Unexpected token '${token.data}'`, token.position);
}

function throwExpressionError(message, position) {
  throw new ExpressionError(message, position);
}

// Expression Tree
// ---------------

function mustEnclose(node) {
  return node.isUnary() ? node.child.isOperator() : node.isBinary() ? true : false;
}

class ExpNode {
  constructor(type) { this.type = type; }

  isImm() { return this.type === "imm"; }
  isVar() { return this.type === "var"; }
  isCall() { return this.type === "call"; }
  isUnary() { return this.type === "unary"; }
  isBinary() { return this.type === "binary"; }
  isOperator() { return this.type === "unary" || this.type === "binary"; }

  info() { return null; }
  clone() { throw new Error("ExpNode.clone() must be overridden"); }
  evaluate(ctx) { throw new Error("ExpNode.evaluate() must be overridden"); }
  toString(ctx) { throw new Error("ExpNode.toString() must be overridden"); }
}

class ImmNode extends ExpNode {
  constructor(imm) {
    super("imm");
    this.imm = imm || 0;
  }

  clone() { return new ImmNode(this.imm); }
  evaluate(ctx) { return this.imm; }
  toString(ctx) { return ctx ? ctx.stringifyImmediate(this.imm) : String(this.imm); }
}

class VarNode extends ExpNode {
  constructor(name) {
    super("var");
    this.name = name || "";
  }

  clone() { return new VarNode(this.name); }
  evaluate(ctx) { return ctx.variable(this.name); }
  toString(ctx) { return ctx ? ctx.stringifyVariable(this.name) : String(this.name); }
}

class CallNode extends ExpNode {
  constructor(name, args) {
    super("call");
    this.name = name || "";
    this.args = args || [];
  }

  clone() {
    return new CallNode(this.name, this.args.map(function(arg) { return arg.clone(); }));
  }

  evaluate(ctx) {
    const evaluatedArgs = this.args.map(function(arg) { return arg.evaluate(ctx); });
    return ctx.function(this.name, evaluatedArgs);
  }

  toString(ctx) {
    if (this.name === "$bit") {
      return `((${this.args[0]} >> ${this.args[1]}) & 1)`;
    }
    else {
      let argsCode = this.args.map(function(arg) { return arg.toString(ctx); }).join(", ");
      if (ctx)
        return `${ctx.stringifyFunction(this.name)}(${argsCode})`;
      else
        return `${this.name}(${argsCode})`;
    }
  }
}

class UnaryNode extends ExpNode {
  constructor(op, child) {
    if (!hasOwn.call(kUnaryOperators, op))
      throw new Error(`Invalid unary operator '${op}`);

    super("unary");
    this.op = op;
    this.child = child || null;
  }

  info() {
    return kUnaryOperators[this.op];
  }

  clone() {
    return new UnaryNode(this.op, this.left ? this.left.clone() : null);
  }

  evaluate(ctx) {
    const val = this.child.evaluate(ctx);
    switch (this.op) {
      case "-": return (-val);
      case "~": return (~val);
      case "!": return (val ? 0 : 1);
      default : return ctx.unary(this.op, val);
    }
  }

  toString(ctx) {
    return this.info().emit.replace(/@1/g, () => {
      const node = this.child;
      const code = node.toString(ctx);
      return mustEnclose(node) ? `(${code})` : code;
    });
  }
}

class BinaryNode extends ExpNode {
  constructor(op, left, right) {
    if (!hasOwn.call(kBinaryOperators, op))
      throw new Error(`Invalid binary operator '${op}`);

    super("binary");
    this.op = op || "";
    this.left = left || null;
    this.right = right || null;
  }

  info() {
    return kBinaryOperators[this.op];
  }

  clone() {
    return new BinaryNode(this.op, this.left ? this.left.clone() : null, this.right ? this.right.clone() : null);
  }

  evaluate(ctx) {
    const left = this.left.evaluate(ctx);
    const right = this.right.evaluate(ctx);

    switch (this.op) {
      case "-" : return left - right;
      case "+" : return left + right;
      case "*" : return left * right;
      case "/" : return (left / right)|0;
      case "%" : return (left % right)|0;
      case "&" : return left & right;
      case "|" : return left | right;
      case "^" : return left ^ right;
      case "<<": return left << right;
      case ">>": return left >> right;
      case "==": return left == right ? 1 : 0;
      case "!=": return left != right ? 1 : 0;
      case "<" : return left <  right ? 1 : 0;
      case "<=": return left <= right ? 1 : 0;
      case ">" : return left >  right ? 1 : 0;
      case ">=": return left >= right ? 1 : 0;
      case "&&": return left && right ? 1 : 0;
      case "||": return left || right ? 1 : 0;
      default  : return ctx.binary(this.op, left, right);
    }
  }

  toString(ctx) {
    return this.info().emit.replace(/@[1-2]/g, (p) => {
      const node = p === "@1" ? this.left : this.right;
      const code = node.toString(ctx);
      return mustEnclose(node) ? `(${code})` : code;
    });
  }
}

function Imm(imm) { return new ImmNode(imm); }
function Var(name) { return new VarNode(name); }
function Call(name, args) { return new CallNode(name, args); }
function Unary(op, child) { return new UnaryNode(op, child); }
function Binary(op, left, right) { return new BinaryNode(op, left, right); }

function Negate(child) { return Unary("-", child); }
function BitNot(child) { return Unary("~", child); }

function Add(left, right) { return Binary("+", left, right); }
function Sub(left, right) { return Binary("-", left, right); }
function Mul(left, right) { return Binary("*", left, right); }
function Div(left, right) { return Binary("/", left, right); }
function Mod(left, right) { return Binary("%", left, right); }
function Shl(left, right) { return Binary("<<", left, right); }
function Shr(left, right) { return Binary(">>", left, right); }
function BitAnd(left, right) { return Binary("&", left, right); }
function BitOr(left, right) { return Binary("|", left, right); }
function BitXor(left, right) { return Binary("^", left, right); }
function Eq(left, right) { return Binary("==", left, right); }
function Ne(left, right) { return Binary("!=", left, right); }
function Lt(left, right) { return Binary("<", left, right); }
function Le(left, right) { return Binary("<=", left, right); }
function Gt(left, right) { return Binary(">", left, right); }
function Ge(left, right) { return Binary(">=", left, right); }
function And(left, right) { return Binary("&&", left, right); }
function Or(left, right) { return Binary("||", left, right); }



// Expression Tokenizer
// --------------------

const kCharNone  = 0; // '_' - Character category - Invalid or <end>.
const kCharSpace = 1; // 'S' - Character category - Space.
const kCharAlpha = 2; // 'A' - Character category - Alpha [A-Za-z_].
const kCharDigit = 3; // 'D' - Character category - Digit [0-9].
const kCharPunct = 4; // '$' - Character category - Punctuation.

const Category = (function(_, S, A, D, $) {
  const Table = [
    _,_,_,_,_,_,_,_,_,S,S,S,S,S,_,_, // 000-015 |.........     ..|
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_, // 016-031 |................|
    S,$,$,$,$,$,$,$,$,$,$,$,$,$,$,$, // 032-047 | !"#$%&'()*+,-./|
    D,D,D,D,D,D,D,D,D,D,$,$,$,$,$,$, // 048-063 |0123456789:;<=>?|
    $,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 064-079 |@ABCDEFGHIJKLMNO|
    A,A,A,A,A,A,A,A,A,A,A,$,$,$,$,A, // 080-095 |PQRSTUVWXYZ[\]^_|
    $,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 096-111 |`abcdefghijklmno|
    A,A,A,A,A,A,A,A,A,A,A,$,$,$,$,_, // 112-127 |pqrstuvwxyz{|}~ |
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_, // 128-143 |................|
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_  // 144-159 |................|
  ];
  const kTableLength = Table.length;

  return function(c) {
    if (c < kTableLength)
      return Table[c];
    return kCharNone;
  };
})(kCharNone, kCharSpace, kCharAlpha, kCharDigit, kCharPunct);

const kTokenNone  = 0;
const kTokenPunct = 1;
const kTokenIdent = 2;
const kTokenValue = 3;

function newToken(type, position, data, value) {
  return {
    type    : type,     // Token type, see `kToken...`.
    position: position, // Token position in expression's source.
    data    : data,     // Token data (content) as string.
    value   : value     // Token value (only if the token is a value).
  };
}
const NoToken = newToken(kTokenNone, -1, "<end>", null);

// Must be reset before it can be used, use `RegExp.lastIndex`.
const reNumValue = /(?:(?:\d*\.\d+|\d+)(?:[E|e][+|-]?\d+)?)/g;

function parseHex(source, from) {
  let i = from;
  let number = 0;

  while (i < source.length) {
    let c = source.charCodeAt(i);
    let n = 0;

    if (c >= '0'.charCodeAt(0) && c <= '9'.charCodeAt(0)) {
      n = c - '0'.charCodeAt(0);
    }
    else if (c >= 'a'.charCodeAt(0) && c <= 'f'.charCodeAt(0)) {
      n = c - 'a'.charCodeAt(0) + 10;
    }
    else if (c >= 'A'.charCodeAt(0) && c <= 'F'.charCodeAt(0)) {
      n = c - 'A'.charCodeAt(0) + 10;
    }
    else if (c >= 'g'.charCodeAt(0) && c <= 'z'.charCodeAt(0) || c >= 'g'.charCodeAt(0) && c <= 'Z'.charCodeAt(0)) {
      throwExpressionError(`Invalid hex number 0x${source.substring(from, i + 1)}`);
    }
    else {
      break;
    }

    number = (number << 4) | n;
    i++;
  }

  if (i === from)
    throwExpressionError(`Invalid number starting with 0x`);

  return {
    number: number,
    end: i
  };
}

function tokenize(source) {
  const len = source.length;
  const tokens = [];

  let i = 0, j = 0; // Current index in `source` and temporary.
  let start = 0;    // Current token start position.
  let data = "";    // Current token data (content) as string.
  let c, cat;       // Current character code and category.

  while (i < len) {
    c = source.charCodeAt(i);
    cat = Category(c);

    if (cat === kCharSpace) {
      i++;
    }
    else if (cat === kCharDigit) {
      const n = tokens.length - 1;

      // Hex number.
      if (c === '0'.charCodeAt(0) && i + 1 < len && source.charCodeAt(i + 1) === 'x'.charCodeAt(0)) {
        const status = parseHex(source, i + 2);
        tokens.push(newToken(kTokenValue, i, source.substring(i, status.end), status.number));
        i = status.end;
      }
      else {
        if (n >= 0 && tokens[n].data === "." && source[i - 1] === ".") {
          tokens.length = n;
          i--;
        }

        reNumValue.lastIndex = i;
        data = reNumValue.exec(source)[0];

        tokens.push(newToken(kTokenValue, i, data, parseFloat(data)));
        i += data.length;
      }
    }
    else if (cat === kCharAlpha) {
      start = i;
      while (++i < len && ((cat = Category(source.charCodeAt(i))) === kCharAlpha || cat === kCharDigit))
        continue;

      data = source.substring(start, i);
      tokens.push(newToken(kTokenIdent, start, data, null));
    }
    else if (cat === kCharPunct) {
      start = i;
      while (++i < len && Category(source.charCodeAt(i)) === kCharPunct)
        continue;

      data = source.substring(start, i);
      do {
        for (j = Math.min(i - start, kMaxOperatorLen); j > 0; j--) {
          const part = source.substr(start, j);
          if (hasOwn.call(kUnaryOperators, part) || hasOwn.call(kBinaryOperators, part) || j === 1) {
            tokens.push(newToken(kTokenPunct, start, part, null));
            start += j;
            break;
          }
        }
      } while (start < i);
    }
    else {
      throwExpressionError(`Unrecognized character '0x${c.toString(16)}'`, i);
    }
  }

  return tokens;
}

// Expression Parser
// -----------------

class Parser {
  constructor(tokens) {
    this.tokens = tokens;
    this.tIndex = 0;
  }

  peek() { return this.tIndex < this.tokens.length ? this.tokens[this.tIndex  ] : NoToken; }
  next() { return this.tIndex < this.tokens.length ? this.tokens[this.tIndex++] : NoToken; }
  skip() { this.tIndex++; return this; }
  back(token) { this.tIndex -= +(token !== NoToken); return this; }

  parse() {
    // The root expression cannot be empty.
    let token = this.peek();
    if (token === NoToken)
      throwExpressionError("Expression cannot be empty", 0);

    const exp = this.parseExpression();

    // The root expression must reach the end of the input.
    token = this.peek();
    if (token !== NoToken)
      throwTokenizerError(token);

    return exp;
  }

  parseExpression() {
    const stack = [];
    let value = null;
    let token = null;

    for (;;) {
      // The only case of value not being `null` is after ternary-if. In that
      // case the value was already parsed so we want to skip this section.
      if (value === null) {
        let unaryFirst = null;
        let unaryLast = null;

        token = this.next();

        // Parse a possible unary operator(s).
        if (token.type === kTokenPunct) {
          do {
            const opName = token.data;
            const opInfo = kUnaryOperators[opName];

            if (!opInfo)
              break;

            const node = Unary(opName);
            if (unaryLast)
              unaryLast.child = node;
            else
              unaryFirst = node;

            unaryLast = node;
            token = this.next();
          } while (token.type === kTokenPunct);
        }

        // Parse a value, variable, function call, or nested expression.
        if (token.type === kTokenValue) {
          value = Imm(token.value);
        }
        else if (token.type === kTokenIdent) {
          const name = token.data;
          const after = this.peek();

          if (after.data === "(")
            value = this.parseCall(token.data);
          else if (after.data === "[")
            value = this.parseBitAccess(token.data);
          else
            value = Var(name);
        }
        else if (token.data === "(") {
          value = this.parseExpression();
          token = this.next();

          if (token.data !== ")")
            throwTokenizerError(token);
        }
        else {
          throwTokenizerError(token);
        }

        // Replace the value with the top-level unary operator, if parsed.
        if (unaryFirst) {
          unaryLast.child = value;
          value = unaryFirst;
        }
      }

      // Parse a possible binary operator - the loop must repeat, if present.
      token = this.peek();
      if (token.type === kTokenPunct && hasOwn.call(kBinaryOperators, token.data)) {
        const opName = token.data;
        if (opName === ":")
          break;

        // Consume the token.
        this.skip();

        const bNode = Binary(opName, null, null);

        if (!stack.length) {
          bNode.left = value;
          stack.push(bNode);
        }
        else {
          let aNode = stack.pop();
          let aPrec = aNode.info().prec;
          let bPrec = bNode.info().prec;

          if (aPrec > bPrec) {
            aNode.right = bNode;
            bNode.left = value;
            stack.push(aNode, bNode);
          }
          else {
            aNode.right = value;

            // Advance to the top-most op that has less/equal precedence than `bPrec`.
            while (stack.length) {
              if (rightAssociate(aNode.info(), bPrec))
                break;
              aNode = stack.pop();
            }

            if (!stack.length && !rightAssociate(aNode.info(), bPrec)) {
              bNode.left = aNode;
              stack.push(bNode);
            }
            else {
              const tmp = aNode.right;
              aNode.right = bNode;
              bNode.left = tmp;
              stack.push(aNode, bNode);
            }
          }
        }

        // Parse "<cond> {ternary-if} <taken> {ternary-else} <not-taken>".
        if (opName === "?") {
          const ternLeft = this.parseExpression();
          const ternTok = this.next();

          if (ternTok.data !== ":")
            throwExpressionError(`Unterminated ternary if '${token.data}'`, token.position);

          const ternRight = this.parseExpression();
          value = Binary(opName, info, ternLeft, ternRight);
        }
        else {
          value = null;
        }

        continue;
      }

      break;
    }

    if (value === null)
      throwExpressionError("Invalid expression");

    if (stack.length !== 0) {
      stack[stack.length - 1].right = value;
      value = stack[0];
    }

    return value;
  }

  parseCall(name) {
    const args = [];

    let token = this.next();
    if (token.data !== "(")
      throwTokenizerError(token);

    for (;;) {
      token = this.peek();
      if (token.data === ")")
        break;

      if (args.length !== 0) {
        if (token.data !== ",")
          throwTokenizerError(token);
        this.skip();
      }

      args.push(this.parseExpression());
    }

    this.skip();
    return Call(name, args);
  }

  parseBitAccess(name) {
    let token = this.next();
    if (token.data !== "[")
      throwTokenizerError(token);

    token = this.next();
    if (token.type != kTokenValue)
      throwTokenizerError(token);

    const index = token.value;

    token = this.next();
    if (token.data !== "]")
      throwTokenizerError(token);

    return Call("$bit", [Var(name), index]);
  }
}

function parse(source) {
  const tokens = tokenize(source);
  return new Parser(tokens).parse();
}

// Expression Visitors
// -------------------

class Visitor {
  visit(node) {
    switch (node.type) {
      case "imm":
      case "var": {
        break;
      }

      case "call": {
        for (let arg of node.args)
          this.visit(arg);
        break;
      }

      case "unary": {
        if (node.child)
          this.visit(node.child);
        break;
      }

      case "binary": {
        if (node.left)
          this.visit(node.left);
        if (node.right)
          this.visit(node.right);
        break;
      }

      default: {
        throw new Error(`Visitor.visit(): Unknown node type '${node.type}'`);
      }
    }
  }
}

class Collector extends Visitor {
  constructor(nodeType, dst) {
    super();
    this.dict = dst || Object.create(null);
    this.nodeType = nodeType;
  }

  visit(node) {
    if (node.type === this.nodeType) {
      if (hasOwn.call(this.dict, node.name))
        this.dict[node.name]++;
      else
        this.dict[node.name] = 1;
    }

    super.visit(node);
  }
}

function collectVars(node, dst) {
  const collector = new Collector("var", dst);
  collector.visit(node)
  return collector.dict;
}

function collectCalls(node, dst) {
  const collector = new Collector("call", dst);
  collector.visit(node)
  return collector.dict;
}

// Exports
// -------

$scope[$as] = {
  Imm: Imm,
  Var: Var,
  Call: Call,
  Unary: Unary,
  Binary: Binary,

  Negate: Negate,
  BitNot: BitNot,

  Add: Add,
  Sub: Sub,
  Mul: Mul,
  Div: Div,
  Mod: Mod,
  Shl: Shl,
  Shr: Shr,
  BitAnd: BitAnd,
  BitOr: BitOr,
  BitXor: BitXor,
  Eq: Eq,
  Ne: Ne,
  Lt: Lt,
  Le: Le,
  Gt: Gt,
  Ge: Ge,
  And: And,
  Or: Or,

  Visitor: Visitor,
  ExpressionError: ExpressionError,

  parse: parse,
  collectVars: collectVars,
  collectCalls: collectCalls
};

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "exp"]);
