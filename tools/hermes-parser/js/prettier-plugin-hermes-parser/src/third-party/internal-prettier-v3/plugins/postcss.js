(function (factory) {
  function interopModuleDefault() {
    var module = factory();
    return module.default || module;
  }

  if (typeof exports === "object" && typeof module === "object") {
    module.exports = interopModuleDefault();
  } else if (typeof define === "function" && define.amd) {
    define(interopModuleDefault);
  } else {
    var root =
      typeof globalThis !== "undefined"
        ? globalThis
        : typeof global !== "undefined"
        ? global
        : typeof self !== "undefined"
        ? self
        : this || {};
    root.prettierPlugins = root.prettierPlugins || {};
    root.prettierPlugins.postcss = interopModuleDefault();
  }
})(function() {
  "use strict";
  var __create = Object.create;
  var __defProp = Object.defineProperty;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __getProtoOf = Object.getPrototypeOf;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
  var __commonJS = (cb, mod) => function __require() {
    return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
  };
  var __export = (target, all) => {
    for (var name in all)
      __defProp(target, name, { get: all[name], enumerable: true });
  };
  var __copyProps = (to, from, except, desc) => {
    if (from && typeof from === "object" || typeof from === "function") {
      for (let key of __getOwnPropNames(from))
        if (!__hasOwnProp.call(to, key) && key !== except)
          __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
    }
    return to;
  };
  var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
    // If the importer is in node compatibility mode or this is not an ESM
    // file that has been converted to a CommonJS file using a Babel-
    // compatible transform (i.e. "__esModule" has not been set), then set
    // "default" to the CommonJS "module.exports" for node compatibility.
    isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
    mod
  ));
  var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

  // node_modules/jest-docblock/build/index.js
  var require_build = __commonJS({
    "node_modules/jest-docblock/build/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.extract = extract2;
      exports.parse = parse3;
      exports.parseWithComments = parseWithComments2;
      exports.print = print3;
      exports.strip = strip2;
      var commentEndRe = /\*\/$/;
      var commentStartRe = /^\/\*\*?/;
      var docblockRe = /^\s*(\/\*\*?(.|\r?\n)*?\*\/)/;
      var lineCommentRe = /(^|\s+)\/\/([^\r\n]*)/g;
      var ltrimNewlineRe = /^(\r?\n)+/;
      var multilineRe = /(?:^|\r?\n) *(@[^\r\n]*?) *\r?\n *(?![^@\r\n]*\/\/[^]*)([^@\r\n\s][^@\r\n]+?) *\r?\n/g;
      var propertyRe = /(?:^|\r?\n) *@(\S+) *([^\r\n]*)/g;
      var stringStartRe = /(\r?\n|^) *\* ?/g;
      var STRING_ARRAY = [];
      function extract2(contents) {
        const match = contents.match(docblockRe);
        return match ? match[0].trimLeft() : "";
      }
      function strip2(contents) {
        const match = contents.match(docblockRe);
        return match && match[0] ? contents.substring(match[0].length) : contents;
      }
      function parse3(docblock) {
        return parseWithComments2(docblock).pragmas;
      }
      function parseWithComments2(docblock) {
        const line2 = "\n";
        docblock = docblock.replace(commentStartRe, "").replace(commentEndRe, "").replace(stringStartRe, "$1");
        let prev = "";
        while (prev !== docblock) {
          prev = docblock;
          docblock = docblock.replace(multilineRe, `${line2}$1 $2${line2}`);
        }
        docblock = docblock.replace(ltrimNewlineRe, "").trimRight();
        const result = /* @__PURE__ */ Object.create(null);
        const comments = docblock.replace(propertyRe, "").replace(ltrimNewlineRe, "").trimRight();
        let match;
        while (match = propertyRe.exec(docblock)) {
          const nextPragma = match[2].replace(lineCommentRe, "");
          if (typeof result[match[1]] === "string" || Array.isArray(result[match[1]])) {
            result[match[1]] = STRING_ARRAY.concat(result[match[1]], nextPragma);
          } else {
            result[match[1]] = nextPragma;
          }
        }
        return {
          comments,
          pragmas: result
        };
      }
      function print3({ comments = "", pragmas = {} }) {
        const line2 = "\n";
        const head = "/**";
        const start = " *";
        const tail = " */";
        const keys = Object.keys(pragmas);
        const printedObject = keys.map((key) => printKeyValues(key, pragmas[key])).reduce((arr, next) => arr.concat(next), []).map((keyValue) => `${start} ${keyValue}${line2}`).join("");
        if (!comments) {
          if (keys.length === 0) {
            return "";
          }
          if (keys.length === 1 && !Array.isArray(pragmas[keys[0]])) {
            const value = pragmas[keys[0]];
            return `${head} ${printKeyValues(keys[0], value)[0]}${tail}`;
          }
        }
        const printedComments = comments.split(line2).map((textLine) => `${start} ${textLine}`).join(line2) + line2;
        return head + line2 + (comments ? printedComments : "") + (comments && keys.length ? start + line2 : "") + printedObject + tail;
      }
      function printKeyValues(key, valueOrArray) {
        return STRING_ARRAY.concat(valueOrArray).map(
          (value) => `@${key} ${value}`.trim()
        );
      }
    }
  });

  // node_modules/postcss/lib/symbols.js
  var require_symbols = __commonJS({
    "node_modules/postcss/lib/symbols.js"(exports, module) {
      "use strict";
      module.exports.isClean = Symbol("isClean");
      module.exports.my = Symbol("my");
    }
  });

  // node_modules/picocolors/picocolors.browser.js
  var require_picocolors_browser = __commonJS({
    "node_modules/picocolors/picocolors.browser.js"(exports, module) {
      var x = String;
      var create = function() {
        return { isColorSupported: false, reset: x, bold: x, dim: x, italic: x, underline: x, inverse: x, hidden: x, strikethrough: x, black: x, red: x, green: x, yellow: x, blue: x, magenta: x, cyan: x, white: x, gray: x, bgBlack: x, bgRed: x, bgGreen: x, bgYellow: x, bgBlue: x, bgMagenta: x, bgCyan: x, bgWhite: x };
      };
      module.exports = create();
      module.exports.createColors = create;
    }
  });

  // (disabled):node_modules/postcss/lib/terminal-highlight
  var require_terminal_highlight = __commonJS({
    "(disabled):node_modules/postcss/lib/terminal-highlight"() {
    }
  });

  // node_modules/postcss/lib/css-syntax-error.js
  var require_css_syntax_error = __commonJS({
    "node_modules/postcss/lib/css-syntax-error.js"(exports, module) {
      "use strict";
      var pico = require_picocolors_browser();
      var terminalHighlight = require_terminal_highlight();
      var CssSyntaxError = class _CssSyntaxError extends Error {
        constructor(message, line2, column, source, file, plugin) {
          super(message);
          this.name = "CssSyntaxError";
          this.reason = message;
          if (file) {
            this.file = file;
          }
          if (source) {
            this.source = source;
          }
          if (plugin) {
            this.plugin = plugin;
          }
          if (typeof line2 !== "undefined" && typeof column !== "undefined") {
            if (typeof line2 === "number") {
              this.line = line2;
              this.column = column;
            } else {
              this.line = line2.line;
              this.column = line2.column;
              this.endLine = column.line;
              this.endColumn = column.column;
            }
          }
          this.setMessage();
          if (Error.captureStackTrace) {
            Error.captureStackTrace(this, _CssSyntaxError);
          }
        }
        setMessage() {
          this.message = this.plugin ? this.plugin + ": " : "";
          this.message += this.file ? this.file : "<css input>";
          if (typeof this.line !== "undefined") {
            this.message += ":" + this.line + ":" + this.column;
          }
          this.message += ": " + this.reason;
        }
        showSourceCode(color) {
          if (!this.source)
            return "";
          let css2 = this.source;
          if (color == null)
            color = pico.isColorSupported;
          if (terminalHighlight) {
            if (color)
              css2 = terminalHighlight(css2);
          }
          let lines = css2.split(/\r?\n/);
          let start = Math.max(this.line - 3, 0);
          let end = Math.min(this.line + 2, lines.length);
          let maxWidth = String(end).length;
          let mark, aside;
          if (color) {
            let { bold, gray, red } = pico.createColors(true);
            mark = (text) => bold(red(text));
            aside = (text) => gray(text);
          } else {
            mark = aside = (str) => str;
          }
          return lines.slice(start, end).map((line2, index) => {
            let number = start + 1 + index;
            let gutter = " " + (" " + number).slice(-maxWidth) + " | ";
            if (number === this.line) {
              let spacing = aside(gutter.replace(/\d/g, " ")) + line2.slice(0, this.column - 1).replace(/[^\t]/g, " ");
              return mark(">") + aside(gutter) + line2 + "\n " + spacing + mark("^");
            }
            return " " + aside(gutter) + line2;
          }).join("\n");
        }
        toString() {
          let code = this.showSourceCode();
          if (code) {
            code = "\n\n" + code + "\n";
          }
          return this.name + ": " + this.message + code;
        }
      };
      module.exports = CssSyntaxError;
      CssSyntaxError.default = CssSyntaxError;
    }
  });

  // node_modules/postcss/lib/stringifier.js
  var require_stringifier = __commonJS({
    "node_modules/postcss/lib/stringifier.js"(exports, module) {
      "use strict";
      var DEFAULT_RAW = {
        after: "\n",
        beforeClose: "\n",
        beforeComment: "\n",
        beforeDecl: "\n",
        beforeOpen: " ",
        beforeRule: "\n",
        colon: ": ",
        commentLeft: " ",
        commentRight: " ",
        emptyBody: "",
        indent: "    ",
        semicolon: false
      };
      function capitalize(str) {
        return str[0].toUpperCase() + str.slice(1);
      }
      var Stringifier = class {
        constructor(builder) {
          this.builder = builder;
        }
        atrule(node, semicolon) {
          let name = "@" + node.name;
          let params = node.params ? this.rawValue(node, "params") : "";
          if (typeof node.raws.afterName !== "undefined") {
            name += node.raws.afterName;
          } else if (params) {
            name += " ";
          }
          if (node.nodes) {
            this.block(node, name + params);
          } else {
            let end = (node.raws.between || "") + (semicolon ? ";" : "");
            this.builder(name + params + end, node);
          }
        }
        beforeAfter(node, detect) {
          let value;
          if (node.type === "decl") {
            value = this.raw(node, null, "beforeDecl");
          } else if (node.type === "comment") {
            value = this.raw(node, null, "beforeComment");
          } else if (detect === "before") {
            value = this.raw(node, null, "beforeRule");
          } else {
            value = this.raw(node, null, "beforeClose");
          }
          let buf = node.parent;
          let depth = 0;
          while (buf && buf.type !== "root") {
            depth += 1;
            buf = buf.parent;
          }
          if (value.includes("\n")) {
            let indent2 = this.raw(node, null, "indent");
            if (indent2.length) {
              for (let step = 0; step < depth; step++)
                value += indent2;
            }
          }
          return value;
        }
        block(node, start) {
          let between = this.raw(node, "between", "beforeOpen");
          this.builder(start + between + "{", node, "start");
          let after;
          if (node.nodes && node.nodes.length) {
            this.body(node);
            after = this.raw(node, "after");
          } else {
            after = this.raw(node, "after", "emptyBody");
          }
          if (after)
            this.builder(after);
          this.builder("}", node, "end");
        }
        body(node) {
          let last = node.nodes.length - 1;
          while (last > 0) {
            if (node.nodes[last].type !== "comment")
              break;
            last -= 1;
          }
          let semicolon = this.raw(node, "semicolon");
          for (let i = 0; i < node.nodes.length; i++) {
            let child = node.nodes[i];
            let before = this.raw(child, "before");
            if (before)
              this.builder(before);
            this.stringify(child, last !== i || semicolon);
          }
        }
        comment(node) {
          let left = this.raw(node, "left", "commentLeft");
          let right = this.raw(node, "right", "commentRight");
          this.builder("/*" + left + node.text + right + "*/", node);
        }
        decl(node, semicolon) {
          let between = this.raw(node, "between", "colon");
          let string = node.prop + between + this.rawValue(node, "value");
          if (node.important) {
            string += node.raws.important || " !important";
          }
          if (semicolon)
            string += ";";
          this.builder(string, node);
        }
        document(node) {
          this.body(node);
        }
        raw(node, own, detect) {
          let value;
          if (!detect)
            detect = own;
          if (own) {
            value = node.raws[own];
            if (typeof value !== "undefined")
              return value;
          }
          let parent = node.parent;
          if (detect === "before") {
            if (!parent || parent.type === "root" && parent.first === node) {
              return "";
            }
            if (parent && parent.type === "document") {
              return "";
            }
          }
          if (!parent)
            return DEFAULT_RAW[detect];
          let root = node.root();
          if (!root.rawCache)
            root.rawCache = {};
          if (typeof root.rawCache[detect] !== "undefined") {
            return root.rawCache[detect];
          }
          if (detect === "before" || detect === "after") {
            return this.beforeAfter(node, detect);
          } else {
            let method = "raw" + capitalize(detect);
            if (this[method]) {
              value = this[method](root, node);
            } else {
              root.walk((i) => {
                value = i.raws[own];
                if (typeof value !== "undefined")
                  return false;
              });
            }
          }
          if (typeof value === "undefined")
            value = DEFAULT_RAW[detect];
          root.rawCache[detect] = value;
          return value;
        }
        rawBeforeClose(root) {
          let value;
          root.walk((i) => {
            if (i.nodes && i.nodes.length > 0) {
              if (typeof i.raws.after !== "undefined") {
                value = i.raws.after;
                if (value.includes("\n")) {
                  value = value.replace(/[^\n]+$/, "");
                }
                return false;
              }
            }
          });
          if (value)
            value = value.replace(/\S/g, "");
          return value;
        }
        rawBeforeComment(root, node) {
          let value;
          root.walkComments((i) => {
            if (typeof i.raws.before !== "undefined") {
              value = i.raws.before;
              if (value.includes("\n")) {
                value = value.replace(/[^\n]+$/, "");
              }
              return false;
            }
          });
          if (typeof value === "undefined") {
            value = this.raw(node, null, "beforeDecl");
          } else if (value) {
            value = value.replace(/\S/g, "");
          }
          return value;
        }
        rawBeforeDecl(root, node) {
          let value;
          root.walkDecls((i) => {
            if (typeof i.raws.before !== "undefined") {
              value = i.raws.before;
              if (value.includes("\n")) {
                value = value.replace(/[^\n]+$/, "");
              }
              return false;
            }
          });
          if (typeof value === "undefined") {
            value = this.raw(node, null, "beforeRule");
          } else if (value) {
            value = value.replace(/\S/g, "");
          }
          return value;
        }
        rawBeforeOpen(root) {
          let value;
          root.walk((i) => {
            if (i.type !== "decl") {
              value = i.raws.between;
              if (typeof value !== "undefined")
                return false;
            }
          });
          return value;
        }
        rawBeforeRule(root) {
          let value;
          root.walk((i) => {
            if (i.nodes && (i.parent !== root || root.first !== i)) {
              if (typeof i.raws.before !== "undefined") {
                value = i.raws.before;
                if (value.includes("\n")) {
                  value = value.replace(/[^\n]+$/, "");
                }
                return false;
              }
            }
          });
          if (value)
            value = value.replace(/\S/g, "");
          return value;
        }
        rawColon(root) {
          let value;
          root.walkDecls((i) => {
            if (typeof i.raws.between !== "undefined") {
              value = i.raws.between.replace(/[^\s:]/g, "");
              return false;
            }
          });
          return value;
        }
        rawEmptyBody(root) {
          let value;
          root.walk((i) => {
            if (i.nodes && i.nodes.length === 0) {
              value = i.raws.after;
              if (typeof value !== "undefined")
                return false;
            }
          });
          return value;
        }
        rawIndent(root) {
          if (root.raws.indent)
            return root.raws.indent;
          let value;
          root.walk((i) => {
            let p = i.parent;
            if (p && p !== root && p.parent && p.parent === root) {
              if (typeof i.raws.before !== "undefined") {
                let parts = i.raws.before.split("\n");
                value = parts[parts.length - 1];
                value = value.replace(/\S/g, "");
                return false;
              }
            }
          });
          return value;
        }
        rawSemicolon(root) {
          let value;
          root.walk((i) => {
            if (i.nodes && i.nodes.length && i.last.type === "decl") {
              value = i.raws.semicolon;
              if (typeof value !== "undefined")
                return false;
            }
          });
          return value;
        }
        rawValue(node, prop) {
          let value = node[prop];
          let raw = node.raws[prop];
          if (raw && raw.value === value) {
            return raw.raw;
          }
          return value;
        }
        root(node) {
          this.body(node);
          if (node.raws.after)
            this.builder(node.raws.after);
        }
        rule(node) {
          this.block(node, this.rawValue(node, "selector"));
          if (node.raws.ownSemicolon) {
            this.builder(node.raws.ownSemicolon, node, "end");
          }
        }
        stringify(node, semicolon) {
          if (!this[node.type]) {
            throw new Error(
              "Unknown AST node type " + node.type + ". Maybe you need to change PostCSS stringifier."
            );
          }
          this[node.type](node, semicolon);
        }
      };
      module.exports = Stringifier;
      Stringifier.default = Stringifier;
    }
  });

  // node_modules/postcss/lib/stringify.js
  var require_stringify = __commonJS({
    "node_modules/postcss/lib/stringify.js"(exports, module) {
      "use strict";
      var Stringifier = require_stringifier();
      function stringify(node, builder) {
        let str = new Stringifier(builder);
        str.stringify(node);
      }
      module.exports = stringify;
      stringify.default = stringify;
    }
  });

  // node_modules/postcss/lib/node.js
  var require_node = __commonJS({
    "node_modules/postcss/lib/node.js"(exports, module) {
      "use strict";
      var { isClean, my } = require_symbols();
      var CssSyntaxError = require_css_syntax_error();
      var Stringifier = require_stringifier();
      var stringify = require_stringify();
      function cloneNode(obj, parent) {
        let cloned = new obj.constructor();
        for (let i in obj) {
          if (!Object.prototype.hasOwnProperty.call(obj, i)) {
            continue;
          }
          if (i === "proxyCache")
            continue;
          let value = obj[i];
          let type = typeof value;
          if (i === "parent" && type === "object") {
            if (parent)
              cloned[i] = parent;
          } else if (i === "source") {
            cloned[i] = value;
          } else if (Array.isArray(value)) {
            cloned[i] = value.map((j) => cloneNode(j, cloned));
          } else {
            if (type === "object" && value !== null)
              value = cloneNode(value);
            cloned[i] = value;
          }
        }
        return cloned;
      }
      var Node = class {
        constructor(defaults = {}) {
          this.raws = {};
          this[isClean] = false;
          this[my] = true;
          for (let name in defaults) {
            if (name === "nodes") {
              this.nodes = [];
              for (let node of defaults[name]) {
                if (typeof node.clone === "function") {
                  this.append(node.clone());
                } else {
                  this.append(node);
                }
              }
            } else {
              this[name] = defaults[name];
            }
          }
        }
        addToError(error) {
          error.postcssNode = this;
          if (error.stack && this.source && /\n\s{4}at /.test(error.stack)) {
            let s = this.source;
            error.stack = error.stack.replace(
              /\n\s{4}at /,
              `$&${s.input.from}:${s.start.line}:${s.start.column}$&`
            );
          }
          return error;
        }
        after(add) {
          this.parent.insertAfter(this, add);
          return this;
        }
        assign(overrides = {}) {
          for (let name in overrides) {
            this[name] = overrides[name];
          }
          return this;
        }
        before(add) {
          this.parent.insertBefore(this, add);
          return this;
        }
        cleanRaws(keepBetween) {
          delete this.raws.before;
          delete this.raws.after;
          if (!keepBetween)
            delete this.raws.between;
        }
        clone(overrides = {}) {
          let cloned = cloneNode(this);
          for (let name in overrides) {
            cloned[name] = overrides[name];
          }
          return cloned;
        }
        cloneAfter(overrides = {}) {
          let cloned = this.clone(overrides);
          this.parent.insertAfter(this, cloned);
          return cloned;
        }
        cloneBefore(overrides = {}) {
          let cloned = this.clone(overrides);
          this.parent.insertBefore(this, cloned);
          return cloned;
        }
        error(message, opts = {}) {
          if (this.source) {
            let { end, start } = this.rangeBy(opts);
            return this.source.input.error(
              message,
              { column: start.column, line: start.line },
              { column: end.column, line: end.line },
              opts
            );
          }
          return new CssSyntaxError(message);
        }
        getProxyProcessor() {
          return {
            get(node, prop) {
              if (prop === "proxyOf") {
                return node;
              } else if (prop === "root") {
                return () => node.root().toProxy();
              } else {
                return node[prop];
              }
            },
            set(node, prop, value) {
              if (node[prop] === value)
                return true;
              node[prop] = value;
              if (prop === "prop" || prop === "value" || prop === "name" || prop === "params" || prop === "important" || /* c8 ignore next */
              prop === "text") {
                node.markDirty();
              }
              return true;
            }
          };
        }
        markDirty() {
          if (this[isClean]) {
            this[isClean] = false;
            let next = this;
            while (next = next.parent) {
              next[isClean] = false;
            }
          }
        }
        next() {
          if (!this.parent)
            return void 0;
          let index = this.parent.index(this);
          return this.parent.nodes[index + 1];
        }
        positionBy(opts, stringRepresentation) {
          let pos = this.source.start;
          if (opts.index) {
            pos = this.positionInside(opts.index, stringRepresentation);
          } else if (opts.word) {
            stringRepresentation = this.toString();
            let index = stringRepresentation.indexOf(opts.word);
            if (index !== -1)
              pos = this.positionInside(index, stringRepresentation);
          }
          return pos;
        }
        positionInside(index, stringRepresentation) {
          let string = stringRepresentation || this.toString();
          let column = this.source.start.column;
          let line2 = this.source.start.line;
          for (let i = 0; i < index; i++) {
            if (string[i] === "\n") {
              column = 1;
              line2 += 1;
            } else {
              column += 1;
            }
          }
          return { column, line: line2 };
        }
        prev() {
          if (!this.parent)
            return void 0;
          let index = this.parent.index(this);
          return this.parent.nodes[index - 1];
        }
        get proxyOf() {
          return this;
        }
        rangeBy(opts) {
          let start = {
            column: this.source.start.column,
            line: this.source.start.line
          };
          let end = this.source.end ? {
            column: this.source.end.column + 1,
            line: this.source.end.line
          } : {
            column: start.column + 1,
            line: start.line
          };
          if (opts.word) {
            let stringRepresentation = this.toString();
            let index = stringRepresentation.indexOf(opts.word);
            if (index !== -1) {
              start = this.positionInside(index, stringRepresentation);
              end = this.positionInside(index + opts.word.length, stringRepresentation);
            }
          } else {
            if (opts.start) {
              start = {
                column: opts.start.column,
                line: opts.start.line
              };
            } else if (opts.index) {
              start = this.positionInside(opts.index);
            }
            if (opts.end) {
              end = {
                column: opts.end.column,
                line: opts.end.line
              };
            } else if (opts.endIndex) {
              end = this.positionInside(opts.endIndex);
            } else if (opts.index) {
              end = this.positionInside(opts.index + 1);
            }
          }
          if (end.line < start.line || end.line === start.line && end.column <= start.column) {
            end = { column: start.column + 1, line: start.line };
          }
          return { end, start };
        }
        raw(prop, defaultType) {
          let str = new Stringifier();
          return str.raw(this, prop, defaultType);
        }
        remove() {
          if (this.parent) {
            this.parent.removeChild(this);
          }
          this.parent = void 0;
          return this;
        }
        replaceWith(...nodes) {
          if (this.parent) {
            let bookmark = this;
            let foundSelf = false;
            for (let node of nodes) {
              if (node === this) {
                foundSelf = true;
              } else if (foundSelf) {
                this.parent.insertAfter(bookmark, node);
                bookmark = node;
              } else {
                this.parent.insertBefore(bookmark, node);
              }
            }
            if (!foundSelf) {
              this.remove();
            }
          }
          return this;
        }
        root() {
          let result = this;
          while (result.parent && result.parent.type !== "document") {
            result = result.parent;
          }
          return result;
        }
        toJSON(_, inputs) {
          let fixed = {};
          let emitInputs = inputs == null;
          inputs = inputs || /* @__PURE__ */ new Map();
          let inputsNextIndex = 0;
          for (let name in this) {
            if (!Object.prototype.hasOwnProperty.call(this, name)) {
              continue;
            }
            if (name === "parent" || name === "proxyCache")
              continue;
            let value = this[name];
            if (Array.isArray(value)) {
              fixed[name] = value.map((i) => {
                if (typeof i === "object" && i.toJSON) {
                  return i.toJSON(null, inputs);
                } else {
                  return i;
                }
              });
            } else if (typeof value === "object" && value.toJSON) {
              fixed[name] = value.toJSON(null, inputs);
            } else if (name === "source") {
              let inputId = inputs.get(value.input);
              if (inputId == null) {
                inputId = inputsNextIndex;
                inputs.set(value.input, inputsNextIndex);
                inputsNextIndex++;
              }
              fixed[name] = {
                end: value.end,
                inputId,
                start: value.start
              };
            } else {
              fixed[name] = value;
            }
          }
          if (emitInputs) {
            fixed.inputs = [...inputs.keys()].map((input) => input.toJSON());
          }
          return fixed;
        }
        toProxy() {
          if (!this.proxyCache) {
            this.proxyCache = new Proxy(this, this.getProxyProcessor());
          }
          return this.proxyCache;
        }
        toString(stringifier = stringify) {
          if (stringifier.stringify)
            stringifier = stringifier.stringify;
          let result = "";
          stringifier(this, (i) => {
            result += i;
          });
          return result;
        }
        warn(result, text, opts) {
          let data = { node: this };
          for (let i in opts)
            data[i] = opts[i];
          return result.warn(text, data);
        }
      };
      module.exports = Node;
      Node.default = Node;
    }
  });

  // node_modules/postcss/lib/declaration.js
  var require_declaration = __commonJS({
    "node_modules/postcss/lib/declaration.js"(exports, module) {
      "use strict";
      var Node = require_node();
      var Declaration = class extends Node {
        constructor(defaults) {
          if (defaults && typeof defaults.value !== "undefined" && typeof defaults.value !== "string") {
            defaults = { ...defaults, value: String(defaults.value) };
          }
          super(defaults);
          this.type = "decl";
        }
        get variable() {
          return this.prop.startsWith("--") || this.prop[0] === "$";
        }
      };
      module.exports = Declaration;
      Declaration.default = Declaration;
    }
  });

  // node_modules/postcss/lib/comment.js
  var require_comment = __commonJS({
    "node_modules/postcss/lib/comment.js"(exports, module) {
      "use strict";
      var Node = require_node();
      var Comment = class extends Node {
        constructor(defaults) {
          super(defaults);
          this.type = "comment";
        }
      };
      module.exports = Comment;
      Comment.default = Comment;
    }
  });

  // node_modules/postcss/lib/container.js
  var require_container = __commonJS({
    "node_modules/postcss/lib/container.js"(exports, module) {
      "use strict";
      var { isClean, my } = require_symbols();
      var Declaration = require_declaration();
      var Comment = require_comment();
      var Node = require_node();
      var parse3;
      var Rule;
      var AtRule;
      var Root;
      function cleanSource(nodes) {
        return nodes.map((i) => {
          if (i.nodes)
            i.nodes = cleanSource(i.nodes);
          delete i.source;
          return i;
        });
      }
      function markDirtyUp(node) {
        node[isClean] = false;
        if (node.proxyOf.nodes) {
          for (let i of node.proxyOf.nodes) {
            markDirtyUp(i);
          }
        }
      }
      var Container = class _Container extends Node {
        append(...children) {
          for (let child of children) {
            let nodes = this.normalize(child, this.last);
            for (let node of nodes)
              this.proxyOf.nodes.push(node);
          }
          this.markDirty();
          return this;
        }
        cleanRaws(keepBetween) {
          super.cleanRaws(keepBetween);
          if (this.nodes) {
            for (let node of this.nodes)
              node.cleanRaws(keepBetween);
          }
        }
        each(callback) {
          if (!this.proxyOf.nodes)
            return void 0;
          let iterator = this.getIterator();
          let index, result;
          while (this.indexes[iterator] < this.proxyOf.nodes.length) {
            index = this.indexes[iterator];
            result = callback(this.proxyOf.nodes[index], index);
            if (result === false)
              break;
            this.indexes[iterator] += 1;
          }
          delete this.indexes[iterator];
          return result;
        }
        every(condition) {
          return this.nodes.every(condition);
        }
        get first() {
          if (!this.proxyOf.nodes)
            return void 0;
          return this.proxyOf.nodes[0];
        }
        getIterator() {
          if (!this.lastEach)
            this.lastEach = 0;
          if (!this.indexes)
            this.indexes = {};
          this.lastEach += 1;
          let iterator = this.lastEach;
          this.indexes[iterator] = 0;
          return iterator;
        }
        getProxyProcessor() {
          return {
            get(node, prop) {
              if (prop === "proxyOf") {
                return node;
              } else if (!node[prop]) {
                return node[prop];
              } else if (prop === "each" || typeof prop === "string" && prop.startsWith("walk")) {
                return (...args) => {
                  return node[prop](
                    ...args.map((i) => {
                      if (typeof i === "function") {
                        return (child, index) => i(child.toProxy(), index);
                      } else {
                        return i;
                      }
                    })
                  );
                };
              } else if (prop === "every" || prop === "some") {
                return (cb) => {
                  return node[prop](
                    (child, ...other) => cb(child.toProxy(), ...other)
                  );
                };
              } else if (prop === "root") {
                return () => node.root().toProxy();
              } else if (prop === "nodes") {
                return node.nodes.map((i) => i.toProxy());
              } else if (prop === "first" || prop === "last") {
                return node[prop].toProxy();
              } else {
                return node[prop];
              }
            },
            set(node, prop, value) {
              if (node[prop] === value)
                return true;
              node[prop] = value;
              if (prop === "name" || prop === "params" || prop === "selector") {
                node.markDirty();
              }
              return true;
            }
          };
        }
        index(child) {
          if (typeof child === "number")
            return child;
          if (child.proxyOf)
            child = child.proxyOf;
          return this.proxyOf.nodes.indexOf(child);
        }
        insertAfter(exist, add) {
          let existIndex = this.index(exist);
          let nodes = this.normalize(add, this.proxyOf.nodes[existIndex]).reverse();
          existIndex = this.index(exist);
          for (let node of nodes)
            this.proxyOf.nodes.splice(existIndex + 1, 0, node);
          let index;
          for (let id in this.indexes) {
            index = this.indexes[id];
            if (existIndex < index) {
              this.indexes[id] = index + nodes.length;
            }
          }
          this.markDirty();
          return this;
        }
        insertBefore(exist, add) {
          let existIndex = this.index(exist);
          let type = existIndex === 0 ? "prepend" : false;
          let nodes = this.normalize(add, this.proxyOf.nodes[existIndex], type).reverse();
          existIndex = this.index(exist);
          for (let node of nodes)
            this.proxyOf.nodes.splice(existIndex, 0, node);
          let index;
          for (let id in this.indexes) {
            index = this.indexes[id];
            if (existIndex <= index) {
              this.indexes[id] = index + nodes.length;
            }
          }
          this.markDirty();
          return this;
        }
        get last() {
          if (!this.proxyOf.nodes)
            return void 0;
          return this.proxyOf.nodes[this.proxyOf.nodes.length - 1];
        }
        normalize(nodes, sample) {
          if (typeof nodes === "string") {
            nodes = cleanSource(parse3(nodes).nodes);
          } else if (Array.isArray(nodes)) {
            nodes = nodes.slice(0);
            for (let i of nodes) {
              if (i.parent)
                i.parent.removeChild(i, "ignore");
            }
          } else if (nodes.type === "root" && this.type !== "document") {
            nodes = nodes.nodes.slice(0);
            for (let i of nodes) {
              if (i.parent)
                i.parent.removeChild(i, "ignore");
            }
          } else if (nodes.type) {
            nodes = [nodes];
          } else if (nodes.prop) {
            if (typeof nodes.value === "undefined") {
              throw new Error("Value field is missed in node creation");
            } else if (typeof nodes.value !== "string") {
              nodes.value = String(nodes.value);
            }
            nodes = [new Declaration(nodes)];
          } else if (nodes.selector) {
            nodes = [new Rule(nodes)];
          } else if (nodes.name) {
            nodes = [new AtRule(nodes)];
          } else if (nodes.text) {
            nodes = [new Comment(nodes)];
          } else {
            throw new Error("Unknown node type in node creation");
          }
          let processed = nodes.map((i) => {
            if (!i[my])
              _Container.rebuild(i);
            i = i.proxyOf;
            if (i.parent)
              i.parent.removeChild(i);
            if (i[isClean])
              markDirtyUp(i);
            if (typeof i.raws.before === "undefined") {
              if (sample && typeof sample.raws.before !== "undefined") {
                i.raws.before = sample.raws.before.replace(/\S/g, "");
              }
            }
            i.parent = this.proxyOf;
            return i;
          });
          return processed;
        }
        prepend(...children) {
          children = children.reverse();
          for (let child of children) {
            let nodes = this.normalize(child, this.first, "prepend").reverse();
            for (let node of nodes)
              this.proxyOf.nodes.unshift(node);
            for (let id in this.indexes) {
              this.indexes[id] = this.indexes[id] + nodes.length;
            }
          }
          this.markDirty();
          return this;
        }
        push(child) {
          child.parent = this;
          this.proxyOf.nodes.push(child);
          return this;
        }
        removeAll() {
          for (let node of this.proxyOf.nodes)
            node.parent = void 0;
          this.proxyOf.nodes = [];
          this.markDirty();
          return this;
        }
        removeChild(child) {
          child = this.index(child);
          this.proxyOf.nodes[child].parent = void 0;
          this.proxyOf.nodes.splice(child, 1);
          let index;
          for (let id in this.indexes) {
            index = this.indexes[id];
            if (index >= child) {
              this.indexes[id] = index - 1;
            }
          }
          this.markDirty();
          return this;
        }
        replaceValues(pattern, opts, callback) {
          if (!callback) {
            callback = opts;
            opts = {};
          }
          this.walkDecls((decl) => {
            if (opts.props && !opts.props.includes(decl.prop))
              return;
            if (opts.fast && !decl.value.includes(opts.fast))
              return;
            decl.value = decl.value.replace(pattern, callback);
          });
          this.markDirty();
          return this;
        }
        some(condition) {
          return this.nodes.some(condition);
        }
        walk(callback) {
          return this.each((child, i) => {
            let result;
            try {
              result = callback(child, i);
            } catch (e) {
              throw child.addToError(e);
            }
            if (result !== false && child.walk) {
              result = child.walk(callback);
            }
            return result;
          });
        }
        walkAtRules(name, callback) {
          if (!callback) {
            callback = name;
            return this.walk((child, i) => {
              if (child.type === "atrule") {
                return callback(child, i);
              }
            });
          }
          if (name instanceof RegExp) {
            return this.walk((child, i) => {
              if (child.type === "atrule" && name.test(child.name)) {
                return callback(child, i);
              }
            });
          }
          return this.walk((child, i) => {
            if (child.type === "atrule" && child.name === name) {
              return callback(child, i);
            }
          });
        }
        walkComments(callback) {
          return this.walk((child, i) => {
            if (child.type === "comment") {
              return callback(child, i);
            }
          });
        }
        walkDecls(prop, callback) {
          if (!callback) {
            callback = prop;
            return this.walk((child, i) => {
              if (child.type === "decl") {
                return callback(child, i);
              }
            });
          }
          if (prop instanceof RegExp) {
            return this.walk((child, i) => {
              if (child.type === "decl" && prop.test(child.prop)) {
                return callback(child, i);
              }
            });
          }
          return this.walk((child, i) => {
            if (child.type === "decl" && child.prop === prop) {
              return callback(child, i);
            }
          });
        }
        walkRules(selector, callback) {
          if (!callback) {
            callback = selector;
            return this.walk((child, i) => {
              if (child.type === "rule") {
                return callback(child, i);
              }
            });
          }
          if (selector instanceof RegExp) {
            return this.walk((child, i) => {
              if (child.type === "rule" && selector.test(child.selector)) {
                return callback(child, i);
              }
            });
          }
          return this.walk((child, i) => {
            if (child.type === "rule" && child.selector === selector) {
              return callback(child, i);
            }
          });
        }
      };
      Container.registerParse = (dependant) => {
        parse3 = dependant;
      };
      Container.registerRule = (dependant) => {
        Rule = dependant;
      };
      Container.registerAtRule = (dependant) => {
        AtRule = dependant;
      };
      Container.registerRoot = (dependant) => {
        Root = dependant;
      };
      module.exports = Container;
      Container.default = Container;
      Container.rebuild = (node) => {
        if (node.type === "atrule") {
          Object.setPrototypeOf(node, AtRule.prototype);
        } else if (node.type === "rule") {
          Object.setPrototypeOf(node, Rule.prototype);
        } else if (node.type === "decl") {
          Object.setPrototypeOf(node, Declaration.prototype);
        } else if (node.type === "comment") {
          Object.setPrototypeOf(node, Comment.prototype);
        } else if (node.type === "root") {
          Object.setPrototypeOf(node, Root.prototype);
        }
        node[my] = true;
        if (node.nodes) {
          node.nodes.forEach((child) => {
            Container.rebuild(child);
          });
        }
      };
    }
  });

  // node_modules/postcss/lib/tokenize.js
  var require_tokenize = __commonJS({
    "node_modules/postcss/lib/tokenize.js"(exports, module) {
      "use strict";
      var SINGLE_QUOTE2 = "'".charCodeAt(0);
      var DOUBLE_QUOTE2 = '"'.charCodeAt(0);
      var BACKSLASH = "\\".charCodeAt(0);
      var SLASH = "/".charCodeAt(0);
      var NEWLINE = "\n".charCodeAt(0);
      var SPACE = " ".charCodeAt(0);
      var FEED = "\f".charCodeAt(0);
      var TAB = "	".charCodeAt(0);
      var CR = "\r".charCodeAt(0);
      var OPEN_SQUARE = "[".charCodeAt(0);
      var CLOSE_SQUARE = "]".charCodeAt(0);
      var OPEN_PARENTHESES = "(".charCodeAt(0);
      var CLOSE_PARENTHESES = ")".charCodeAt(0);
      var OPEN_CURLY = "{".charCodeAt(0);
      var CLOSE_CURLY = "}".charCodeAt(0);
      var SEMICOLON = ";".charCodeAt(0);
      var ASTERISK = "*".charCodeAt(0);
      var COLON = ":".charCodeAt(0);
      var AT = "@".charCodeAt(0);
      var RE_AT_END = /[\t\n\f\r "#'()/;[\\\]{}]/g;
      var RE_WORD_END = /[\t\n\f\r !"#'():;@[\\\]{}]|\/(?=\*)/g;
      var RE_BAD_BRACKET = /.[\n"'(/\\]/;
      var RE_HEX_ESCAPE = /[\da-f]/i;
      module.exports = function tokenizer(input, options2 = {}) {
        let css2 = input.css.valueOf();
        let ignore = options2.ignoreErrors;
        let code, next, quote, content, escape;
        let escaped, escapePos, prev, n, currentToken;
        let length = css2.length;
        let pos = 0;
        let buffer = [];
        let returned = [];
        function position() {
          return pos;
        }
        function unclosed(what) {
          throw input.error("Unclosed " + what, pos);
        }
        function endOfFile() {
          return returned.length === 0 && pos >= length;
        }
        function nextToken(opts) {
          if (returned.length)
            return returned.pop();
          if (pos >= length)
            return;
          let ignoreUnclosed = opts ? opts.ignoreUnclosed : false;
          code = css2.charCodeAt(pos);
          switch (code) {
            case NEWLINE:
            case SPACE:
            case TAB:
            case CR:
            case FEED: {
              next = pos;
              do {
                next += 1;
                code = css2.charCodeAt(next);
              } while (code === SPACE || code === NEWLINE || code === TAB || code === CR || code === FEED);
              currentToken = ["space", css2.slice(pos, next)];
              pos = next - 1;
              break;
            }
            case OPEN_SQUARE:
            case CLOSE_SQUARE:
            case OPEN_CURLY:
            case CLOSE_CURLY:
            case COLON:
            case SEMICOLON:
            case CLOSE_PARENTHESES: {
              let controlChar = String.fromCharCode(code);
              currentToken = [controlChar, controlChar, pos];
              break;
            }
            case OPEN_PARENTHESES: {
              prev = buffer.length ? buffer.pop()[1] : "";
              n = css2.charCodeAt(pos + 1);
              if (prev === "url" && n !== SINGLE_QUOTE2 && n !== DOUBLE_QUOTE2 && n !== SPACE && n !== NEWLINE && n !== TAB && n !== FEED && n !== CR) {
                next = pos;
                do {
                  escaped = false;
                  next = css2.indexOf(")", next + 1);
                  if (next === -1) {
                    if (ignore || ignoreUnclosed) {
                      next = pos;
                      break;
                    } else {
                      unclosed("bracket");
                    }
                  }
                  escapePos = next;
                  while (css2.charCodeAt(escapePos - 1) === BACKSLASH) {
                    escapePos -= 1;
                    escaped = !escaped;
                  }
                } while (escaped);
                currentToken = ["brackets", css2.slice(pos, next + 1), pos, next];
                pos = next;
              } else {
                next = css2.indexOf(")", pos + 1);
                content = css2.slice(pos, next + 1);
                if (next === -1 || RE_BAD_BRACKET.test(content)) {
                  currentToken = ["(", "(", pos];
                } else {
                  currentToken = ["brackets", content, pos, next];
                  pos = next;
                }
              }
              break;
            }
            case SINGLE_QUOTE2:
            case DOUBLE_QUOTE2: {
              quote = code === SINGLE_QUOTE2 ? "'" : '"';
              next = pos;
              do {
                escaped = false;
                next = css2.indexOf(quote, next + 1);
                if (next === -1) {
                  if (ignore || ignoreUnclosed) {
                    next = pos + 1;
                    break;
                  } else {
                    unclosed("string");
                  }
                }
                escapePos = next;
                while (css2.charCodeAt(escapePos - 1) === BACKSLASH) {
                  escapePos -= 1;
                  escaped = !escaped;
                }
              } while (escaped);
              currentToken = ["string", css2.slice(pos, next + 1), pos, next];
              pos = next;
              break;
            }
            case AT: {
              RE_AT_END.lastIndex = pos + 1;
              RE_AT_END.test(css2);
              if (RE_AT_END.lastIndex === 0) {
                next = css2.length - 1;
              } else {
                next = RE_AT_END.lastIndex - 2;
              }
              currentToken = ["at-word", css2.slice(pos, next + 1), pos, next];
              pos = next;
              break;
            }
            case BACKSLASH: {
              next = pos;
              escape = true;
              while (css2.charCodeAt(next + 1) === BACKSLASH) {
                next += 1;
                escape = !escape;
              }
              code = css2.charCodeAt(next + 1);
              if (escape && code !== SLASH && code !== SPACE && code !== NEWLINE && code !== TAB && code !== CR && code !== FEED) {
                next += 1;
                if (RE_HEX_ESCAPE.test(css2.charAt(next))) {
                  while (RE_HEX_ESCAPE.test(css2.charAt(next + 1))) {
                    next += 1;
                  }
                  if (css2.charCodeAt(next + 1) === SPACE) {
                    next += 1;
                  }
                }
              }
              currentToken = ["word", css2.slice(pos, next + 1), pos, next];
              pos = next;
              break;
            }
            default: {
              if (code === SLASH && css2.charCodeAt(pos + 1) === ASTERISK) {
                next = css2.indexOf("*/", pos + 2) + 1;
                if (next === 0) {
                  if (ignore || ignoreUnclosed) {
                    next = css2.length;
                  } else {
                    unclosed("comment");
                  }
                }
                currentToken = ["comment", css2.slice(pos, next + 1), pos, next];
                pos = next;
              } else {
                RE_WORD_END.lastIndex = pos + 1;
                RE_WORD_END.test(css2);
                if (RE_WORD_END.lastIndex === 0) {
                  next = css2.length - 1;
                } else {
                  next = RE_WORD_END.lastIndex - 2;
                }
                currentToken = ["word", css2.slice(pos, next + 1), pos, next];
                buffer.push(currentToken);
                pos = next;
              }
              break;
            }
          }
          pos++;
          return currentToken;
        }
        function back(token) {
          returned.push(token);
        }
        return {
          back,
          endOfFile,
          nextToken,
          position
        };
      };
    }
  });

  // node_modules/postcss/lib/at-rule.js
  var require_at_rule = __commonJS({
    "node_modules/postcss/lib/at-rule.js"(exports, module) {
      "use strict";
      var Container = require_container();
      var AtRule = class extends Container {
        constructor(defaults) {
          super(defaults);
          this.type = "atrule";
        }
        append(...children) {
          if (!this.proxyOf.nodes)
            this.nodes = [];
          return super.append(...children);
        }
        prepend(...children) {
          if (!this.proxyOf.nodes)
            this.nodes = [];
          return super.prepend(...children);
        }
      };
      module.exports = AtRule;
      AtRule.default = AtRule;
      Container.registerAtRule(AtRule);
    }
  });

  // node_modules/postcss/lib/root.js
  var require_root = __commonJS({
    "node_modules/postcss/lib/root.js"(exports, module) {
      "use strict";
      var Container = require_container();
      var LazyResult;
      var Processor;
      var Root = class extends Container {
        constructor(defaults) {
          super(defaults);
          this.type = "root";
          if (!this.nodes)
            this.nodes = [];
        }
        normalize(child, sample, type) {
          let nodes = super.normalize(child);
          if (sample) {
            if (type === "prepend") {
              if (this.nodes.length > 1) {
                sample.raws.before = this.nodes[1].raws.before;
              } else {
                delete sample.raws.before;
              }
            } else if (this.first !== sample) {
              for (let node of nodes) {
                node.raws.before = sample.raws.before;
              }
            }
          }
          return nodes;
        }
        removeChild(child, ignore) {
          let index = this.index(child);
          if (!ignore && index === 0 && this.nodes.length > 1) {
            this.nodes[1].raws.before = this.nodes[index].raws.before;
          }
          return super.removeChild(child);
        }
        toResult(opts = {}) {
          let lazy = new LazyResult(new Processor(), this, opts);
          return lazy.stringify();
        }
      };
      Root.registerLazyResult = (dependant) => {
        LazyResult = dependant;
      };
      Root.registerProcessor = (dependant) => {
        Processor = dependant;
      };
      module.exports = Root;
      Root.default = Root;
      Container.registerRoot(Root);
    }
  });

  // node_modules/postcss/lib/list.js
  var require_list = __commonJS({
    "node_modules/postcss/lib/list.js"(exports, module) {
      "use strict";
      var list = {
        comma(string) {
          return list.split(string, [","], true);
        },
        space(string) {
          let spaces = [" ", "\n", "	"];
          return list.split(string, spaces);
        },
        split(string, separators, last) {
          let array = [];
          let current = "";
          let split = false;
          let func = 0;
          let inQuote = false;
          let prevQuote = "";
          let escape = false;
          for (let letter of string) {
            if (escape) {
              escape = false;
            } else if (letter === "\\") {
              escape = true;
            } else if (inQuote) {
              if (letter === prevQuote) {
                inQuote = false;
              }
            } else if (letter === '"' || letter === "'") {
              inQuote = true;
              prevQuote = letter;
            } else if (letter === "(") {
              func += 1;
            } else if (letter === ")") {
              if (func > 0)
                func -= 1;
            } else if (func === 0) {
              if (separators.includes(letter))
                split = true;
            }
            if (split) {
              if (current !== "")
                array.push(current.trim());
              current = "";
              split = false;
            } else {
              current += letter;
            }
          }
          if (last || current !== "")
            array.push(current.trim());
          return array;
        }
      };
      module.exports = list;
      list.default = list;
    }
  });

  // node_modules/postcss/lib/rule.js
  var require_rule = __commonJS({
    "node_modules/postcss/lib/rule.js"(exports, module) {
      "use strict";
      var Container = require_container();
      var list = require_list();
      var Rule = class extends Container {
        constructor(defaults) {
          super(defaults);
          this.type = "rule";
          if (!this.nodes)
            this.nodes = [];
        }
        get selectors() {
          return list.comma(this.selector);
        }
        set selectors(values) {
          let match = this.selector ? this.selector.match(/,\s*/) : null;
          let sep = match ? match[0] : "," + this.raw("between", "beforeOpen");
          this.selector = values.join(sep);
        }
      };
      module.exports = Rule;
      Rule.default = Rule;
      Container.registerRule(Rule);
    }
  });

  // node_modules/postcss/lib/parser.js
  var require_parser = __commonJS({
    "node_modules/postcss/lib/parser.js"(exports, module) {
      "use strict";
      var Declaration = require_declaration();
      var tokenizer = require_tokenize();
      var Comment = require_comment();
      var AtRule = require_at_rule();
      var Root = require_root();
      var Rule = require_rule();
      var SAFE_COMMENT_NEIGHBOR = {
        empty: true,
        space: true
      };
      function findLastWithPosition(tokens) {
        for (let i = tokens.length - 1; i >= 0; i--) {
          let token = tokens[i];
          let pos = token[3] || token[2];
          if (pos)
            return pos;
        }
      }
      var Parser = class {
        constructor(input) {
          this.input = input;
          this.root = new Root();
          this.current = this.root;
          this.spaces = "";
          this.semicolon = false;
          this.customProperty = false;
          this.createTokenizer();
          this.root.source = { input, start: { column: 1, line: 1, offset: 0 } };
        }
        atrule(token) {
          let node = new AtRule();
          node.name = token[1].slice(1);
          if (node.name === "") {
            this.unnamedAtrule(node, token);
          }
          this.init(node, token[2]);
          let type;
          let prev;
          let shift;
          let last = false;
          let open = false;
          let params = [];
          let brackets = [];
          while (!this.tokenizer.endOfFile()) {
            token = this.tokenizer.nextToken();
            type = token[0];
            if (type === "(" || type === "[") {
              brackets.push(type === "(" ? ")" : "]");
            } else if (type === "{" && brackets.length > 0) {
              brackets.push("}");
            } else if (type === brackets[brackets.length - 1]) {
              brackets.pop();
            }
            if (brackets.length === 0) {
              if (type === ";") {
                node.source.end = this.getPosition(token[2]);
                this.semicolon = true;
                break;
              } else if (type === "{") {
                open = true;
                break;
              } else if (type === "}") {
                if (params.length > 0) {
                  shift = params.length - 1;
                  prev = params[shift];
                  while (prev && prev[0] === "space") {
                    prev = params[--shift];
                  }
                  if (prev) {
                    node.source.end = this.getPosition(prev[3] || prev[2]);
                  }
                }
                this.end(token);
                break;
              } else {
                params.push(token);
              }
            } else {
              params.push(token);
            }
            if (this.tokenizer.endOfFile()) {
              last = true;
              break;
            }
          }
          node.raws.between = this.spacesAndCommentsFromEnd(params);
          if (params.length) {
            node.raws.afterName = this.spacesAndCommentsFromStart(params);
            this.raw(node, "params", params);
            if (last) {
              token = params[params.length - 1];
              node.source.end = this.getPosition(token[3] || token[2]);
              this.spaces = node.raws.between;
              node.raws.between = "";
            }
          } else {
            node.raws.afterName = "";
            node.params = "";
          }
          if (open) {
            node.nodes = [];
            this.current = node;
          }
        }
        checkMissedSemicolon(tokens) {
          let colon = this.colon(tokens);
          if (colon === false)
            return;
          let founded = 0;
          let token;
          for (let j = colon - 1; j >= 0; j--) {
            token = tokens[j];
            if (token[0] !== "space") {
              founded += 1;
              if (founded === 2)
                break;
            }
          }
          throw this.input.error(
            "Missed semicolon",
            token[0] === "word" ? token[3] + 1 : token[2]
          );
        }
        colon(tokens) {
          let brackets = 0;
          let token, type, prev;
          for (let [i, element] of tokens.entries()) {
            token = element;
            type = token[0];
            if (type === "(") {
              brackets += 1;
            }
            if (type === ")") {
              brackets -= 1;
            }
            if (brackets === 0 && type === ":") {
              if (!prev) {
                this.doubleColon(token);
              } else if (prev[0] === "word" && prev[1] === "progid") {
                continue;
              } else {
                return i;
              }
            }
            prev = token;
          }
          return false;
        }
        comment(token) {
          let node = new Comment();
          this.init(node, token[2]);
          node.source.end = this.getPosition(token[3] || token[2]);
          let text = token[1].slice(2, -2);
          if (/^\s*$/.test(text)) {
            node.text = "";
            node.raws.left = text;
            node.raws.right = "";
          } else {
            let match = text.match(/^(\s*)([^]*\S)(\s*)$/);
            node.text = match[2];
            node.raws.left = match[1];
            node.raws.right = match[3];
          }
        }
        createTokenizer() {
          this.tokenizer = tokenizer(this.input);
        }
        decl(tokens, customProperty) {
          let node = new Declaration();
          this.init(node, tokens[0][2]);
          let last = tokens[tokens.length - 1];
          if (last[0] === ";") {
            this.semicolon = true;
            tokens.pop();
          }
          node.source.end = this.getPosition(
            last[3] || last[2] || findLastWithPosition(tokens)
          );
          while (tokens[0][0] !== "word") {
            if (tokens.length === 1)
              this.unknownWord(tokens);
            node.raws.before += tokens.shift()[1];
          }
          node.source.start = this.getPosition(tokens[0][2]);
          node.prop = "";
          while (tokens.length) {
            let type = tokens[0][0];
            if (type === ":" || type === "space" || type === "comment") {
              break;
            }
            node.prop += tokens.shift()[1];
          }
          node.raws.between = "";
          let token;
          while (tokens.length) {
            token = tokens.shift();
            if (token[0] === ":") {
              node.raws.between += token[1];
              break;
            } else {
              if (token[0] === "word" && /\w/.test(token[1])) {
                this.unknownWord([token]);
              }
              node.raws.between += token[1];
            }
          }
          if (node.prop[0] === "_" || node.prop[0] === "*") {
            node.raws.before += node.prop[0];
            node.prop = node.prop.slice(1);
          }
          let firstSpaces = [];
          let next;
          while (tokens.length) {
            next = tokens[0][0];
            if (next !== "space" && next !== "comment")
              break;
            firstSpaces.push(tokens.shift());
          }
          this.precheckMissedSemicolon(tokens);
          for (let i = tokens.length - 1; i >= 0; i--) {
            token = tokens[i];
            if (token[1].toLowerCase() === "!important") {
              node.important = true;
              let string = this.stringFrom(tokens, i);
              string = this.spacesFromEnd(tokens) + string;
              if (string !== " !important")
                node.raws.important = string;
              break;
            } else if (token[1].toLowerCase() === "important") {
              let cache = tokens.slice(0);
              let str = "";
              for (let j = i; j > 0; j--) {
                let type = cache[j][0];
                if (str.trim().indexOf("!") === 0 && type !== "space") {
                  break;
                }
                str = cache.pop()[1] + str;
              }
              if (str.trim().indexOf("!") === 0) {
                node.important = true;
                node.raws.important = str;
                tokens = cache;
              }
            }
            if (token[0] !== "space" && token[0] !== "comment") {
              break;
            }
          }
          let hasWord = tokens.some((i) => i[0] !== "space" && i[0] !== "comment");
          if (hasWord) {
            node.raws.between += firstSpaces.map((i) => i[1]).join("");
            firstSpaces = [];
          }
          this.raw(node, "value", firstSpaces.concat(tokens), customProperty);
          if (node.value.includes(":") && !customProperty) {
            this.checkMissedSemicolon(tokens);
          }
        }
        doubleColon(token) {
          throw this.input.error(
            "Double colon",
            { offset: token[2] },
            { offset: token[2] + token[1].length }
          );
        }
        emptyRule(token) {
          let node = new Rule();
          this.init(node, token[2]);
          node.selector = "";
          node.raws.between = "";
          this.current = node;
        }
        end(token) {
          if (this.current.nodes && this.current.nodes.length) {
            this.current.raws.semicolon = this.semicolon;
          }
          this.semicolon = false;
          this.current.raws.after = (this.current.raws.after || "") + this.spaces;
          this.spaces = "";
          if (this.current.parent) {
            this.current.source.end = this.getPosition(token[2]);
            this.current = this.current.parent;
          } else {
            this.unexpectedClose(token);
          }
        }
        endFile() {
          if (this.current.parent)
            this.unclosedBlock();
          if (this.current.nodes && this.current.nodes.length) {
            this.current.raws.semicolon = this.semicolon;
          }
          this.current.raws.after = (this.current.raws.after || "") + this.spaces;
          this.root.source.end = this.getPosition(this.tokenizer.position());
        }
        freeSemicolon(token) {
          this.spaces += token[1];
          if (this.current.nodes) {
            let prev = this.current.nodes[this.current.nodes.length - 1];
            if (prev && prev.type === "rule" && !prev.raws.ownSemicolon) {
              prev.raws.ownSemicolon = this.spaces;
              this.spaces = "";
            }
          }
        }
        // Helpers
        getPosition(offset) {
          let pos = this.input.fromOffset(offset);
          return {
            column: pos.col,
            line: pos.line,
            offset
          };
        }
        init(node, offset) {
          this.current.push(node);
          node.source = {
            input: this.input,
            start: this.getPosition(offset)
          };
          node.raws.before = this.spaces;
          this.spaces = "";
          if (node.type !== "comment")
            this.semicolon = false;
        }
        other(start) {
          let end = false;
          let type = null;
          let colon = false;
          let bracket = null;
          let brackets = [];
          let customProperty = start[1].startsWith("--");
          let tokens = [];
          let token = start;
          while (token) {
            type = token[0];
            tokens.push(token);
            if (type === "(" || type === "[") {
              if (!bracket)
                bracket = token;
              brackets.push(type === "(" ? ")" : "]");
            } else if (customProperty && colon && type === "{") {
              if (!bracket)
                bracket = token;
              brackets.push("}");
            } else if (brackets.length === 0) {
              if (type === ";") {
                if (colon) {
                  this.decl(tokens, customProperty);
                  return;
                } else {
                  break;
                }
              } else if (type === "{") {
                this.rule(tokens);
                return;
              } else if (type === "}") {
                this.tokenizer.back(tokens.pop());
                end = true;
                break;
              } else if (type === ":") {
                colon = true;
              }
            } else if (type === brackets[brackets.length - 1]) {
              brackets.pop();
              if (brackets.length === 0)
                bracket = null;
            }
            token = this.tokenizer.nextToken();
          }
          if (this.tokenizer.endOfFile())
            end = true;
          if (brackets.length > 0)
            this.unclosedBracket(bracket);
          if (end && colon) {
            if (!customProperty) {
              while (tokens.length) {
                token = tokens[tokens.length - 1][0];
                if (token !== "space" && token !== "comment")
                  break;
                this.tokenizer.back(tokens.pop());
              }
            }
            this.decl(tokens, customProperty);
          } else {
            this.unknownWord(tokens);
          }
        }
        parse() {
          let token;
          while (!this.tokenizer.endOfFile()) {
            token = this.tokenizer.nextToken();
            switch (token[0]) {
              case "space":
                this.spaces += token[1];
                break;
              case ";":
                this.freeSemicolon(token);
                break;
              case "}":
                this.end(token);
                break;
              case "comment":
                this.comment(token);
                break;
              case "at-word":
                this.atrule(token);
                break;
              case "{":
                this.emptyRule(token);
                break;
              default:
                this.other(token);
                break;
            }
          }
          this.endFile();
        }
        precheckMissedSemicolon() {
        }
        raw(node, prop, tokens, customProperty) {
          let token, type;
          let length = tokens.length;
          let value = "";
          let clean2 = true;
          let next, prev;
          for (let i = 0; i < length; i += 1) {
            token = tokens[i];
            type = token[0];
            if (type === "space" && i === length - 1 && !customProperty) {
              clean2 = false;
            } else if (type === "comment") {
              prev = tokens[i - 1] ? tokens[i - 1][0] : "empty";
              next = tokens[i + 1] ? tokens[i + 1][0] : "empty";
              if (!SAFE_COMMENT_NEIGHBOR[prev] && !SAFE_COMMENT_NEIGHBOR[next]) {
                if (value.slice(-1) === ",") {
                  clean2 = false;
                } else {
                  value += token[1];
                }
              } else {
                clean2 = false;
              }
            } else {
              value += token[1];
            }
          }
          if (!clean2) {
            let raw = tokens.reduce((all, i) => all + i[1], "");
            node.raws[prop] = { raw, value };
          }
          node[prop] = value;
        }
        rule(tokens) {
          tokens.pop();
          let node = new Rule();
          this.init(node, tokens[0][2]);
          node.raws.between = this.spacesAndCommentsFromEnd(tokens);
          this.raw(node, "selector", tokens);
          this.current = node;
        }
        spacesAndCommentsFromEnd(tokens) {
          let lastTokenType;
          let spaces = "";
          while (tokens.length) {
            lastTokenType = tokens[tokens.length - 1][0];
            if (lastTokenType !== "space" && lastTokenType !== "comment")
              break;
            spaces = tokens.pop()[1] + spaces;
          }
          return spaces;
        }
        // Errors
        spacesAndCommentsFromStart(tokens) {
          let next;
          let spaces = "";
          while (tokens.length) {
            next = tokens[0][0];
            if (next !== "space" && next !== "comment")
              break;
            spaces += tokens.shift()[1];
          }
          return spaces;
        }
        spacesFromEnd(tokens) {
          let lastTokenType;
          let spaces = "";
          while (tokens.length) {
            lastTokenType = tokens[tokens.length - 1][0];
            if (lastTokenType !== "space")
              break;
            spaces = tokens.pop()[1] + spaces;
          }
          return spaces;
        }
        stringFrom(tokens, from) {
          let result = "";
          for (let i = from; i < tokens.length; i++) {
            result += tokens[i][1];
          }
          tokens.splice(from, tokens.length - from);
          return result;
        }
        unclosedBlock() {
          let pos = this.current.source.start;
          throw this.input.error("Unclosed block", pos.line, pos.column);
        }
        unclosedBracket(bracket) {
          throw this.input.error(
            "Unclosed bracket",
            { offset: bracket[2] },
            { offset: bracket[2] + 1 }
          );
        }
        unexpectedClose(token) {
          throw this.input.error(
            "Unexpected }",
            { offset: token[2] },
            { offset: token[2] + 1 }
          );
        }
        unknownWord(tokens) {
          throw this.input.error(
            "Unknown word",
            { offset: tokens[0][2] },
            { offset: tokens[0][2] + tokens[0][1].length }
          );
        }
        unnamedAtrule(node, token) {
          throw this.input.error(
            "At-rule without name",
            { offset: token[2] },
            { offset: token[2] + token[1].length }
          );
        }
      };
      module.exports = Parser;
    }
  });

  // (disabled):node_modules/source-map-js/source-map.js
  var require_source_map = __commonJS({
    "(disabled):node_modules/source-map-js/source-map.js"() {
    }
  });

  // node_modules/nanoid/non-secure/index.cjs
  var require_non_secure = __commonJS({
    "node_modules/nanoid/non-secure/index.cjs"(exports, module) {
      var urlAlphabet = "useandom-26T198340PX75pxJACKVERYMINDBUSHWOLF_GQZbfghjklqvwyzrict";
      var customAlphabet = (alphabet, defaultSize = 21) => {
        return (size = defaultSize) => {
          let id = "";
          let i = size;
          while (i--) {
            id += alphabet[Math.random() * alphabet.length | 0];
          }
          return id;
        };
      };
      var nanoid = (size = 21) => {
        let id = "";
        let i = size;
        while (i--) {
          id += urlAlphabet[Math.random() * 64 | 0];
        }
        return id;
      };
      module.exports = { nanoid, customAlphabet };
    }
  });

  // node_modules/postcss/lib/previous-map.js
  var require_previous_map = __commonJS({
    "node_modules/postcss/lib/previous-map.js"(exports, module) {
      module.exports = class {
      };
    }
  });

  // node_modules/postcss/lib/input.js
  var require_input = __commonJS({
    "node_modules/postcss/lib/input.js"(exports, module) {
      "use strict";
      var { SourceMapConsumer, SourceMapGenerator } = require_source_map();
      var { fileURLToPath, pathToFileURL } = {};
      var { isAbsolute, resolve } = {};
      var { nanoid } = require_non_secure();
      var terminalHighlight = require_terminal_highlight();
      var CssSyntaxError = require_css_syntax_error();
      var PreviousMap = require_previous_map();
      var fromOffsetCache = Symbol("fromOffsetCache");
      var sourceMapAvailable = Boolean(SourceMapConsumer && SourceMapGenerator);
      var pathAvailable = Boolean(resolve && isAbsolute);
      var Input = class {
        constructor(css2, opts = {}) {
          if (css2 === null || typeof css2 === "undefined" || typeof css2 === "object" && !css2.toString) {
            throw new Error(`PostCSS received ${css2} instead of CSS string`);
          }
          this.css = css2.toString();
          if (this.css[0] === "\uFEFF" || this.css[0] === "\uFFFE") {
            this.hasBOM = true;
            this.css = this.css.slice(1);
          } else {
            this.hasBOM = false;
          }
          if (opts.from) {
            if (!pathAvailable || /^\w+:\/\//.test(opts.from) || isAbsolute(opts.from)) {
              this.file = opts.from;
            } else {
              this.file = resolve(opts.from);
            }
          }
          if (pathAvailable && sourceMapAvailable) {
            let map = new PreviousMap(this.css, opts);
            if (map.text) {
              this.map = map;
              let file = map.consumer().file;
              if (!this.file && file)
                this.file = this.mapResolve(file);
            }
          }
          if (!this.file) {
            this.id = "<input css " + nanoid(6) + ">";
          }
          if (this.map)
            this.map.file = this.from;
        }
        error(message, line2, column, opts = {}) {
          let result, endLine, endColumn;
          if (line2 && typeof line2 === "object") {
            let start = line2;
            let end = column;
            if (typeof start.offset === "number") {
              let pos = this.fromOffset(start.offset);
              line2 = pos.line;
              column = pos.col;
            } else {
              line2 = start.line;
              column = start.column;
            }
            if (typeof end.offset === "number") {
              let pos = this.fromOffset(end.offset);
              endLine = pos.line;
              endColumn = pos.col;
            } else {
              endLine = end.line;
              endColumn = end.column;
            }
          } else if (!column) {
            let pos = this.fromOffset(line2);
            line2 = pos.line;
            column = pos.col;
          }
          let origin = this.origin(line2, column, endLine, endColumn);
          if (origin) {
            result = new CssSyntaxError(
              message,
              origin.endLine === void 0 ? origin.line : { column: origin.column, line: origin.line },
              origin.endLine === void 0 ? origin.column : { column: origin.endColumn, line: origin.endLine },
              origin.source,
              origin.file,
              opts.plugin
            );
          } else {
            result = new CssSyntaxError(
              message,
              endLine === void 0 ? line2 : { column, line: line2 },
              endLine === void 0 ? column : { column: endColumn, line: endLine },
              this.css,
              this.file,
              opts.plugin
            );
          }
          result.input = { column, endColumn, endLine, line: line2, source: this.css };
          if (this.file) {
            if (pathToFileURL) {
              result.input.url = pathToFileURL(this.file).toString();
            }
            result.input.file = this.file;
          }
          return result;
        }
        get from() {
          return this.file || this.id;
        }
        fromOffset(offset) {
          let lastLine, lineToIndex;
          if (!this[fromOffsetCache]) {
            let lines = this.css.split("\n");
            lineToIndex = new Array(lines.length);
            let prevIndex = 0;
            for (let i = 0, l = lines.length; i < l; i++) {
              lineToIndex[i] = prevIndex;
              prevIndex += lines[i].length + 1;
            }
            this[fromOffsetCache] = lineToIndex;
          } else {
            lineToIndex = this[fromOffsetCache];
          }
          lastLine = lineToIndex[lineToIndex.length - 1];
          let min = 0;
          if (offset >= lastLine) {
            min = lineToIndex.length - 1;
          } else {
            let max = lineToIndex.length - 2;
            let mid;
            while (min < max) {
              mid = min + (max - min >> 1);
              if (offset < lineToIndex[mid]) {
                max = mid - 1;
              } else if (offset >= lineToIndex[mid + 1]) {
                min = mid + 1;
              } else {
                min = mid;
                break;
              }
            }
          }
          return {
            col: offset - lineToIndex[min] + 1,
            line: min + 1
          };
        }
        mapResolve(file) {
          if (/^\w+:\/\//.test(file)) {
            return file;
          }
          return resolve(this.map.consumer().sourceRoot || this.map.root || ".", file);
        }
        origin(line2, column, endLine, endColumn) {
          if (!this.map)
            return false;
          let consumer = this.map.consumer();
          let from = consumer.originalPositionFor({ column, line: line2 });
          if (!from.source)
            return false;
          let to;
          if (typeof endLine === "number") {
            to = consumer.originalPositionFor({ column: endColumn, line: endLine });
          }
          let fromUrl;
          if (isAbsolute(from.source)) {
            fromUrl = pathToFileURL(from.source);
          } else {
            fromUrl = new URL(
              from.source,
              this.map.consumer().sourceRoot || pathToFileURL(this.map.mapFile)
            );
          }
          let result = {
            column: from.column,
            endColumn: to && to.column,
            endLine: to && to.line,
            line: from.line,
            url: fromUrl.toString()
          };
          if (fromUrl.protocol === "file:") {
            if (fileURLToPath) {
              result.file = fileURLToPath(fromUrl);
            } else {
              throw new Error(`file: protocol is not available in this PostCSS build`);
            }
          }
          let source = consumer.sourceContentFor(from.source);
          if (source)
            result.source = source;
          return result;
        }
        toJSON() {
          let json = {};
          for (let name of ["hasBOM", "css", "file", "id"]) {
            if (this[name] != null) {
              json[name] = this[name];
            }
          }
          if (this.map) {
            json.map = { ...this.map };
            if (json.map.consumerCache) {
              json.map.consumerCache = void 0;
            }
          }
          return json;
        }
      };
      module.exports = Input;
      Input.default = Input;
      if (terminalHighlight && terminalHighlight.registerInput) {
        terminalHighlight.registerInput(Input);
      }
    }
  });

  // node_modules/postcss/lib/parse.js
  var require_parse = __commonJS({
    "node_modules/postcss/lib/parse.js"(exports, module) {
      "use strict";
      var Container = require_container();
      var Parser = require_parser();
      var Input = require_input();
      function parse3(css2, opts) {
        let input = new Input(css2, opts);
        let parser = new Parser(input);
        try {
          parser.parse();
        } catch (e) {
          if (false) {
            if (e.name === "CssSyntaxError" && opts && opts.from) {
              if (/\.scss$/i.test(opts.from)) {
                e.message += "\nYou tried to parse SCSS with the standard CSS parser; try again with the postcss-scss parser";
              } else if (/\.sass/i.test(opts.from)) {
                e.message += "\nYou tried to parse Sass with the standard CSS parser; try again with the postcss-sass parser";
              } else if (/\.less$/i.test(opts.from)) {
                e.message += "\nYou tried to parse Less with the standard CSS parser; try again with the postcss-less parser";
              }
            }
          }
          throw e;
        }
        return parser.root;
      }
      module.exports = parse3;
      parse3.default = parse3;
      Container.registerParse(parse3);
    }
  });

  // node_modules/postcss-less/lib/nodes/inline-comment.js
  var require_inline_comment = __commonJS({
    "node_modules/postcss-less/lib/nodes/inline-comment.js"(exports, module) {
      var tokenizer = require_tokenize();
      var Input = require_input();
      module.exports = {
        isInlineComment(token) {
          if (token[0] === "word" && token[1].slice(0, 2) === "//") {
            const first = token;
            const bits = [];
            let endOffset;
            let remainingInput;
            while (token) {
              if (/\r?\n/.test(token[1])) {
                if (/['"].*\r?\n/.test(token[1])) {
                  bits.push(token[1].substring(0, token[1].indexOf("\n")));
                  remainingInput = token[1].substring(token[1].indexOf("\n"));
                  const untokenizedRemainingInput = this.input.css.valueOf().substring(this.tokenizer.position());
                  remainingInput += untokenizedRemainingInput;
                  endOffset = token[3] + untokenizedRemainingInput.length - remainingInput.length;
                } else {
                  this.tokenizer.back(token);
                }
                break;
              }
              bits.push(token[1]);
              endOffset = token[2];
              token = this.tokenizer.nextToken({ ignoreUnclosed: true });
            }
            const newToken = ["comment", bits.join(""), first[2], endOffset];
            this.inlineComment(newToken);
            if (remainingInput) {
              this.input = new Input(remainingInput);
              this.tokenizer = tokenizer(this.input);
            }
            return true;
          } else if (token[1] === "/") {
            const next = this.tokenizer.nextToken({ ignoreUnclosed: true });
            if (next[0] === "comment" && /^\/\*/.test(next[1])) {
              next[0] = "word";
              next[1] = next[1].slice(1);
              token[1] = "//";
              this.tokenizer.back(next);
              return module.exports.isInlineComment.bind(this)(token);
            }
          }
          return false;
        }
      };
    }
  });

  // node_modules/postcss-less/lib/nodes/interpolation.js
  var require_interpolation = __commonJS({
    "node_modules/postcss-less/lib/nodes/interpolation.js"(exports, module) {
      module.exports = {
        interpolation(token) {
          const tokens = [token, this.tokenizer.nextToken()];
          const validTypes = ["word", "}"];
          if (tokens[0][1].length > 1 || tokens[1][0] !== "{") {
            this.tokenizer.back(tokens[1]);
            return false;
          }
          token = this.tokenizer.nextToken();
          while (token && validTypes.includes(token[0])) {
            tokens.push(token);
            token = this.tokenizer.nextToken();
          }
          const words = tokens.map((tokn) => tokn[1]);
          const [first] = tokens;
          const last = tokens.pop();
          const newToken = ["word", words.join(""), first[2], last[2]];
          this.tokenizer.back(token);
          this.tokenizer.back(newToken);
          return true;
        }
      };
    }
  });

  // node_modules/postcss-less/lib/nodes/mixin.js
  var require_mixin = __commonJS({
    "node_modules/postcss-less/lib/nodes/mixin.js"(exports, module) {
      var hashColorPattern = /^#[0-9a-fA-F]{6}$|^#[0-9a-fA-F]{3}$/;
      var unpaddedFractionalNumbersPattern = /\.[0-9]/;
      var isMixinToken = (token) => {
        const [, symbol] = token;
        const [char] = symbol;
        return (char === "." || char === "#") && // ignore hashes used for colors
        hashColorPattern.test(symbol) === false && // ignore dots used for unpadded fractional numbers
        unpaddedFractionalNumbersPattern.test(symbol) === false;
      };
      module.exports = { isMixinToken };
    }
  });

  // node_modules/postcss-less/lib/nodes/import.js
  var require_import = __commonJS({
    "node_modules/postcss-less/lib/nodes/import.js"(exports, module) {
      var tokenize = require_tokenize();
      var urlPattern = /^url\((.+)\)/;
      module.exports = (node) => {
        const { name, params = "" } = node;
        if (name === "import" && params.length) {
          node.import = true;
          const tokenizer = tokenize({ css: params });
          node.filename = params.replace(urlPattern, "$1");
          while (!tokenizer.endOfFile()) {
            const [type, content] = tokenizer.nextToken();
            if (type === "word" && content === "url") {
              return;
            } else if (type === "brackets") {
              node.options = content;
              node.filename = params.replace(content, "").trim();
              break;
            }
          }
        }
      };
    }
  });

  // node_modules/postcss-less/lib/nodes/variable.js
  var require_variable = __commonJS({
    "node_modules/postcss-less/lib/nodes/variable.js"(exports, module) {
      var afterPattern = /:$/;
      var beforePattern = /^:(\s+)?/;
      module.exports = (node) => {
        const { name, params = "" } = node;
        if (node.name.slice(-1) !== ":") {
          return;
        }
        if (afterPattern.test(name)) {
          const [match] = name.match(afterPattern);
          node.name = name.replace(match, "");
          node.raws.afterName = match + (node.raws.afterName || "");
          node.variable = true;
          node.value = node.params;
        }
        if (beforePattern.test(params)) {
          const [match] = params.match(beforePattern);
          node.value = params.replace(match, "");
          node.raws.afterName = (node.raws.afterName || "") + match;
          node.variable = true;
        }
      };
    }
  });

  // node_modules/postcss-less/lib/LessParser.js
  var require_LessParser = __commonJS({
    "node_modules/postcss-less/lib/LessParser.js"(exports, module) {
      var Comment = require_comment();
      var Parser = require_parser();
      var { isInlineComment } = require_inline_comment();
      var { interpolation } = require_interpolation();
      var { isMixinToken } = require_mixin();
      var importNode = require_import();
      var variableNode = require_variable();
      var importantPattern = /(!\s*important)$/i;
      module.exports = class LessParser extends Parser {
        constructor(...args) {
          super(...args);
          this.lastNode = null;
        }
        atrule(token) {
          if (interpolation.bind(this)(token)) {
            return;
          }
          super.atrule(token);
          importNode(this.lastNode);
          variableNode(this.lastNode);
        }
        decl(...args) {
          super.decl(...args);
          const extendPattern = /extend\(.+\)/i;
          if (extendPattern.test(this.lastNode.value)) {
            this.lastNode.extend = true;
          }
        }
        each(tokens) {
          tokens[0][1] = ` ${tokens[0][1]}`;
          const firstParenIndex = tokens.findIndex((t) => t[0] === "(");
          const lastParen = tokens.reverse().find((t) => t[0] === ")");
          const lastParenIndex = tokens.reverse().indexOf(lastParen);
          const paramTokens = tokens.splice(firstParenIndex, lastParenIndex);
          const params = paramTokens.map((t) => t[1]).join("");
          for (const token of tokens.reverse()) {
            this.tokenizer.back(token);
          }
          this.atrule(this.tokenizer.nextToken());
          this.lastNode.function = true;
          this.lastNode.params = params;
        }
        init(node, line2, column) {
          super.init(node, line2, column);
          this.lastNode = node;
        }
        inlineComment(token) {
          const node = new Comment();
          const text = token[1].slice(2);
          this.init(node, token[2]);
          node.source.end = this.getPosition(token[3] || token[2]);
          node.inline = true;
          node.raws.begin = "//";
          if (/^\s*$/.test(text)) {
            node.text = "";
            node.raws.left = text;
            node.raws.right = "";
          } else {
            const match = text.match(/^(\s*)([^]*[^\s])(\s*)$/);
            [, node.raws.left, node.text, node.raws.right] = match;
          }
        }
        mixin(tokens) {
          const [first] = tokens;
          const identifier = first[1].slice(0, 1);
          const bracketsIndex = tokens.findIndex((t) => t[0] === "brackets");
          const firstParenIndex = tokens.findIndex((t) => t[0] === "(");
          let important = "";
          if ((bracketsIndex < 0 || bracketsIndex > 3) && firstParenIndex > 0) {
            const lastParenIndex = tokens.reduce((last2, t, i) => t[0] === ")" ? i : last2);
            const contents = tokens.slice(firstParenIndex, lastParenIndex + firstParenIndex);
            const brackets = contents.map((t) => t[1]).join("");
            const [paren] = tokens.slice(firstParenIndex);
            const start = [paren[2], paren[3]];
            const [last] = tokens.slice(lastParenIndex, lastParenIndex + 1);
            const end = [last[2], last[3]];
            const newToken = ["brackets", brackets].concat(start, end);
            const tokensBefore = tokens.slice(0, firstParenIndex);
            const tokensAfter = tokens.slice(lastParenIndex + 1);
            tokens = tokensBefore;
            tokens.push(newToken);
            tokens = tokens.concat(tokensAfter);
          }
          const importantTokens = [];
          for (const token of tokens) {
            if (token[1] === "!" || importantTokens.length) {
              importantTokens.push(token);
            }
            if (token[1] === "important") {
              break;
            }
          }
          if (importantTokens.length) {
            const [bangToken] = importantTokens;
            const bangIndex = tokens.indexOf(bangToken);
            const last = importantTokens[importantTokens.length - 1];
            const start = [bangToken[2], bangToken[3]];
            const end = [last[4], last[5]];
            const combined = importantTokens.map((t) => t[1]).join("");
            const newToken = ["word", combined].concat(start, end);
            tokens.splice(bangIndex, importantTokens.length, newToken);
          }
          const importantIndex = tokens.findIndex((t) => importantPattern.test(t[1]));
          if (importantIndex > 0) {
            [, important] = tokens[importantIndex];
            tokens.splice(importantIndex, 1);
          }
          for (const token of tokens.reverse()) {
            this.tokenizer.back(token);
          }
          this.atrule(this.tokenizer.nextToken());
          this.lastNode.mixin = true;
          this.lastNode.raws.identifier = identifier;
          if (important) {
            this.lastNode.important = true;
            this.lastNode.raws.important = important;
          }
        }
        other(token) {
          if (!isInlineComment.bind(this)(token)) {
            super.other(token);
          }
        }
        rule(tokens) {
          const last = tokens[tokens.length - 1];
          const prev = tokens[tokens.length - 2];
          if (prev[0] === "at-word" && last[0] === "{") {
            this.tokenizer.back(last);
            if (interpolation.bind(this)(prev)) {
              const newToken = this.tokenizer.nextToken();
              tokens = tokens.slice(0, tokens.length - 2).concat([newToken]);
              for (const tokn of tokens.reverse()) {
                this.tokenizer.back(tokn);
              }
              return;
            }
          }
          super.rule(tokens);
          const extendPattern = /:extend\(.+\)/i;
          if (extendPattern.test(this.lastNode.selector)) {
            this.lastNode.extend = true;
          }
        }
        unknownWord(tokens) {
          const [first] = tokens;
          if (tokens[0][1] === "each" && tokens[1][0] === "(") {
            this.each(tokens);
            return;
          }
          if (isMixinToken(first)) {
            this.mixin(tokens);
            return;
          }
          super.unknownWord(tokens);
        }
      };
    }
  });

  // node_modules/postcss-less/lib/LessStringifier.js
  var require_LessStringifier = __commonJS({
    "node_modules/postcss-less/lib/LessStringifier.js"(exports, module) {
      var Stringifier = require_stringifier();
      module.exports = class LessStringifier extends Stringifier {
        atrule(node, semicolon) {
          if (!node.mixin && !node.variable && !node.function) {
            super.atrule(node, semicolon);
            return;
          }
          const identifier = node.function ? "" : node.raws.identifier || "@";
          let name = `${identifier}${node.name}`;
          let params = node.params ? this.rawValue(node, "params") : "";
          const important = node.raws.important || "";
          if (node.variable) {
            params = node.value;
          }
          if (typeof node.raws.afterName !== "undefined") {
            name += node.raws.afterName;
          } else if (params) {
            name += " ";
          }
          if (node.nodes) {
            this.block(node, name + params + important);
          } else {
            const end = (node.raws.between || "") + important + (semicolon ? ";" : "");
            this.builder(name + params + end, node);
          }
        }
        comment(node) {
          if (node.inline) {
            const left = this.raw(node, "left", "commentLeft");
            const right = this.raw(node, "right", "commentRight");
            this.builder(`//${left}${node.text}${right}`, node);
          } else {
            super.comment(node);
          }
        }
      };
    }
  });

  // node_modules/postcss-less/lib/index.js
  var require_lib = __commonJS({
    "node_modules/postcss-less/lib/index.js"(exports, module) {
      var Input = require_input();
      var LessParser = require_LessParser();
      var LessStringifier = require_LessStringifier();
      module.exports = {
        parse(less2, options2) {
          const input = new Input(less2, options2);
          const parser = new LessParser(input);
          parser.parse();
          parser.root.walk((node) => {
            const offset = input.css.lastIndexOf(node.source.input.css);
            if (offset === 0) {
              return;
            }
            if (offset + node.source.input.css.length !== input.css.length) {
              throw new Error("Invalid state detected in postcss-less");
            }
            const newStartOffset = offset + node.source.start.offset;
            const newStartPosition = input.fromOffset(offset + node.source.start.offset);
            node.source.start = {
              offset: newStartOffset,
              line: newStartPosition.line,
              column: newStartPosition.col
            };
            if (node.source.end) {
              const newEndOffset = offset + node.source.end.offset;
              const newEndPosition = input.fromOffset(offset + node.source.end.offset);
              node.source.end = {
                offset: newEndOffset,
                line: newEndPosition.line,
                column: newEndPosition.col
              };
            }
          });
          return parser.root;
        },
        stringify(node, builder) {
          const stringifier = new LessStringifier(builder);
          stringifier.stringify(node);
        },
        nodeToString(node) {
          let result = "";
          module.exports.stringify(node, (bit) => {
            result += bit;
          });
          return result;
        }
      };
    }
  });

  // node_modules/postcss/lib/map-generator.js
  var require_map_generator = __commonJS({
    "node_modules/postcss/lib/map-generator.js"(exports, module) {
      module.exports = class {
        generate() {
        }
      };
    }
  });

  // node_modules/postcss/lib/document.js
  var require_document = __commonJS({
    "node_modules/postcss/lib/document.js"(exports, module) {
      "use strict";
      var Container = require_container();
      var LazyResult;
      var Processor;
      var Document = class extends Container {
        constructor(defaults) {
          super({ type: "document", ...defaults });
          if (!this.nodes) {
            this.nodes = [];
          }
        }
        toResult(opts = {}) {
          let lazy = new LazyResult(new Processor(), this, opts);
          return lazy.stringify();
        }
      };
      Document.registerLazyResult = (dependant) => {
        LazyResult = dependant;
      };
      Document.registerProcessor = (dependant) => {
        Processor = dependant;
      };
      module.exports = Document;
      Document.default = Document;
    }
  });

  // node_modules/postcss/lib/warn-once.js
  var require_warn_once = __commonJS({
    "node_modules/postcss/lib/warn-once.js"(exports, module) {
      "use strict";
      var printed = {};
      module.exports = function warnOnce(message) {
        if (printed[message])
          return;
        printed[message] = true;
        if (typeof console !== "undefined" && console.warn) {
          console.warn(message);
        }
      };
    }
  });

  // node_modules/postcss/lib/warning.js
  var require_warning = __commonJS({
    "node_modules/postcss/lib/warning.js"(exports, module) {
      "use strict";
      var Warning = class {
        constructor(text, opts = {}) {
          this.type = "warning";
          this.text = text;
          if (opts.node && opts.node.source) {
            let range = opts.node.rangeBy(opts);
            this.line = range.start.line;
            this.column = range.start.column;
            this.endLine = range.end.line;
            this.endColumn = range.end.column;
          }
          for (let opt in opts)
            this[opt] = opts[opt];
        }
        toString() {
          if (this.node) {
            return this.node.error(this.text, {
              index: this.index,
              plugin: this.plugin,
              word: this.word
            }).message;
          }
          if (this.plugin) {
            return this.plugin + ": " + this.text;
          }
          return this.text;
        }
      };
      module.exports = Warning;
      Warning.default = Warning;
    }
  });

  // node_modules/postcss/lib/result.js
  var require_result = __commonJS({
    "node_modules/postcss/lib/result.js"(exports, module) {
      "use strict";
      var Warning = require_warning();
      var Result = class {
        constructor(processor, root, opts) {
          this.processor = processor;
          this.messages = [];
          this.root = root;
          this.opts = opts;
          this.css = void 0;
          this.map = void 0;
        }
        get content() {
          return this.css;
        }
        toString() {
          return this.css;
        }
        warn(text, opts = {}) {
          if (!opts.plugin) {
            if (this.lastPlugin && this.lastPlugin.postcssPlugin) {
              opts.plugin = this.lastPlugin.postcssPlugin;
            }
          }
          let warning = new Warning(text, opts);
          this.messages.push(warning);
          return warning;
        }
        warnings() {
          return this.messages.filter((i) => i.type === "warning");
        }
      };
      module.exports = Result;
      Result.default = Result;
    }
  });

  // node_modules/postcss/lib/lazy-result.js
  var require_lazy_result = __commonJS({
    "node_modules/postcss/lib/lazy-result.js"(exports, module) {
      "use strict";
      var { isClean, my } = require_symbols();
      var MapGenerator = require_map_generator();
      var stringify = require_stringify();
      var Container = require_container();
      var Document = require_document();
      var warnOnce = require_warn_once();
      var Result = require_result();
      var parse3 = require_parse();
      var Root = require_root();
      var TYPE_TO_CLASS_NAME = {
        atrule: "AtRule",
        comment: "Comment",
        decl: "Declaration",
        document: "Document",
        root: "Root",
        rule: "Rule"
      };
      var PLUGIN_PROPS = {
        AtRule: true,
        AtRuleExit: true,
        Comment: true,
        CommentExit: true,
        Declaration: true,
        DeclarationExit: true,
        Document: true,
        DocumentExit: true,
        Once: true,
        OnceExit: true,
        postcssPlugin: true,
        prepare: true,
        Root: true,
        RootExit: true,
        Rule: true,
        RuleExit: true
      };
      var NOT_VISITORS = {
        Once: true,
        postcssPlugin: true,
        prepare: true
      };
      var CHILDREN = 0;
      function isPromise(obj) {
        return typeof obj === "object" && typeof obj.then === "function";
      }
      function getEvents(node) {
        let key = false;
        let type = TYPE_TO_CLASS_NAME[node.type];
        if (node.type === "decl") {
          key = node.prop.toLowerCase();
        } else if (node.type === "atrule") {
          key = node.name.toLowerCase();
        }
        if (key && node.append) {
          return [
            type,
            type + "-" + key,
            CHILDREN,
            type + "Exit",
            type + "Exit-" + key
          ];
        } else if (key) {
          return [type, type + "-" + key, type + "Exit", type + "Exit-" + key];
        } else if (node.append) {
          return [type, CHILDREN, type + "Exit"];
        } else {
          return [type, type + "Exit"];
        }
      }
      function toStack(node) {
        let events;
        if (node.type === "document") {
          events = ["Document", CHILDREN, "DocumentExit"];
        } else if (node.type === "root") {
          events = ["Root", CHILDREN, "RootExit"];
        } else {
          events = getEvents(node);
        }
        return {
          eventIndex: 0,
          events,
          iterator: 0,
          node,
          visitorIndex: 0,
          visitors: []
        };
      }
      function cleanMarks(node) {
        node[isClean] = false;
        if (node.nodes)
          node.nodes.forEach((i) => cleanMarks(i));
        return node;
      }
      var postcss = {};
      var LazyResult = class _LazyResult {
        constructor(processor, css2, opts) {
          this.stringified = false;
          this.processed = false;
          let root;
          if (typeof css2 === "object" && css2 !== null && (css2.type === "root" || css2.type === "document")) {
            root = cleanMarks(css2);
          } else if (css2 instanceof _LazyResult || css2 instanceof Result) {
            root = cleanMarks(css2.root);
            if (css2.map) {
              if (typeof opts.map === "undefined")
                opts.map = {};
              if (!opts.map.inline)
                opts.map.inline = false;
              opts.map.prev = css2.map;
            }
          } else {
            let parser = parse3;
            if (opts.syntax)
              parser = opts.syntax.parse;
            if (opts.parser)
              parser = opts.parser;
            if (parser.parse)
              parser = parser.parse;
            try {
              root = parser(css2, opts);
            } catch (error) {
              this.processed = true;
              this.error = error;
            }
            if (root && !root[my]) {
              Container.rebuild(root);
            }
          }
          this.result = new Result(processor, root, opts);
          this.helpers = { ...postcss, postcss, result: this.result };
          this.plugins = this.processor.plugins.map((plugin) => {
            if (typeof plugin === "object" && plugin.prepare) {
              return { ...plugin, ...plugin.prepare(this.result) };
            } else {
              return plugin;
            }
          });
        }
        async() {
          if (this.error)
            return Promise.reject(this.error);
          if (this.processed)
            return Promise.resolve(this.result);
          if (!this.processing) {
            this.processing = this.runAsync();
          }
          return this.processing;
        }
        catch(onRejected) {
          return this.async().catch(onRejected);
        }
        get content() {
          return this.stringify().content;
        }
        get css() {
          return this.stringify().css;
        }
        finally(onFinally) {
          return this.async().then(onFinally, onFinally);
        }
        getAsyncError() {
          throw new Error("Use process(css).then(cb) to work with async plugins");
        }
        handleError(error, node) {
          let plugin = this.result.lastPlugin;
          try {
            if (node)
              node.addToError(error);
            this.error = error;
            if (error.name === "CssSyntaxError" && !error.plugin) {
              error.plugin = plugin.postcssPlugin;
              error.setMessage();
            } else if (plugin.postcssVersion) {
              if (false) {
                let pluginName = plugin.postcssPlugin;
                let pluginVer = plugin.postcssVersion;
                let runtimeVer = this.result.processor.version;
                let a = pluginVer.split(".");
                let b = runtimeVer.split(".");
                if (a[0] !== b[0] || parseInt(a[1]) > parseInt(b[1])) {
                  console.error(
                    "Unknown error from PostCSS plugin. Your current PostCSS version is " + runtimeVer + ", but " + pluginName + " uses " + pluginVer + ". Perhaps this is the source of the error below."
                  );
                }
              }
            }
          } catch (err) {
            if (console && console.error)
              console.error(err);
          }
          return error;
        }
        get map() {
          return this.stringify().map;
        }
        get messages() {
          return this.sync().messages;
        }
        get opts() {
          return this.result.opts;
        }
        prepareVisitors() {
          this.listeners = {};
          let add = (plugin, type, cb) => {
            if (!this.listeners[type])
              this.listeners[type] = [];
            this.listeners[type].push([plugin, cb]);
          };
          for (let plugin of this.plugins) {
            if (typeof plugin === "object") {
              for (let event in plugin) {
                if (!PLUGIN_PROPS[event] && /^[A-Z]/.test(event)) {
                  throw new Error(
                    `Unknown event ${event} in ${plugin.postcssPlugin}. Try to update PostCSS (${this.processor.version} now).`
                  );
                }
                if (!NOT_VISITORS[event]) {
                  if (typeof plugin[event] === "object") {
                    for (let filter in plugin[event]) {
                      if (filter === "*") {
                        add(plugin, event, plugin[event][filter]);
                      } else {
                        add(
                          plugin,
                          event + "-" + filter.toLowerCase(),
                          plugin[event][filter]
                        );
                      }
                    }
                  } else if (typeof plugin[event] === "function") {
                    add(plugin, event, plugin[event]);
                  }
                }
              }
            }
          }
          this.hasListener = Object.keys(this.listeners).length > 0;
        }
        get processor() {
          return this.result.processor;
        }
        get root() {
          return this.sync().root;
        }
        async runAsync() {
          this.plugin = 0;
          for (let i = 0; i < this.plugins.length; i++) {
            let plugin = this.plugins[i];
            let promise = this.runOnRoot(plugin);
            if (isPromise(promise)) {
              try {
                await promise;
              } catch (error) {
                throw this.handleError(error);
              }
            }
          }
          this.prepareVisitors();
          if (this.hasListener) {
            let root = this.result.root;
            while (!root[isClean]) {
              root[isClean] = true;
              let stack = [toStack(root)];
              while (stack.length > 0) {
                let promise = this.visitTick(stack);
                if (isPromise(promise)) {
                  try {
                    await promise;
                  } catch (e) {
                    let node = stack[stack.length - 1].node;
                    throw this.handleError(e, node);
                  }
                }
              }
            }
            if (this.listeners.OnceExit) {
              for (let [plugin, visitor] of this.listeners.OnceExit) {
                this.result.lastPlugin = plugin;
                try {
                  if (root.type === "document") {
                    let roots = root.nodes.map(
                      (subRoot) => visitor(subRoot, this.helpers)
                    );
                    await Promise.all(roots);
                  } else {
                    await visitor(root, this.helpers);
                  }
                } catch (e) {
                  throw this.handleError(e);
                }
              }
            }
          }
          this.processed = true;
          return this.stringify();
        }
        runOnRoot(plugin) {
          this.result.lastPlugin = plugin;
          try {
            if (typeof plugin === "object" && plugin.Once) {
              if (this.result.root.type === "document") {
                let roots = this.result.root.nodes.map(
                  (root) => plugin.Once(root, this.helpers)
                );
                if (isPromise(roots[0])) {
                  return Promise.all(roots);
                }
                return roots;
              }
              return plugin.Once(this.result.root, this.helpers);
            } else if (typeof plugin === "function") {
              return plugin(this.result.root, this.result);
            }
          } catch (error) {
            throw this.handleError(error);
          }
        }
        stringify() {
          if (this.error)
            throw this.error;
          if (this.stringified)
            return this.result;
          this.stringified = true;
          this.sync();
          let opts = this.result.opts;
          let str = stringify;
          if (opts.syntax)
            str = opts.syntax.stringify;
          if (opts.stringifier)
            str = opts.stringifier;
          if (str.stringify)
            str = str.stringify;
          let map = new MapGenerator(str, this.result.root, this.result.opts);
          let data = map.generate();
          this.result.css = data[0];
          this.result.map = data[1];
          return this.result;
        }
        get [Symbol.toStringTag]() {
          return "LazyResult";
        }
        sync() {
          if (this.error)
            throw this.error;
          if (this.processed)
            return this.result;
          this.processed = true;
          if (this.processing) {
            throw this.getAsyncError();
          }
          for (let plugin of this.plugins) {
            let promise = this.runOnRoot(plugin);
            if (isPromise(promise)) {
              throw this.getAsyncError();
            }
          }
          this.prepareVisitors();
          if (this.hasListener) {
            let root = this.result.root;
            while (!root[isClean]) {
              root[isClean] = true;
              this.walkSync(root);
            }
            if (this.listeners.OnceExit) {
              if (root.type === "document") {
                for (let subRoot of root.nodes) {
                  this.visitSync(this.listeners.OnceExit, subRoot);
                }
              } else {
                this.visitSync(this.listeners.OnceExit, root);
              }
            }
          }
          return this.result;
        }
        then(onFulfilled, onRejected) {
          if (false) {
            if (!("from" in this.opts)) {
              warnOnce(
                "Without `from` option PostCSS could generate wrong source map and will not find Browserslist config. Set it to CSS file path or to `undefined` to prevent this warning."
              );
            }
          }
          return this.async().then(onFulfilled, onRejected);
        }
        toString() {
          return this.css;
        }
        visitSync(visitors, node) {
          for (let [plugin, visitor] of visitors) {
            this.result.lastPlugin = plugin;
            let promise;
            try {
              promise = visitor(node, this.helpers);
            } catch (e) {
              throw this.handleError(e, node.proxyOf);
            }
            if (node.type !== "root" && node.type !== "document" && !node.parent) {
              return true;
            }
            if (isPromise(promise)) {
              throw this.getAsyncError();
            }
          }
        }
        visitTick(stack) {
          let visit = stack[stack.length - 1];
          let { node, visitors } = visit;
          if (node.type !== "root" && node.type !== "document" && !node.parent) {
            stack.pop();
            return;
          }
          if (visitors.length > 0 && visit.visitorIndex < visitors.length) {
            let [plugin, visitor] = visitors[visit.visitorIndex];
            visit.visitorIndex += 1;
            if (visit.visitorIndex === visitors.length) {
              visit.visitors = [];
              visit.visitorIndex = 0;
            }
            this.result.lastPlugin = plugin;
            try {
              return visitor(node.toProxy(), this.helpers);
            } catch (e) {
              throw this.handleError(e, node);
            }
          }
          if (visit.iterator !== 0) {
            let iterator = visit.iterator;
            let child;
            while (child = node.nodes[node.indexes[iterator]]) {
              node.indexes[iterator] += 1;
              if (!child[isClean]) {
                child[isClean] = true;
                stack.push(toStack(child));
                return;
              }
            }
            visit.iterator = 0;
            delete node.indexes[iterator];
          }
          let events = visit.events;
          while (visit.eventIndex < events.length) {
            let event = events[visit.eventIndex];
            visit.eventIndex += 1;
            if (event === CHILDREN) {
              if (node.nodes && node.nodes.length) {
                node[isClean] = true;
                visit.iterator = node.getIterator();
              }
              return;
            } else if (this.listeners[event]) {
              visit.visitors = this.listeners[event];
              return;
            }
          }
          stack.pop();
        }
        walkSync(node) {
          node[isClean] = true;
          let events = getEvents(node);
          for (let event of events) {
            if (event === CHILDREN) {
              if (node.nodes) {
                node.each((child) => {
                  if (!child[isClean])
                    this.walkSync(child);
                });
              }
            } else {
              let visitors = this.listeners[event];
              if (visitors) {
                if (this.visitSync(visitors, node.toProxy()))
                  return;
              }
            }
          }
        }
        warnings() {
          return this.sync().warnings();
        }
      };
      LazyResult.registerPostcss = (dependant) => {
        postcss = dependant;
      };
      module.exports = LazyResult;
      LazyResult.default = LazyResult;
      Root.registerLazyResult(LazyResult);
      Document.registerLazyResult(LazyResult);
    }
  });

  // node_modules/postcss/lib/no-work-result.js
  var require_no_work_result = __commonJS({
    "node_modules/postcss/lib/no-work-result.js"(exports, module) {
      "use strict";
      var MapGenerator = require_map_generator();
      var stringify = require_stringify();
      var warnOnce = require_warn_once();
      var parse3 = require_parse();
      var Result = require_result();
      var NoWorkResult = class {
        constructor(processor, css2, opts) {
          css2 = css2.toString();
          this.stringified = false;
          this._processor = processor;
          this._css = css2;
          this._opts = opts;
          this._map = void 0;
          let root;
          let str = stringify;
          this.result = new Result(this._processor, root, this._opts);
          this.result.css = css2;
          let self = this;
          Object.defineProperty(this.result, "root", {
            get() {
              return self.root;
            }
          });
          let map = new MapGenerator(str, root, this._opts, css2);
          if (map.isMap()) {
            let [generatedCSS, generatedMap] = map.generate();
            if (generatedCSS) {
              this.result.css = generatedCSS;
            }
            if (generatedMap) {
              this.result.map = generatedMap;
            }
          }
        }
        async() {
          if (this.error)
            return Promise.reject(this.error);
          return Promise.resolve(this.result);
        }
        catch(onRejected) {
          return this.async().catch(onRejected);
        }
        get content() {
          return this.result.css;
        }
        get css() {
          return this.result.css;
        }
        finally(onFinally) {
          return this.async().then(onFinally, onFinally);
        }
        get map() {
          return this.result.map;
        }
        get messages() {
          return [];
        }
        get opts() {
          return this.result.opts;
        }
        get processor() {
          return this.result.processor;
        }
        get root() {
          if (this._root) {
            return this._root;
          }
          let root;
          let parser = parse3;
          try {
            root = parser(this._css, this._opts);
          } catch (error) {
            this.error = error;
          }
          if (this.error) {
            throw this.error;
          } else {
            this._root = root;
            return root;
          }
        }
        get [Symbol.toStringTag]() {
          return "NoWorkResult";
        }
        sync() {
          if (this.error)
            throw this.error;
          return this.result;
        }
        then(onFulfilled, onRejected) {
          if (false) {
            if (!("from" in this._opts)) {
              warnOnce(
                "Without `from` option PostCSS could generate wrong source map and will not find Browserslist config. Set it to CSS file path or to `undefined` to prevent this warning."
              );
            }
          }
          return this.async().then(onFulfilled, onRejected);
        }
        toString() {
          return this._css;
        }
        warnings() {
          return [];
        }
      };
      module.exports = NoWorkResult;
      NoWorkResult.default = NoWorkResult;
    }
  });

  // node_modules/postcss/lib/processor.js
  var require_processor = __commonJS({
    "node_modules/postcss/lib/processor.js"(exports, module) {
      "use strict";
      var NoWorkResult = require_no_work_result();
      var LazyResult = require_lazy_result();
      var Document = require_document();
      var Root = require_root();
      var Processor = class {
        constructor(plugins = []) {
          this.version = "8.4.28";
          this.plugins = this.normalize(plugins);
        }
        normalize(plugins) {
          let normalized = [];
          for (let i of plugins) {
            if (i.postcss === true) {
              i = i();
            } else if (i.postcss) {
              i = i.postcss;
            }
            if (typeof i === "object" && Array.isArray(i.plugins)) {
              normalized = normalized.concat(i.plugins);
            } else if (typeof i === "object" && i.postcssPlugin) {
              normalized.push(i);
            } else if (typeof i === "function") {
              normalized.push(i);
            } else if (typeof i === "object" && (i.parse || i.stringify)) {
              if (false) {
                throw new Error(
                  "PostCSS syntaxes cannot be used as plugins. Instead, please use one of the syntax/parser/stringifier options as outlined in your PostCSS runner documentation."
                );
              }
            } else {
              throw new Error(i + " is not a PostCSS plugin");
            }
          }
          return normalized;
        }
        process(css2, opts = {}) {
          if (this.plugins.length === 0 && typeof opts.parser === "undefined" && typeof opts.stringifier === "undefined" && typeof opts.syntax === "undefined") {
            return new NoWorkResult(this, css2, opts);
          } else {
            return new LazyResult(this, css2, opts);
          }
        }
        use(plugin) {
          this.plugins = this.plugins.concat(this.normalize([plugin]));
          return this;
        }
      };
      module.exports = Processor;
      Processor.default = Processor;
      Root.registerProcessor(Processor);
      Document.registerProcessor(Processor);
    }
  });

  // node_modules/postcss/lib/fromJSON.js
  var require_fromJSON = __commonJS({
    "node_modules/postcss/lib/fromJSON.js"(exports, module) {
      "use strict";
      var Declaration = require_declaration();
      var PreviousMap = require_previous_map();
      var Comment = require_comment();
      var AtRule = require_at_rule();
      var Input = require_input();
      var Root = require_root();
      var Rule = require_rule();
      function fromJSON(json, inputs) {
        if (Array.isArray(json))
          return json.map((n) => fromJSON(n));
        let { inputs: ownInputs, ...defaults } = json;
        if (ownInputs) {
          inputs = [];
          for (let input of ownInputs) {
            let inputHydrated = { ...input, __proto__: Input.prototype };
            if (inputHydrated.map) {
              inputHydrated.map = {
                ...inputHydrated.map,
                __proto__: PreviousMap.prototype
              };
            }
            inputs.push(inputHydrated);
          }
        }
        if (defaults.nodes) {
          defaults.nodes = json.nodes.map((n) => fromJSON(n, inputs));
        }
        if (defaults.source) {
          let { inputId, ...source } = defaults.source;
          defaults.source = source;
          if (inputId != null) {
            defaults.source.input = inputs[inputId];
          }
        }
        if (defaults.type === "root") {
          return new Root(defaults);
        } else if (defaults.type === "decl") {
          return new Declaration(defaults);
        } else if (defaults.type === "rule") {
          return new Rule(defaults);
        } else if (defaults.type === "comment") {
          return new Comment(defaults);
        } else if (defaults.type === "atrule") {
          return new AtRule(defaults);
        } else {
          throw new Error("Unknown node type: " + json.type);
        }
      }
      module.exports = fromJSON;
      fromJSON.default = fromJSON;
    }
  });

  // node_modules/postcss/lib/postcss.js
  var require_postcss = __commonJS({
    "node_modules/postcss/lib/postcss.js"(exports, module) {
      "use strict";
      var CssSyntaxError = require_css_syntax_error();
      var Declaration = require_declaration();
      var LazyResult = require_lazy_result();
      var Container = require_container();
      var Processor = require_processor();
      var stringify = require_stringify();
      var fromJSON = require_fromJSON();
      var Document = require_document();
      var Warning = require_warning();
      var Comment = require_comment();
      var AtRule = require_at_rule();
      var Result = require_result();
      var Input = require_input();
      var parse3 = require_parse();
      var list = require_list();
      var Rule = require_rule();
      var Root = require_root();
      var Node = require_node();
      function postcss(...plugins) {
        if (plugins.length === 1 && Array.isArray(plugins[0])) {
          plugins = plugins[0];
        }
        return new Processor(plugins);
      }
      postcss.plugin = function plugin(name, initializer) {
        let warningPrinted = false;
        function creator(...args) {
          if (console && console.warn && !warningPrinted) {
            warningPrinted = true;
            console.warn(
              name + ": postcss.plugin was deprecated. Migration guide:\nhttps://evilmartians.com/chronicles/postcss-8-plugin-migration"
            );
            if ("") {
              console.warn(
                name + ": \u91CC\u9762 postcss.plugin \u88AB\u5F03\u7528. \u8FC1\u79FB\u6307\u5357:\nhttps://www.w3ctech.com/topic/2226"
              );
            }
          }
          let transformer = initializer(...args);
          transformer.postcssPlugin = name;
          transformer.postcssVersion = new Processor().version;
          return transformer;
        }
        let cache;
        Object.defineProperty(creator, "postcss", {
          get() {
            if (!cache)
              cache = creator();
            return cache;
          }
        });
        creator.process = function(css2, processOpts, pluginOpts) {
          return postcss([creator(pluginOpts)]).process(css2, processOpts);
        };
        return creator;
      };
      postcss.stringify = stringify;
      postcss.parse = parse3;
      postcss.fromJSON = fromJSON;
      postcss.list = list;
      postcss.comment = (defaults) => new Comment(defaults);
      postcss.atRule = (defaults) => new AtRule(defaults);
      postcss.decl = (defaults) => new Declaration(defaults);
      postcss.rule = (defaults) => new Rule(defaults);
      postcss.root = (defaults) => new Root(defaults);
      postcss.document = (defaults) => new Document(defaults);
      postcss.CssSyntaxError = CssSyntaxError;
      postcss.Declaration = Declaration;
      postcss.Container = Container;
      postcss.Processor = Processor;
      postcss.Document = Document;
      postcss.Comment = Comment;
      postcss.Warning = Warning;
      postcss.AtRule = AtRule;
      postcss.Result = Result;
      postcss.Input = Input;
      postcss.Rule = Rule;
      postcss.Root = Root;
      postcss.Node = Node;
      LazyResult.registerPostcss(postcss);
      module.exports = postcss;
      postcss.default = postcss;
    }
  });

  // node_modules/postcss-scss/lib/nested-declaration.js
  var require_nested_declaration = __commonJS({
    "node_modules/postcss-scss/lib/nested-declaration.js"(exports, module) {
      var { Container } = require_postcss();
      var NestedDeclaration = class extends Container {
        constructor(defaults) {
          super(defaults);
          this.type = "decl";
          this.isNested = true;
          if (!this.nodes)
            this.nodes = [];
        }
      };
      module.exports = NestedDeclaration;
    }
  });

  // node_modules/postcss-scss/lib/scss-tokenize.js
  var require_scss_tokenize = __commonJS({
    "node_modules/postcss-scss/lib/scss-tokenize.js"(exports, module) {
      "use strict";
      var SINGLE_QUOTE2 = "'".charCodeAt(0);
      var DOUBLE_QUOTE2 = '"'.charCodeAt(0);
      var BACKSLASH = "\\".charCodeAt(0);
      var SLASH = "/".charCodeAt(0);
      var NEWLINE = "\n".charCodeAt(0);
      var SPACE = " ".charCodeAt(0);
      var FEED = "\f".charCodeAt(0);
      var TAB = "	".charCodeAt(0);
      var CR = "\r".charCodeAt(0);
      var OPEN_SQUARE = "[".charCodeAt(0);
      var CLOSE_SQUARE = "]".charCodeAt(0);
      var OPEN_PARENTHESES = "(".charCodeAt(0);
      var CLOSE_PARENTHESES = ")".charCodeAt(0);
      var OPEN_CURLY = "{".charCodeAt(0);
      var CLOSE_CURLY = "}".charCodeAt(0);
      var SEMICOLON = ";".charCodeAt(0);
      var ASTERISK = "*".charCodeAt(0);
      var COLON = ":".charCodeAt(0);
      var AT = "@".charCodeAt(0);
      var COMMA = ",".charCodeAt(0);
      var HASH = "#".charCodeAt(0);
      var RE_AT_END = /[\t\n\f\r "#'()/;[\\\]{}]/g;
      var RE_WORD_END = /[,\t\n\f\r !"#'():;@[\\\]{}]|\/(?=\*)/g;
      var RE_BAD_BRACKET = /.[\n"'(/\\]/;
      var RE_HEX_ESCAPE = /[\da-f]/i;
      var RE_NEW_LINE = /[\n\f\r]/g;
      module.exports = function scssTokenize(input, options2 = {}) {
        let css2 = input.css.valueOf();
        let ignore = options2.ignoreErrors;
        let code, next, quote, content, escape;
        let escaped, prev, n, currentToken;
        let length = css2.length;
        let pos = 0;
        let buffer = [];
        let returned = [];
        let brackets;
        function position() {
          return pos;
        }
        function unclosed(what) {
          throw input.error("Unclosed " + what, pos);
        }
        function endOfFile() {
          return returned.length === 0 && pos >= length;
        }
        function interpolation() {
          let deep = 1;
          let stringQuote = false;
          let stringEscaped = false;
          while (deep > 0) {
            next += 1;
            if (css2.length <= next)
              unclosed("interpolation");
            code = css2.charCodeAt(next);
            n = css2.charCodeAt(next + 1);
            if (stringQuote) {
              if (!stringEscaped && code === stringQuote) {
                stringQuote = false;
                stringEscaped = false;
              } else if (code === BACKSLASH) {
                stringEscaped = !stringEscaped;
              } else if (stringEscaped) {
                stringEscaped = false;
              }
            } else if (code === SINGLE_QUOTE2 || code === DOUBLE_QUOTE2) {
              stringQuote = code;
            } else if (code === CLOSE_CURLY) {
              deep -= 1;
            } else if (code === HASH && n === OPEN_CURLY) {
              deep += 1;
            }
          }
        }
        function nextToken(opts) {
          if (returned.length)
            return returned.pop();
          if (pos >= length)
            return void 0;
          let ignoreUnclosed = opts ? opts.ignoreUnclosed : false;
          code = css2.charCodeAt(pos);
          switch (code) {
            case NEWLINE:
            case SPACE:
            case TAB:
            case CR:
            case FEED: {
              next = pos;
              do {
                next += 1;
                code = css2.charCodeAt(next);
              } while (code === SPACE || code === NEWLINE || code === TAB || code === CR || code === FEED);
              currentToken = ["space", css2.slice(pos, next)];
              pos = next - 1;
              break;
            }
            case OPEN_SQUARE:
            case CLOSE_SQUARE:
            case OPEN_CURLY:
            case CLOSE_CURLY:
            case COLON:
            case SEMICOLON:
            case CLOSE_PARENTHESES: {
              let controlChar = String.fromCharCode(code);
              currentToken = [controlChar, controlChar, pos];
              break;
            }
            case COMMA: {
              currentToken = ["word", ",", pos, pos + 1];
              break;
            }
            case OPEN_PARENTHESES: {
              prev = buffer.length ? buffer.pop()[1] : "";
              n = css2.charCodeAt(pos + 1);
              if (prev === "url" && n !== SINGLE_QUOTE2 && n !== DOUBLE_QUOTE2) {
                brackets = 1;
                escaped = false;
                next = pos + 1;
                while (next <= css2.length - 1) {
                  n = css2.charCodeAt(next);
                  if (n === BACKSLASH) {
                    escaped = !escaped;
                  } else if (n === OPEN_PARENTHESES) {
                    brackets += 1;
                  } else if (n === CLOSE_PARENTHESES) {
                    brackets -= 1;
                    if (brackets === 0)
                      break;
                  }
                  next += 1;
                }
                content = css2.slice(pos, next + 1);
                currentToken = ["brackets", content, pos, next];
                pos = next;
              } else {
                next = css2.indexOf(")", pos + 1);
                content = css2.slice(pos, next + 1);
                if (next === -1 || RE_BAD_BRACKET.test(content)) {
                  currentToken = ["(", "(", pos];
                } else {
                  currentToken = ["brackets", content, pos, next];
                  pos = next;
                }
              }
              break;
            }
            case SINGLE_QUOTE2:
            case DOUBLE_QUOTE2: {
              quote = code;
              next = pos;
              escaped = false;
              while (next < length) {
                next++;
                if (next === length)
                  unclosed("string");
                code = css2.charCodeAt(next);
                n = css2.charCodeAt(next + 1);
                if (!escaped && code === quote) {
                  break;
                } else if (code === BACKSLASH) {
                  escaped = !escaped;
                } else if (escaped) {
                  escaped = false;
                } else if (code === HASH && n === OPEN_CURLY) {
                  interpolation();
                }
              }
              currentToken = ["string", css2.slice(pos, next + 1), pos, next];
              pos = next;
              break;
            }
            case AT: {
              RE_AT_END.lastIndex = pos + 1;
              RE_AT_END.test(css2);
              if (RE_AT_END.lastIndex === 0) {
                next = css2.length - 1;
              } else {
                next = RE_AT_END.lastIndex - 2;
              }
              currentToken = ["at-word", css2.slice(pos, next + 1), pos, next];
              pos = next;
              break;
            }
            case BACKSLASH: {
              next = pos;
              escape = true;
              while (css2.charCodeAt(next + 1) === BACKSLASH) {
                next += 1;
                escape = !escape;
              }
              code = css2.charCodeAt(next + 1);
              if (escape && code !== SLASH && code !== SPACE && code !== NEWLINE && code !== TAB && code !== CR && code !== FEED) {
                next += 1;
                if (RE_HEX_ESCAPE.test(css2.charAt(next))) {
                  while (RE_HEX_ESCAPE.test(css2.charAt(next + 1))) {
                    next += 1;
                  }
                  if (css2.charCodeAt(next + 1) === SPACE) {
                    next += 1;
                  }
                }
              }
              currentToken = ["word", css2.slice(pos, next + 1), pos, next];
              pos = next;
              break;
            }
            default:
              n = css2.charCodeAt(pos + 1);
              if (code === HASH && n === OPEN_CURLY) {
                next = pos;
                interpolation();
                content = css2.slice(pos, next + 1);
                currentToken = ["word", content, pos, next];
                pos = next;
              } else if (code === SLASH && n === ASTERISK) {
                next = css2.indexOf("*/", pos + 2) + 1;
                if (next === 0) {
                  if (ignore || ignoreUnclosed) {
                    next = css2.length;
                  } else {
                    unclosed("comment");
                  }
                }
                currentToken = ["comment", css2.slice(pos, next + 1), pos, next];
                pos = next;
              } else if (code === SLASH && n === SLASH) {
                RE_NEW_LINE.lastIndex = pos + 1;
                RE_NEW_LINE.test(css2);
                if (RE_NEW_LINE.lastIndex === 0) {
                  next = css2.length - 1;
                } else {
                  next = RE_NEW_LINE.lastIndex - 2;
                }
                content = css2.slice(pos, next + 1);
                currentToken = ["comment", content, pos, next, "inline"];
                pos = next;
              } else {
                RE_WORD_END.lastIndex = pos + 1;
                RE_WORD_END.test(css2);
                if (RE_WORD_END.lastIndex === 0) {
                  next = css2.length - 1;
                } else {
                  next = RE_WORD_END.lastIndex - 2;
                }
                currentToken = ["word", css2.slice(pos, next + 1), pos, next];
                buffer.push(currentToken);
                pos = next;
              }
              break;
          }
          pos++;
          return currentToken;
        }
        function back(token) {
          returned.push(token);
        }
        return {
          back,
          endOfFile,
          nextToken,
          position
        };
      };
    }
  });

  // node_modules/postcss-scss/lib/scss-parser.js
  var require_scss_parser = __commonJS({
    "node_modules/postcss-scss/lib/scss-parser.js"(exports, module) {
      var { Comment } = require_postcss();
      var Parser = require_parser();
      var NestedDeclaration = require_nested_declaration();
      var scssTokenizer = require_scss_tokenize();
      var ScssParser = class extends Parser {
        atrule(token) {
          let name = token[1];
          let prev = token;
          while (!this.tokenizer.endOfFile()) {
            let next = this.tokenizer.nextToken();
            if (next[0] === "word" && next[2] === prev[3] + 1) {
              name += next[1];
              prev = next;
            } else {
              this.tokenizer.back(next);
              break;
            }
          }
          super.atrule(["at-word", name, token[2], prev[3]]);
        }
        comment(token) {
          if (token[4] === "inline") {
            let node = new Comment();
            this.init(node, token[2]);
            node.raws.inline = true;
            let pos = this.input.fromOffset(token[3]);
            node.source.end = { column: pos.col, line: pos.line, offset: token[3] };
            let text = token[1].slice(2);
            if (/^\s*$/.test(text)) {
              node.text = "";
              node.raws.left = text;
              node.raws.right = "";
            } else {
              let match = text.match(/^(\s*)([^]*\S)(\s*)$/);
              let fixed = match[2].replace(/(\*\/|\/\*)/g, "*//*");
              node.text = fixed;
              node.raws.left = match[1];
              node.raws.right = match[3];
              node.raws.text = match[2];
            }
          } else {
            super.comment(token);
          }
        }
        createTokenizer() {
          this.tokenizer = scssTokenizer(this.input);
        }
        raw(node, prop, tokens, customProperty) {
          super.raw(node, prop, tokens, customProperty);
          if (node.raws[prop]) {
            let scss2 = node.raws[prop].raw;
            node.raws[prop].raw = tokens.reduce((all, i) => {
              if (i[0] === "comment" && i[4] === "inline") {
                let text = i[1].slice(2).replace(/(\*\/|\/\*)/g, "*//*");
                return all + "/*" + text + "*/";
              } else {
                return all + i[1];
              }
            }, "");
            if (scss2 !== node.raws[prop].raw) {
              node.raws[prop].scss = scss2;
            }
          }
        }
        rule(tokens) {
          let withColon = false;
          let brackets = 0;
          let value = "";
          for (let i of tokens) {
            if (withColon) {
              if (i[0] !== "comment" && i[0] !== "{") {
                value += i[1];
              }
            } else if (i[0] === "space" && i[1].includes("\n")) {
              break;
            } else if (i[0] === "(") {
              brackets += 1;
            } else if (i[0] === ")") {
              brackets -= 1;
            } else if (brackets === 0 && i[0] === ":") {
              withColon = true;
            }
          }
          if (!withColon || value.trim() === "" || /^[#:A-Za-z-]/.test(value)) {
            super.rule(tokens);
          } else {
            tokens.pop();
            let node = new NestedDeclaration();
            this.init(node, tokens[0][2]);
            let last;
            for (let i = tokens.length - 1; i >= 0; i--) {
              if (tokens[i][0] !== "space") {
                last = tokens[i];
                break;
              }
            }
            if (last[3]) {
              let pos = this.input.fromOffset(last[3]);
              node.source.end = { column: pos.col, line: pos.line, offset: last[3] };
            } else {
              let pos = this.input.fromOffset(last[2]);
              node.source.end = { column: pos.col, line: pos.line, offset: last[2] };
            }
            while (tokens[0][0] !== "word") {
              node.raws.before += tokens.shift()[1];
            }
            if (tokens[0][2]) {
              let pos = this.input.fromOffset(tokens[0][2]);
              node.source.start = {
                column: pos.col,
                line: pos.line,
                offset: tokens[0][2]
              };
            }
            node.prop = "";
            while (tokens.length) {
              let type = tokens[0][0];
              if (type === ":" || type === "space" || type === "comment") {
                break;
              }
              node.prop += tokens.shift()[1];
            }
            node.raws.between = "";
            let token;
            while (tokens.length) {
              token = tokens.shift();
              if (token[0] === ":") {
                node.raws.between += token[1];
                break;
              } else {
                node.raws.between += token[1];
              }
            }
            if (node.prop[0] === "_" || node.prop[0] === "*") {
              node.raws.before += node.prop[0];
              node.prop = node.prop.slice(1);
            }
            node.raws.between += this.spacesAndCommentsFromStart(tokens);
            this.precheckMissedSemicolon(tokens);
            for (let i = tokens.length - 1; i > 0; i--) {
              token = tokens[i];
              if (token[1] === "!important") {
                node.important = true;
                let string = this.stringFrom(tokens, i);
                string = this.spacesFromEnd(tokens) + string;
                if (string !== " !important") {
                  node.raws.important = string;
                }
                break;
              } else if (token[1] === "important") {
                let cache = tokens.slice(0);
                let str = "";
                for (let j = i; j > 0; j--) {
                  let type = cache[j][0];
                  if (str.trim().indexOf("!") === 0 && type !== "space") {
                    break;
                  }
                  str = cache.pop()[1] + str;
                }
                if (str.trim().indexOf("!") === 0) {
                  node.important = true;
                  node.raws.important = str;
                  tokens = cache;
                }
              }
              if (token[0] !== "space" && token[0] !== "comment") {
                break;
              }
            }
            this.raw(node, "value", tokens);
            if (node.value.includes(":")) {
              this.checkMissedSemicolon(tokens);
            }
            this.current = node;
          }
        }
      };
      module.exports = ScssParser;
    }
  });

  // node_modules/postcss-scss/lib/scss-parse.js
  var require_scss_parse = __commonJS({
    "node_modules/postcss-scss/lib/scss-parse.js"(exports, module) {
      var { Input } = require_postcss();
      var ScssParser = require_scss_parser();
      module.exports = function scssParse(scss2, opts) {
        let input = new Input(scss2, opts);
        let parser = new ScssParser(input);
        parser.parse();
        return parser.root;
      };
    }
  });

  // node_modules/postcss-values-parser/lib/node.js
  var require_node2 = __commonJS({
    "node_modules/postcss-values-parser/lib/node.js"(exports, module) {
      "use strict";
      var cloneNode = function(obj, parent) {
        let cloned = new obj.constructor();
        for (let i in obj) {
          if (!obj.hasOwnProperty(i))
            continue;
          let value = obj[i], type = typeof value;
          if (i === "parent" && type === "object") {
            if (parent)
              cloned[i] = parent;
          } else if (i === "source") {
            cloned[i] = value;
          } else if (value instanceof Array) {
            cloned[i] = value.map((j) => cloneNode(j, cloned));
          } else if (i !== "before" && i !== "after" && i !== "between" && i !== "semicolon") {
            if (type === "object" && value !== null)
              value = cloneNode(value);
            cloned[i] = value;
          }
        }
        return cloned;
      };
      module.exports = class Node {
        constructor(defaults) {
          defaults = defaults || {};
          this.raws = { before: "", after: "" };
          for (let name in defaults) {
            this[name] = defaults[name];
          }
        }
        remove() {
          if (this.parent) {
            this.parent.removeChild(this);
          }
          this.parent = void 0;
          return this;
        }
        toString() {
          return [
            this.raws.before,
            String(this.value),
            this.raws.after
          ].join("");
        }
        clone(overrides) {
          overrides = overrides || {};
          let cloned = cloneNode(this);
          for (let name in overrides) {
            cloned[name] = overrides[name];
          }
          return cloned;
        }
        cloneBefore(overrides) {
          overrides = overrides || {};
          let cloned = this.clone(overrides);
          this.parent.insertBefore(this, cloned);
          return cloned;
        }
        cloneAfter(overrides) {
          overrides = overrides || {};
          let cloned = this.clone(overrides);
          this.parent.insertAfter(this, cloned);
          return cloned;
        }
        replaceWith() {
          let nodes = Array.prototype.slice.call(arguments);
          if (this.parent) {
            for (let node of nodes) {
              this.parent.insertBefore(this, node);
            }
            this.remove();
          }
          return this;
        }
        moveTo(container) {
          this.cleanRaws(this.root() === container.root());
          this.remove();
          container.append(this);
          return this;
        }
        moveBefore(node) {
          this.cleanRaws(this.root() === node.root());
          this.remove();
          node.parent.insertBefore(node, this);
          return this;
        }
        moveAfter(node) {
          this.cleanRaws(this.root() === node.root());
          this.remove();
          node.parent.insertAfter(node, this);
          return this;
        }
        next() {
          let index = this.parent.index(this);
          return this.parent.nodes[index + 1];
        }
        prev() {
          let index = this.parent.index(this);
          return this.parent.nodes[index - 1];
        }
        toJSON() {
          let fixed = {};
          for (let name in this) {
            if (!this.hasOwnProperty(name))
              continue;
            if (name === "parent")
              continue;
            let value = this[name];
            if (value instanceof Array) {
              fixed[name] = value.map((i) => {
                if (typeof i === "object" && i.toJSON) {
                  return i.toJSON();
                } else {
                  return i;
                }
              });
            } else if (typeof value === "object" && value.toJSON) {
              fixed[name] = value.toJSON();
            } else {
              fixed[name] = value;
            }
          }
          return fixed;
        }
        root() {
          let result = this;
          while (result.parent)
            result = result.parent;
          return result;
        }
        cleanRaws(keepBetween) {
          delete this.raws.before;
          delete this.raws.after;
          if (!keepBetween)
            delete this.raws.between;
        }
        positionInside(index) {
          let string = this.toString(), column = this.source.start.column, line2 = this.source.start.line;
          for (let i = 0; i < index; i++) {
            if (string[i] === "\n") {
              column = 1;
              line2 += 1;
            } else {
              column += 1;
            }
          }
          return { line: line2, column };
        }
        positionBy(opts) {
          let pos = this.source.start;
          if (Object(opts).index) {
            pos = this.positionInside(opts.index);
          } else if (Object(opts).word) {
            let index = this.toString().indexOf(opts.word);
            if (index !== -1)
              pos = this.positionInside(index);
          }
          return pos;
        }
      };
    }
  });

  // node_modules/postcss-values-parser/lib/container.js
  var require_container2 = __commonJS({
    "node_modules/postcss-values-parser/lib/container.js"(exports, module) {
      "use strict";
      var Node = require_node2();
      var Container = class extends Node {
        constructor(opts) {
          super(opts);
          if (!this.nodes) {
            this.nodes = [];
          }
        }
        push(child) {
          child.parent = this;
          this.nodes.push(child);
          return this;
        }
        each(callback) {
          if (!this.lastEach)
            this.lastEach = 0;
          if (!this.indexes)
            this.indexes = {};
          this.lastEach += 1;
          let id = this.lastEach, index, result;
          this.indexes[id] = 0;
          if (!this.nodes)
            return void 0;
          while (this.indexes[id] < this.nodes.length) {
            index = this.indexes[id];
            result = callback(this.nodes[index], index);
            if (result === false)
              break;
            this.indexes[id] += 1;
          }
          delete this.indexes[id];
          return result;
        }
        walk(callback) {
          return this.each((child, i) => {
            let result = callback(child, i);
            if (result !== false && child.walk) {
              result = child.walk(callback);
            }
            return result;
          });
        }
        walkType(type, callback) {
          if (!type || !callback) {
            throw new Error("Parameters {type} and {callback} are required.");
          }
          const isTypeCallable = typeof type === "function";
          return this.walk((node, index) => {
            if (isTypeCallable && node instanceof type || !isTypeCallable && node.type === type) {
              return callback.call(this, node, index);
            }
          });
        }
        append(node) {
          node.parent = this;
          this.nodes.push(node);
          return this;
        }
        prepend(node) {
          node.parent = this;
          this.nodes.unshift(node);
          return this;
        }
        cleanRaws(keepBetween) {
          super.cleanRaws(keepBetween);
          if (this.nodes) {
            for (let node of this.nodes)
              node.cleanRaws(keepBetween);
          }
        }
        insertAfter(oldNode, newNode) {
          let oldIndex = this.index(oldNode), index;
          this.nodes.splice(oldIndex + 1, 0, newNode);
          for (let id in this.indexes) {
            index = this.indexes[id];
            if (oldIndex <= index) {
              this.indexes[id] = index + this.nodes.length;
            }
          }
          return this;
        }
        insertBefore(oldNode, newNode) {
          let oldIndex = this.index(oldNode), index;
          this.nodes.splice(oldIndex, 0, newNode);
          for (let id in this.indexes) {
            index = this.indexes[id];
            if (oldIndex <= index) {
              this.indexes[id] = index + this.nodes.length;
            }
          }
          return this;
        }
        removeChild(child) {
          child = this.index(child);
          this.nodes[child].parent = void 0;
          this.nodes.splice(child, 1);
          let index;
          for (let id in this.indexes) {
            index = this.indexes[id];
            if (index >= child) {
              this.indexes[id] = index - 1;
            }
          }
          return this;
        }
        removeAll() {
          for (let node of this.nodes)
            node.parent = void 0;
          this.nodes = [];
          return this;
        }
        every(condition) {
          return this.nodes.every(condition);
        }
        some(condition) {
          return this.nodes.some(condition);
        }
        index(child) {
          if (typeof child === "number") {
            return child;
          } else {
            return this.nodes.indexOf(child);
          }
        }
        get first() {
          if (!this.nodes)
            return void 0;
          return this.nodes[0];
        }
        get last() {
          if (!this.nodes)
            return void 0;
          return this.nodes[this.nodes.length - 1];
        }
        toString() {
          let result = this.nodes.map(String).join("");
          if (this.value) {
            result = this.value + result;
          }
          if (this.raws.before) {
            result = this.raws.before + result;
          }
          if (this.raws.after) {
            result += this.raws.after;
          }
          return result;
        }
      };
      Container.registerWalker = (constructor) => {
        let walkerName = "walk" + constructor.name;
        if (walkerName.lastIndexOf("s") !== walkerName.length - 1) {
          walkerName += "s";
        }
        if (Container.prototype[walkerName]) {
          return;
        }
        Container.prototype[walkerName] = function(callback) {
          return this.walkType(constructor, callback);
        };
      };
      module.exports = Container;
    }
  });

  // node_modules/postcss-values-parser/lib/root.js
  var require_root2 = __commonJS({
    "node_modules/postcss-values-parser/lib/root.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      module.exports = class Root extends Container {
        constructor(opts) {
          super(opts);
          this.type = "root";
        }
      };
    }
  });

  // node_modules/postcss-values-parser/lib/value.js
  var require_value = __commonJS({
    "node_modules/postcss-values-parser/lib/value.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      module.exports = class Value extends Container {
        constructor(opts) {
          super(opts);
          this.type = "value";
          this.unbalanced = 0;
        }
      };
    }
  });

  // node_modules/postcss-values-parser/lib/atword.js
  var require_atword = __commonJS({
    "node_modules/postcss-values-parser/lib/atword.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var AtWord = class extends Container {
        constructor(opts) {
          super(opts);
          this.type = "atword";
        }
        toString() {
          let quote = this.quoted ? this.raws.quote : "";
          return [
            this.raws.before,
            "@",
            // we can't use String() here because it'll try using itself
            // as the constructor
            String.prototype.toString.call(this.value),
            this.raws.after
          ].join("");
        }
      };
      Container.registerWalker(AtWord);
      module.exports = AtWord;
    }
  });

  // node_modules/postcss-values-parser/lib/colon.js
  var require_colon = __commonJS({
    "node_modules/postcss-values-parser/lib/colon.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var Colon = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "colon";
        }
      };
      Container.registerWalker(Colon);
      module.exports = Colon;
    }
  });

  // node_modules/postcss-values-parser/lib/comma.js
  var require_comma = __commonJS({
    "node_modules/postcss-values-parser/lib/comma.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var Comma = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "comma";
        }
      };
      Container.registerWalker(Comma);
      module.exports = Comma;
    }
  });

  // node_modules/postcss-values-parser/lib/comment.js
  var require_comment2 = __commonJS({
    "node_modules/postcss-values-parser/lib/comment.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var Comment = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "comment";
          this.inline = Object(opts).inline || false;
        }
        toString() {
          return [
            this.raws.before,
            this.inline ? "//" : "/*",
            String(this.value),
            this.inline ? "" : "*/",
            this.raws.after
          ].join("");
        }
      };
      Container.registerWalker(Comment);
      module.exports = Comment;
    }
  });

  // node_modules/postcss-values-parser/lib/function.js
  var require_function = __commonJS({
    "node_modules/postcss-values-parser/lib/function.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var FunctionNode = class extends Container {
        constructor(opts) {
          super(opts);
          this.type = "func";
          this.unbalanced = -1;
        }
      };
      Container.registerWalker(FunctionNode);
      module.exports = FunctionNode;
    }
  });

  // node_modules/postcss-values-parser/lib/number.js
  var require_number = __commonJS({
    "node_modules/postcss-values-parser/lib/number.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var NumberNode = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "number";
          this.unit = Object(opts).unit || "";
        }
        toString() {
          return [
            this.raws.before,
            String(this.value),
            this.unit,
            this.raws.after
          ].join("");
        }
      };
      Container.registerWalker(NumberNode);
      module.exports = NumberNode;
    }
  });

  // node_modules/postcss-values-parser/lib/operator.js
  var require_operator = __commonJS({
    "node_modules/postcss-values-parser/lib/operator.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var Operator = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "operator";
        }
      };
      Container.registerWalker(Operator);
      module.exports = Operator;
    }
  });

  // node_modules/postcss-values-parser/lib/paren.js
  var require_paren = __commonJS({
    "node_modules/postcss-values-parser/lib/paren.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var Parenthesis = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "paren";
          this.parenType = "";
        }
      };
      Container.registerWalker(Parenthesis);
      module.exports = Parenthesis;
    }
  });

  // node_modules/postcss-values-parser/lib/string.js
  var require_string = __commonJS({
    "node_modules/postcss-values-parser/lib/string.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var StringNode = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "string";
        }
        toString() {
          let quote = this.quoted ? this.raws.quote : "";
          return [
            this.raws.before,
            quote,
            // we can't use String() here because it'll try using itself
            // as the constructor
            this.value + "",
            quote,
            this.raws.after
          ].join("");
        }
      };
      Container.registerWalker(StringNode);
      module.exports = StringNode;
    }
  });

  // node_modules/postcss-values-parser/lib/word.js
  var require_word = __commonJS({
    "node_modules/postcss-values-parser/lib/word.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var Word = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "word";
        }
      };
      Container.registerWalker(Word);
      module.exports = Word;
    }
  });

  // node_modules/postcss-values-parser/lib/unicode-range.js
  var require_unicode_range = __commonJS({
    "node_modules/postcss-values-parser/lib/unicode-range.js"(exports, module) {
      "use strict";
      var Container = require_container2();
      var Node = require_node2();
      var UnicodeRange = class extends Node {
        constructor(opts) {
          super(opts);
          this.type = "unicode-range";
        }
      };
      Container.registerWalker(UnicodeRange);
      module.exports = UnicodeRange;
    }
  });

  // node_modules/postcss-values-parser/lib/errors/TokenizeError.js
  var require_TokenizeError = __commonJS({
    "node_modules/postcss-values-parser/lib/errors/TokenizeError.js"(exports, module) {
      "use strict";
      var TokenizeError = class extends Error {
        constructor(message) {
          super(message);
          this.name = this.constructor.name;
          this.message = message || "An error ocurred while tokzenizing.";
          if (typeof Error.captureStackTrace === "function") {
            Error.captureStackTrace(this, this.constructor);
          } else {
            this.stack = new Error(message).stack;
          }
        }
      };
      module.exports = TokenizeError;
    }
  });

  // node_modules/postcss-values-parser/lib/tokenize.js
  var require_tokenize2 = __commonJS({
    "node_modules/postcss-values-parser/lib/tokenize.js"(exports, module) {
      "use strict";
      var openBracket = "{".charCodeAt(0);
      var closeBracket = "}".charCodeAt(0);
      var openParen = "(".charCodeAt(0);
      var closeParen = ")".charCodeAt(0);
      var singleQuote = "'".charCodeAt(0);
      var doubleQuote = '"'.charCodeAt(0);
      var backslash = "\\".charCodeAt(0);
      var slash = "/".charCodeAt(0);
      var period = ".".charCodeAt(0);
      var comma = ",".charCodeAt(0);
      var colon = ":".charCodeAt(0);
      var asterisk = "*".charCodeAt(0);
      var minus = "-".charCodeAt(0);
      var plus = "+".charCodeAt(0);
      var pound = "#".charCodeAt(0);
      var newline = "\n".charCodeAt(0);
      var space = " ".charCodeAt(0);
      var feed = "\f".charCodeAt(0);
      var tab = "	".charCodeAt(0);
      var cr = "\r".charCodeAt(0);
      var at2 = "@".charCodeAt(0);
      var lowerE = "e".charCodeAt(0);
      var upperE = "E".charCodeAt(0);
      var digit0 = "0".charCodeAt(0);
      var digit9 = "9".charCodeAt(0);
      var lowerU = "u".charCodeAt(0);
      var upperU = "U".charCodeAt(0);
      var atEnd = /[ \n\t\r\{\(\)'"\\;,/]/g;
      var wordEnd = /[ \n\t\r\(\)\{\}\*:;@!&'"\+\|~>,\[\]\\]|\/(?=\*)/g;
      var wordEndNum = /[ \n\t\r\(\)\{\}\*:;@!&'"\-\+\|~>,\[\]\\]|\//g;
      var alphaNum = /^[a-z0-9]/i;
      var unicodeRange = /^[a-f0-9?\-]/i;
      var TokenizeError = require_TokenizeError();
      module.exports = function tokenize(input, options2) {
        options2 = options2 || {};
        let tokens = [], css2 = input.valueOf(), length = css2.length, offset = -1, line2 = 1, pos = 0, parentCount = 0, isURLArg = null, code, next, quote, lines, last, content, escape, nextLine, nextOffset, escaped, escapePos, nextChar;
        function unclosed(what) {
          let message = `Unclosed ${what} at line: ${line2}, column: ${pos - offset}, token: ${pos}`;
          throw new TokenizeError(message);
        }
        function tokenizeError() {
          let message = `Syntax error at line: ${line2}, column: ${pos - offset}, token: ${pos}`;
          throw new TokenizeError(message);
        }
        while (pos < length) {
          code = css2.charCodeAt(pos);
          if (code === newline) {
            offset = pos;
            line2 += 1;
          }
          switch (code) {
            case newline:
            case space:
            case tab:
            case cr:
            case feed:
              next = pos;
              do {
                next += 1;
                code = css2.charCodeAt(next);
                if (code === newline) {
                  offset = next;
                  line2 += 1;
                }
              } while (code === space || code === newline || code === tab || code === cr || code === feed);
              tokens.push([
                "space",
                css2.slice(pos, next),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next - 1;
              break;
            case colon:
              next = pos + 1;
              tokens.push([
                "colon",
                css2.slice(pos, next),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next - 1;
              break;
            case comma:
              next = pos + 1;
              tokens.push([
                "comma",
                css2.slice(pos, next),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next - 1;
              break;
            case openBracket:
              tokens.push([
                "{",
                "{",
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              break;
            case closeBracket:
              tokens.push([
                "}",
                "}",
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              break;
            case openParen:
              parentCount++;
              isURLArg = !isURLArg && parentCount === 1 && tokens.length > 0 && tokens[tokens.length - 1][0] === "word" && tokens[tokens.length - 1][1] === "url";
              tokens.push([
                "(",
                "(",
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              break;
            case closeParen:
              parentCount--;
              isURLArg = isURLArg && parentCount > 0;
              tokens.push([
                ")",
                ")",
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              break;
            case singleQuote:
            case doubleQuote:
              quote = code === singleQuote ? "'" : '"';
              next = pos;
              do {
                escaped = false;
                next = css2.indexOf(quote, next + 1);
                if (next === -1) {
                  unclosed("quote", quote);
                }
                escapePos = next;
                while (css2.charCodeAt(escapePos - 1) === backslash) {
                  escapePos -= 1;
                  escaped = !escaped;
                }
              } while (escaped);
              tokens.push([
                "string",
                css2.slice(pos, next + 1),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next;
              break;
            case at2:
              atEnd.lastIndex = pos + 1;
              atEnd.test(css2);
              if (atEnd.lastIndex === 0) {
                next = css2.length - 1;
              } else {
                next = atEnd.lastIndex - 2;
              }
              tokens.push([
                "atword",
                css2.slice(pos, next + 1),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next;
              break;
            case backslash:
              next = pos;
              code = css2.charCodeAt(next + 1);
              if (escape && (code !== slash && code !== space && code !== newline && code !== tab && code !== cr && code !== feed)) {
                next += 1;
              }
              tokens.push([
                "word",
                css2.slice(pos, next + 1),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next;
              break;
            case plus:
            case minus:
            case asterisk:
              next = pos + 1;
              nextChar = css2.slice(pos + 1, next + 1);
              let prevChar = css2.slice(pos - 1, pos);
              if (code === minus && nextChar.charCodeAt(0) === minus) {
                next++;
                tokens.push([
                  "word",
                  css2.slice(pos, next),
                  line2,
                  pos - offset,
                  line2,
                  next - offset,
                  pos
                ]);
                pos = next - 1;
                break;
              }
              tokens.push([
                "operator",
                css2.slice(pos, next),
                line2,
                pos - offset,
                line2,
                next - offset,
                pos
              ]);
              pos = next - 1;
              break;
            default:
              if (code === slash && (css2.charCodeAt(pos + 1) === asterisk || options2.loose && !isURLArg && css2.charCodeAt(pos + 1) === slash)) {
                const isStandardComment = css2.charCodeAt(pos + 1) === asterisk;
                if (isStandardComment) {
                  next = css2.indexOf("*/", pos + 2) + 1;
                  if (next === 0) {
                    unclosed("comment", "*/");
                  }
                } else {
                  const newlinePos = css2.indexOf("\n", pos + 2);
                  next = newlinePos !== -1 ? newlinePos - 1 : length;
                }
                content = css2.slice(pos, next + 1);
                lines = content.split("\n");
                last = lines.length - 1;
                if (last > 0) {
                  nextLine = line2 + last;
                  nextOffset = next - lines[last].length;
                } else {
                  nextLine = line2;
                  nextOffset = offset;
                }
                tokens.push([
                  "comment",
                  content,
                  line2,
                  pos - offset,
                  nextLine,
                  next - nextOffset,
                  pos
                ]);
                offset = nextOffset;
                line2 = nextLine;
                pos = next;
              } else if (code === pound && !alphaNum.test(css2.slice(pos + 1, pos + 2))) {
                next = pos + 1;
                tokens.push([
                  "#",
                  css2.slice(pos, next),
                  line2,
                  pos - offset,
                  line2,
                  next - offset,
                  pos
                ]);
                pos = next - 1;
              } else if ((code === lowerU || code === upperU) && css2.charCodeAt(pos + 1) === plus) {
                next = pos + 2;
                do {
                  next += 1;
                  code = css2.charCodeAt(next);
                } while (next < length && unicodeRange.test(css2.slice(next, next + 1)));
                tokens.push([
                  "unicoderange",
                  css2.slice(pos, next),
                  line2,
                  pos - offset,
                  line2,
                  next - offset,
                  pos
                ]);
                pos = next - 1;
              } else if (code === slash) {
                next = pos + 1;
                tokens.push([
                  "operator",
                  css2.slice(pos, next),
                  line2,
                  pos - offset,
                  line2,
                  next - offset,
                  pos
                ]);
                pos = next - 1;
              } else {
                let regex = wordEnd;
                if (code >= digit0 && code <= digit9) {
                  regex = wordEndNum;
                }
                regex.lastIndex = pos + 1;
                regex.test(css2);
                if (regex.lastIndex === 0) {
                  next = css2.length - 1;
                } else {
                  next = regex.lastIndex - 2;
                }
                if (regex === wordEndNum || code === period) {
                  let ncode = css2.charCodeAt(next), ncode1 = css2.charCodeAt(next + 1), ncode2 = css2.charCodeAt(next + 2);
                  if ((ncode === lowerE || ncode === upperE) && (ncode1 === minus || ncode1 === plus) && (ncode2 >= digit0 && ncode2 <= digit9)) {
                    wordEndNum.lastIndex = next + 2;
                    wordEndNum.test(css2);
                    if (wordEndNum.lastIndex === 0) {
                      next = css2.length - 1;
                    } else {
                      next = wordEndNum.lastIndex - 2;
                    }
                  }
                }
                tokens.push([
                  "word",
                  css2.slice(pos, next + 1),
                  line2,
                  pos - offset,
                  line2,
                  next - offset,
                  pos
                ]);
                pos = next;
              }
              break;
          }
          pos++;
        }
        return tokens;
      };
    }
  });

  // node_modules/flatten/index.js
  var require_flatten = __commonJS({
    "node_modules/flatten/index.js"(exports, module) {
      module.exports = function flatten(list, depth) {
        depth = typeof depth == "number" ? depth : Infinity;
        if (!depth) {
          if (Array.isArray(list)) {
            return list.map(function(i) {
              return i;
            });
          }
          return list;
        }
        return _flatten(list, 1);
        function _flatten(list2, d) {
          return list2.reduce(function(acc, item) {
            if (Array.isArray(item) && d < depth) {
              return acc.concat(_flatten(item, d + 1));
            } else {
              return acc.concat(item);
            }
          }, []);
        }
      };
    }
  });

  // node_modules/indexes-of/index.js
  var require_indexes_of = __commonJS({
    "node_modules/indexes-of/index.js"(exports, module) {
      module.exports = function(ary, item) {
        var i = -1, indexes = [];
        while ((i = ary.indexOf(item, i + 1)) !== -1)
          indexes.push(i);
        return indexes;
      };
    }
  });

  // node_modules/uniq/uniq.js
  var require_uniq = __commonJS({
    "node_modules/uniq/uniq.js"(exports, module) {
      "use strict";
      function unique_pred(list, compare) {
        var ptr = 1, len = list.length, a = list[0], b = list[0];
        for (var i = 1; i < len; ++i) {
          b = a;
          a = list[i];
          if (compare(a, b)) {
            if (i === ptr) {
              ptr++;
              continue;
            }
            list[ptr++] = a;
          }
        }
        list.length = ptr;
        return list;
      }
      function unique_eq(list) {
        var ptr = 1, len = list.length, a = list[0], b = list[0];
        for (var i = 1; i < len; ++i, b = a) {
          b = a;
          a = list[i];
          if (a !== b) {
            if (i === ptr) {
              ptr++;
              continue;
            }
            list[ptr++] = a;
          }
        }
        list.length = ptr;
        return list;
      }
      function unique(list, compare, sorted) {
        if (list.length === 0) {
          return list;
        }
        if (compare) {
          if (!sorted) {
            list.sort(compare);
          }
          return unique_pred(list, compare);
        }
        if (!sorted) {
          list.sort();
        }
        return unique_eq(list);
      }
      module.exports = unique;
    }
  });

  // node_modules/postcss-values-parser/lib/errors/ParserError.js
  var require_ParserError = __commonJS({
    "node_modules/postcss-values-parser/lib/errors/ParserError.js"(exports, module) {
      "use strict";
      var ParserError = class extends Error {
        constructor(message) {
          super(message);
          this.name = this.constructor.name;
          this.message = message || "An error ocurred while parsing.";
          if (typeof Error.captureStackTrace === "function") {
            Error.captureStackTrace(this, this.constructor);
          } else {
            this.stack = new Error(message).stack;
          }
        }
      };
      module.exports = ParserError;
    }
  });

  // node_modules/postcss-values-parser/lib/parser.js
  var require_parser2 = __commonJS({
    "node_modules/postcss-values-parser/lib/parser.js"(exports, module) {
      "use strict";
      var Root = require_root2();
      var Value = require_value();
      var AtWord = require_atword();
      var Colon = require_colon();
      var Comma = require_comma();
      var Comment = require_comment2();
      var Func = require_function();
      var Numbr = require_number();
      var Operator = require_operator();
      var Paren = require_paren();
      var Str = require_string();
      var Word = require_word();
      var UnicodeRange = require_unicode_range();
      var tokenize = require_tokenize2();
      var flatten = require_flatten();
      var indexesOf = require_indexes_of();
      var uniq = require_uniq();
      var ParserError = require_ParserError();
      function sortAscending(list) {
        return list.sort((a, b) => a - b);
      }
      module.exports = class Parser {
        constructor(input, options2) {
          const defaults = { loose: false };
          this.cache = [];
          this.input = input;
          this.options = Object.assign({}, defaults, options2);
          this.position = 0;
          this.unbalanced = 0;
          this.root = new Root();
          let value = new Value();
          this.root.append(value);
          this.current = value;
          this.tokens = tokenize(input, this.options);
        }
        parse() {
          return this.loop();
        }
        colon() {
          let token = this.currToken;
          this.newNode(new Colon({
            value: token[1],
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6]
          }));
          this.position++;
        }
        comma() {
          let token = this.currToken;
          this.newNode(new Comma({
            value: token[1],
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6]
          }));
          this.position++;
        }
        comment() {
          let inline = false, value = this.currToken[1].replace(/\/\*|\*\//g, ""), node;
          if (this.options.loose && value.startsWith("//")) {
            value = value.substring(2);
            inline = true;
          }
          node = new Comment({
            value,
            inline,
            source: {
              start: {
                line: this.currToken[2],
                column: this.currToken[3]
              },
              end: {
                line: this.currToken[4],
                column: this.currToken[5]
              }
            },
            sourceIndex: this.currToken[6]
          });
          this.newNode(node);
          this.position++;
        }
        error(message, token) {
          throw new ParserError(message + ` at line: ${token[2]}, column ${token[3]}`);
        }
        loop() {
          while (this.position < this.tokens.length) {
            this.parseTokens();
          }
          if (!this.current.last && this.spaces) {
            this.current.raws.before += this.spaces;
          } else if (this.spaces) {
            this.current.last.raws.after += this.spaces;
          }
          this.spaces = "";
          return this.root;
        }
        operator() {
          let char = this.currToken[1], node;
          if (char === "+" || char === "-") {
            if (!this.options.loose) {
              if (this.position > 0) {
                if (this.current.type === "func" && this.current.value === "calc") {
                  if (this.prevToken[0] !== "space" && this.prevToken[0] !== "(") {
                    this.error("Syntax Error", this.currToken);
                  } else if (this.nextToken[0] !== "space" && this.nextToken[0] !== "word") {
                    this.error("Syntax Error", this.currToken);
                  } else if (this.nextToken[0] === "word" && this.current.last.type !== "operator" && this.current.last.value !== "(") {
                    this.error("Syntax Error", this.currToken);
                  }
                } else if (this.nextToken[0] === "space" || this.nextToken[0] === "operator" || this.prevToken[0] === "operator") {
                  this.error("Syntax Error", this.currToken);
                }
              }
            }
            if (!this.options.loose) {
              if (this.nextToken[0] === "word") {
                return this.word();
              }
            } else {
              if ((!this.current.nodes.length || this.current.last && this.current.last.type === "operator") && this.nextToken[0] === "word") {
                return this.word();
              }
            }
          }
          node = new Operator({
            value: this.currToken[1],
            source: {
              start: {
                line: this.currToken[2],
                column: this.currToken[3]
              },
              end: {
                line: this.currToken[2],
                column: this.currToken[3]
              }
            },
            sourceIndex: this.currToken[4]
          });
          this.position++;
          return this.newNode(node);
        }
        parseTokens() {
          switch (this.currToken[0]) {
            case "space":
              this.space();
              break;
            case "colon":
              this.colon();
              break;
            case "comma":
              this.comma();
              break;
            case "comment":
              this.comment();
              break;
            case "(":
              this.parenOpen();
              break;
            case ")":
              this.parenClose();
              break;
            case "atword":
            case "word":
              this.word();
              break;
            case "operator":
              this.operator();
              break;
            case "string":
              this.string();
              break;
            case "unicoderange":
              this.unicodeRange();
              break;
            default:
              this.word();
              break;
          }
        }
        parenOpen() {
          let unbalanced = 1, pos = this.position + 1, token = this.currToken, last;
          while (pos < this.tokens.length && unbalanced) {
            let tkn = this.tokens[pos];
            if (tkn[0] === "(") {
              unbalanced++;
            }
            if (tkn[0] === ")") {
              unbalanced--;
            }
            pos++;
          }
          if (unbalanced) {
            this.error("Expected closing parenthesis", token);
          }
          last = this.current.last;
          if (last && last.type === "func" && last.unbalanced < 0) {
            last.unbalanced = 0;
            this.current = last;
          }
          this.current.unbalanced++;
          this.newNode(new Paren({
            value: token[1],
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6]
          }));
          this.position++;
          if (this.current.type === "func" && this.current.unbalanced && this.current.value === "url" && this.currToken[0] !== "string" && this.currToken[0] !== ")" && !this.options.loose) {
            let nextToken = this.nextToken, value = this.currToken[1], start = {
              line: this.currToken[2],
              column: this.currToken[3]
            };
            while (nextToken && nextToken[0] !== ")" && this.current.unbalanced) {
              this.position++;
              value += this.currToken[1];
              nextToken = this.nextToken;
            }
            if (this.position !== this.tokens.length - 1) {
              this.position++;
              this.newNode(new Word({
                value,
                source: {
                  start,
                  end: {
                    line: this.currToken[4],
                    column: this.currToken[5]
                  }
                },
                sourceIndex: this.currToken[6]
              }));
            }
          }
        }
        parenClose() {
          let token = this.currToken;
          this.newNode(new Paren({
            value: token[1],
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6]
          }));
          this.position++;
          if (this.position >= this.tokens.length - 1 && !this.current.unbalanced) {
            return;
          }
          this.current.unbalanced--;
          if (this.current.unbalanced < 0) {
            this.error("Expected opening parenthesis", token);
          }
          if (!this.current.unbalanced && this.cache.length) {
            this.current = this.cache.pop();
          }
        }
        space() {
          let token = this.currToken;
          if (this.position === this.tokens.length - 1 || this.nextToken[0] === "," || this.nextToken[0] === ")") {
            this.current.last.raws.after += token[1];
            this.position++;
          } else {
            this.spaces = token[1];
            this.position++;
          }
        }
        unicodeRange() {
          let token = this.currToken;
          this.newNode(new UnicodeRange({
            value: token[1],
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6]
          }));
          this.position++;
        }
        splitWord() {
          let nextToken = this.nextToken, word = this.currToken[1], rNumber = /^[\+\-]?((\d+(\.\d*)?)|(\.\d+))([eE][\+\-]?\d+)?/, rNoFollow = /^(?!\#([a-z0-9]+))[\#\{\}]/gi, hasAt, indices;
          if (!rNoFollow.test(word)) {
            while (nextToken && nextToken[0] === "word") {
              this.position++;
              let current = this.currToken[1];
              word += current;
              nextToken = this.nextToken;
            }
          }
          hasAt = indexesOf(word, "@");
          indices = sortAscending(uniq(flatten([[0], hasAt])));
          indices.forEach((ind, i) => {
            let index = indices[i + 1] || word.length, value = word.slice(ind, index), node;
            if (~hasAt.indexOf(ind)) {
              node = new AtWord({
                value: value.slice(1),
                source: {
                  start: {
                    line: this.currToken[2],
                    column: this.currToken[3] + ind
                  },
                  end: {
                    line: this.currToken[4],
                    column: this.currToken[3] + (index - 1)
                  }
                },
                sourceIndex: this.currToken[6] + indices[i]
              });
            } else if (rNumber.test(this.currToken[1])) {
              let unit = value.replace(rNumber, "");
              node = new Numbr({
                value: value.replace(unit, ""),
                source: {
                  start: {
                    line: this.currToken[2],
                    column: this.currToken[3] + ind
                  },
                  end: {
                    line: this.currToken[4],
                    column: this.currToken[3] + (index - 1)
                  }
                },
                sourceIndex: this.currToken[6] + indices[i],
                unit
              });
            } else {
              node = new (nextToken && nextToken[0] === "(" ? Func : Word)({
                value,
                source: {
                  start: {
                    line: this.currToken[2],
                    column: this.currToken[3] + ind
                  },
                  end: {
                    line: this.currToken[4],
                    column: this.currToken[3] + (index - 1)
                  }
                },
                sourceIndex: this.currToken[6] + indices[i]
              });
              if (node.type === "word") {
                node.isHex = /^#(.+)/.test(value);
                node.isColor = /^#([0-9a-f]{3}|[0-9a-f]{4}|[0-9a-f]{6}|[0-9a-f]{8})$/i.test(value);
              } else {
                this.cache.push(this.current);
              }
            }
            this.newNode(node);
          });
          this.position++;
        }
        string() {
          let token = this.currToken, value = this.currToken[1], rQuote = /^(\"|\')/, quoted = rQuote.test(value), quote = "", node;
          if (quoted) {
            quote = value.match(rQuote)[0];
            value = value.slice(1, value.length - 1);
          }
          node = new Str({
            value,
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6],
            quoted
          });
          node.raws.quote = quote;
          this.newNode(node);
          this.position++;
        }
        word() {
          return this.splitWord();
        }
        newNode(node) {
          if (this.spaces) {
            node.raws.before += this.spaces;
            this.spaces = "";
          }
          return this.current.append(node);
        }
        get currToken() {
          return this.tokens[this.position];
        }
        get nextToken() {
          return this.tokens[this.position + 1];
        }
        get prevToken() {
          return this.tokens[this.position - 1];
        }
      };
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/node.js
  var require_node3 = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/node.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function(obj) {
        return typeof obj;
      } : function(obj) {
        return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj;
      };
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      var cloneNode = function cloneNode2(obj, parent) {
        if ((typeof obj === "undefined" ? "undefined" : _typeof(obj)) !== "object") {
          return obj;
        }
        var cloned = new obj.constructor();
        for (var i in obj) {
          if (!obj.hasOwnProperty(i)) {
            continue;
          }
          var value = obj[i];
          var type = typeof value === "undefined" ? "undefined" : _typeof(value);
          if (i === "parent" && type === "object") {
            if (parent) {
              cloned[i] = parent;
            }
          } else if (value instanceof Array) {
            cloned[i] = value.map(function(j) {
              return cloneNode2(j, cloned);
            });
          } else {
            cloned[i] = cloneNode2(value, cloned);
          }
        }
        return cloned;
      };
      var _class = function() {
        function _class2() {
          var opts = arguments.length > 0 && arguments[0] !== void 0 ? arguments[0] : {};
          _classCallCheck(this, _class2);
          for (var key in opts) {
            this[key] = opts[key];
          }
          var _opts$spaces = opts.spaces;
          _opts$spaces = _opts$spaces === void 0 ? {} : _opts$spaces;
          var _opts$spaces$before = _opts$spaces.before, before = _opts$spaces$before === void 0 ? "" : _opts$spaces$before, _opts$spaces$after = _opts$spaces.after, after = _opts$spaces$after === void 0 ? "" : _opts$spaces$after;
          this.spaces = { before, after };
        }
        _class2.prototype.remove = function remove() {
          if (this.parent) {
            this.parent.removeChild(this);
          }
          this.parent = void 0;
          return this;
        };
        _class2.prototype.replaceWith = function replaceWith() {
          if (this.parent) {
            for (var index in arguments) {
              this.parent.insertBefore(this, arguments[index]);
            }
            this.remove();
          }
          return this;
        };
        _class2.prototype.next = function next() {
          return this.parent.at(this.parent.index(this) + 1);
        };
        _class2.prototype.prev = function prev() {
          return this.parent.at(this.parent.index(this) - 1);
        };
        _class2.prototype.clone = function clone() {
          var overrides = arguments.length > 0 && arguments[0] !== void 0 ? arguments[0] : {};
          var cloned = cloneNode(this);
          for (var name in overrides) {
            cloned[name] = overrides[name];
          }
          return cloned;
        };
        _class2.prototype.toString = function toString() {
          return [this.spaces.before, String(this.value), this.spaces.after].join("");
        };
        return _class2;
      }();
      exports.default = _class;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/types.js
  var require_types = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/types.js"(exports) {
      "use strict";
      exports.__esModule = true;
      var TAG = exports.TAG = "tag";
      var STRING = exports.STRING = "string";
      var SELECTOR = exports.SELECTOR = "selector";
      var ROOT = exports.ROOT = "root";
      var PSEUDO = exports.PSEUDO = "pseudo";
      var NESTING = exports.NESTING = "nesting";
      var ID = exports.ID = "id";
      var COMMENT = exports.COMMENT = "comment";
      var COMBINATOR = exports.COMBINATOR = "combinator";
      var CLASS = exports.CLASS = "class";
      var ATTRIBUTE = exports.ATTRIBUTE = "attribute";
      var UNIVERSAL = exports.UNIVERSAL = "universal";
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/container.js
  var require_container3 = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/container.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _createClass = function() {
        function defineProperties(target, props) {
          for (var i = 0; i < props.length; i++) {
            var descriptor = props[i];
            descriptor.enumerable = descriptor.enumerable || false;
            descriptor.configurable = true;
            if ("value" in descriptor)
              descriptor.writable = true;
            Object.defineProperty(target, descriptor.key, descriptor);
          }
        }
        return function(Constructor, protoProps, staticProps) {
          if (protoProps)
            defineProperties(Constructor.prototype, protoProps);
          if (staticProps)
            defineProperties(Constructor, staticProps);
          return Constructor;
        };
      }();
      var _node = require_node3();
      var _node2 = _interopRequireDefault(_node);
      var _types = require_types();
      var types = _interopRequireWildcard(_types);
      function _interopRequireWildcard(obj) {
        if (obj && obj.__esModule) {
          return obj;
        } else {
          var newObj = {};
          if (obj != null) {
            for (var key in obj) {
              if (Object.prototype.hasOwnProperty.call(obj, key))
                newObj[key] = obj[key];
            }
          }
          newObj.default = obj;
          return newObj;
        }
      }
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Container = function(_Node) {
        _inherits(Container2, _Node);
        function Container2(opts) {
          _classCallCheck(this, Container2);
          var _this = _possibleConstructorReturn(this, _Node.call(this, opts));
          if (!_this.nodes) {
            _this.nodes = [];
          }
          return _this;
        }
        Container2.prototype.append = function append(selector) {
          selector.parent = this;
          this.nodes.push(selector);
          return this;
        };
        Container2.prototype.prepend = function prepend(selector) {
          selector.parent = this;
          this.nodes.unshift(selector);
          return this;
        };
        Container2.prototype.at = function at2(index) {
          return this.nodes[index];
        };
        Container2.prototype.index = function index(child) {
          if (typeof child === "number") {
            return child;
          }
          return this.nodes.indexOf(child);
        };
        Container2.prototype.removeChild = function removeChild(child) {
          child = this.index(child);
          this.at(child).parent = void 0;
          this.nodes.splice(child, 1);
          var index = void 0;
          for (var id in this.indexes) {
            index = this.indexes[id];
            if (index >= child) {
              this.indexes[id] = index - 1;
            }
          }
          return this;
        };
        Container2.prototype.removeAll = function removeAll() {
          for (var _iterator = this.nodes, _isArray = Array.isArray(_iterator), _i = 0, _iterator = _isArray ? _iterator : _iterator[Symbol.iterator](); ; ) {
            var _ref;
            if (_isArray) {
              if (_i >= _iterator.length)
                break;
              _ref = _iterator[_i++];
            } else {
              _i = _iterator.next();
              if (_i.done)
                break;
              _ref = _i.value;
            }
            var node = _ref;
            node.parent = void 0;
          }
          this.nodes = [];
          return this;
        };
        Container2.prototype.empty = function empty() {
          return this.removeAll();
        };
        Container2.prototype.insertAfter = function insertAfter(oldNode, newNode) {
          var oldIndex = this.index(oldNode);
          this.nodes.splice(oldIndex + 1, 0, newNode);
          var index = void 0;
          for (var id in this.indexes) {
            index = this.indexes[id];
            if (oldIndex <= index) {
              this.indexes[id] = index + this.nodes.length;
            }
          }
          return this;
        };
        Container2.prototype.insertBefore = function insertBefore(oldNode, newNode) {
          var oldIndex = this.index(oldNode);
          this.nodes.splice(oldIndex, 0, newNode);
          var index = void 0;
          for (var id in this.indexes) {
            index = this.indexes[id];
            if (oldIndex <= index) {
              this.indexes[id] = index + this.nodes.length;
            }
          }
          return this;
        };
        Container2.prototype.each = function each(callback) {
          if (!this.lastEach) {
            this.lastEach = 0;
          }
          if (!this.indexes) {
            this.indexes = {};
          }
          this.lastEach++;
          var id = this.lastEach;
          this.indexes[id] = 0;
          if (!this.length) {
            return void 0;
          }
          var index = void 0, result = void 0;
          while (this.indexes[id] < this.length) {
            index = this.indexes[id];
            result = callback(this.at(index), index);
            if (result === false) {
              break;
            }
            this.indexes[id] += 1;
          }
          delete this.indexes[id];
          if (result === false) {
            return false;
          }
        };
        Container2.prototype.walk = function walk(callback) {
          return this.each(function(node, i) {
            var result = callback(node, i);
            if (result !== false && node.length) {
              result = node.walk(callback);
            }
            if (result === false) {
              return false;
            }
          });
        };
        Container2.prototype.walkAttributes = function walkAttributes(callback) {
          var _this2 = this;
          return this.walk(function(selector) {
            if (selector.type === types.ATTRIBUTE) {
              return callback.call(_this2, selector);
            }
          });
        };
        Container2.prototype.walkClasses = function walkClasses(callback) {
          var _this3 = this;
          return this.walk(function(selector) {
            if (selector.type === types.CLASS) {
              return callback.call(_this3, selector);
            }
          });
        };
        Container2.prototype.walkCombinators = function walkCombinators(callback) {
          var _this4 = this;
          return this.walk(function(selector) {
            if (selector.type === types.COMBINATOR) {
              return callback.call(_this4, selector);
            }
          });
        };
        Container2.prototype.walkComments = function walkComments(callback) {
          var _this5 = this;
          return this.walk(function(selector) {
            if (selector.type === types.COMMENT) {
              return callback.call(_this5, selector);
            }
          });
        };
        Container2.prototype.walkIds = function walkIds(callback) {
          var _this6 = this;
          return this.walk(function(selector) {
            if (selector.type === types.ID) {
              return callback.call(_this6, selector);
            }
          });
        };
        Container2.prototype.walkNesting = function walkNesting(callback) {
          var _this7 = this;
          return this.walk(function(selector) {
            if (selector.type === types.NESTING) {
              return callback.call(_this7, selector);
            }
          });
        };
        Container2.prototype.walkPseudos = function walkPseudos(callback) {
          var _this8 = this;
          return this.walk(function(selector) {
            if (selector.type === types.PSEUDO) {
              return callback.call(_this8, selector);
            }
          });
        };
        Container2.prototype.walkTags = function walkTags(callback) {
          var _this9 = this;
          return this.walk(function(selector) {
            if (selector.type === types.TAG) {
              return callback.call(_this9, selector);
            }
          });
        };
        Container2.prototype.walkUniversals = function walkUniversals(callback) {
          var _this10 = this;
          return this.walk(function(selector) {
            if (selector.type === types.UNIVERSAL) {
              return callback.call(_this10, selector);
            }
          });
        };
        Container2.prototype.split = function split(callback) {
          var _this11 = this;
          var current = [];
          return this.reduce(function(memo, node, index) {
            var split2 = callback.call(_this11, node);
            current.push(node);
            if (split2) {
              memo.push(current);
              current = [];
            } else if (index === _this11.length - 1) {
              memo.push(current);
            }
            return memo;
          }, []);
        };
        Container2.prototype.map = function map(callback) {
          return this.nodes.map(callback);
        };
        Container2.prototype.reduce = function reduce(callback, memo) {
          return this.nodes.reduce(callback, memo);
        };
        Container2.prototype.every = function every(callback) {
          return this.nodes.every(callback);
        };
        Container2.prototype.some = function some(callback) {
          return this.nodes.some(callback);
        };
        Container2.prototype.filter = function filter(callback) {
          return this.nodes.filter(callback);
        };
        Container2.prototype.sort = function sort(callback) {
          return this.nodes.sort(callback);
        };
        Container2.prototype.toString = function toString() {
          return this.map(String).join("");
        };
        _createClass(Container2, [{
          key: "first",
          get: function get() {
            return this.at(0);
          }
        }, {
          key: "last",
          get: function get() {
            return this.at(this.length - 1);
          }
        }, {
          key: "length",
          get: function get() {
            return this.nodes.length;
          }
        }]);
        return Container2;
      }(_node2.default);
      exports.default = Container;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/root.js
  var require_root3 = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/root.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _container = require_container3();
      var _container2 = _interopRequireDefault(_container);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Root = function(_Container) {
        _inherits(Root2, _Container);
        function Root2(opts) {
          _classCallCheck(this, Root2);
          var _this = _possibleConstructorReturn(this, _Container.call(this, opts));
          _this.type = _types.ROOT;
          return _this;
        }
        Root2.prototype.toString = function toString() {
          var str = this.reduce(function(memo, selector) {
            var sel = String(selector);
            return sel ? memo + sel + "," : "";
          }, "").slice(0, -1);
          return this.trailingComma ? str + "," : str;
        };
        return Root2;
      }(_container2.default);
      exports.default = Root;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/selector.js
  var require_selector = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/selector.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _container = require_container3();
      var _container2 = _interopRequireDefault(_container);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Selector = function(_Container) {
        _inherits(Selector2, _Container);
        function Selector2(opts) {
          _classCallCheck(this, Selector2);
          var _this = _possibleConstructorReturn(this, _Container.call(this, opts));
          _this.type = _types.SELECTOR;
          return _this;
        }
        return Selector2;
      }(_container2.default);
      exports.default = Selector;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/namespace.js
  var require_namespace = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/namespace.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _createClass = function() {
        function defineProperties(target, props) {
          for (var i = 0; i < props.length; i++) {
            var descriptor = props[i];
            descriptor.enumerable = descriptor.enumerable || false;
            descriptor.configurable = true;
            if ("value" in descriptor)
              descriptor.writable = true;
            Object.defineProperty(target, descriptor.key, descriptor);
          }
        }
        return function(Constructor, protoProps, staticProps) {
          if (protoProps)
            defineProperties(Constructor.prototype, protoProps);
          if (staticProps)
            defineProperties(Constructor, staticProps);
          return Constructor;
        };
      }();
      var _node = require_node3();
      var _node2 = _interopRequireDefault(_node);
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Namespace = function(_Node) {
        _inherits(Namespace2, _Node);
        function Namespace2() {
          _classCallCheck(this, Namespace2);
          return _possibleConstructorReturn(this, _Node.apply(this, arguments));
        }
        Namespace2.prototype.toString = function toString() {
          return [this.spaces.before, this.ns, String(this.value), this.spaces.after].join("");
        };
        _createClass(Namespace2, [{
          key: "ns",
          get: function get() {
            var n = this.namespace;
            return n ? (typeof n === "string" ? n : "") + "|" : "";
          }
        }]);
        return Namespace2;
      }(_node2.default);
      exports.default = Namespace;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/className.js
  var require_className = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/className.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _namespace = require_namespace();
      var _namespace2 = _interopRequireDefault(_namespace);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var ClassName = function(_Namespace) {
        _inherits(ClassName2, _Namespace);
        function ClassName2(opts) {
          _classCallCheck(this, ClassName2);
          var _this = _possibleConstructorReturn(this, _Namespace.call(this, opts));
          _this.type = _types.CLASS;
          return _this;
        }
        ClassName2.prototype.toString = function toString() {
          return [this.spaces.before, this.ns, String("." + this.value), this.spaces.after].join("");
        };
        return ClassName2;
      }(_namespace2.default);
      exports.default = ClassName;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/comment.js
  var require_comment3 = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/comment.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _node = require_node3();
      var _node2 = _interopRequireDefault(_node);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Comment = function(_Node) {
        _inherits(Comment2, _Node);
        function Comment2(opts) {
          _classCallCheck(this, Comment2);
          var _this = _possibleConstructorReturn(this, _Node.call(this, opts));
          _this.type = _types.COMMENT;
          return _this;
        }
        return Comment2;
      }(_node2.default);
      exports.default = Comment;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/id.js
  var require_id = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/id.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _namespace = require_namespace();
      var _namespace2 = _interopRequireDefault(_namespace);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var ID = function(_Namespace) {
        _inherits(ID2, _Namespace);
        function ID2(opts) {
          _classCallCheck(this, ID2);
          var _this = _possibleConstructorReturn(this, _Namespace.call(this, opts));
          _this.type = _types.ID;
          return _this;
        }
        ID2.prototype.toString = function toString() {
          return [this.spaces.before, this.ns, String("#" + this.value), this.spaces.after].join("");
        };
        return ID2;
      }(_namespace2.default);
      exports.default = ID;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/tag.js
  var require_tag = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/tag.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _namespace = require_namespace();
      var _namespace2 = _interopRequireDefault(_namespace);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Tag = function(_Namespace) {
        _inherits(Tag2, _Namespace);
        function Tag2(opts) {
          _classCallCheck(this, Tag2);
          var _this = _possibleConstructorReturn(this, _Namespace.call(this, opts));
          _this.type = _types.TAG;
          return _this;
        }
        return Tag2;
      }(_namespace2.default);
      exports.default = Tag;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/string.js
  var require_string2 = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/string.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _node = require_node3();
      var _node2 = _interopRequireDefault(_node);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var String2 = function(_Node) {
        _inherits(String3, _Node);
        function String3(opts) {
          _classCallCheck(this, String3);
          var _this = _possibleConstructorReturn(this, _Node.call(this, opts));
          _this.type = _types.STRING;
          return _this;
        }
        return String3;
      }(_node2.default);
      exports.default = String2;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/pseudo.js
  var require_pseudo = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/pseudo.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _container = require_container3();
      var _container2 = _interopRequireDefault(_container);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Pseudo = function(_Container) {
        _inherits(Pseudo2, _Container);
        function Pseudo2(opts) {
          _classCallCheck(this, Pseudo2);
          var _this = _possibleConstructorReturn(this, _Container.call(this, opts));
          _this.type = _types.PSEUDO;
          return _this;
        }
        Pseudo2.prototype.toString = function toString() {
          var params = this.length ? "(" + this.map(String).join(",") + ")" : "";
          return [this.spaces.before, String(this.value), params, this.spaces.after].join("");
        };
        return Pseudo2;
      }(_container2.default);
      exports.default = Pseudo;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/attribute.js
  var require_attribute = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/attribute.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _namespace = require_namespace();
      var _namespace2 = _interopRequireDefault(_namespace);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Attribute = function(_Namespace) {
        _inherits(Attribute2, _Namespace);
        function Attribute2(opts) {
          _classCallCheck(this, Attribute2);
          var _this = _possibleConstructorReturn(this, _Namespace.call(this, opts));
          _this.type = _types.ATTRIBUTE;
          _this.raws = {};
          return _this;
        }
        Attribute2.prototype.toString = function toString() {
          var selector = [this.spaces.before, "[", this.ns, this.attribute];
          if (this.operator) {
            selector.push(this.operator);
          }
          if (this.value) {
            selector.push(this.value);
          }
          if (this.raws.insensitive) {
            selector.push(this.raws.insensitive);
          } else if (this.insensitive) {
            selector.push(" i");
          }
          selector.push("]");
          return selector.concat(this.spaces.after).join("");
        };
        return Attribute2;
      }(_namespace2.default);
      exports.default = Attribute;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/universal.js
  var require_universal = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/universal.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _namespace = require_namespace();
      var _namespace2 = _interopRequireDefault(_namespace);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Universal = function(_Namespace) {
        _inherits(Universal2, _Namespace);
        function Universal2(opts) {
          _classCallCheck(this, Universal2);
          var _this = _possibleConstructorReturn(this, _Namespace.call(this, opts));
          _this.type = _types.UNIVERSAL;
          _this.value = "*";
          return _this;
        }
        return Universal2;
      }(_namespace2.default);
      exports.default = Universal;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/combinator.js
  var require_combinator = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/combinator.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _node = require_node3();
      var _node2 = _interopRequireDefault(_node);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Combinator = function(_Node) {
        _inherits(Combinator2, _Node);
        function Combinator2(opts) {
          _classCallCheck(this, Combinator2);
          var _this = _possibleConstructorReturn(this, _Node.call(this, opts));
          _this.type = _types.COMBINATOR;
          return _this;
        }
        return Combinator2;
      }(_node2.default);
      exports.default = Combinator;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/selectors/nesting.js
  var require_nesting = __commonJS({
    "node_modules/postcss-selector-parser/dist/selectors/nesting.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _node = require_node3();
      var _node2 = _interopRequireDefault(_node);
      var _types = require_types();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      function _possibleConstructorReturn(self, call) {
        if (!self) {
          throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
        }
        return call && (typeof call === "object" || typeof call === "function") ? call : self;
      }
      function _inherits(subClass, superClass) {
        if (typeof superClass !== "function" && superClass !== null) {
          throw new TypeError("Super expression must either be null or a function, not " + typeof superClass);
        }
        subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } });
        if (superClass)
          Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass;
      }
      var Nesting = function(_Node) {
        _inherits(Nesting2, _Node);
        function Nesting2(opts) {
          _classCallCheck(this, Nesting2);
          var _this = _possibleConstructorReturn(this, _Node.call(this, opts));
          _this.type = _types.NESTING;
          _this.value = "&";
          return _this;
        }
        return Nesting2;
      }(_node2.default);
      exports.default = Nesting;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/sortAscending.js
  var require_sortAscending = __commonJS({
    "node_modules/postcss-selector-parser/dist/sortAscending.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      exports.default = sortAscending;
      function sortAscending(list) {
        return list.sort(function(a, b) {
          return a - b;
        });
      }
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/tokenize.js
  var require_tokenize3 = __commonJS({
    "node_modules/postcss-selector-parser/dist/tokenize.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      exports.default = tokenize;
      var singleQuote = 39;
      var doubleQuote = 34;
      var backslash = 92;
      var slash = 47;
      var newline = 10;
      var space = 32;
      var feed = 12;
      var tab = 9;
      var cr = 13;
      var plus = 43;
      var gt = 62;
      var tilde = 126;
      var pipe = 124;
      var comma = 44;
      var openBracket = 40;
      var closeBracket = 41;
      var openSq = 91;
      var closeSq = 93;
      var semicolon = 59;
      var asterisk = 42;
      var colon = 58;
      var ampersand = 38;
      var at2 = 64;
      var atEnd = /[ \n\t\r\{\(\)'"\\;/]/g;
      var wordEnd = /[ \n\t\r\(\)\*:;@!&'"\+\|~>,\[\]\\]|\/(?=\*)/g;
      function tokenize(input) {
        var tokens = [];
        var css2 = input.css.valueOf();
        var code = void 0, next = void 0, quote = void 0, lines = void 0, last = void 0, content = void 0, escape = void 0, nextLine = void 0, nextOffset = void 0, escaped = void 0, escapePos = void 0;
        var length = css2.length;
        var offset = -1;
        var line2 = 1;
        var pos = 0;
        var unclosed = function unclosed2(what, end) {
          if (input.safe) {
            css2 += end;
            next = css2.length - 1;
          } else {
            throw input.error("Unclosed " + what, line2, pos - offset, pos);
          }
        };
        while (pos < length) {
          code = css2.charCodeAt(pos);
          if (code === newline) {
            offset = pos;
            line2 += 1;
          }
          switch (code) {
            case newline:
            case space:
            case tab:
            case cr:
            case feed:
              next = pos;
              do {
                next += 1;
                code = css2.charCodeAt(next);
                if (code === newline) {
                  offset = next;
                  line2 += 1;
                }
              } while (code === space || code === newline || code === tab || code === cr || code === feed);
              tokens.push(["space", css2.slice(pos, next), line2, pos - offset, pos]);
              pos = next - 1;
              break;
            case plus:
            case gt:
            case tilde:
            case pipe:
              next = pos;
              do {
                next += 1;
                code = css2.charCodeAt(next);
              } while (code === plus || code === gt || code === tilde || code === pipe);
              tokens.push(["combinator", css2.slice(pos, next), line2, pos - offset, pos]);
              pos = next - 1;
              break;
            case asterisk:
              tokens.push(["*", "*", line2, pos - offset, pos]);
              break;
            case ampersand:
              tokens.push(["&", "&", line2, pos - offset, pos]);
              break;
            case comma:
              tokens.push([",", ",", line2, pos - offset, pos]);
              break;
            case openSq:
              tokens.push(["[", "[", line2, pos - offset, pos]);
              break;
            case closeSq:
              tokens.push(["]", "]", line2, pos - offset, pos]);
              break;
            case colon:
              tokens.push([":", ":", line2, pos - offset, pos]);
              break;
            case semicolon:
              tokens.push([";", ";", line2, pos - offset, pos]);
              break;
            case openBracket:
              tokens.push(["(", "(", line2, pos - offset, pos]);
              break;
            case closeBracket:
              tokens.push([")", ")", line2, pos - offset, pos]);
              break;
            case singleQuote:
            case doubleQuote:
              quote = code === singleQuote ? "'" : '"';
              next = pos;
              do {
                escaped = false;
                next = css2.indexOf(quote, next + 1);
                if (next === -1) {
                  unclosed("quote", quote);
                }
                escapePos = next;
                while (css2.charCodeAt(escapePos - 1) === backslash) {
                  escapePos -= 1;
                  escaped = !escaped;
                }
              } while (escaped);
              tokens.push(["string", css2.slice(pos, next + 1), line2, pos - offset, line2, next - offset, pos]);
              pos = next;
              break;
            case at2:
              atEnd.lastIndex = pos + 1;
              atEnd.test(css2);
              if (atEnd.lastIndex === 0) {
                next = css2.length - 1;
              } else {
                next = atEnd.lastIndex - 2;
              }
              tokens.push(["at-word", css2.slice(pos, next + 1), line2, pos - offset, line2, next - offset, pos]);
              pos = next;
              break;
            case backslash:
              next = pos;
              escape = true;
              while (css2.charCodeAt(next + 1) === backslash) {
                next += 1;
                escape = !escape;
              }
              code = css2.charCodeAt(next + 1);
              if (escape && code !== slash && code !== space && code !== newline && code !== tab && code !== cr && code !== feed) {
                next += 1;
              }
              tokens.push(["word", css2.slice(pos, next + 1), line2, pos - offset, line2, next - offset, pos]);
              pos = next;
              break;
            default:
              if (code === slash && css2.charCodeAt(pos + 1) === asterisk) {
                next = css2.indexOf("*/", pos + 2) + 1;
                if (next === 0) {
                  unclosed("comment", "*/");
                }
                content = css2.slice(pos, next + 1);
                lines = content.split("\n");
                last = lines.length - 1;
                if (last > 0) {
                  nextLine = line2 + last;
                  nextOffset = next - lines[last].length;
                } else {
                  nextLine = line2;
                  nextOffset = offset;
                }
                tokens.push(["comment", content, line2, pos - offset, nextLine, next - nextOffset, pos]);
                offset = nextOffset;
                line2 = nextLine;
                pos = next;
              } else {
                wordEnd.lastIndex = pos + 1;
                wordEnd.test(css2);
                if (wordEnd.lastIndex === 0) {
                  next = css2.length - 1;
                } else {
                  next = wordEnd.lastIndex - 2;
                }
                tokens.push(["word", css2.slice(pos, next + 1), line2, pos - offset, line2, next - offset, pos]);
                pos = next;
              }
              break;
          }
          pos++;
        }
        return tokens;
      }
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/parser.js
  var require_parser3 = __commonJS({
    "node_modules/postcss-selector-parser/dist/parser.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _createClass = function() {
        function defineProperties(target, props) {
          for (var i = 0; i < props.length; i++) {
            var descriptor = props[i];
            descriptor.enumerable = descriptor.enumerable || false;
            descriptor.configurable = true;
            if ("value" in descriptor)
              descriptor.writable = true;
            Object.defineProperty(target, descriptor.key, descriptor);
          }
        }
        return function(Constructor, protoProps, staticProps) {
          if (protoProps)
            defineProperties(Constructor.prototype, protoProps);
          if (staticProps)
            defineProperties(Constructor, staticProps);
          return Constructor;
        };
      }();
      var _flatten = require_flatten();
      var _flatten2 = _interopRequireDefault(_flatten);
      var _indexesOf = require_indexes_of();
      var _indexesOf2 = _interopRequireDefault(_indexesOf);
      var _uniq = require_uniq();
      var _uniq2 = _interopRequireDefault(_uniq);
      var _root = require_root3();
      var _root2 = _interopRequireDefault(_root);
      var _selector = require_selector();
      var _selector2 = _interopRequireDefault(_selector);
      var _className = require_className();
      var _className2 = _interopRequireDefault(_className);
      var _comment = require_comment3();
      var _comment2 = _interopRequireDefault(_comment);
      var _id = require_id();
      var _id2 = _interopRequireDefault(_id);
      var _tag = require_tag();
      var _tag2 = _interopRequireDefault(_tag);
      var _string = require_string2();
      var _string2 = _interopRequireDefault(_string);
      var _pseudo = require_pseudo();
      var _pseudo2 = _interopRequireDefault(_pseudo);
      var _attribute = require_attribute();
      var _attribute2 = _interopRequireDefault(_attribute);
      var _universal = require_universal();
      var _universal2 = _interopRequireDefault(_universal);
      var _combinator = require_combinator();
      var _combinator2 = _interopRequireDefault(_combinator);
      var _nesting = require_nesting();
      var _nesting2 = _interopRequireDefault(_nesting);
      var _sortAscending = require_sortAscending();
      var _sortAscending2 = _interopRequireDefault(_sortAscending);
      var _tokenize = require_tokenize3();
      var _tokenize2 = _interopRequireDefault(_tokenize);
      var _types = require_types();
      var types = _interopRequireWildcard(_types);
      function _interopRequireWildcard(obj) {
        if (obj && obj.__esModule) {
          return obj;
        } else {
          var newObj = {};
          if (obj != null) {
            for (var key in obj) {
              if (Object.prototype.hasOwnProperty.call(obj, key))
                newObj[key] = obj[key];
            }
          }
          newObj.default = obj;
          return newObj;
        }
      }
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      var Parser = function() {
        function Parser2(input) {
          _classCallCheck(this, Parser2);
          this.input = input;
          this.lossy = input.options.lossless === false;
          this.position = 0;
          this.root = new _root2.default();
          var selectors = new _selector2.default();
          this.root.append(selectors);
          this.current = selectors;
          if (this.lossy) {
            this.tokens = (0, _tokenize2.default)({ safe: input.safe, css: input.css.trim() });
          } else {
            this.tokens = (0, _tokenize2.default)(input);
          }
          return this.loop();
        }
        Parser2.prototype.attribute = function attribute() {
          var str = "";
          var attr = void 0;
          var startingToken = this.currToken;
          this.position++;
          while (this.position < this.tokens.length && this.currToken[0] !== "]") {
            str += this.tokens[this.position][1];
            this.position++;
          }
          if (this.position === this.tokens.length && !~str.indexOf("]")) {
            this.error("Expected a closing square bracket.");
          }
          var parts = str.split(/((?:[*~^$|]?=))([^]*)/);
          var namespace = parts[0].split(/(\|)/g);
          var attributeProps = {
            operator: parts[1],
            value: parts[2],
            source: {
              start: {
                line: startingToken[2],
                column: startingToken[3]
              },
              end: {
                line: this.currToken[2],
                column: this.currToken[3]
              }
            },
            sourceIndex: startingToken[4]
          };
          if (namespace.length > 1) {
            if (namespace[0] === "") {
              namespace[0] = true;
            }
            attributeProps.attribute = this.parseValue(namespace[2]);
            attributeProps.namespace = this.parseNamespace(namespace[0]);
          } else {
            attributeProps.attribute = this.parseValue(parts[0]);
          }
          attr = new _attribute2.default(attributeProps);
          if (parts[2]) {
            var insensitive = parts[2].split(/(\s+i\s*?)$/);
            var trimmedValue = insensitive[0].trim();
            attr.value = this.lossy ? trimmedValue : insensitive[0];
            if (insensitive[1]) {
              attr.insensitive = true;
              if (!this.lossy) {
                attr.raws.insensitive = insensitive[1];
              }
            }
            attr.quoted = trimmedValue[0] === "'" || trimmedValue[0] === '"';
            attr.raws.unquoted = attr.quoted ? trimmedValue.slice(1, -1) : trimmedValue;
          }
          this.newNode(attr);
          this.position++;
        };
        Parser2.prototype.combinator = function combinator() {
          if (this.currToken[1] === "|") {
            return this.namespace();
          }
          var node = new _combinator2.default({
            value: "",
            source: {
              start: {
                line: this.currToken[2],
                column: this.currToken[3]
              },
              end: {
                line: this.currToken[2],
                column: this.currToken[3]
              }
            },
            sourceIndex: this.currToken[4]
          });
          while (this.position < this.tokens.length && this.currToken && (this.currToken[0] === "space" || this.currToken[0] === "combinator")) {
            if (this.nextToken && this.nextToken[0] === "combinator") {
              node.spaces.before = this.parseSpace(this.currToken[1]);
              node.source.start.line = this.nextToken[2];
              node.source.start.column = this.nextToken[3];
              node.source.end.column = this.nextToken[3];
              node.source.end.line = this.nextToken[2];
              node.sourceIndex = this.nextToken[4];
            } else if (this.prevToken && this.prevToken[0] === "combinator") {
              node.spaces.after = this.parseSpace(this.currToken[1]);
            } else if (this.currToken[0] === "combinator") {
              node.value = this.currToken[1];
            } else if (this.currToken[0] === "space") {
              node.value = this.parseSpace(this.currToken[1], " ");
            }
            this.position++;
          }
          return this.newNode(node);
        };
        Parser2.prototype.comma = function comma() {
          if (this.position === this.tokens.length - 1) {
            this.root.trailingComma = true;
            this.position++;
            return;
          }
          var selectors = new _selector2.default();
          this.current.parent.append(selectors);
          this.current = selectors;
          this.position++;
        };
        Parser2.prototype.comment = function comment() {
          var node = new _comment2.default({
            value: this.currToken[1],
            source: {
              start: {
                line: this.currToken[2],
                column: this.currToken[3]
              },
              end: {
                line: this.currToken[4],
                column: this.currToken[5]
              }
            },
            sourceIndex: this.currToken[6]
          });
          this.newNode(node);
          this.position++;
        };
        Parser2.prototype.error = function error(message) {
          throw new this.input.error(message);
        };
        Parser2.prototype.missingBackslash = function missingBackslash() {
          return this.error("Expected a backslash preceding the semicolon.");
        };
        Parser2.prototype.missingParenthesis = function missingParenthesis() {
          return this.error("Expected opening parenthesis.");
        };
        Parser2.prototype.missingSquareBracket = function missingSquareBracket() {
          return this.error("Expected opening square bracket.");
        };
        Parser2.prototype.namespace = function namespace() {
          var before = this.prevToken && this.prevToken[1] || true;
          if (this.nextToken[0] === "word") {
            this.position++;
            return this.word(before);
          } else if (this.nextToken[0] === "*") {
            this.position++;
            return this.universal(before);
          }
        };
        Parser2.prototype.nesting = function nesting() {
          this.newNode(new _nesting2.default({
            value: this.currToken[1],
            source: {
              start: {
                line: this.currToken[2],
                column: this.currToken[3]
              },
              end: {
                line: this.currToken[2],
                column: this.currToken[3]
              }
            },
            sourceIndex: this.currToken[4]
          }));
          this.position++;
        };
        Parser2.prototype.parentheses = function parentheses() {
          var last = this.current.last;
          if (last && last.type === types.PSEUDO) {
            var selector = new _selector2.default();
            var cache = this.current;
            last.append(selector);
            this.current = selector;
            var balanced = 1;
            this.position++;
            while (this.position < this.tokens.length && balanced) {
              if (this.currToken[0] === "(") {
                balanced++;
              }
              if (this.currToken[0] === ")") {
                balanced--;
              }
              if (balanced) {
                this.parse();
              } else {
                selector.parent.source.end.line = this.currToken[2];
                selector.parent.source.end.column = this.currToken[3];
                this.position++;
              }
            }
            if (balanced) {
              this.error("Expected closing parenthesis.");
            }
            this.current = cache;
          } else {
            var _balanced = 1;
            this.position++;
            last.value += "(";
            while (this.position < this.tokens.length && _balanced) {
              if (this.currToken[0] === "(") {
                _balanced++;
              }
              if (this.currToken[0] === ")") {
                _balanced--;
              }
              last.value += this.parseParenthesisToken(this.currToken);
              this.position++;
            }
            if (_balanced) {
              this.error("Expected closing parenthesis.");
            }
          }
        };
        Parser2.prototype.pseudo = function pseudo() {
          var _this = this;
          var pseudoStr = "";
          var startingToken = this.currToken;
          while (this.currToken && this.currToken[0] === ":") {
            pseudoStr += this.currToken[1];
            this.position++;
          }
          if (!this.currToken) {
            return this.error("Expected pseudo-class or pseudo-element");
          }
          if (this.currToken[0] === "word") {
            var pseudo2 = void 0;
            this.splitWord(false, function(first, length) {
              pseudoStr += first;
              pseudo2 = new _pseudo2.default({
                value: pseudoStr,
                source: {
                  start: {
                    line: startingToken[2],
                    column: startingToken[3]
                  },
                  end: {
                    line: _this.currToken[4],
                    column: _this.currToken[5]
                  }
                },
                sourceIndex: startingToken[4]
              });
              _this.newNode(pseudo2);
              if (length > 1 && _this.nextToken && _this.nextToken[0] === "(") {
                _this.error("Misplaced parenthesis.");
              }
            });
          } else {
            this.error('Unexpected "' + this.currToken[0] + '" found.');
          }
        };
        Parser2.prototype.space = function space() {
          var token = this.currToken;
          if (this.position === 0 || this.prevToken[0] === "," || this.prevToken[0] === "(") {
            this.spaces = this.parseSpace(token[1]);
            this.position++;
          } else if (this.position === this.tokens.length - 1 || this.nextToken[0] === "," || this.nextToken[0] === ")") {
            this.current.last.spaces.after = this.parseSpace(token[1]);
            this.position++;
          } else {
            this.combinator();
          }
        };
        Parser2.prototype.string = function string() {
          var token = this.currToken;
          this.newNode(new _string2.default({
            value: this.currToken[1],
            source: {
              start: {
                line: token[2],
                column: token[3]
              },
              end: {
                line: token[4],
                column: token[5]
              }
            },
            sourceIndex: token[6]
          }));
          this.position++;
        };
        Parser2.prototype.universal = function universal(namespace) {
          var nextToken = this.nextToken;
          if (nextToken && nextToken[1] === "|") {
            this.position++;
            return this.namespace();
          }
          this.newNode(new _universal2.default({
            value: this.currToken[1],
            source: {
              start: {
                line: this.currToken[2],
                column: this.currToken[3]
              },
              end: {
                line: this.currToken[2],
                column: this.currToken[3]
              }
            },
            sourceIndex: this.currToken[4]
          }), namespace);
          this.position++;
        };
        Parser2.prototype.splitWord = function splitWord(namespace, firstCallback) {
          var _this2 = this;
          var nextToken = this.nextToken;
          var word = this.currToken[1];
          while (nextToken && nextToken[0] === "word") {
            this.position++;
            var current = this.currToken[1];
            word += current;
            if (current.lastIndexOf("\\") === current.length - 1) {
              var next = this.nextToken;
              if (next && next[0] === "space") {
                word += this.parseSpace(next[1], " ");
                this.position++;
              }
            }
            nextToken = this.nextToken;
          }
          var hasClass = (0, _indexesOf2.default)(word, ".");
          var hasId = (0, _indexesOf2.default)(word, "#");
          var interpolations = (0, _indexesOf2.default)(word, "#{");
          if (interpolations.length) {
            hasId = hasId.filter(function(hashIndex) {
              return !~interpolations.indexOf(hashIndex);
            });
          }
          var indices = (0, _sortAscending2.default)((0, _uniq2.default)((0, _flatten2.default)([[0], hasClass, hasId])));
          indices.forEach(function(ind, i) {
            var index = indices[i + 1] || word.length;
            var value = word.slice(ind, index);
            if (i === 0 && firstCallback) {
              return firstCallback.call(_this2, value, indices.length);
            }
            var node = void 0;
            if (~hasClass.indexOf(ind)) {
              node = new _className2.default({
                value: value.slice(1),
                source: {
                  start: {
                    line: _this2.currToken[2],
                    column: _this2.currToken[3] + ind
                  },
                  end: {
                    line: _this2.currToken[4],
                    column: _this2.currToken[3] + (index - 1)
                  }
                },
                sourceIndex: _this2.currToken[6] + indices[i]
              });
            } else if (~hasId.indexOf(ind)) {
              node = new _id2.default({
                value: value.slice(1),
                source: {
                  start: {
                    line: _this2.currToken[2],
                    column: _this2.currToken[3] + ind
                  },
                  end: {
                    line: _this2.currToken[4],
                    column: _this2.currToken[3] + (index - 1)
                  }
                },
                sourceIndex: _this2.currToken[6] + indices[i]
              });
            } else {
              node = new _tag2.default({
                value,
                source: {
                  start: {
                    line: _this2.currToken[2],
                    column: _this2.currToken[3] + ind
                  },
                  end: {
                    line: _this2.currToken[4],
                    column: _this2.currToken[3] + (index - 1)
                  }
                },
                sourceIndex: _this2.currToken[6] + indices[i]
              });
            }
            _this2.newNode(node, namespace);
          });
          this.position++;
        };
        Parser2.prototype.word = function word(namespace) {
          var nextToken = this.nextToken;
          if (nextToken && nextToken[1] === "|") {
            this.position++;
            return this.namespace();
          }
          return this.splitWord(namespace);
        };
        Parser2.prototype.loop = function loop() {
          while (this.position < this.tokens.length) {
            this.parse(true);
          }
          return this.root;
        };
        Parser2.prototype.parse = function parse3(throwOnParenthesis) {
          switch (this.currToken[0]) {
            case "space":
              this.space();
              break;
            case "comment":
              this.comment();
              break;
            case "(":
              this.parentheses();
              break;
            case ")":
              if (throwOnParenthesis) {
                this.missingParenthesis();
              }
              break;
            case "[":
              this.attribute();
              break;
            case "]":
              this.missingSquareBracket();
              break;
            case "at-word":
            case "word":
              this.word();
              break;
            case ":":
              this.pseudo();
              break;
            case ";":
              this.missingBackslash();
              break;
            case ",":
              this.comma();
              break;
            case "*":
              this.universal();
              break;
            case "&":
              this.nesting();
              break;
            case "combinator":
              this.combinator();
              break;
            case "string":
              this.string();
              break;
          }
        };
        Parser2.prototype.parseNamespace = function parseNamespace(namespace) {
          if (this.lossy && typeof namespace === "string") {
            var trimmed = namespace.trim();
            if (!trimmed.length) {
              return true;
            }
            return trimmed;
          }
          return namespace;
        };
        Parser2.prototype.parseSpace = function parseSpace(space, replacement) {
          return this.lossy ? replacement || "" : space;
        };
        Parser2.prototype.parseValue = function parseValue2(value) {
          return this.lossy && value && typeof value === "string" ? value.trim() : value;
        };
        Parser2.prototype.parseParenthesisToken = function parseParenthesisToken(token) {
          if (!this.lossy) {
            return token[1];
          }
          if (token[0] === "space") {
            return this.parseSpace(token[1], " ");
          }
          return this.parseValue(token[1]);
        };
        Parser2.prototype.newNode = function newNode(node, namespace) {
          if (namespace) {
            node.namespace = this.parseNamespace(namespace);
          }
          if (this.spaces) {
            node.spaces.before = this.spaces;
            this.spaces = "";
          }
          return this.current.append(node);
        };
        _createClass(Parser2, [{
          key: "currToken",
          get: function get() {
            return this.tokens[this.position];
          }
        }, {
          key: "nextToken",
          get: function get() {
            return this.tokens[this.position + 1];
          }
        }, {
          key: "prevToken",
          get: function get() {
            return this.tokens[this.position - 1];
          }
        }]);
        return Parser2;
      }();
      exports.default = Parser;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-selector-parser/dist/processor.js
  var require_processor2 = __commonJS({
    "node_modules/postcss-selector-parser/dist/processor.js"(exports, module) {
      "use strict";
      exports.__esModule = true;
      var _createClass = function() {
        function defineProperties(target, props) {
          for (var i = 0; i < props.length; i++) {
            var descriptor = props[i];
            descriptor.enumerable = descriptor.enumerable || false;
            descriptor.configurable = true;
            if ("value" in descriptor)
              descriptor.writable = true;
            Object.defineProperty(target, descriptor.key, descriptor);
          }
        }
        return function(Constructor, protoProps, staticProps) {
          if (protoProps)
            defineProperties(Constructor.prototype, protoProps);
          if (staticProps)
            defineProperties(Constructor, staticProps);
          return Constructor;
        };
      }();
      var _parser = require_parser3();
      var _parser2 = _interopRequireDefault(_parser);
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function _classCallCheck(instance, Constructor) {
        if (!(instance instanceof Constructor)) {
          throw new TypeError("Cannot call a class as a function");
        }
      }
      var Processor = function() {
        function Processor2(func) {
          _classCallCheck(this, Processor2);
          this.func = func || function noop2() {
          };
          return this;
        }
        Processor2.prototype.process = function process2(selectors) {
          var options2 = arguments.length > 1 && arguments[1] !== void 0 ? arguments[1] : {};
          var input = new _parser2.default({
            css: selectors,
            error: function error(e) {
              throw new Error(e);
            },
            options: options2
          });
          this.res = input;
          this.func(input);
          return this;
        };
        _createClass(Processor2, [{
          key: "result",
          get: function get() {
            return String(this.res);
          }
        }]);
        return Processor2;
      }();
      exports.default = Processor;
      module.exports = exports["default"];
    }
  });

  // node_modules/postcss-media-query-parser/dist/nodes/Node.js
  var require_Node = __commonJS({
    "node_modules/postcss-media-query-parser/dist/nodes/Node.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      function Node(opts) {
        this.after = opts.after;
        this.before = opts.before;
        this.type = opts.type;
        this.value = opts.value;
        this.sourceIndex = opts.sourceIndex;
      }
      exports.default = Node;
    }
  });

  // node_modules/postcss-media-query-parser/dist/nodes/Container.js
  var require_Container = __commonJS({
    "node_modules/postcss-media-query-parser/dist/nodes/Container.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      var _Node = require_Node();
      var _Node2 = _interopRequireDefault(_Node);
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function Container(opts) {
        var _this = this;
        this.constructor(opts);
        this.nodes = opts.nodes;
        if (this.after === void 0) {
          this.after = this.nodes.length > 0 ? this.nodes[this.nodes.length - 1].after : "";
        }
        if (this.before === void 0) {
          this.before = this.nodes.length > 0 ? this.nodes[0].before : "";
        }
        if (this.sourceIndex === void 0) {
          this.sourceIndex = this.before.length;
        }
        this.nodes.forEach(function(node) {
          node.parent = _this;
        });
      }
      Container.prototype = Object.create(_Node2.default.prototype);
      Container.constructor = _Node2.default;
      Container.prototype.walk = function walk(filter, cb) {
        var hasFilter = typeof filter === "string" || filter instanceof RegExp;
        var callback = hasFilter ? cb : filter;
        var filterReg = typeof filter === "string" ? new RegExp(filter) : filter;
        for (var i = 0; i < this.nodes.length; i++) {
          var node = this.nodes[i];
          var filtered = hasFilter ? filterReg.test(node.type) : true;
          if (filtered && callback && callback(node, i, this.nodes) === false) {
            return false;
          }
          if (node.nodes && node.walk(filter, cb) === false) {
            return false;
          }
        }
        return true;
      };
      Container.prototype.each = function each() {
        var cb = arguments.length <= 0 || arguments[0] === void 0 ? function() {
        } : arguments[0];
        for (var i = 0; i < this.nodes.length; i++) {
          var node = this.nodes[i];
          if (cb(node, i, this.nodes) === false) {
            return false;
          }
        }
        return true;
      };
      exports.default = Container;
    }
  });

  // node_modules/postcss-media-query-parser/dist/parsers.js
  var require_parsers = __commonJS({
    "node_modules/postcss-media-query-parser/dist/parsers.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.parseMediaFeature = parseMediaFeature;
      exports.parseMediaQuery = parseMediaQuery2;
      exports.parseMediaList = parseMediaList;
      var _Node = require_Node();
      var _Node2 = _interopRequireDefault(_Node);
      var _Container = require_Container();
      var _Container2 = _interopRequireDefault(_Container);
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function parseMediaFeature(string) {
        var index = arguments.length <= 1 || arguments[1] === void 0 ? 0 : arguments[1];
        var modesEntered = [{
          mode: "normal",
          character: null
        }];
        var result = [];
        var lastModeIndex = 0;
        var mediaFeature = "";
        var colon = null;
        var mediaFeatureValue = null;
        var indexLocal = index;
        var stringNormalized = string;
        if (string[0] === "(" && string[string.length - 1] === ")") {
          stringNormalized = string.substring(1, string.length - 1);
          indexLocal++;
        }
        for (var i = 0; i < stringNormalized.length; i++) {
          var character = stringNormalized[i];
          if (character === "'" || character === '"') {
            if (modesEntered[lastModeIndex].isCalculationEnabled === true) {
              modesEntered.push({
                mode: "string",
                isCalculationEnabled: false,
                character
              });
              lastModeIndex++;
            } else if (modesEntered[lastModeIndex].mode === "string" && modesEntered[lastModeIndex].character === character && stringNormalized[i - 1] !== "\\") {
              modesEntered.pop();
              lastModeIndex--;
            }
          }
          if (character === "{") {
            modesEntered.push({
              mode: "interpolation",
              isCalculationEnabled: true
            });
            lastModeIndex++;
          } else if (character === "}") {
            modesEntered.pop();
            lastModeIndex--;
          }
          if (modesEntered[lastModeIndex].mode === "normal" && character === ":") {
            var mediaFeatureValueStr = stringNormalized.substring(i + 1);
            mediaFeatureValue = {
              type: "value",
              before: /^(\s*)/.exec(mediaFeatureValueStr)[1],
              after: /(\s*)$/.exec(mediaFeatureValueStr)[1],
              value: mediaFeatureValueStr.trim()
            };
            mediaFeatureValue.sourceIndex = mediaFeatureValue.before.length + i + 1 + indexLocal;
            colon = {
              type: "colon",
              sourceIndex: i + indexLocal,
              after: mediaFeatureValue.before,
              value: ":"
            };
            break;
          }
          mediaFeature += character;
        }
        mediaFeature = {
          type: "media-feature",
          before: /^(\s*)/.exec(mediaFeature)[1],
          after: /(\s*)$/.exec(mediaFeature)[1],
          value: mediaFeature.trim()
        };
        mediaFeature.sourceIndex = mediaFeature.before.length + indexLocal;
        result.push(mediaFeature);
        if (colon !== null) {
          colon.before = mediaFeature.after;
          result.push(colon);
        }
        if (mediaFeatureValue !== null) {
          result.push(mediaFeatureValue);
        }
        return result;
      }
      function parseMediaQuery2(string) {
        var index = arguments.length <= 1 || arguments[1] === void 0 ? 0 : arguments[1];
        var result = [];
        var localLevel = 0;
        var insideSomeValue = false;
        var node = void 0;
        function resetNode() {
          return {
            before: "",
            after: "",
            value: ""
          };
        }
        node = resetNode();
        for (var i = 0; i < string.length; i++) {
          var character = string[i];
          if (!insideSomeValue) {
            if (character.search(/\s/) !== -1) {
              node.before += character;
            } else {
              if (character === "(") {
                node.type = "media-feature-expression";
                localLevel++;
              }
              node.value = character;
              node.sourceIndex = index + i;
              insideSomeValue = true;
            }
          } else {
            node.value += character;
            if (character === "{" || character === "(") {
              localLevel++;
            }
            if (character === ")" || character === "}") {
              localLevel--;
            }
          }
          if (insideSomeValue && localLevel === 0 && (character === ")" || i === string.length - 1 || string[i + 1].search(/\s/) !== -1)) {
            if (["not", "only", "and"].indexOf(node.value) !== -1) {
              node.type = "keyword";
            }
            if (node.type === "media-feature-expression") {
              node.nodes = parseMediaFeature(node.value, node.sourceIndex);
            }
            result.push(Array.isArray(node.nodes) ? new _Container2.default(node) : new _Node2.default(node));
            node = resetNode();
            insideSomeValue = false;
          }
        }
        for (var _i = 0; _i < result.length; _i++) {
          node = result[_i];
          if (_i > 0) {
            result[_i - 1].after = node.before;
          }
          if (node.type === void 0) {
            if (_i > 0) {
              if (result[_i - 1].type === "media-feature-expression") {
                node.type = "keyword";
                continue;
              }
              if (result[_i - 1].value === "not" || result[_i - 1].value === "only") {
                node.type = "media-type";
                continue;
              }
              if (result[_i - 1].value === "and") {
                node.type = "media-feature-expression";
                continue;
              }
              if (result[_i - 1].type === "media-type") {
                if (!result[_i + 1]) {
                  node.type = "media-feature-expression";
                } else {
                  node.type = result[_i + 1].type === "media-feature-expression" ? "keyword" : "media-feature-expression";
                }
              }
            }
            if (_i === 0) {
              if (!result[_i + 1]) {
                node.type = "media-type";
                continue;
              }
              if (result[_i + 1] && (result[_i + 1].type === "media-feature-expression" || result[_i + 1].type === "keyword")) {
                node.type = "media-type";
                continue;
              }
              if (result[_i + 2]) {
                if (result[_i + 2].type === "media-feature-expression") {
                  node.type = "media-type";
                  result[_i + 1].type = "keyword";
                  continue;
                }
                if (result[_i + 2].type === "keyword") {
                  node.type = "keyword";
                  result[_i + 1].type = "media-type";
                  continue;
                }
              }
              if (result[_i + 3]) {
                if (result[_i + 3].type === "media-feature-expression") {
                  node.type = "keyword";
                  result[_i + 1].type = "media-type";
                  result[_i + 2].type = "keyword";
                  continue;
                }
              }
            }
          }
        }
        return result;
      }
      function parseMediaList(string) {
        var result = [];
        var interimIndex = 0;
        var levelLocal = 0;
        var doesHaveUrl = /^(\s*)url\s*\(/.exec(string);
        if (doesHaveUrl !== null) {
          var i = doesHaveUrl[0].length;
          var parenthesesLv = 1;
          while (parenthesesLv > 0) {
            var character = string[i];
            if (character === "(") {
              parenthesesLv++;
            }
            if (character === ")") {
              parenthesesLv--;
            }
            i++;
          }
          result.unshift(new _Node2.default({
            type: "url",
            value: string.substring(0, i).trim(),
            sourceIndex: doesHaveUrl[1].length,
            before: doesHaveUrl[1],
            after: /^(\s*)/.exec(string.substring(i))[1]
          }));
          interimIndex = i;
        }
        for (var _i2 = interimIndex; _i2 < string.length; _i2++) {
          var _character = string[_i2];
          if (_character === "(") {
            levelLocal++;
          }
          if (_character === ")") {
            levelLocal--;
          }
          if (levelLocal === 0 && _character === ",") {
            var _mediaQueryString = string.substring(interimIndex, _i2);
            var _spaceBefore = /^(\s*)/.exec(_mediaQueryString)[1];
            result.push(new _Container2.default({
              type: "media-query",
              value: _mediaQueryString.trim(),
              sourceIndex: interimIndex + _spaceBefore.length,
              nodes: parseMediaQuery2(_mediaQueryString, interimIndex),
              before: _spaceBefore,
              after: /(\s*)$/.exec(_mediaQueryString)[1]
            }));
            interimIndex = _i2 + 1;
          }
        }
        var mediaQueryString = string.substring(interimIndex);
        var spaceBefore = /^(\s*)/.exec(mediaQueryString)[1];
        result.push(new _Container2.default({
          type: "media-query",
          value: mediaQueryString.trim(),
          sourceIndex: interimIndex + spaceBefore.length,
          nodes: parseMediaQuery2(mediaQueryString, interimIndex),
          before: spaceBefore,
          after: /(\s*)$/.exec(mediaQueryString)[1]
        }));
        return result;
      }
    }
  });

  // node_modules/postcss-media-query-parser/dist/index.js
  var require_dist = __commonJS({
    "node_modules/postcss-media-query-parser/dist/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.default = parseMedia;
      var _Container = require_Container();
      var _Container2 = _interopRequireDefault(_Container);
      var _parsers = require_parsers();
      function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : { default: obj };
      }
      function parseMedia(value) {
        return new _Container2.default({
          nodes: (0, _parsers.parseMediaList)(value),
          type: "media-query-list",
          value: value.trim()
        });
      }
    }
  });

  // src/plugins/postcss.js
  var postcss_exports = {};
  __export(postcss_exports, {
    languages: () => languages_evaluate_default,
    options: () => options_default,
    parsers: () => parser_postcss_exports,
    printers: () => printers
  });

  // scripts/build/shims/string-replace-all.js
  var stringReplaceAll = (isOptionalObject, original, pattern, replacement) => {
    if (isOptionalObject && (original === void 0 || original === null)) {
      return;
    }
    if (original.replaceAll) {
      return original.replaceAll(pattern, replacement);
    }
    if (pattern.global) {
      return original.replace(pattern, replacement);
    }
    return original.split(pattern).join(replacement);
  };
  var string_replace_all_default = stringReplaceAll;

  // src/utils/get-preferred-quote.js
  var SINGLE_QUOTE = "'";
  var DOUBLE_QUOTE = '"';
  function getPreferredQuote(rawContent, preferredQuoteOrPreferSingleQuote) {
    const preferred = preferredQuoteOrPreferSingleQuote === true || preferredQuoteOrPreferSingleQuote === SINGLE_QUOTE ? SINGLE_QUOTE : DOUBLE_QUOTE;
    const alternate = preferred === SINGLE_QUOTE ? DOUBLE_QUOTE : SINGLE_QUOTE;
    let preferredQuoteCount = 0;
    let alternateQuoteCount = 0;
    for (const character of rawContent) {
      if (character === preferred) {
        preferredQuoteCount++;
      } else if (character === alternate) {
        alternateQuoteCount++;
      }
    }
    return preferredQuoteCount > alternateQuoteCount ? alternate : preferred;
  }
  var get_preferred_quote_default = getPreferredQuote;

  // src/utils/make-string.js
  function makeString(rawText, enclosingQuote, unescapeUnnecessaryEscapes) {
    const otherQuote = enclosingQuote === '"' ? "'" : '"';
    const regex = /\\(.)|(["'])/gs;
    const raw = string_replace_all_default(
      /* isOptionalObject*/
      false,
      rawText,
      regex,
      (match, escaped, quote) => {
        if (escaped === otherQuote) {
          return escaped;
        }
        if (quote === enclosingQuote) {
          return "\\" + quote;
        }
        if (quote) {
          return quote;
        }
        return unescapeUnnecessaryEscapes && /^[^\n\r"'0-7\\bfnrt-vx\u2028\u2029]$/.test(escaped) ? escaped : "\\" + escaped;
      }
    );
    return enclosingQuote + raw + enclosingQuote;
  }
  var make_string_default = makeString;

  // src/utils/print-string.js
  function printString(raw, options2) {
    const rawContent = raw.slice(1, -1);
    const enclosingQuote = options2.parser === "json" || options2.parser === "json5" && options2.quoteProps === "preserve" && !options2.singleQuote ? '"' : options2.__isInHtmlAttribute ? "'" : get_preferred_quote_default(rawContent, options2.singleQuote);
    return make_string_default(
      rawContent,
      enclosingQuote,
      !(options2.parser === "css" || options2.parser === "less" || options2.parser === "scss" || options2.__embeddedInHtml)
    );
  }
  var print_string_default = printString;

  // src/utils/is-non-empty-array.js
  function isNonEmptyArray(object) {
    return Array.isArray(object) && object.length > 0;
  }
  var is_non_empty_array_default = isNonEmptyArray;

  // src/document/constants.js
  var DOC_TYPE_STRING = "string";
  var DOC_TYPE_ARRAY = "array";
  var DOC_TYPE_CURSOR = "cursor";
  var DOC_TYPE_INDENT = "indent";
  var DOC_TYPE_ALIGN = "align";
  var DOC_TYPE_TRIM = "trim";
  var DOC_TYPE_GROUP = "group";
  var DOC_TYPE_FILL = "fill";
  var DOC_TYPE_IF_BREAK = "if-break";
  var DOC_TYPE_INDENT_IF_BREAK = "indent-if-break";
  var DOC_TYPE_LINE_SUFFIX = "line-suffix";
  var DOC_TYPE_LINE_SUFFIX_BOUNDARY = "line-suffix-boundary";
  var DOC_TYPE_LINE = "line";
  var DOC_TYPE_LABEL = "label";
  var DOC_TYPE_BREAK_PARENT = "break-parent";
  var VALID_OBJECT_DOC_TYPES = /* @__PURE__ */ new Set([
    DOC_TYPE_CURSOR,
    DOC_TYPE_INDENT,
    DOC_TYPE_ALIGN,
    DOC_TYPE_TRIM,
    DOC_TYPE_GROUP,
    DOC_TYPE_FILL,
    DOC_TYPE_IF_BREAK,
    DOC_TYPE_INDENT_IF_BREAK,
    DOC_TYPE_LINE_SUFFIX,
    DOC_TYPE_LINE_SUFFIX_BOUNDARY,
    DOC_TYPE_LINE,
    DOC_TYPE_LABEL,
    DOC_TYPE_BREAK_PARENT
  ]);

  // src/document/utils/get-doc-type.js
  function getDocType(doc) {
    if (typeof doc === "string") {
      return DOC_TYPE_STRING;
    }
    if (Array.isArray(doc)) {
      return DOC_TYPE_ARRAY;
    }
    if (!doc) {
      return;
    }
    const { type } = doc;
    if (VALID_OBJECT_DOC_TYPES.has(type)) {
      return type;
    }
  }
  var get_doc_type_default = getDocType;

  // src/document/invalid-doc-error.js
  var disjunctionListFormat = (list) => new Intl.ListFormat("en-US", { type: "disjunction" }).format(list);
  function getDocErrorMessage(doc) {
    const type = doc === null ? "null" : typeof doc;
    if (type !== "string" && type !== "object") {
      return `Unexpected doc '${type}', 
Expected it to be 'string' or 'object'.`;
    }
    if (get_doc_type_default(doc)) {
      throw new Error("doc is valid.");
    }
    const objectType = Object.prototype.toString.call(doc);
    if (objectType !== "[object Object]") {
      return `Unexpected doc '${objectType}'.`;
    }
    const EXPECTED_TYPE_VALUES = disjunctionListFormat(
      [...VALID_OBJECT_DOC_TYPES].map((type2) => `'${type2}'`)
    );
    return `Unexpected doc.type '${doc.type}'.
Expected it to be ${EXPECTED_TYPE_VALUES}.`;
  }
  var InvalidDocError = class extends Error {
    name = "InvalidDocError";
    constructor(doc) {
      super(getDocErrorMessage(doc));
      this.doc = doc;
    }
  };
  var invalid_doc_error_default = InvalidDocError;

  // src/document/utils/assert-doc.js
  var noop = () => {
  };
  var assertDoc = true ? noop : function(doc) {
    traverse_doc_default(doc, (doc2) => {
      if (checked.has(doc2)) {
        return false;
      }
      if (typeof doc2 !== "string") {
        checked.add(doc2);
      }
    });
  };
  var assertDocArray = true ? noop : function(docs, optional = false) {
    if (optional && !docs) {
      return;
    }
    if (!Array.isArray(docs)) {
      throw new TypeError("Unexpected doc array.");
    }
    for (const doc of docs) {
      assertDoc(doc);
    }
  };

  // src/document/builders.js
  function indent(contents) {
    assertDoc(contents);
    return { type: DOC_TYPE_INDENT, contents };
  }
  function align(widthOrString, contents) {
    assertDoc(contents);
    return { type: DOC_TYPE_ALIGN, contents, n: widthOrString };
  }
  function group(contents, opts = {}) {
    assertDoc(contents);
    assertDocArray(
      opts.expandedStates,
      /* optional */
      true
    );
    return {
      type: DOC_TYPE_GROUP,
      id: opts.id,
      contents,
      break: Boolean(opts.shouldBreak),
      expandedStates: opts.expandedStates
    };
  }
  function markAsRoot(contents) {
    return align({ type: "root" }, contents);
  }
  function dedent(contents) {
    return align(-1, contents);
  }
  function fill(parts) {
    assertDocArray(parts);
    return { type: DOC_TYPE_FILL, parts };
  }
  function ifBreak(breakContents, flatContents = "", opts = {}) {
    assertDoc(breakContents);
    if (flatContents !== "") {
      assertDoc(flatContents);
    }
    return {
      type: DOC_TYPE_IF_BREAK,
      breakContents,
      flatContents,
      groupId: opts.groupId
    };
  }
  var breakParent = { type: DOC_TYPE_BREAK_PARENT };
  var hardlineWithoutBreakParent = { type: DOC_TYPE_LINE, hard: true };
  var line = { type: DOC_TYPE_LINE };
  var softline = { type: DOC_TYPE_LINE, soft: true };
  var hardline = [hardlineWithoutBreakParent, breakParent];
  function join(separator, docs) {
    assertDoc(separator);
    assertDocArray(docs);
    const parts = [];
    for (let i = 0; i < docs.length; i++) {
      if (i !== 0) {
        parts.push(separator);
      }
      parts.push(docs[i]);
    }
    return parts;
  }

  // scripts/build/shims/at.js
  var at = (isOptionalObject, object, index) => {
    if (isOptionalObject && (object === void 0 || object === null)) {
      return;
    }
    if (Array.isArray(object) || typeof object === "string") {
      return object[index < 0 ? object.length + index : index];
    }
    return object.at(index);
  };
  var at_default = at;

  // src/document/utils.js
  var getDocParts = (doc) => {
    if (Array.isArray(doc)) {
      return doc;
    }
    if (doc.type !== DOC_TYPE_FILL) {
      throw new Error(`Expect doc to be 'array' or '${DOC_TYPE_FILL}'.`);
    }
    return doc.parts;
  };
  function mapDoc(doc, cb) {
    if (typeof doc === "string") {
      return cb(doc);
    }
    const mapped = /* @__PURE__ */ new Map();
    return rec(doc);
    function rec(doc2) {
      if (mapped.has(doc2)) {
        return mapped.get(doc2);
      }
      const result = process2(doc2);
      mapped.set(doc2, result);
      return result;
    }
    function process2(doc2) {
      switch (get_doc_type_default(doc2)) {
        case DOC_TYPE_ARRAY:
          return cb(doc2.map(rec));
        case DOC_TYPE_FILL:
          return cb({
            ...doc2,
            parts: doc2.parts.map(rec)
          });
        case DOC_TYPE_IF_BREAK:
          return cb({
            ...doc2,
            breakContents: rec(doc2.breakContents),
            flatContents: rec(doc2.flatContents)
          });
        case DOC_TYPE_GROUP: {
          let {
            expandedStates,
            contents
          } = doc2;
          if (expandedStates) {
            expandedStates = expandedStates.map(rec);
            contents = expandedStates[0];
          } else {
            contents = rec(contents);
          }
          return cb({
            ...doc2,
            contents,
            expandedStates
          });
        }
        case DOC_TYPE_ALIGN:
        case DOC_TYPE_INDENT:
        case DOC_TYPE_INDENT_IF_BREAK:
        case DOC_TYPE_LABEL:
        case DOC_TYPE_LINE_SUFFIX:
          return cb({
            ...doc2,
            contents: rec(doc2.contents)
          });
        case DOC_TYPE_STRING:
        case DOC_TYPE_CURSOR:
        case DOC_TYPE_TRIM:
        case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
        case DOC_TYPE_LINE:
        case DOC_TYPE_BREAK_PARENT:
          return cb(doc2);
        default:
          throw new invalid_doc_error_default(doc2);
      }
    }
  }
  function removeLinesFn(doc) {
    if (doc.type === DOC_TYPE_LINE && !doc.hard) {
      return doc.soft ? "" : " ";
    }
    if (doc.type === DOC_TYPE_IF_BREAK) {
      return doc.flatContents;
    }
    return doc;
  }
  function removeLines(doc) {
    return mapDoc(doc, removeLinesFn);
  }

  // src/utils/unexpected-node-error.js
  var UnexpectedNodeError = class extends Error {
    name = "UnexpectedNodeError";
    constructor(node, language, typeProperty = "type") {
      super(
        `Unexpected ${language} node ${typeProperty}: ${JSON.stringify(
          node[typeProperty]
        )}.`
      );
      this.node = node;
    }
  };
  var unexpected_node_error_default = UnexpectedNodeError;

  // src/utils/front-matter/is-front-matter.js
  function isFrontMatter(node) {
    return (node == null ? void 0 : node.type) === "front-matter";
  }
  var is_front_matter_default = isFrontMatter;

  // src/language-css/clean.js
  var ignoredProperties = /* @__PURE__ */ new Set([
    "raw",
    // front-matter
    "raws",
    "sourceIndex",
    "source",
    "before",
    "after",
    "trailingComma",
    "spaces"
  ]);
  function clean(ast, newObj, parent) {
    if (is_front_matter_default(ast) && ast.lang === "yaml") {
      delete newObj.value;
    }
    if (ast.type === "css-comment" && parent.type === "css-root" && parent.nodes.length > 0) {
      if (parent.nodes[0] === ast || is_front_matter_default(parent.nodes[0]) && parent.nodes[1] === ast) {
        delete newObj.text;
        if (/^\*\s*@(?:format|prettier)\s*$/.test(ast.text)) {
          return null;
        }
      }
      if (parent.type === "css-root" && at_default(
        /* isOptionalObject*/
        false,
        parent.nodes,
        -1
      ) === ast) {
        return null;
      }
    }
    if (ast.type === "value-root") {
      delete newObj.text;
    }
    if (ast.type === "media-query" || ast.type === "media-query-list" || ast.type === "media-feature-expression") {
      delete newObj.value;
    }
    if (ast.type === "css-rule") {
      delete newObj.params;
    }
    if (ast.type === "selector-combinator") {
      newObj.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        newObj.value,
        /\s+/g,
        " "
      );
    }
    if (ast.type === "media-feature") {
      newObj.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        newObj.value,
        " ",
        ""
      );
    }
    if (ast.type === "value-word" && (ast.isColor && ast.isHex || ["initial", "inherit", "unset", "revert"].includes(newObj.value.toLowerCase())) || ast.type === "media-feature" || ast.type === "selector-root-invalid" || ast.type === "selector-pseudo") {
      newObj.value = newObj.value.toLowerCase();
    }
    if (ast.type === "css-decl") {
      newObj.prop = newObj.prop.toLowerCase();
    }
    if (ast.type === "css-atrule" || ast.type === "css-import") {
      newObj.name = newObj.name.toLowerCase();
    }
    if (ast.type === "value-number") {
      newObj.unit = newObj.unit.toLowerCase();
    }
    if (ast.type === "value-unknown") {
      newObj.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        newObj.value,
        /;$/g,
        ""
      );
    }
    if ((ast.type === "media-feature" || ast.type === "media-keyword" || ast.type === "media-type" || ast.type === "media-unknown" || ast.type === "media-url" || ast.type === "media-value" || ast.type === "selector-attribute" || ast.type === "selector-string" || ast.type === "selector-class" || ast.type === "selector-combinator" || ast.type === "value-string") && newObj.value) {
      newObj.value = cleanCSSStrings(newObj.value);
    }
    if (ast.type === "selector-attribute") {
      newObj.attribute = newObj.attribute.trim();
      if (newObj.namespace && typeof newObj.namespace === "string") {
        newObj.namespace = newObj.namespace.trim();
        if (newObj.namespace.length === 0) {
          newObj.namespace = true;
        }
      }
      if (newObj.value) {
        newObj.value = string_replace_all_default(
          /* isOptionalObject*/
          false,
          newObj.value.trim(),
          /^["']|["']$/g,
          ""
        );
        delete newObj.quoted;
      }
    }
    if ((ast.type === "media-value" || ast.type === "media-type" || ast.type === "value-number" || ast.type === "selector-root-invalid" || ast.type === "selector-class" || ast.type === "selector-combinator" || ast.type === "selector-tag") && newObj.value) {
      newObj.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        newObj.value,
        /([\d+.Ee-]+)([A-Za-z]*)/g,
        (match, numStr, unit) => {
          const num = Number(numStr);
          return Number.isNaN(num) ? match : num + unit.toLowerCase();
        }
      );
    }
    if (ast.type === "selector-tag") {
      const lowercasedValue = ast.value.toLowerCase();
      if (["from", "to"].includes(lowercasedValue)) {
        newObj.value = lowercasedValue;
      }
    }
    if (ast.type === "css-atrule" && ast.name.toLowerCase() === "supports") {
      delete newObj.value;
    }
    if (ast.type === "selector-unknown") {
      delete newObj.value;
    }
    if (ast.type === "value-comma_group") {
      const index = ast.groups.findIndex((node) => node.type === "value-number" && node.unit === "...");
      if (index !== -1) {
        newObj.groups[index].unit = "";
        newObj.groups.splice(index + 1, 0, {
          type: "value-word",
          value: "...",
          isColor: false,
          isHex: false
        });
      }
    }
    if (ast.type === "value-comma_group" && ast.groups.some((node) => node.type === "value-atword" && node.value.endsWith("[") || node.type === "value-word" && node.value.startsWith("]"))) {
      return {
        type: "value-atword",
        value: ast.groups.map((node) => node.value).join(""),
        group: {
          open: null,
          close: null,
          groups: [],
          type: "value-paren_group"
        }
      };
    }
  }
  clean.ignoredProperties = ignoredProperties;
  function cleanCSSStrings(value) {
    return string_replace_all_default(
      /* isOptionalObject*/
      false,
      string_replace_all_default(
        /* isOptionalObject*/
        false,
        value,
        "'",
        '"'
      ),
      /\\([^\dA-Fa-f])/g,
      "$1"
    );
  }
  var clean_default = clean;

  // src/utils/front-matter/print.js
  function print(node, textToDoc) {
    if (node.lang === "yaml") {
      const value = node.value.trim();
      const doc = value ? textToDoc(value, { parser: "yaml" }) : "";
      return markAsRoot([
        node.startDelimiter,
        hardline,
        doc,
        doc ? hardline : "",
        node.endDelimiter
      ]);
    }
  }
  var print_default = print;

  // src/language-css/embed.js
  function embed(path) {
    const { node } = path;
    if (node.type === "front-matter") {
      return (textToDoc) => {
        const doc = print_default(node, textToDoc);
        return doc ? [doc, hardline] : void 0;
      };
    }
  }
  embed.getVisitorKeys = (node) => node.type === "css-root" ? ["frontMatter"] : [];
  var embed_default = embed;

  // src/language-js/pragma.js
  var import_jest_docblock = __toESM(require_build(), 1);

  // src/language-js/utils/get-shebang.js
  function getShebang(text) {
    if (!text.startsWith("#!")) {
      return "";
    }
    const index = text.indexOf("\n");
    if (index === -1) {
      return text;
    }
    return text.slice(0, index);
  }
  var get_shebang_default = getShebang;

  // src/language-js/pragma.js
  function parseDocBlock(text) {
    const shebang = get_shebang_default(text);
    if (shebang) {
      text = text.slice(shebang.length + 1);
    }
    const docBlock = (0, import_jest_docblock.extract)(text);
    const {
      pragmas,
      comments
    } = (0, import_jest_docblock.parseWithComments)(docBlock);
    return {
      shebang,
      text,
      pragmas,
      comments
    };
  }
  function hasPragma(text) {
    const {
      pragmas
    } = parseDocBlock(text);
    return Object.prototype.hasOwnProperty.call(pragmas, "prettier") || Object.prototype.hasOwnProperty.call(pragmas, "format");
  }
  function insertPragma(originalText) {
    const {
      shebang,
      text,
      pragmas,
      comments
    } = parseDocBlock(originalText);
    const strippedText = (0, import_jest_docblock.strip)(text);
    let docBlock = (0, import_jest_docblock.print)({
      pragmas: {
        format: "",
        ...pragmas
      },
      comments: comments.trimStart()
    });
    if (false) {
      docBlock = normalizeEndOfLine(docBlock);
    }
    return (shebang ? `${shebang}
` : "") + docBlock + (strippedText.startsWith("\n") ? "\n" : "\n\n") + strippedText;
  }

  // src/utils/front-matter/parse.js
  var frontMatterRegex = new RegExp("^(?<startDelimiter>-{3}|\\+{3})(?<language>[^\\n]*)\\n(?:|(?<value>.*?)\\n)(?<endDelimiter>\\k<startDelimiter>|\\.{3})[^\\S\\n]*(?:\\n|$)", "s");
  function parse(text) {
    const match = text.match(frontMatterRegex);
    if (!match) {
      return {
        content: text
      };
    }
    const {
      startDelimiter,
      language,
      value = "",
      endDelimiter
    } = match.groups;
    let lang = language.trim() || "yaml";
    if (startDelimiter === "+++") {
      lang = "toml";
    }
    if (lang !== "yaml" && startDelimiter !== endDelimiter) {
      return {
        content: text
      };
    }
    const [raw] = match;
    const frontMatter = {
      type: "front-matter",
      lang,
      value,
      startDelimiter,
      endDelimiter,
      raw: raw.replace(/\n$/, "")
    };
    return {
      frontMatter,
      content: string_replace_all_default(
        /* isOptionalObject*/
        false,
        raw,
        /[^\n]/g,
        " "
      ) + text.slice(raw.length)
    };
  }
  var parse_default = parse;

  // src/language-css/pragma.js
  function hasPragma2(text) {
    return hasPragma(parse_default(text).content);
  }
  function insertPragma2(text) {
    const { frontMatter, content } = parse_default(text);
    return (frontMatter ? frontMatter.raw + "\n\n" : "") + insertPragma(content);
  }

  // node_modules/to-fast-properties/index.js
  var fastProto = null;
  function FastObject(object) {
    if (fastProto !== null && typeof fastProto.property) {
      const result = fastProto;
      fastProto = FastObject.prototype = null;
      return result;
    }
    fastProto = FastObject.prototype = object == null ? /* @__PURE__ */ Object.create(null) : object;
    return new FastObject();
  }
  var inlineCacheCutoff = 10;
  for (let index = 0; index <= inlineCacheCutoff; index++) {
    FastObject();
  }
  function toFastproperties(object) {
    return FastObject(object);
  }

  // src/utils/create-get-visitor-keys.js
  function createGetVisitorKeys(visitorKeys2, typeProperty = "type") {
    toFastproperties(visitorKeys2);
    function getVisitorKeys2(node) {
      const type = node[typeProperty];
      if (false) {
        throw new Error(
          `Can't get node type, you must pass the wrong typeProperty '${typeProperty}'`
        );
      }
      const keys = visitorKeys2[type];
      if (!Array.isArray(keys)) {
        throw Object.assign(new Error(`Missing visitor keys for '${type}'.`), {
          node
        });
      }
      return keys;
    }
    return getVisitorKeys2;
  }
  var create_get_visitor_keys_default = createGetVisitorKeys;

  // src/language-css/visitor-keys.js
  var visitorKeys = {
    "front-matter": [],
    "css-root": ["frontMatter", "nodes"],
    "css-comment": [],
    "css-rule": ["selector", "nodes"],
    "css-decl": ["value", "selector", "nodes"],
    "css-atrule": ["selector", "params", "value", "nodes"],
    "media-query-list": ["nodes"],
    "media-query": ["nodes"],
    "media-type": [],
    "media-feature-expression": ["nodes"],
    "media-feature": [],
    "media-colon": [],
    "media-value": [],
    "media-keyword": [],
    "media-url": [],
    "media-unknown": [],
    "selector-root": ["nodes"],
    "selector-selector": ["nodes"],
    "selector-comment": [],
    "selector-string": [],
    "selector-tag": [],
    "selector-id": [],
    "selector-class": [],
    "selector-attribute": [],
    "selector-combinator": ["nodes"],
    "selector-universal": [],
    "selector-pseudo": ["nodes"],
    "selector-nesting": [],
    "selector-unknown": [],
    "value-value": ["group"],
    "value-root": ["group"],
    "value-comment": [],
    "value-comma_group": ["groups"],
    "value-paren_group": ["open", "groups", "close"],
    "value-func": ["group"],
    "value-paren": [],
    "value-number": [],
    "value-operator": [],
    "value-word": [],
    "value-colon": [],
    "value-comma": [],
    "value-string": [],
    "value-atword": [],
    "value-unicode-range": [],
    "value-unknown": []
  };
  var visitor_keys_default = visitorKeys;

  // src/language-css/get-visitor-keys.js
  var getVisitorKeys = create_get_visitor_keys_default(visitor_keys_default);
  var get_visitor_keys_default = getVisitorKeys;

  // src/language-css/utils/index.js
  var colorAdjusterFunctions = /* @__PURE__ */ new Set([
    "red",
    "green",
    "blue",
    "alpha",
    "a",
    "rgb",
    "hue",
    "h",
    "saturation",
    "s",
    "lightness",
    "l",
    "whiteness",
    "w",
    "blackness",
    "b",
    "tint",
    "shade",
    "blend",
    "blenda",
    "contrast",
    "hsl",
    "hsla",
    "hwb",
    "hwba"
  ]);
  function getPropOfDeclNode(path) {
    var _a, _b;
    return (_b = (_a = path.findAncestor((node) => node.type === "css-decl")) == null ? void 0 : _a.prop) == null ? void 0 : _b.toLowerCase();
  }
  var wideKeywords = /* @__PURE__ */ new Set(["initial", "inherit", "unset", "revert"]);
  function isWideKeywords(value) {
    return wideKeywords.has(value.toLowerCase());
  }
  function isKeyframeAtRuleKeywords(path, value) {
    var _a;
    const atRuleAncestorNode = path.findAncestor(
      (node) => node.type === "css-atrule"
    );
    return ((_a = atRuleAncestorNode == null ? void 0 : atRuleAncestorNode.name) == null ? void 0 : _a.toLowerCase().endsWith("keyframes")) && ["from", "to"].includes(value.toLowerCase());
  }
  function maybeToLowerCase(value) {
    return value.includes("$") || value.includes("@") || value.includes("#") || value.startsWith("%") || value.startsWith("--") || value.startsWith(":--") || value.includes("(") && value.includes(")") ? value : value.toLowerCase();
  }
  function insideValueFunctionNode(path, functionName) {
    var _a;
    const funcAncestorNode = path.findAncestor(
      (node) => node.type === "value-func"
    );
    return ((_a = funcAncestorNode == null ? void 0 : funcAncestorNode.value) == null ? void 0 : _a.toLowerCase()) === functionName;
  }
  function insideICSSRuleNode(path) {
    var _a;
    const ruleAncestorNode = path.findAncestor(
      (node) => node.type === "css-rule"
    );
    const selector = (_a = ruleAncestorNode == null ? void 0 : ruleAncestorNode.raws) == null ? void 0 : _a.selector;
    return selector && (selector.startsWith(":import") || selector.startsWith(":export"));
  }
  function insideAtRuleNode(path, atRuleNameOrAtRuleNames) {
    const atRuleNames = Array.isArray(atRuleNameOrAtRuleNames) ? atRuleNameOrAtRuleNames : [atRuleNameOrAtRuleNames];
    const atRuleAncestorNode = path.findAncestor(
      (node) => node.type === "css-atrule"
    );
    return atRuleAncestorNode && atRuleNames.includes(atRuleAncestorNode.name.toLowerCase());
  }
  function insideURLFunctionInImportAtRuleNode(path) {
    var _a;
    const { node } = path;
    return node.groups[0].value === "url" && node.groups.length === 2 && ((_a = path.findAncestor((node2) => node2.type === "css-atrule")) == null ? void 0 : _a.name) === "import";
  }
  function isURLFunctionNode(node) {
    return node.type === "value-func" && node.value.toLowerCase() === "url";
  }
  function isVarFunctionNode(node) {
    return node.type === "value-func" && node.value.toLowerCase() === "var";
  }
  function isLastNode(path, node) {
    var _a;
    const nodes = (_a = path.parent) == null ? void 0 : _a.nodes;
    return nodes && nodes.indexOf(node) === nodes.length - 1;
  }
  function isDetachedRulesetDeclarationNode(node) {
    const { selector } = node;
    if (!selector) {
      return false;
    }
    return typeof selector === "string" && /^@.+:.*$/.test(selector) || selector.value && /^@.+:.*$/.test(selector.value);
  }
  function isForKeywordNode(node) {
    return node.type === "value-word" && ["from", "through", "end"].includes(node.value);
  }
  function isIfElseKeywordNode(node) {
    return node.type === "value-word" && ["and", "or", "not"].includes(node.value);
  }
  function isEachKeywordNode(node) {
    return node.type === "value-word" && node.value === "in";
  }
  function isMultiplicationNode(node) {
    return node.type === "value-operator" && node.value === "*";
  }
  function isDivisionNode(node) {
    return node.type === "value-operator" && node.value === "/";
  }
  function isAdditionNode(node) {
    return node.type === "value-operator" && node.value === "+";
  }
  function isSubtractionNode(node) {
    return node.type === "value-operator" && node.value === "-";
  }
  function isModuloNode(node) {
    return node.type === "value-operator" && node.value === "%";
  }
  function isMathOperatorNode(node) {
    return isMultiplicationNode(node) || isDivisionNode(node) || isAdditionNode(node) || isSubtractionNode(node) || isModuloNode(node);
  }
  function isEqualityOperatorNode(node) {
    return node.type === "value-word" && ["==", "!="].includes(node.value);
  }
  function isRelationalOperatorNode(node) {
    return node.type === "value-word" && ["<", ">", "<=", ">="].includes(node.value);
  }
  function isSCSSControlDirectiveNode(node, options2) {
    return options2.parser === "scss" && node.type === "css-atrule" && ["if", "else", "for", "each", "while"].includes(node.name);
  }
  function isDetachedRulesetCallNode(node) {
    var _a;
    return ((_a = node.raws) == null ? void 0 : _a.params) && /^\(\s*\)$/.test(node.raws.params);
  }
  function isTemplatePlaceholderNode(node) {
    return node.name.startsWith("prettier-placeholder");
  }
  function isTemplatePropNode(node) {
    return node.prop.startsWith("@prettier-placeholder");
  }
  function isPostcssSimpleVarNode(currentNode, nextNode) {
    return currentNode.value === "$$" && currentNode.type === "value-func" && (nextNode == null ? void 0 : nextNode.type) === "value-word" && !nextNode.raws.before;
  }
  function hasComposesNode(node) {
    var _a, _b;
    return ((_a = node.value) == null ? void 0 : _a.type) === "value-root" && ((_b = node.value.group) == null ? void 0 : _b.type) === "value-value" && node.prop.toLowerCase() === "composes";
  }
  function hasParensAroundNode(node) {
    var _a, _b, _c;
    return ((_c = (_b = (_a = node.value) == null ? void 0 : _a.group) == null ? void 0 : _b.group) == null ? void 0 : _c.type) === "value-paren_group" && node.value.group.group.open !== null && node.value.group.group.close !== null;
  }
  function hasEmptyRawBefore(node) {
    var _a;
    return ((_a = node.raws) == null ? void 0 : _a.before) === "";
  }
  function isKeyValuePairNode(node) {
    var _a, _b;
    return node.type === "value-comma_group" && ((_b = (_a = node.groups) == null ? void 0 : _a[1]) == null ? void 0 : _b.type) === "value-colon";
  }
  function isKeyValuePairInParenGroupNode(node) {
    var _a;
    return node.type === "value-paren_group" && ((_a = node.groups) == null ? void 0 : _a[0]) && isKeyValuePairNode(node.groups[0]);
  }
  function isSCSSMapItemNode(path, options2) {
    var _a;
    if (options2.parser !== "scss") {
      return false;
    }
    const { node } = path;
    if (node.groups.length === 0) {
      return false;
    }
    const parentParentNode = path.grandparent;
    if (!isKeyValuePairInParenGroupNode(node) && !(parentParentNode && isKeyValuePairInParenGroupNode(parentParentNode))) {
      return false;
    }
    const declNode = path.findAncestor((node2) => node2.type === "css-decl");
    if ((_a = declNode == null ? void 0 : declNode.prop) == null ? void 0 : _a.startsWith("$")) {
      return true;
    }
    if (isKeyValuePairInParenGroupNode(parentParentNode)) {
      return true;
    }
    if (parentParentNode.type === "value-func") {
      return true;
    }
    return false;
  }
  function isInlineValueCommentNode(node) {
    return node.type === "value-comment" && node.inline;
  }
  function isHashNode(node) {
    return node.type === "value-word" && node.value === "#";
  }
  function isLeftCurlyBraceNode(node) {
    return node.type === "value-word" && node.value === "{";
  }
  function isRightCurlyBraceNode(node) {
    return node.type === "value-word" && node.value === "}";
  }
  function isWordNode(node) {
    return ["value-word", "value-atword"].includes(node.type);
  }
  function isColonNode(node) {
    return (node == null ? void 0 : node.type) === "value-colon";
  }
  function isKeyInValuePairNode(node, parentNode) {
    if (!isKeyValuePairNode(parentNode)) {
      return false;
    }
    const { groups } = parentNode;
    const index = groups.indexOf(node);
    if (index === -1) {
      return false;
    }
    return isColonNode(groups[index + 1]);
  }
  function isMediaAndSupportsKeywords(node) {
    return node.value && ["not", "and", "or"].includes(node.value.toLowerCase());
  }
  function isColorAdjusterFuncNode(node) {
    if (node.type !== "value-func") {
      return false;
    }
    return colorAdjusterFunctions.has(node.value.toLowerCase());
  }
  function lastLineHasInlineComment(text) {
    return /\/\//.test(text.split(/[\n\r]/).pop());
  }
  function isAtWordPlaceholderNode(node) {
    return (node == null ? void 0 : node.type) === "value-atword" && node.value.startsWith("prettier-placeholder-");
  }
  function isConfigurationNode(node, parentNode) {
    var _a, _b;
    if (((_a = node.open) == null ? void 0 : _a.value) !== "(" || ((_b = node.close) == null ? void 0 : _b.value) !== ")" || node.groups.some((group2) => group2.type !== "value-comma_group")) {
      return false;
    }
    if (parentNode.type === "value-comma_group") {
      const prevIdx = parentNode.groups.indexOf(node) - 1;
      const maybeWithNode = parentNode.groups[prevIdx];
      if ((maybeWithNode == null ? void 0 : maybeWithNode.type) === "value-word" && maybeWithNode.value === "with") {
        return true;
      }
    }
    return false;
  }
  function isParenGroupNode(node) {
    var _a, _b;
    return node.type === "value-paren_group" && ((_a = node.open) == null ? void 0 : _a.value) === "(" && ((_b = node.close) == null ? void 0 : _b.value) === ")";
  }

  // src/utils/skip.js
  function skip(characters) {
    return (text, startIndex, options2) => {
      const backwards = Boolean(options2 == null ? void 0 : options2.backwards);
      if (startIndex === false) {
        return false;
      }
      const { length } = text;
      let cursor = startIndex;
      while (cursor >= 0 && cursor < length) {
        const character = text.charAt(cursor);
        if (characters instanceof RegExp) {
          if (!characters.test(character)) {
            return cursor;
          }
        } else if (!characters.includes(character)) {
          return cursor;
        }
        backwards ? cursor-- : cursor++;
      }
      if (cursor === -1 || cursor === length) {
        return cursor;
      }
      return false;
    };
  }
  var skipWhitespace = skip(/\s/);
  var skipSpaces = skip(" 	");
  var skipToLineEnd = skip(",; 	");
  var skipEverythingButNewLine = skip(/[^\n\r]/);

  // src/utils/line-column-to-index.js
  function lineColumnToIndex(lineColumn, text) {
    let index = 0;
    for (let i = 0; i < lineColumn.line - 1; ++i) {
      index = text.indexOf("\n", index) + 1;
    }
    return index + lineColumn.column;
  }
  var line_column_to_index_default = lineColumnToIndex;

  // src/language-css/loc.js
  function calculateLocStart(node, text) {
    var _a, _b, _c;
    if (typeof ((_b = (_a = node.source) == null ? void 0 : _a.start) == null ? void 0 : _b.offset) === "number") {
      return node.source.start.offset;
    }
    if (typeof node.sourceIndex === "number") {
      return node.sourceIndex;
    }
    if ((_c = node.source) == null ? void 0 : _c.start) {
      return line_column_to_index_default(node.source.start, text);
    }
    throw Object.assign(new Error("Can not locate node."), {
      node
    });
  }
  function calculateLocEnd(node, text) {
    var _a, _b;
    if (node.type === "css-comment" && node.inline) {
      return skipEverythingButNewLine(text, node.source.startOffset);
    }
    if (typeof ((_b = (_a = node.source) == null ? void 0 : _a.end) == null ? void 0 : _b.offset) === "number") {
      return node.source.end.offset + 1;
    }
    if (node.source) {
      if (node.source.end) {
        return line_column_to_index_default(node.source.end, text);
      }
      if (is_non_empty_array_default(node.nodes)) {
        return calculateLocEnd(at_default(
          /* isOptionalObject*/
          false,
          node.nodes,
          -1
        ), text);
      }
    }
    return null;
  }
  function calculateLoc(node, text) {
    if (node.source) {
      node.source.startOffset = calculateLocStart(node, text);
      node.source.endOffset = calculateLocEnd(node, text);
    }
    for (const key in node) {
      const child = node[key];
      if (key === "source" || !child || typeof child !== "object") {
        continue;
      }
      if (child.type === "value-root" || child.type === "value-unknown") {
        calculateValueNodeLoc(child, getValueRootOffset(node), child.text || child.value);
      } else {
        calculateLoc(child, text);
      }
    }
  }
  function calculateValueNodeLoc(node, rootOffset, text) {
    if (node.source) {
      node.source.startOffset = calculateLocStart(node, text) + rootOffset;
      node.source.endOffset = calculateLocEnd(node, text) + rootOffset;
    }
    for (const key in node) {
      const child = node[key];
      if (key === "source" || !child || typeof child !== "object") {
        continue;
      }
      calculateValueNodeLoc(child, rootOffset, text);
    }
  }
  function getValueRootOffset(node) {
    var _a;
    let result = node.source.startOffset;
    if (typeof node.prop === "string") {
      result += node.prop.length;
    }
    if (node.type === "css-atrule" && typeof node.name === "string") {
      result += 1 + node.name.length + node.raws.afterName.match(/^\s*:?\s*/)[0].length;
    }
    if (node.type !== "css-atrule" && typeof ((_a = node.raws) == null ? void 0 : _a.between) === "string") {
      result += node.raws.between.length;
    }
    return result;
  }
  function replaceQuotesInInlineComments(text) {
    let state = "initial";
    let stateToReturnFromQuotes = "initial";
    let inlineCommentStartIndex;
    let inlineCommentContainsQuotes = false;
    const inlineCommentsToReplace = [];
    for (let i = 0; i < text.length; i++) {
      const c = text[i];
      switch (state) {
        case "initial":
          if (c === "'") {
            state = "single-quotes";
            continue;
          }
          if (c === '"') {
            state = "double-quotes";
            continue;
          }
          if ((c === "u" || c === "U") && text.slice(i, i + 4).toLowerCase() === "url(") {
            state = "url";
            i += 3;
            continue;
          }
          if (c === "*" && text[i - 1] === "/") {
            state = "comment-block";
            continue;
          }
          if (c === "/" && text[i - 1] === "/") {
            state = "comment-inline";
            inlineCommentStartIndex = i - 1;
            continue;
          }
          continue;
        case "single-quotes":
          if (c === "'" && text[i - 1] !== "\\") {
            state = stateToReturnFromQuotes;
            stateToReturnFromQuotes = "initial";
          }
          if (c === "\n" || c === "\r") {
            return text;
          }
          continue;
        case "double-quotes":
          if (c === '"' && text[i - 1] !== "\\") {
            state = stateToReturnFromQuotes;
            stateToReturnFromQuotes = "initial";
          }
          if (c === "\n" || c === "\r") {
            return text;
          }
          continue;
        case "url":
          if (c === ")") {
            state = "initial";
          }
          if (c === "\n" || c === "\r") {
            return text;
          }
          if (c === "'") {
            state = "single-quotes";
            stateToReturnFromQuotes = "url";
            continue;
          }
          if (c === '"') {
            state = "double-quotes";
            stateToReturnFromQuotes = "url";
            continue;
          }
          continue;
        case "comment-block":
          if (c === "/" && text[i - 1] === "*") {
            state = "initial";
          }
          continue;
        case "comment-inline":
          if (c === '"' || c === "'" || c === "*") {
            inlineCommentContainsQuotes = true;
          }
          if (c === "\n" || c === "\r") {
            if (inlineCommentContainsQuotes) {
              inlineCommentsToReplace.push([inlineCommentStartIndex, i]);
            }
            state = "initial";
            inlineCommentContainsQuotes = false;
          }
          continue;
      }
    }
    for (const [start, end] of inlineCommentsToReplace) {
      text = text.slice(0, start) + string_replace_all_default(
        /* isOptionalObject*/
        false,
        text.slice(start, end),
        /["'*]/g,
        " "
      ) + text.slice(end);
    }
    return text;
  }
  function locStart(node) {
    var _a;
    return (_a = node.source) == null ? void 0 : _a.startOffset;
  }
  function locEnd(node) {
    var _a;
    return (_a = node.source) == null ? void 0 : _a.endOffset;
  }

  // src/utils/print-number.js
  function printNumber(rawNumber) {
    return rawNumber.toLowerCase().replace(/^([+-]?[\d.]+e)(?:\+|(-))?0*(?=\d)/, "$1$2").replace(/^([+-]?[\d.]+)e[+-]?0+$/, "$1").replace(/^([+-])?\./, "$10.").replace(/(\.\d+?)0+(?=e|$)/, "$1").replace(/\.(?=e|$)/, "");
  }
  var print_number_default = printNumber;

  // src/language-css/print/css-units.evaluate.js
  var css_units_evaluate_default = /* @__PURE__ */ new Map([
    [
      "em",
      "em"
    ],
    [
      "rem",
      "rem"
    ],
    [
      "ex",
      "ex"
    ],
    [
      "rex",
      "rex"
    ],
    [
      "cap",
      "cap"
    ],
    [
      "rcap",
      "rcap"
    ],
    [
      "ch",
      "ch"
    ],
    [
      "rch",
      "rch"
    ],
    [
      "ic",
      "ic"
    ],
    [
      "ric",
      "ric"
    ],
    [
      "lh",
      "lh"
    ],
    [
      "rlh",
      "rlh"
    ],
    [
      "vw",
      "vw"
    ],
    [
      "svw",
      "svw"
    ],
    [
      "lvw",
      "lvw"
    ],
    [
      "dvw",
      "dvw"
    ],
    [
      "vh",
      "vh"
    ],
    [
      "svh",
      "svh"
    ],
    [
      "lvh",
      "lvh"
    ],
    [
      "dvh",
      "dvh"
    ],
    [
      "vi",
      "vi"
    ],
    [
      "svi",
      "svi"
    ],
    [
      "lvi",
      "lvi"
    ],
    [
      "dvi",
      "dvi"
    ],
    [
      "vb",
      "vb"
    ],
    [
      "svb",
      "svb"
    ],
    [
      "lvb",
      "lvb"
    ],
    [
      "dvb",
      "dvb"
    ],
    [
      "vmin",
      "vmin"
    ],
    [
      "svmin",
      "svmin"
    ],
    [
      "lvmin",
      "lvmin"
    ],
    [
      "dvmin",
      "dvmin"
    ],
    [
      "vmax",
      "vmax"
    ],
    [
      "svmax",
      "svmax"
    ],
    [
      "lvmax",
      "lvmax"
    ],
    [
      "dvmax",
      "dvmax"
    ],
    [
      "cm",
      "cm"
    ],
    [
      "mm",
      "mm"
    ],
    [
      "q",
      "Q"
    ],
    [
      "in",
      "in"
    ],
    [
      "pt",
      "pt"
    ],
    [
      "pc",
      "pc"
    ],
    [
      "px",
      "px"
    ],
    [
      "deg",
      "deg"
    ],
    [
      "grad",
      "grad"
    ],
    [
      "rad",
      "rad"
    ],
    [
      "turn",
      "turn"
    ],
    [
      "s",
      "s"
    ],
    [
      "ms",
      "ms"
    ],
    [
      "hz",
      "Hz"
    ],
    [
      "khz",
      "kHz"
    ],
    [
      "dpi",
      "dpi"
    ],
    [
      "dpcm",
      "dpcm"
    ],
    [
      "dppx",
      "dppx"
    ],
    [
      "x",
      "x"
    ],
    [
      "cqw",
      "cqw"
    ],
    [
      "cqh",
      "cqh"
    ],
    [
      "cqi",
      "cqi"
    ],
    [
      "cqb",
      "cqb"
    ],
    [
      "cqmin",
      "cqmin"
    ],
    [
      "cqmax",
      "cqmax"
    ]
  ]);

  // src/language-css/print/misc.js
  function printUnit(unit) {
    const lowercased = unit.toLowerCase();
    return css_units_evaluate_default.has(lowercased) ? css_units_evaluate_default.get(lowercased) : unit;
  }
  var STRING_REGEX = /(["'])(?:(?!\1)[^\\]|\\.)*\1/gs;
  var NUMBER_REGEX = /(?:\d*\.\d+|\d+\.?)(?:[Ee][+-]?\d+)?/g;
  var STANDARD_UNIT_REGEX = /[A-Za-z]+/g;
  var WORD_PART_REGEX = /[$@]?[A-Z_a-z\u0080-\uFFFF][\w\u0080-\uFFFF-]*/g;
  var ADJUST_NUMBERS_REGEX = new RegExp(STRING_REGEX.source + `|(${WORD_PART_REGEX.source})?(${NUMBER_REGEX.source})(${STANDARD_UNIT_REGEX.source})?`, "g");
  function adjustStrings(value, options2) {
    return string_replace_all_default(
      /* isOptionalObject*/
      false,
      value,
      STRING_REGEX,
      (match) => print_string_default(match, options2)
    );
  }
  function quoteAttributeValue(value, options2) {
    const quote = options2.singleQuote ? "'" : '"';
    return value.includes('"') || value.includes("'") ? value : quote + value + quote;
  }
  function adjustNumbers(value) {
    return string_replace_all_default(
      /* isOptionalObject*/
      false,
      value,
      ADJUST_NUMBERS_REGEX,
      (match, quote, wordPart, number, unit) => !wordPart && number ? printCssNumber(number) + maybeToLowerCase(unit || "") : match
    );
  }
  function printCssNumber(rawNumber) {
    return print_number_default(rawNumber).replace(/\.0(?=$|e)/, "");
  }
  function shouldPrintTrailingComma(options2) {
    return options2.trailingComma === "es5" || options2.trailingComma === "all";
  }

  // src/language-css/print/comma-separated-value-group.js
  function printCommaSeparatedValueGroup(path, options2, print3) {
    var _a;
    const { node } = path;
    const parentNode = path.parent;
    const parentParentNode = path.grandparent;
    const declAncestorProp = getPropOfDeclNode(path);
    const isGridValue = declAncestorProp && parentNode.type === "value-value" && (declAncestorProp === "grid" || declAncestorProp.startsWith("grid-template"));
    const atRuleAncestorNode = path.findAncestor(
      (node2) => node2.type === "css-atrule"
    );
    const isControlDirective = atRuleAncestorNode && isSCSSControlDirectiveNode(atRuleAncestorNode, options2);
    const hasInlineComment = node.groups.some(
      (node2) => isInlineValueCommentNode(node2)
    );
    const printed = path.map(print3, "groups");
    const parts = [];
    const insideURLFunction = insideValueFunctionNode(path, "url");
    let insideSCSSInterpolationInString = false;
    let didBreak = false;
    for (let i = 0; i < node.groups.length; ++i) {
      parts.push(printed[i]);
      const iPrevNode = node.groups[i - 1];
      const iNode = node.groups[i];
      const iNextNode = node.groups[i + 1];
      const iNextNextNode = node.groups[i + 2];
      if (insideURLFunction) {
        if (iNextNode && isAdditionNode(iNextNode) || isAdditionNode(iNode)) {
          parts.push(" ");
        }
        continue;
      }
      if (insideAtRuleNode(path, "forward") && iNode.type === "value-word" && iNode.value && iPrevNode !== void 0 && iPrevNode.type === "value-word" && iPrevNode.value === "as" && iNextNode.type === "value-operator" && iNextNode.value === "*") {
        continue;
      }
      if (!iNextNode) {
        continue;
      }
      if (iNode.type === "value-word" && iNode.value.endsWith("-") && isAtWordPlaceholderNode(iNextNode)) {
        continue;
      }
      if (iNode.type === "value-string" && iNode.quoted) {
        const positionOfOpeningInterpolation = iNode.value.lastIndexOf("#{");
        const positionOfClosingInterpolation = iNode.value.lastIndexOf("}");
        if (positionOfOpeningInterpolation !== -1 && positionOfClosingInterpolation !== -1) {
          insideSCSSInterpolationInString = positionOfOpeningInterpolation > positionOfClosingInterpolation;
        } else if (positionOfOpeningInterpolation !== -1) {
          insideSCSSInterpolationInString = true;
        } else if (positionOfClosingInterpolation !== -1) {
          insideSCSSInterpolationInString = false;
        }
      }
      if (insideSCSSInterpolationInString) {
        continue;
      }
      if (isColonNode(iNode) || isColonNode(iNextNode)) {
        continue;
      }
      if (iNode.type === "value-atword" && (iNode.value === "" || /*
          @var[ @notVarNested ][notVar]
          ^^^^^
          */
      iNode.value.endsWith("["))) {
        continue;
      }
      if (iNextNode.type === "value-word" && iNextNode.value.startsWith("]")) {
        continue;
      }
      if (iNode.value === "~") {
        continue;
      }
      if (iNode.type !== "value-string" && iNode.value && iNode.value.includes("\\") && iNextNode && iNextNode.type !== "value-comment") {
        continue;
      }
      if ((iPrevNode == null ? void 0 : iPrevNode.value) && iPrevNode.value.indexOf("\\") === iPrevNode.value.length - 1 && iNode.type === "value-operator" && iNode.value === "/") {
        continue;
      }
      if (iNode.value === "\\") {
        continue;
      }
      if (isPostcssSimpleVarNode(iNode, iNextNode)) {
        continue;
      }
      if (isHashNode(iNode) || isLeftCurlyBraceNode(iNode) || isRightCurlyBraceNode(iNextNode) || isLeftCurlyBraceNode(iNextNode) && hasEmptyRawBefore(iNextNode) || isRightCurlyBraceNode(iNode) && hasEmptyRawBefore(iNextNode)) {
        continue;
      }
      if (iNode.value === "--" && isHashNode(iNextNode)) {
        continue;
      }
      const isMathOperator = isMathOperatorNode(iNode);
      const isNextMathOperator = isMathOperatorNode(iNextNode);
      if ((isMathOperator && isHashNode(iNextNode) || isNextMathOperator && isRightCurlyBraceNode(iNode)) && hasEmptyRawBefore(iNextNode)) {
        continue;
      }
      if (!iPrevNode && isDivisionNode(iNode)) {
        continue;
      }
      if (insideValueFunctionNode(path, "calc") && (isAdditionNode(iNode) || isAdditionNode(iNextNode) || isSubtractionNode(iNode) || isSubtractionNode(iNextNode)) && hasEmptyRawBefore(iNextNode)) {
        continue;
      }
      const isColorAdjusterNode = (isAdditionNode(iNode) || isSubtractionNode(iNode)) && i === 0 && (iNextNode.type === "value-number" || iNextNode.isHex) && parentParentNode && isColorAdjusterFuncNode(parentParentNode) && !hasEmptyRawBefore(iNextNode);
      const requireSpaceBeforeOperator = (iNextNextNode == null ? void 0 : iNextNextNode.type) === "value-func" || iNextNextNode && isWordNode(iNextNextNode) || iNode.type === "value-func" || isWordNode(iNode);
      const requireSpaceAfterOperator = iNextNode.type === "value-func" || isWordNode(iNextNode) || (iPrevNode == null ? void 0 : iPrevNode.type) === "value-func" || iPrevNode && isWordNode(iPrevNode);
      if (options2.parser === "scss" && isMathOperator && iNode.value === "-" && iNextNode.type === "value-func") {
        parts.push(" ");
        continue;
      }
      if (!(isMultiplicationNode(iNextNode) || isMultiplicationNode(iNode)) && !insideValueFunctionNode(path, "calc") && !isColorAdjusterNode && (isDivisionNode(iNextNode) && !requireSpaceBeforeOperator || isDivisionNode(iNode) && !requireSpaceAfterOperator || isAdditionNode(iNextNode) && !requireSpaceBeforeOperator || isAdditionNode(iNode) && !requireSpaceAfterOperator || isSubtractionNode(iNextNode) || isSubtractionNode(iNode)) && (hasEmptyRawBefore(iNextNode) || isMathOperator && (!iPrevNode || iPrevNode && isMathOperatorNode(iPrevNode)))) {
        continue;
      }
      if ((options2.parser === "scss" || options2.parser === "less") && isMathOperator && iNode.value === "-" && isParenGroupNode(iNextNode) && locEnd(iNode) === locStart(iNextNode.open) && iNextNode.open.value === "(") {
        continue;
      }
      if (isInlineValueCommentNode(iNode)) {
        if (parentNode.type === "value-paren_group") {
          parts.push(dedent(hardline));
          continue;
        }
        parts.push(hardline);
        continue;
      }
      if (isControlDirective && (isEqualityOperatorNode(iNextNode) || isRelationalOperatorNode(iNextNode) || isIfElseKeywordNode(iNextNode) || isEachKeywordNode(iNode) || isForKeywordNode(iNode))) {
        parts.push(" ");
        continue;
      }
      if (atRuleAncestorNode && atRuleAncestorNode.name.toLowerCase() === "namespace") {
        parts.push(" ");
        continue;
      }
      if (isGridValue) {
        if (iNode.source && iNextNode.source && iNode.source.start.line !== iNextNode.source.start.line) {
          parts.push(hardline);
          didBreak = true;
        } else {
          parts.push(" ");
        }
        continue;
      }
      if (isNextMathOperator) {
        parts.push(" ");
        continue;
      }
      if ((iNextNode == null ? void 0 : iNextNode.value) === "...") {
        continue;
      }
      if (isAtWordPlaceholderNode(iNode) && isAtWordPlaceholderNode(iNextNode) && locEnd(iNode) === locStart(iNextNode)) {
        continue;
      }
      if (isAtWordPlaceholderNode(iNode) && isParenGroupNode(iNextNode) && locEnd(iNode) === locStart(iNextNode.open)) {
        parts.push(softline);
        continue;
      }
      if (iNode.value === "with" && isParenGroupNode(iNextNode)) {
        parts.push(" ");
        continue;
      }
      if (((_a = iNode.value) == null ? void 0 : _a.endsWith("#")) && iNextNode.value === "{" && isParenGroupNode(iNextNode.group)) {
        continue;
      }
      parts.push(line);
    }
    if (hasInlineComment) {
      parts.push(breakParent);
    }
    if (didBreak) {
      parts.unshift(hardline);
    }
    if (isControlDirective) {
      return group(indent(parts));
    }
    if (insideURLFunctionInImportAtRuleNode(path)) {
      return group(fill(parts));
    }
    return group(indent(fill(parts)));
  }
  var comma_separated_value_group_default = printCommaSeparatedValueGroup;

  // src/utils/skip-newline.js
  function skipNewline(text, startIndex, options2) {
    const backwards = Boolean(options2 == null ? void 0 : options2.backwards);
    if (startIndex === false) {
      return false;
    }
    const character = text.charAt(startIndex);
    if (backwards) {
      if (text.charAt(startIndex - 1) === "\r" && character === "\n") {
        return startIndex - 2;
      }
      if (character === "\n" || character === "\r" || character === "\u2028" || character === "\u2029") {
        return startIndex - 1;
      }
    } else {
      if (character === "\r" && text.charAt(startIndex + 1) === "\n") {
        return startIndex + 2;
      }
      if (character === "\n" || character === "\r" || character === "\u2028" || character === "\u2029") {
        return startIndex + 1;
      }
    }
    return startIndex;
  }
  var skip_newline_default = skipNewline;

  // src/utils/skip-inline-comment.js
  function skipInlineComment(text, startIndex) {
    if (startIndex === false) {
      return false;
    }
    if (text.charAt(startIndex) === "/" && text.charAt(startIndex + 1) === "*") {
      for (let i = startIndex + 2; i < text.length; ++i) {
        if (text.charAt(i) === "*" && text.charAt(i + 1) === "/") {
          return i + 2;
        }
      }
    }
    return startIndex;
  }
  var skip_inline_comment_default = skipInlineComment;

  // src/utils/skip-trailing-comment.js
  function skipTrailingComment(text, startIndex) {
    if (startIndex === false) {
      return false;
    }
    if (text.charAt(startIndex) === "/" && text.charAt(startIndex + 1) === "/") {
      return skipEverythingButNewLine(text, startIndex);
    }
    return startIndex;
  }
  var skip_trailing_comment_default = skipTrailingComment;

  // src/utils/has-newline.js
  function hasNewline(text, startIndex, options2 = {}) {
    const idx = skipSpaces(
      text,
      options2.backwards ? startIndex - 1 : startIndex,
      options2
    );
    const idx2 = skip_newline_default(text, idx, options2);
    return idx !== idx2;
  }
  var has_newline_default = hasNewline;

  // src/utils/is-next-line-empty.js
  function isNextLineEmpty(text, startIndex) {
    let oldIdx = null;
    let idx = startIndex;
    while (idx !== oldIdx) {
      oldIdx = idx;
      idx = skipToLineEnd(text, idx);
      idx = skip_inline_comment_default(text, idx);
      idx = skipSpaces(text, idx);
    }
    idx = skip_trailing_comment_default(text, idx);
    idx = skip_newline_default(text, idx);
    return idx !== false && has_newline_default(text, idx);
  }
  var is_next_line_empty_default = isNextLineEmpty;

  // src/language-css/print/parenthesized-value-group.js
  function hasComma({
    node,
    parent
  }, options2) {
    return Boolean(node.source && options2.originalText.slice(locStart(node), locStart(parent.close)).trimEnd().endsWith(","));
  }
  function printTrailingComma(path, options2) {
    if (isVarFunctionNode(path.grandparent) && hasComma(path, options2)) {
      return ",";
    }
    if (path.node.type !== "value-comment" && !(path.node.type === "value-comma_group" && path.node.groups.every((group2) => group2.type === "value-comment")) && shouldPrintTrailingComma(options2) && path.callParent(() => isSCSSMapItemNode(path, options2))) {
      return ifBreak(",");
    }
    return "";
  }
  function printParenthesizedValueGroup(path, options2, print3) {
    const {
      node,
      parent
    } = path;
    const groupDocs = path.map(({
      node: node2
    }) => typeof node2 === "string" ? node2 : print3(), "groups");
    if (parent && isURLFunctionNode(parent) && (node.groups.length === 1 || node.groups.length > 0 && node.groups[0].type === "value-comma_group" && node.groups[0].groups.length > 0 && node.groups[0].groups[0].type === "value-word" && node.groups[0].groups[0].value.startsWith("data:"))) {
      return [node.open ? print3("open") : "", join(",", groupDocs), node.close ? print3("close") : ""];
    }
    if (!node.open) {
      const forceHardLine = shouldBreakList(path);
      const parts2 = join([",", forceHardLine ? hardline : line], groupDocs);
      return indent(forceHardLine ? [hardline, parts2] : group(fill(parts2)));
    }
    const parts = path.map(({
      node: child,
      isLast,
      index
    }) => {
      var _a;
      let doc2 = groupDocs[index];
      if (isKeyValuePairNode(child) && child.type === "value-comma_group" && child.groups && child.groups[0].type !== "value-paren_group" && ((_a = child.groups[2]) == null ? void 0 : _a.type) === "value-paren_group") {
        const parts3 = getDocParts(doc2.contents.contents);
        parts3[1] = group(parts3[1]);
        doc2 = group(dedent(doc2));
      }
      const parts2 = [doc2, isLast ? printTrailingComma(path, options2) : ","];
      if (!isLast && child.type === "value-comma_group" && is_non_empty_array_default(child.groups)) {
        let last = at_default(
          /* isOptionalObject*/
          false,
          child.groups,
          -1
        );
        if (!last.source && last.close) {
          last = last.close;
        }
        if (last.source && is_next_line_empty_default(options2.originalText, locEnd(last))) {
          parts2.push(hardline);
        }
      }
      return parts2;
    }, "groups");
    const isKey = isKeyInValuePairNode(node, parent);
    const isConfiguration = isConfigurationNode(node, parent);
    const isSCSSMapItem = isSCSSMapItemNode(path, options2);
    const shouldBreak = isConfiguration || isSCSSMapItem && !isKey;
    const shouldDedent = isConfiguration || isKey;
    const doc = group([node.open ? print3("open") : "", indent([softline, join(line, parts)]), softline, node.close ? print3("close") : ""], {
      shouldBreak
    });
    return shouldDedent ? dedent(doc) : doc;
  }
  function shouldBreakList(path) {
    return path.match((node) => node.type === "value-paren_group" && !node.open && node.groups.some((node2) => node2.type === "value-comma_group"), (node, key) => key === "group" && node.type === "value-value", (node, key) => key === "group" && node.type === "value-root", (node, key) => key === "value" && (node.type === "css-decl" && !node.prop.startsWith("--") || node.type === "css-atrule" && node.variable));
  }

  // src/language-css/print/sequence.js
  function printSequence(path, options2, print3) {
    const parts = [];
    path.each(() => {
      const { node, previous } = path;
      if ((previous == null ? void 0 : previous.type) === "css-comment" && previous.text.trim() === "prettier-ignore") {
        parts.push(options2.originalText.slice(locStart(node), locEnd(node)));
      } else {
        parts.push(print3());
      }
      if (path.isLast) {
        return;
      }
      const { next } = path;
      if (next.type === "css-comment" && !has_newline_default(options2.originalText, locStart(next), {
        backwards: true
      }) && !is_front_matter_default(node) || next.type === "css-atrule" && next.name === "else" && node.type !== "css-comment") {
        parts.push(" ");
      } else {
        parts.push(options2.__isHTMLStyleAttribute ? line : hardline);
        if (is_next_line_empty_default(options2.originalText, locEnd(node)) && !is_front_matter_default(node)) {
          parts.push(hardline);
        }
      }
    }, "nodes");
    return parts;
  }
  var sequence_default = printSequence;

  // src/language-css/printer-postcss.js
  function genericPrint(path, options2, print3) {
    var _a, _b, _c, _d, _e, _f;
    const {
      node
    } = path;
    switch (node.type) {
      case "front-matter":
        return [node.raw, hardline];
      case "css-root": {
        const nodes = sequence_default(path, options2, print3);
        let after = node.raws.after.trim();
        if (after.startsWith(";")) {
          after = after.slice(1).trim();
        }
        return [node.frontMatter ? [print3("frontMatter"), hardline] : "", nodes, after ? ` ${after}` : "", node.nodes.length > 0 ? hardline : ""];
      }
      case "css-comment": {
        const isInlineComment = node.inline || node.raws.inline;
        const text = options2.originalText.slice(locStart(node), locEnd(node));
        return isInlineComment ? text.trimEnd() : text;
      }
      case "css-rule":
        return [print3("selector"), node.important ? " !important" : "", node.nodes ? [((_a = node.selector) == null ? void 0 : _a.type) === "selector-unknown" && lastLineHasInlineComment(node.selector.value) ? line : node.selector ? " " : "", "{", node.nodes.length > 0 ? indent([hardline, sequence_default(path, options2, print3)]) : "", hardline, "}", isDetachedRulesetDeclarationNode(node) ? ";" : ""] : ";"];
      case "css-decl": {
        const parentNode = path.parent;
        const {
          between: rawBetween
        } = node.raws;
        const trimmedBetween = rawBetween.trim();
        const isColon = trimmedBetween === ":";
        const isValueAllSpace = typeof node.value === "string" && /^ *$/.test(node.value);
        let value = typeof node.value === "string" ? node.value : print3("value");
        value = hasComposesNode(node) ? removeLines(value) : value;
        if (!isColon && lastLineHasInlineComment(trimmedBetween) && !(((_c = (_b = node.value) == null ? void 0 : _b.group) == null ? void 0 : _c.group) && path.call(() => shouldBreakList(path), "value", "group", "group"))) {
          value = indent([hardline, dedent(value)]);
        }
        return [
          string_replace_all_default(
            /* isOptionalObject*/
            false,
            node.raws.before,
            /[\s;]/g,
            ""
          ),
          // Less variable
          parentNode.type === "css-atrule" && parentNode.variable || insideICSSRuleNode(path) ? node.prop : maybeToLowerCase(node.prop),
          trimmedBetween.startsWith("//") ? " " : "",
          trimmedBetween,
          node.extend || isValueAllSpace ? "" : " ",
          options2.parser === "less" && node.extend && node.selector ? ["extend(", print3("selector"), ")"] : "",
          value,
          node.raws.important ? node.raws.important.replace(/\s*!\s*important/i, " !important") : node.important ? " !important" : "",
          node.raws.scssDefault ? node.raws.scssDefault.replace(/\s*!default/i, " !default") : node.scssDefault ? " !default" : "",
          node.raws.scssGlobal ? node.raws.scssGlobal.replace(/\s*!global/i, " !global") : node.scssGlobal ? " !global" : "",
          node.nodes ? [" {", indent([softline, sequence_default(path, options2, print3)]), softline, "}"] : isTemplatePropNode(node) && !parentNode.raws.semicolon && options2.originalText[locEnd(node) - 1] !== ";" ? "" : options2.__isHTMLStyleAttribute && isLastNode(path, node) ? ifBreak(";") : ";"
        ];
      }
      case "css-atrule": {
        const parentNode = path.parent;
        const isTemplatePlaceholderNodeWithoutSemiColon = isTemplatePlaceholderNode(node) && !parentNode.raws.semicolon && options2.originalText[locEnd(node) - 1] !== ";";
        if (options2.parser === "less") {
          if (node.mixin) {
            return [print3("selector"), node.important ? " !important" : "", isTemplatePlaceholderNodeWithoutSemiColon ? "" : ";"];
          }
          if (node.function) {
            return [node.name, typeof node.params === "string" ? node.params : print3("params"), isTemplatePlaceholderNodeWithoutSemiColon ? "" : ";"];
          }
          if (node.variable) {
            return ["@", node.name, ": ", node.value ? print3("value") : "", node.raws.between.trim() ? node.raws.between.trim() + " " : "", node.nodes ? ["{", indent([node.nodes.length > 0 ? softline : "", sequence_default(path, options2, print3)]), softline, "}"] : "", isTemplatePlaceholderNodeWithoutSemiColon ? "" : ";"];
          }
        }
        const isImportUnknownValueEndsWithSemiColon = node.name === "import" && ((_d = node.params) == null ? void 0 : _d.type) === "value-unknown" && node.params.value.endsWith(";");
        return [
          "@",
          // If a Less file ends up being parsed with the SCSS parser, Less
          // variable declarations will be parsed as at-rules with names ending
          // with a colon, so keep the original case then.
          isDetachedRulesetCallNode(node) || node.name.endsWith(":") ? node.name : maybeToLowerCase(node.name),
          node.params ? [isDetachedRulesetCallNode(node) ? "" : isTemplatePlaceholderNode(node) ? node.raws.afterName === "" ? "" : node.name.endsWith(":") ? " " : /^\s*\n\s*\n/.test(node.raws.afterName) ? [hardline, hardline] : /^\s*\n/.test(node.raws.afterName) ? hardline : " " : " ", typeof node.params === "string" ? node.params : print3("params")] : "",
          node.selector ? indent([" ", print3("selector")]) : "",
          node.value ? group([" ", print3("value"), isSCSSControlDirectiveNode(node, options2) ? hasParensAroundNode(node) ? " " : line : ""]) : node.name === "else" ? " " : "",
          node.nodes ? [isSCSSControlDirectiveNode(node, options2) ? "" : node.selector && !node.selector.nodes && typeof node.selector.value === "string" && lastLineHasInlineComment(node.selector.value) || !node.selector && typeof node.params === "string" && lastLineHasInlineComment(node.params) ? line : " ", "{", indent([node.nodes.length > 0 ? softline : "", sequence_default(path, options2, print3)]), softline, "}"] : isTemplatePlaceholderNodeWithoutSemiColon || isImportUnknownValueEndsWithSemiColon ? "" : ";"
        ];
      }
      case "media-query-list": {
        const parts = [];
        path.each(({
          node: node2
        }) => {
          if (node2.type === "media-query" && node2.value === "") {
            return;
          }
          parts.push(print3());
        }, "nodes");
        return group(indent(join(line, parts)));
      }
      case "media-query":
        return [join(" ", path.map(print3, "nodes")), isLastNode(path, node) ? "" : ","];
      case "media-type":
        return adjustNumbers(adjustStrings(node.value, options2));
      case "media-feature-expression":
        if (!node.nodes) {
          return node.value;
        }
        return ["(", ...path.map(print3, "nodes"), ")"];
      case "media-feature":
        return maybeToLowerCase(adjustStrings(string_replace_all_default(
          /* isOptionalObject*/
          false,
          node.value,
          / +/g,
          " "
        ), options2));
      case "media-colon":
        return [node.value, " "];
      case "media-value":
        return adjustNumbers(adjustStrings(node.value, options2));
      case "media-keyword":
        return adjustStrings(node.value, options2);
      case "media-url":
        return adjustStrings(string_replace_all_default(
          /* isOptionalObject*/
          false,
          string_replace_all_default(
            /* isOptionalObject*/
            false,
            node.value,
            /^url\(\s+/gi,
            "url("
          ),
          /\s+\)$/g,
          ")"
        ), options2);
      case "media-unknown":
        return node.value;
      case "selector-root":
        return group([insideAtRuleNode(path, "custom-selector") ? [path.findAncestor((node2) => node2.type === "css-atrule").customSelector, line] : "", join([",", insideAtRuleNode(path, ["extend", "custom-selector", "nest"]) ? line : hardline], path.map(print3, "nodes"))]);
      case "selector-selector":
        return group(indent(path.map(print3, "nodes")));
      case "selector-comment":
        return node.value;
      case "selector-string":
        return adjustStrings(node.value, options2);
      case "selector-tag":
        return [node.namespace ? [node.namespace === true ? "" : node.namespace.trim(), "|"] : "", ((_e = path.previous) == null ? void 0 : _e.type) === "selector-nesting" ? node.value : adjustNumbers(isKeyframeAtRuleKeywords(path, node.value) ? node.value.toLowerCase() : node.value)];
      case "selector-id":
        return ["#", node.value];
      case "selector-class":
        return [".", adjustNumbers(adjustStrings(node.value, options2))];
      case "selector-attribute":
        return ["[", node.namespace ? [node.namespace === true ? "" : node.namespace.trim(), "|"] : "", node.attribute.trim(), node.operator ?? "", node.value ? quoteAttributeValue(adjustStrings(node.value.trim(), options2), options2) : "", node.insensitive ? " i" : "", "]"];
      case "selector-combinator": {
        if (node.value === "+" || node.value === ">" || node.value === "~" || node.value === ">>>") {
          const parentNode = path.parent;
          const leading2 = parentNode.type === "selector-selector" && parentNode.nodes[0] === node ? "" : line;
          return [leading2, node.value, isLastNode(path, node) ? "" : " "];
        }
        const leading = node.value.trim().startsWith("(") ? line : "";
        const value = adjustNumbers(adjustStrings(node.value.trim(), options2)) || line;
        return [leading, value];
      }
      case "selector-universal":
        return [node.namespace ? [node.namespace === true ? "" : node.namespace.trim(), "|"] : "", node.value];
      case "selector-pseudo":
        return [maybeToLowerCase(node.value), is_non_empty_array_default(node.nodes) ? group(["(", indent([softline, join([",", line], path.map(print3, "nodes"))]), softline, ")"]) : ""];
      case "selector-nesting":
        return node.value;
      case "selector-unknown": {
        const ruleAncestorNode = path.findAncestor((node2) => node2.type === "css-rule");
        if (ruleAncestorNode == null ? void 0 : ruleAncestorNode.isSCSSNesterProperty) {
          return adjustNumbers(adjustStrings(maybeToLowerCase(node.value), options2));
        }
        const parentNode = path.parent;
        if ((_f = parentNode.raws) == null ? void 0 : _f.selector) {
          const start = locStart(parentNode);
          const end = start + parentNode.raws.selector.length;
          return options2.originalText.slice(start, end).trim();
        }
        const grandParent = path.grandparent;
        if (parentNode.type === "value-paren_group" && (grandParent == null ? void 0 : grandParent.type) === "value-func" && grandParent.value === "selector") {
          const start = locEnd(parentNode.open) + 1;
          const end = locStart(parentNode.close);
          const selector = options2.originalText.slice(start, end).trim();
          return lastLineHasInlineComment(selector) ? [breakParent, selector] : selector;
        }
        return node.value;
      }
      case "value-value":
      case "value-root":
        return print3("group");
      case "value-comment":
        return options2.originalText.slice(locStart(node), locEnd(node));
      case "value-comma_group":
        return comma_separated_value_group_default(path, options2, print3);
      case "value-paren_group":
        return printParenthesizedValueGroup(path, options2, print3);
      case "value-func":
        return [node.value, insideAtRuleNode(path, "supports") && isMediaAndSupportsKeywords(node) ? " " : "", print3("group")];
      case "value-paren":
        return node.value;
      case "value-number":
        return [printCssNumber(node.value), printUnit(node.unit)];
      case "value-operator":
        return node.value;
      case "value-word":
        if (node.isColor && node.isHex || isWideKeywords(node.value)) {
          return node.value.toLowerCase();
        }
        return node.value;
      case "value-colon": {
        const {
          previous
        } = path;
        return [
          node.value,
          // Don't add spaces on escaped colon `:`, e.g: grid-template-rows: [row-1-00\:00] auto;
          typeof (previous == null ? void 0 : previous.value) === "string" && previous.value.endsWith("\\") || // Don't add spaces on `:` in `url` function (i.e. `url(fbglyph: cross-outline, fig-white)`)
          insideValueFunctionNode(path, "url") ? "" : line
        ];
      }
      case "value-string":
        return print_string_default(node.raws.quote + node.value + node.raws.quote, options2);
      case "value-atword":
        return ["@", node.value];
      case "value-unicode-range":
        return node.value;
      case "value-unknown":
        return node.value;
      case "value-comma":
      default:
        throw new unexpected_node_error_default(node, "PostCSS");
    }
  }
  var printer = {
    print: genericPrint,
    embed: embed_default,
    insertPragma: insertPragma2,
    massageAstNode: clean_default,
    getVisitorKeys: get_visitor_keys_default
  };
  var printer_postcss_default = printer;

  // src/language-css/parser-postcss.js
  var parser_postcss_exports = {};
  __export(parser_postcss_exports, {
    css: () => css,
    less: () => less,
    scss: () => scss
  });
  var import_parse2 = __toESM(require_parse(), 1);
  var import_postcss_less = __toESM(require_lib(), 1);
  var import_scss_parse = __toESM(require_scss_parse(), 1);

  // src/common/parser-create-error.js
  function createError(message, options2) {
    const error = new SyntaxError(
      message + " (" + options2.loc.start.line + ":" + options2.loc.start.column + ")"
    );
    return Object.assign(error, options2);
  }
  var parser_create_error_default = createError;

  // src/language-css/utils/is-scss-nested-property-node.js
  function isSCSSNestedPropertyNode(node, options2) {
    if (options2.parser !== "scss") {
      return false;
    }
    if (!node.selector) {
      return false;
    }
    return node.selector.replace(/\/\*.*?\*\//, "").replace(/\/\/.*\n/, "").trim().endsWith(":");
  }
  var is_scss_nested_property_node_default = isSCSSNestedPropertyNode;

  // src/language-css/utils/is-module-rule-name.js
  var moduleRuleNames = /* @__PURE__ */ new Set(["import", "use", "forward"]);
  function isModuleRuleName(name) {
    return moduleRuleNames.has(name);
  }
  var is_module_rule_name_default = isModuleRuleName;

  // src/language-css/parse/parse-value.js
  var import_parser = __toESM(require_parser2(), 1);

  // src/language-css/utils/get-value-root.js
  var getValueRoot = (node) => {
    while (node.parent) {
      node = node.parent;
    }
    return node;
  };
  var get_value_root_default = getValueRoot;

  // src/language-css/utils/get-function-arguments-text.js
  function getFunctionArgumentsText(node) {
    return get_value_root_default(node).text.slice(node.group.open.sourceIndex + 1, node.group.close.sourceIndex).trim();
  }
  var get_function_arguments_text_default = getFunctionArgumentsText;

  // src/language-css/utils/has-scss-interpolation.js
  function hasSCSSInterpolation(groupList) {
    if (is_non_empty_array_default(groupList)) {
      for (let i = groupList.length - 1; i > 0; i--) {
        if (groupList[i].type === "word" && groupList[i].value === "{" && groupList[i - 1].type === "word" && groupList[i - 1].value.endsWith("#")) {
          return true;
        }
      }
    }
    return false;
  }
  var has_scss_interpolation_default = hasSCSSInterpolation;

  // src/language-css/utils/has-string-or-function.js
  function hasStringOrFunction(groupList) {
    return groupList.some(
      (group2) => group2.type === "string" || group2.type === "func" && // workaround false-positive func
      !group2.value.endsWith("\\")
    );
  }
  var has_string_or_function_default = hasStringOrFunction;

  // src/language-css/utils/is-scss-variable.js
  function isSCSSVariable(node, options2) {
    return Boolean(
      options2.parser === "scss" && (node == null ? void 0 : node.type) === "word" && node.value.startsWith("$")
    );
  }
  var is_scss_variable_default = isSCSSVariable;

  // src/language-css/parse/parse-selector.js
  var import_processor = __toESM(require_processor2(), 1);

  // src/language-css/parse/utils.js
  function addTypePrefix(node, prefix, skipPrefix) {
    if (node && typeof node === "object") {
      delete node.parent;
      for (const key in node) {
        addTypePrefix(node[key], prefix, skipPrefix);
        if (key === "type" && typeof node[key] === "string" && !node[key].startsWith(prefix) && (!skipPrefix || !skipPrefix.test(node[key]))) {
          node[key] = prefix + node[key];
        }
      }
    }
    return node;
  }
  function addMissingType(node) {
    if (node && typeof node === "object") {
      delete node.parent;
      for (const key in node) {
        addMissingType(node[key]);
      }
      if (!Array.isArray(node) && node.value && !node.type) {
        node.type = "unknown";
      }
    }
    return node;
  }

  // src/language-css/parse/parse-selector.js
  function parseSelector(selector) {
    if (/\/\/|\/\*/.test(selector)) {
      return {
        type: "selector-unknown",
        value: selector.trim()
      };
    }
    let result;
    try {
      new import_processor.default((selectors) => {
        result = selectors;
      }).process(selector);
    } catch {
      return {
        type: "selector-unknown",
        value: selector
      };
    }
    return addTypePrefix(result, "selector-");
  }
  var parse_selector_default = parseSelector;

  // src/language-css/parse/parse-value.js
  function parseValueNode(valueNode, options2) {
    var _a;
    const {
      nodes
    } = valueNode;
    let parenGroup = {
      open: null,
      close: null,
      groups: [],
      type: "paren_group"
    };
    const parenGroupStack = [parenGroup];
    const rootParenGroup = parenGroup;
    let commaGroup = {
      groups: [],
      type: "comma_group"
    };
    const commaGroupStack = [commaGroup];
    for (let i = 0; i < nodes.length; ++i) {
      const node = nodes[i];
      if (options2.parser === "scss" && node.type === "number" && node.unit === ".." && node.value.endsWith(".")) {
        node.value = node.value.slice(0, -1);
        node.unit = "...";
      }
      if (node.type === "func" && node.value === "selector") {
        node.group.groups = [parse_selector_default(get_value_root_default(valueNode).text.slice(node.group.open.sourceIndex + 1, node.group.close.sourceIndex))];
      }
      if (node.type === "func" && node.value === "url") {
        const groups = ((_a = node.group) == null ? void 0 : _a.groups) ?? [];
        let groupList = [];
        for (let i2 = 0; i2 < groups.length; i2++) {
          const group2 = groups[i2];
          if (group2.type === "comma_group") {
            groupList = [...groupList, ...group2.groups];
          } else {
            groupList.push(group2);
          }
        }
        if (has_scss_interpolation_default(groupList) || !has_string_or_function_default(groupList) && !is_scss_variable_default(groupList[0], options2)) {
          node.group.groups = [get_function_arguments_text_default(node)];
        }
      }
      if (node.type === "paren" && node.value === "(") {
        parenGroup = {
          open: node,
          close: null,
          groups: [],
          type: "paren_group"
        };
        parenGroupStack.push(parenGroup);
        commaGroup = {
          groups: [],
          type: "comma_group"
        };
        commaGroupStack.push(commaGroup);
      } else if (node.type === "paren" && node.value === ")") {
        if (commaGroup.groups.length > 0) {
          parenGroup.groups.push(commaGroup);
        }
        parenGroup.close = node;
        if (commaGroupStack.length === 1) {
          throw new Error("Unbalanced parenthesis");
        }
        commaGroupStack.pop();
        commaGroup = at_default(
          /* isOptionalObject*/
          false,
          commaGroupStack,
          -1
        );
        commaGroup.groups.push(parenGroup);
        parenGroupStack.pop();
        parenGroup = at_default(
          /* isOptionalObject*/
          false,
          parenGroupStack,
          -1
        );
      } else if (node.type === "comma") {
        parenGroup.groups.push(commaGroup);
        commaGroup = {
          groups: [],
          type: "comma_group"
        };
        commaGroupStack[commaGroupStack.length - 1] = commaGroup;
      } else {
        commaGroup.groups.push(node);
      }
    }
    if (commaGroup.groups.length > 0) {
      parenGroup.groups.push(commaGroup);
    }
    return rootParenGroup;
  }
  function flattenGroups(node) {
    if (node.type === "paren_group" && !node.open && !node.close && node.groups.length === 1) {
      return flattenGroups(node.groups[0]);
    }
    if (node.type === "comma_group" && node.groups.length === 1) {
      return flattenGroups(node.groups[0]);
    }
    if (node.type === "paren_group" || node.type === "comma_group") {
      return {
        ...node,
        groups: node.groups.map(flattenGroups)
      };
    }
    return node;
  }
  function parseNestedValue(node, options2) {
    if (node && typeof node === "object") {
      for (const key in node) {
        if (key !== "parent") {
          parseNestedValue(node[key], options2);
          if (key === "nodes") {
            node.group = flattenGroups(parseValueNode(node, options2));
            delete node[key];
          }
        }
      }
    }
    return node;
  }
  function parseValue(value, options2) {
    if (options2.parser === "less" && value.startsWith("~`")) {
      return {
        type: "value-unknown",
        value
      };
    }
    let result = null;
    try {
      result = new import_parser.default(value, {
        loose: true
      }).parse();
    } catch {
      return {
        type: "value-unknown",
        value
      };
    }
    result.text = value;
    const parsedResult = parseNestedValue(result, options2);
    return addTypePrefix(parsedResult, "value-", /^selector-/);
  }
  var parse_value_default = parseValue;

  // src/language-css/parse/parse-media-query.js
  var import_postcss_media_query_parser = __toESM(require_dist(), 1);
  var parse2 = import_postcss_media_query_parser.default.default;
  function parseMediaQuery(params) {
    let result;
    try {
      result = parse2(params);
    } catch {
      return {
        type: "selector-unknown",
        value: params
      };
    }
    return addTypePrefix(addMissingType(result), "media-");
  }
  var parse_media_query_default = parseMediaQuery;

  // src/language-css/parser-postcss.js
  var DEFAULT_SCSS_DIRECTIVE = /(\s*)(!default).*$/;
  var GLOBAL_SCSS_DIRECTIVE = /(\s*)(!global).*$/;
  function parseNestedCSS(node, options2) {
    var _a, _b;
    if (node && typeof node === "object") {
      delete node.parent;
      for (const key in node) {
        parseNestedCSS(node[key], options2);
      }
      if (!node.type) {
        return node;
      }
      node.raws ?? (node.raws = {});
      if (node.type === "css-decl" && typeof node.prop === "string" && node.prop.startsWith("--") && typeof node.value === "string" && node.value.startsWith("{")) {
        let rules;
        if (node.value.trimEnd().endsWith("}")) {
          const textBefore = options2.originalText.slice(0, node.source.start.offset);
          const nodeText = "a".repeat(node.prop.length) + options2.originalText.slice(node.source.start.offset + node.prop.length, node.source.end.offset + 1);
          const fakeContent = string_replace_all_default(
            /* isOptionalObject*/
            false,
            textBefore,
            /[^\n]/g,
            " "
          ) + nodeText;
          let parse3;
          if (options2.parser === "scss") {
            parse3 = parseScss;
          } else if (options2.parser === "less") {
            parse3 = parseLess;
          } else {
            parse3 = parseCss;
          }
          let ast;
          try {
            ast = parse3(fakeContent, {
              ...options2
            });
          } catch {
          }
          if (((_a = ast == null ? void 0 : ast.nodes) == null ? void 0 : _a.length) === 1 && ast.nodes[0].type === "css-rule") {
            rules = ast.nodes[0].nodes;
          }
        }
        if (rules) {
          node.value = {
            type: "css-rule",
            nodes: rules
          };
        } else {
          node.value = {
            type: "value-unknown",
            value: node.raws.value.raw
          };
        }
        return node;
      }
      let selector = "";
      if (typeof node.selector === "string") {
        selector = node.raws.selector ? node.raws.selector.scss ?? node.raws.selector.raw : node.selector;
        if (node.raws.between && node.raws.between.trim().length > 0) {
          selector += node.raws.between;
        }
        node.raws.selector = selector;
      }
      let value = "";
      if (typeof node.value === "string") {
        value = node.raws.value ? node.raws.value.scss ?? node.raws.value.raw : node.value;
        value = value.trim();
        node.raws.value = value;
      }
      let params = "";
      if (typeof node.params === "string") {
        params = node.raws.params ? node.raws.params.scss ?? node.raws.params.raw : node.params;
        if (node.raws.afterName && node.raws.afterName.trim().length > 0) {
          params = node.raws.afterName + params;
        }
        if (node.raws.between && node.raws.between.trim().length > 0) {
          params = params + node.raws.between;
        }
        params = params.trim();
        node.raws.params = params;
      }
      if (selector.trim().length > 0) {
        if (selector.startsWith("@") && selector.endsWith(":")) {
          return node;
        }
        if (node.mixin) {
          node.selector = parse_value_default(selector, options2);
          return node;
        }
        if (is_scss_nested_property_node_default(node, options2)) {
          node.isSCSSNesterProperty = true;
        }
        node.selector = parse_selector_default(selector);
        return node;
      }
      if (value.length > 0) {
        const defaultSCSSDirectiveIndex = value.match(DEFAULT_SCSS_DIRECTIVE);
        if (defaultSCSSDirectiveIndex) {
          value = value.slice(0, defaultSCSSDirectiveIndex.index);
          node.scssDefault = true;
          if (defaultSCSSDirectiveIndex[0].trim() !== "!default") {
            node.raws.scssDefault = defaultSCSSDirectiveIndex[0];
          }
        }
        const globalSCSSDirectiveIndex = value.match(GLOBAL_SCSS_DIRECTIVE);
        if (globalSCSSDirectiveIndex) {
          value = value.slice(0, globalSCSSDirectiveIndex.index);
          node.scssGlobal = true;
          if (globalSCSSDirectiveIndex[0].trim() !== "!global") {
            node.raws.scssGlobal = globalSCSSDirectiveIndex[0];
          }
        }
        if (value.startsWith("progid:")) {
          return {
            type: "value-unknown",
            value
          };
        }
        node.value = parse_value_default(value, options2);
      }
      if (options2.parser === "less" && node.type === "css-decl" && value.startsWith("extend(")) {
        node.extend || (node.extend = node.raws.between === ":");
        if (node.extend && !node.selector) {
          delete node.value;
          node.selector = parse_selector_default(value.slice("extend(".length, -1));
        }
      }
      if (node.type === "css-atrule") {
        if (options2.parser === "less") {
          if (node.mixin) {
            const source = node.raws.identifier + node.name + node.raws.afterName + node.raws.params;
            node.selector = parse_selector_default(source);
            delete node.params;
            return node;
          }
          if (node.function) {
            return node;
          }
        }
        if (options2.parser === "css" && node.name === "custom-selector") {
          const customSelector = node.params.match(/:--\S+\s+/)[0].trim();
          node.customSelector = customSelector;
          node.selector = parse_selector_default(node.params.slice(customSelector.length).trim());
          delete node.params;
          return node;
        }
        if (options2.parser === "less") {
          if (node.name.includes(":") && !node.params) {
            node.variable = true;
            const parts = node.name.split(":");
            node.name = parts[0];
            node.value = parse_value_default(parts.slice(1).join(":"), options2);
          }
          if (!["page", "nest", "keyframes"].includes(node.name) && ((_b = node.params) == null ? void 0 : _b[0]) === ":") {
            node.variable = true;
            const text = node.params.slice(1);
            if (text) {
              node.value = parse_value_default(text, options2);
            }
            node.raws.afterName += ":";
          }
          if (node.variable) {
            delete node.params;
            if (!node.value) {
              delete node.value;
            }
            return node;
          }
        }
      }
      if (node.type === "css-atrule" && params.length > 0) {
        const {
          name
        } = node;
        const lowercasedName = node.name.toLowerCase();
        if (name === "warn" || name === "error") {
          node.params = {
            type: "media-unknown",
            value: params
          };
          return node;
        }
        if (name === "extend" || name === "nest") {
          node.selector = parse_selector_default(params);
          delete node.params;
          return node;
        }
        if (name === "at-root") {
          if (/^\(\s*(?:without|with)\s*:.+\)$/s.test(params)) {
            node.params = parse_value_default(params, options2);
          } else {
            node.selector = parse_selector_default(params);
            delete node.params;
          }
          return node;
        }
        if (is_module_rule_name_default(lowercasedName)) {
          node.import = true;
          delete node.filename;
          node.params = parse_value_default(params, options2);
          return node;
        }
        if (["namespace", "supports", "if", "else", "for", "each", "while", "debug", "mixin", "include", "function", "return", "define-mixin", "add-mixin"].includes(name)) {
          params = params.replace(/(\$\S+?)(\s+)?\.{3}/, "$1...$2");
          params = params.replace(/^(?!if)(\S+)(\s+)\(/, "$1($2");
          node.value = parse_value_default(params, options2);
          delete node.params;
          return node;
        }
        if (["media", "custom-media"].includes(lowercasedName)) {
          if (params.includes("#{")) {
            return {
              type: "media-unknown",
              value: params
            };
          }
          node.params = parse_media_query_default(params);
          return node;
        }
        node.params = params;
        return node;
      }
    }
    return node;
  }
  function parseWithParser(parse3, text, options2) {
    const parsed = parse_default(text);
    const {
      frontMatter
    } = parsed;
    text = parsed.content;
    let result;
    try {
      result = parse3(text, {
        // Prevent file access https://github.com/postcss/postcss/blob/4f4e2932fc97e2c117e1a4b15f0272ed551ed59d/lib/previous-map.js#L18
        map: false
      });
    } catch (error) {
      const {
        name,
        reason,
        line: line2,
        column
      } = error;
      if (typeof line2 !== "number") {
        throw error;
      }
      throw parser_create_error_default(`${name}: ${reason}`, {
        loc: {
          start: {
            line: line2,
            column
          }
        },
        cause: error
      });
    }
    options2.originalText = text;
    result = parseNestedCSS(addTypePrefix(result, "css-"), options2);
    calculateLoc(result, text);
    if (frontMatter) {
      frontMatter.source = {
        startOffset: 0,
        endOffset: frontMatter.raw.length
      };
      result.frontMatter = frontMatter;
    }
    return result;
  }
  function parseCss(text, options2 = {}) {
    return parseWithParser(import_parse2.default.default, text, options2);
  }
  function parseLess(text, options2 = {}) {
    return parseWithParser(
      // Workaround for https://github.com/shellscape/postcss-less/issues/145
      // See comments for `replaceQuotesInInlineComments` in `loc.js`.
      (text2) => import_postcss_less.default.parse(replaceQuotesInInlineComments(text2)),
      text,
      options2
    );
  }
  function parseScss(text, options2 = {}) {
    return parseWithParser(import_scss_parse.default, text, options2);
  }
  var postCssParser = {
    astFormat: "postcss",
    hasPragma: hasPragma2,
    locStart,
    locEnd
  };
  var css = {
    ...postCssParser,
    parse: parseCss
  };
  var less = {
    ...postCssParser,
    parse: parseLess
  };
  var scss = {
    ...postCssParser,
    parse: parseScss
  };

  // src/language-css/languages.evaluate.js
  var languages_evaluate_default = [
    {
      "linguistLanguageId": 50,
      "name": "CSS",
      "type": "markup",
      "tmScope": "source.css",
      "aceMode": "css",
      "codemirrorMode": "css",
      "codemirrorMimeType": "text/css",
      "color": "#563d7c",
      "extensions": [
        ".css",
        ".wxss"
      ],
      "parsers": [
        "css"
      ],
      "vscodeLanguageIds": [
        "css"
      ]
    },
    {
      "linguistLanguageId": 262764437,
      "name": "PostCSS",
      "type": "markup",
      "color": "#dc3a0c",
      "tmScope": "source.postcss",
      "group": "CSS",
      "extensions": [
        ".pcss",
        ".postcss"
      ],
      "aceMode": "text",
      "parsers": [
        "css"
      ],
      "vscodeLanguageIds": [
        "postcss"
      ]
    },
    {
      "linguistLanguageId": 198,
      "name": "Less",
      "type": "markup",
      "color": "#1d365d",
      "aliases": [
        "less-css"
      ],
      "extensions": [
        ".less"
      ],
      "tmScope": "source.css.less",
      "aceMode": "less",
      "codemirrorMode": "css",
      "codemirrorMimeType": "text/css",
      "parsers": [
        "less"
      ],
      "vscodeLanguageIds": [
        "less"
      ]
    },
    {
      "linguistLanguageId": 329,
      "name": "SCSS",
      "type": "markup",
      "color": "#c6538c",
      "tmScope": "source.css.scss",
      "aceMode": "scss",
      "codemirrorMode": "css",
      "codemirrorMimeType": "text/x-scss",
      "extensions": [
        ".scss"
      ],
      "parsers": [
        "scss"
      ],
      "vscodeLanguageIds": [
        "scss"
      ]
    }
  ];

  // src/common/common-options.evaluate.js
  var common_options_evaluate_default = {
    "bracketSpacing": {
      "category": "Common",
      "type": "boolean",
      "default": true,
      "description": "Print spaces between brackets.",
      "oppositeDescription": "Do not print spaces between brackets."
    },
    "singleQuote": {
      "category": "Common",
      "type": "boolean",
      "default": false,
      "description": "Use single quotes instead of double quotes."
    },
    "proseWrap": {
      "category": "Common",
      "type": "choice",
      "default": "preserve",
      "description": "How to wrap prose.",
      "choices": [
        {
          "value": "always",
          "description": "Wrap prose if it exceeds the print width."
        },
        {
          "value": "never",
          "description": "Do not wrap prose."
        },
        {
          "value": "preserve",
          "description": "Wrap prose as-is."
        }
      ]
    },
    "bracketSameLine": {
      "category": "Common",
      "type": "boolean",
      "default": false,
      "description": "Put > of opening tags on the last line instead of on a new line."
    },
    "singleAttributePerLine": {
      "category": "Common",
      "type": "boolean",
      "default": false,
      "description": "Enforce single attribute per line in HTML, Vue and JSX."
    }
  };

  // src/language-css/options.js
  var options = {
    singleQuote: common_options_evaluate_default.singleQuote
  };
  var options_default = options;

  // src/language-css/index.js
  var printers = {
    postcss: printer_postcss_default
  };
  return __toCommonJS(postcss_exports);
});