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
    root.astToDoc = interopModuleDefault();
  }
})(function() {
  "use strict";
  var __create = Object.create;
  var __defProp = Object.defineProperty;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __getProtoOf = Object.getPrototypeOf;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
  var __esm = (fn, res) => function __init() {
    return fn && (res = (0, fn[__getOwnPropNames(fn)[0]])(fn = 0)), res;
  };
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
  var __accessCheck = (obj, member, msg) => {
    if (!member.has(obj))
      throw TypeError("Cannot " + msg);
  };
  var __privateAdd = (obj, member, value) => {
    if (member.has(obj))
      throw TypeError("Cannot add the same private member more than once");
    member instanceof WeakSet ? member.add(obj) : member.set(obj, value);
  };
  var __privateMethod = (obj, member, method) => {
    __accessCheck(obj, member, "access private method");
    return method;
  };

  // scripts/build/shims/chalk.cjs
  var require_chalk = __commonJS({
    "scripts/build/shims/chalk.cjs"(exports, module) {
      "use strict";
      var chalk4 = new Proxy(String, { get: () => chalk4 });
      module.exports = chalk4;
    }
  });

  // scripts/build/shims/babel-highlight.js
  var babel_highlight_exports = {};
  __export(babel_highlight_exports, {
    default: () => babel_highlight_default,
    shouldHighlight: () => shouldHighlight
  });
  var shouldHighlight, babel_highlight_default;
  var init_babel_highlight = __esm({
    "scripts/build/shims/babel-highlight.js"() {
      shouldHighlight = () => false;
      babel_highlight_default = String;
    }
  });

  // node_modules/@babel/code-frame/lib/index.js
  var require_lib = __commonJS({
    "node_modules/@babel/code-frame/lib/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.codeFrameColumns = codeFrameColumns2;
      exports.default = _default;
      var _highlight = (init_babel_highlight(), __toCommonJS(babel_highlight_exports));
      var _chalk2 = require_chalk();
      var chalk4 = _chalk2;
      var chalkWithForcedColor = void 0;
      function getChalk(forceColor) {
        if (forceColor) {
          var _chalkWithForcedColor;
          (_chalkWithForcedColor = chalkWithForcedColor) != null ? _chalkWithForcedColor : chalkWithForcedColor = new chalk4.constructor({
            enabled: true,
            level: 1
          });
          return chalkWithForcedColor;
        }
        return chalk4;
      }
      var deprecationWarningShown = false;
      function getDefs(chalk5) {
        return {
          gutter: chalk5.grey,
          marker: chalk5.red.bold,
          message: chalk5.red.bold
        };
      }
      var NEWLINE = /\r\n|[\n\r\u2028\u2029]/;
      function getMarkerLines(loc, source, opts) {
        const startLoc = Object.assign({
          column: 0,
          line: -1
        }, loc.start);
        const endLoc = Object.assign({}, startLoc, loc.end);
        const {
          linesAbove = 2,
          linesBelow = 3
        } = opts || {};
        const startLine = startLoc.line;
        const startColumn = startLoc.column;
        const endLine = endLoc.line;
        const endColumn = endLoc.column;
        let start = Math.max(startLine - (linesAbove + 1), 0);
        let end = Math.min(source.length, endLine + linesBelow);
        if (startLine === -1) {
          start = 0;
        }
        if (endLine === -1) {
          end = source.length;
        }
        const lineDiff = endLine - startLine;
        const markerLines = {};
        if (lineDiff) {
          for (let i = 0; i <= lineDiff; i++) {
            const lineNumber = i + startLine;
            if (!startColumn) {
              markerLines[lineNumber] = true;
            } else if (i === 0) {
              const sourceLength = source[lineNumber - 1].length;
              markerLines[lineNumber] = [startColumn, sourceLength - startColumn + 1];
            } else if (i === lineDiff) {
              markerLines[lineNumber] = [0, endColumn];
            } else {
              const sourceLength = source[lineNumber - i].length;
              markerLines[lineNumber] = [0, sourceLength];
            }
          }
        } else {
          if (startColumn === endColumn) {
            if (startColumn) {
              markerLines[startLine] = [startColumn, 0];
            } else {
              markerLines[startLine] = true;
            }
          } else {
            markerLines[startLine] = [startColumn, endColumn - startColumn];
          }
        }
        return {
          start,
          end,
          markerLines
        };
      }
      function codeFrameColumns2(rawLines, loc, opts = {}) {
        const highlighted = (opts.highlightCode || opts.forceColor) && (0, _highlight.shouldHighlight)(opts);
        const chalk5 = getChalk(opts.forceColor);
        const defs = getDefs(chalk5);
        const maybeHighlight = (chalkFn, string) => {
          return highlighted ? chalkFn(string) : string;
        };
        const lines = rawLines.split(NEWLINE);
        const {
          start,
          end,
          markerLines
        } = getMarkerLines(loc, lines, opts);
        const hasColumns = loc.start && typeof loc.start.column === "number";
        const numberMaxWidth = String(end).length;
        const highlightedLines = highlighted ? (0, _highlight.default)(rawLines, opts) : rawLines;
        let frame = highlightedLines.split(NEWLINE, end).slice(start, end).map((line2, index) => {
          const number = start + 1 + index;
          const paddedNumber = ` ${number}`.slice(-numberMaxWidth);
          const gutter = ` ${paddedNumber} |`;
          const hasMarker = markerLines[number];
          const lastMarkerLine = !markerLines[number + 1];
          if (hasMarker) {
            let markerLine = "";
            if (Array.isArray(hasMarker)) {
              const markerSpacing = line2.slice(0, Math.max(hasMarker[0] - 1, 0)).replace(/[^\t]/g, " ");
              const numberOfMarkers = hasMarker[1] || 1;
              markerLine = ["\n ", maybeHighlight(defs.gutter, gutter.replace(/\d/g, " ")), " ", markerSpacing, maybeHighlight(defs.marker, "^").repeat(numberOfMarkers)].join("");
              if (lastMarkerLine && opts.message) {
                markerLine += " " + maybeHighlight(defs.message, opts.message);
              }
            }
            return [maybeHighlight(defs.marker, ">"), maybeHighlight(defs.gutter, gutter), line2.length > 0 ? ` ${line2}` : "", markerLine].join("");
          } else {
            return ` ${maybeHighlight(defs.gutter, gutter)}${line2.length > 0 ? ` ${line2}` : ""}`;
          }
        }).join("\n");
        if (opts.message && !hasColumns) {
          frame = `${" ".repeat(numberMaxWidth + 1)}${opts.message}
${frame}`;
        }
        if (highlighted) {
          return chalk5.reset(frame);
        } else {
          return frame;
        }
      }
      function _default(rawLines, lineNumber, colNumber, opts = {}) {
        if (!deprecationWarningShown) {
          deprecationWarningShown = true;
          const message = "Passing lineNumber and colNumber is deprecated to @babel/code-frame. Please use `codeFrameColumns`.";
          if (void 0) {
            (void 0)(message, "DeprecationWarning");
          } else {
            const deprecationError = new Error(message);
            deprecationError.name = "DeprecationWarning";
            console.warn(new Error(message));
          }
        }
        colNumber = Math.max(colNumber, 0);
        const location = {
          start: {
            column: colNumber,
            line: lineNumber
          }
        };
        return codeFrameColumns2(rawLines, location, opts);
      }
    }
  });

  // src/main/ast-to-doc.js
  var ast_to_doc_exports = {};
  __export(ast_to_doc_exports, {
    prepareToPrint: () => prepareToPrint,
    printAstToDoc: () => printAstToDoc
  });

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

  // src/common/ast-path.js
  var _getNodeStackIndex, getNodeStackIndex_fn, _getAncestors, getAncestors_fn;
  var AstPath = class {
    constructor(value) {
      __privateAdd(this, _getNodeStackIndex);
      __privateAdd(this, _getAncestors);
      this.stack = [value];
    }
    /** @type {string | null} */
    get key() {
      const {
        stack,
        siblings
      } = this;
      return at_default(
        /* isOptionalObject*/
        false,
        stack,
        siblings === null ? -2 : -4
      ) ?? null;
    }
    /** @type {number | null} */
    get index() {
      return this.siblings === null ? null : at_default(
        /* isOptionalObject*/
        false,
        this.stack,
        -2
      );
    }
    /** @type {object} */
    get node() {
      return at_default(
        /* isOptionalObject*/
        false,
        this.stack,
        -1
      );
    }
    /** @type {object | null} */
    get parent() {
      return this.getNode(1);
    }
    /** @type {object | null} */
    get grandparent() {
      return this.getNode(2);
    }
    /** @type {boolean} */
    get isInArray() {
      return this.siblings !== null;
    }
    /** @type {object[] | null} */
    get siblings() {
      const {
        stack
      } = this;
      const maybeArray = at_default(
        /* isOptionalObject*/
        false,
        stack,
        -3
      );
      return Array.isArray(maybeArray) ? maybeArray : null;
    }
    /** @type {object | null} */
    get next() {
      const {
        siblings
      } = this;
      return siblings === null ? null : siblings[this.index + 1];
    }
    /** @type {object | null} */
    get previous() {
      const {
        siblings
      } = this;
      return siblings === null ? null : siblings[this.index - 1];
    }
    /** @type {boolean} */
    get isFirst() {
      return this.index === 0;
    }
    /** @type {boolean} */
    get isLast() {
      const {
        siblings,
        index
      } = this;
      return siblings !== null && index === siblings.length - 1;
    }
    /** @type {boolean} */
    get isRoot() {
      return this.stack.length === 1;
    }
    /** @type {object} */
    get root() {
      return this.stack[0];
    }
    /** @type {object[]} */
    get ancestors() {
      return [...__privateMethod(this, _getAncestors, getAncestors_fn).call(this)];
    }
    // The name of the current property is always the penultimate element of
    // this.stack, and always a string/number/symbol.
    getName() {
      const {
        stack
      } = this;
      const {
        length
      } = stack;
      if (length > 1) {
        return at_default(
          /* isOptionalObject*/
          false,
          stack,
          -2
        );
      }
      return null;
    }
    // The value of the current property is always the final element of
    // this.stack.
    getValue() {
      return at_default(
        /* isOptionalObject*/
        false,
        this.stack,
        -1
      );
    }
    getNode(count = 0) {
      const stackIndex = __privateMethod(this, _getNodeStackIndex, getNodeStackIndex_fn).call(this, count);
      return stackIndex === -1 ? null : this.stack[stackIndex];
    }
    getParentNode(count = 0) {
      return this.getNode(count + 1);
    }
    // Temporarily push properties named by string arguments given after the
    // callback function onto this.stack, then call the callback with a
    // reference to this (modified) AstPath object. Note that the stack will
    // be restored to its original state after the callback is finished, so it
    // is probably a mistake to retain a reference to the path.
    call(callback, ...names) {
      const {
        stack
      } = this;
      const {
        length
      } = stack;
      let value = at_default(
        /* isOptionalObject*/
        false,
        stack,
        -1
      );
      for (const name of names) {
        value = value[name];
        stack.push(name, value);
      }
      try {
        return callback(this);
      } finally {
        stack.length = length;
      }
    }
    callParent(callback, count = 0) {
      const stackIndex = __privateMethod(this, _getNodeStackIndex, getNodeStackIndex_fn).call(this, count + 1);
      const parentValues = this.stack.splice(stackIndex + 1);
      try {
        return callback(this);
      } finally {
        this.stack.push(...parentValues);
      }
    }
    // Similar to AstPath.prototype.call, except that the value obtained by
    // accessing this.getValue()[name1][name2]... should be array. The
    // callback will be called with a reference to this path object for each
    // element of the array.
    each(callback, ...names) {
      const {
        stack
      } = this;
      const {
        length
      } = stack;
      let value = at_default(
        /* isOptionalObject*/
        false,
        stack,
        -1
      );
      for (const name of names) {
        value = value[name];
        stack.push(name, value);
      }
      try {
        for (let i = 0; i < value.length; ++i) {
          stack.push(i, value[i]);
          callback(this, i, value);
          stack.length -= 2;
        }
      } finally {
        stack.length = length;
      }
    }
    // Similar to AstPath.prototype.each, except that the results of the
    // callback function invocations are stored in an array and returned at
    // the end of the iteration.
    map(callback, ...names) {
      const result = [];
      this.each((path, index, value) => {
        result[index] = callback(path, index, value);
      }, ...names);
      return result;
    }
    /**
     * @param {...(
     *   | ((node: any, name: string | null, number: number | null) => boolean)
     *   | undefined
     * )} predicates
     */
    match(...predicates) {
      let stackPointer = this.stack.length - 1;
      let name = null;
      let node = this.stack[stackPointer--];
      for (const predicate of predicates) {
        if (node === void 0) {
          return false;
        }
        let number = null;
        if (typeof name === "number") {
          number = name;
          name = this.stack[stackPointer--];
          node = this.stack[stackPointer--];
        }
        if (predicate && !predicate(node, name, number)) {
          return false;
        }
        name = this.stack[stackPointer--];
        node = this.stack[stackPointer--];
      }
      return true;
    }
    /**
     * Traverses the ancestors of the current node heading toward the tree root
     * until it finds a node that matches the provided predicate function. Will
     * return the first matching ancestor. If no such node exists, returns undefined.
     * @param {(node: any) => boolean} predicate
     * @internal Unstable API. Don't use in plugins for now.
     */
    findAncestor(predicate) {
      for (const node of __privateMethod(this, _getAncestors, getAncestors_fn).call(this)) {
        if (predicate(node)) {
          return node;
        }
      }
    }
    /**
     * Traverses the ancestors of the current node heading toward the tree root
     * until it finds a node that matches the provided predicate function.
     * returns true if matched node found.
     * @param {(node: any) => boolean} predicate
     * @returns {boolean}
     * @internal Unstable API. Don't use in plugins for now.
     */
    hasAncestor(predicate) {
      for (const node of __privateMethod(this, _getAncestors, getAncestors_fn).call(this)) {
        if (predicate(node)) {
          return true;
        }
      }
      return false;
    }
  };
  _getNodeStackIndex = new WeakSet();
  getNodeStackIndex_fn = function(count) {
    const {
      stack
    } = this;
    for (let i = stack.length - 1; i >= 0; i -= 2) {
      if (!Array.isArray(stack[i]) && --count < 0) {
        return i;
      }
    }
    return -1;
  };
  _getAncestors = new WeakSet();
  getAncestors_fn = function* () {
    const {
      stack
    } = this;
    for (let index = stack.length - 3; index >= 0; index -= 2) {
      const value = stack[index];
      if (!Array.isArray(value)) {
        yield value;
      }
    }
  };
  var ast_path_default = AstPath;

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

  // src/document/builders.js
  function lineSuffix(contents) {
    assertDoc(contents);
    return { type: DOC_TYPE_LINE_SUFFIX, contents };
  }
  var breakParent = { type: DOC_TYPE_BREAK_PARENT };
  var hardlineWithoutBreakParent = { type: DOC_TYPE_LINE, hard: true };
  var line = { type: DOC_TYPE_LINE };
  var hardline = [hardlineWithoutBreakParent, breakParent];
  var cursor = { type: DOC_TYPE_CURSOR };

  // src/document/utils.js
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
  function stripTrailingHardlineFromParts(parts) {
    parts = [...parts];
    while (parts.length >= 2 && at_default(
      /* isOptionalObject*/
      false,
      parts,
      -2
    ).type === DOC_TYPE_LINE && at_default(
      /* isOptionalObject*/
      false,
      parts,
      -1
    ).type === DOC_TYPE_BREAK_PARENT) {
      parts.length -= 2;
    }
    if (parts.length > 0) {
      const lastPart = stripTrailingHardlineFromDoc(at_default(
        /* isOptionalObject*/
        false,
        parts,
        -1
      ));
      parts[parts.length - 1] = lastPart;
    }
    return parts;
  }
  function stripTrailingHardlineFromDoc(doc) {
    switch (get_doc_type_default(doc)) {
      case DOC_TYPE_ALIGN:
      case DOC_TYPE_INDENT:
      case DOC_TYPE_INDENT_IF_BREAK:
      case DOC_TYPE_GROUP:
      case DOC_TYPE_LINE_SUFFIX:
      case DOC_TYPE_LABEL: {
        const contents = stripTrailingHardlineFromDoc(doc.contents);
        return {
          ...doc,
          contents
        };
      }
      case DOC_TYPE_IF_BREAK:
        return {
          ...doc,
          breakContents: stripTrailingHardlineFromDoc(doc.breakContents),
          flatContents: stripTrailingHardlineFromDoc(doc.flatContents)
        };
      case DOC_TYPE_FILL:
        return {
          ...doc,
          parts: stripTrailingHardlineFromParts(doc.parts)
        };
      case DOC_TYPE_ARRAY:
        return stripTrailingHardlineFromParts(doc);
      case DOC_TYPE_STRING:
        return doc.replace(/[\n\r]*$/, "");
      case DOC_TYPE_CURSOR:
      case DOC_TYPE_TRIM:
      case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
      case DOC_TYPE_LINE:
      case DOC_TYPE_BREAK_PARENT:
        break;
      default:
        throw new invalid_doc_error_default(doc);
    }
    return doc;
  }
  function stripTrailingHardline(doc) {
    return stripTrailingHardlineFromDoc(cleanDoc(doc));
  }
  function cleanDocFn(doc) {
    switch (get_doc_type_default(doc)) {
      case DOC_TYPE_FILL:
        if (doc.parts.every((part) => part === "")) {
          return "";
        }
        break;
      case DOC_TYPE_GROUP:
        if (!doc.contents && !doc.id && !doc.break && !doc.expandedStates) {
          return "";
        }
        if (doc.contents.type === DOC_TYPE_GROUP && doc.contents.id === doc.id && doc.contents.break === doc.break && doc.contents.expandedStates === doc.expandedStates) {
          return doc.contents;
        }
        break;
      case DOC_TYPE_ALIGN:
      case DOC_TYPE_INDENT:
      case DOC_TYPE_INDENT_IF_BREAK:
      case DOC_TYPE_LINE_SUFFIX:
        if (!doc.contents) {
          return "";
        }
        break;
      case DOC_TYPE_IF_BREAK:
        if (!doc.flatContents && !doc.breakContents) {
          return "";
        }
        break;
      case DOC_TYPE_ARRAY: {
        const parts = [];
        for (const part of doc) {
          if (!part) {
            continue;
          }
          const [currentPart, ...restParts] = Array.isArray(part) ? part : [part];
          if (typeof currentPart === "string" && typeof at_default(
            /* isOptionalObject*/
            false,
            parts,
            -1
          ) === "string") {
            parts[parts.length - 1] += currentPart;
          } else {
            parts.push(currentPart);
          }
          parts.push(...restParts);
        }
        if (parts.length === 0) {
          return "";
        }
        if (parts.length === 1) {
          return parts[0];
        }
        return parts;
      }
      case DOC_TYPE_STRING:
      case DOC_TYPE_CURSOR:
      case DOC_TYPE_TRIM:
      case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
      case DOC_TYPE_LINE:
      case DOC_TYPE_LABEL:
      case DOC_TYPE_BREAK_PARENT:
        break;
      default:
        throw new invalid_doc_error_default(doc);
    }
    return doc;
  }
  function cleanDoc(doc) {
    return mapDoc(doc, (currentDoc) => cleanDocFn(currentDoc));
  }
  function inheritLabel(doc, fn) {
    return doc.type === DOC_TYPE_LABEL ? {
      ...doc,
      contents: fn(doc.contents)
    } : fn(doc);
  }

  // scripts/build/shims/assert.js
  var assert = new Proxy(() => {
  }, { get: () => assert });
  var assert_default = assert;

  // src/utils/skip.js
  function skip(characters) {
    return (text, startIndex, options) => {
      const backwards = Boolean(options == null ? void 0 : options.backwards);
      if (startIndex === false) {
        return false;
      }
      const { length } = text;
      let cursor2 = startIndex;
      while (cursor2 >= 0 && cursor2 < length) {
        const character = text.charAt(cursor2);
        if (characters instanceof RegExp) {
          if (!characters.test(character)) {
            return cursor2;
          }
        } else if (!characters.includes(character)) {
          return cursor2;
        }
        backwards ? cursor2-- : cursor2++;
      }
      if (cursor2 === -1 || cursor2 === length) {
        return cursor2;
      }
      return false;
    };
  }
  var skipWhitespace = skip(/\s/);
  var skipSpaces = skip(" 	");
  var skipToLineEnd = skip(",; 	");
  var skipEverythingButNewLine = skip(/[^\n\r]/);

  // src/utils/skip-newline.js
  function skipNewline(text, startIndex, options) {
    const backwards = Boolean(options == null ? void 0 : options.backwards);
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

  // src/utils/has-newline.js
  function hasNewline(text, startIndex, options = {}) {
    const idx = skipSpaces(
      text,
      options.backwards ? startIndex - 1 : startIndex,
      options
    );
    const idx2 = skip_newline_default(text, idx, options);
    return idx !== idx2;
  }
  var has_newline_default = hasNewline;

  // src/utils/is-non-empty-array.js
  function isNonEmptyArray(object) {
    return Array.isArray(object) && object.length > 0;
  }
  var is_non_empty_array_default = isNonEmptyArray;

  // src/utils/is-object.js
  function isObject(object) {
    return object !== null && typeof object === "object";
  }
  var is_object_default = isObject;

  // src/utils/ast-utils.js
  function* getChildren(node, options) {
    const { getVisitorKeys, filter = () => true } = options;
    const isMatchedNode = (node2) => is_object_default(node2) && filter(node2);
    for (const key of getVisitorKeys(node)) {
      const value = node[key];
      if (Array.isArray(value)) {
        for (const child of value) {
          if (isMatchedNode(child)) {
            yield child;
          }
        }
      } else if (isMatchedNode(value)) {
        yield value;
      }
    }
  }

  // src/main/create-get-visitor-keys-function.js
  var nonTraversableKeys = /* @__PURE__ */ new Set([
    "tokens",
    "comments",
    "parent",
    "enclosingNode",
    "precedingNode",
    "followingNode"
  ]);
  var defaultGetVisitorKeys = (node) => Object.keys(node).filter((key) => !nonTraversableKeys.has(key));
  function createGetVisitorKeysFunction(printerGetVisitorKeys) {
    return printerGetVisitorKeys ? (node) => printerGetVisitorKeys(node, nonTraversableKeys) : defaultGetVisitorKeys;
  }
  var create_get_visitor_keys_function_default = createGetVisitorKeysFunction;

  // src/main/comments/utils.js
  function describeNodeForDebugging(node) {
    const nodeType = node.type || node.kind || "(unknown type)";
    let nodeName = String(
      node.name || node.id && (typeof node.id === "object" ? node.id.name : node.id) || node.key && (typeof node.key === "object" ? node.key.name : node.key) || node.value && (typeof node.value === "object" ? "" : String(node.value)) || node.operator || ""
    );
    if (nodeName.length > 20) {
      nodeName = nodeName.slice(0, 19) + "\u2026";
    }
    return nodeType + (nodeName ? " " + nodeName : "");
  }
  function addCommentHelper(node, comment) {
    const comments = node.comments ?? (node.comments = []);
    comments.push(comment);
    comment.printed = false;
    comment.nodeDescription = describeNodeForDebugging(node);
  }
  function addLeadingComment(node, comment) {
    comment.leading = true;
    comment.trailing = false;
    addCommentHelper(node, comment);
  }
  function addDanglingComment(node, comment, marker) {
    comment.leading = false;
    comment.trailing = false;
    if (marker) {
      comment.marker = marker;
    }
    addCommentHelper(node, comment);
  }
  function addTrailingComment(node, comment) {
    comment.leading = false;
    comment.trailing = true;
    addCommentHelper(node, comment);
  }

  // src/main/comments/attach.js
  var childNodesCache = /* @__PURE__ */ new WeakMap();
  function getSortedChildNodes(node, options) {
    if (childNodesCache.has(node)) {
      return childNodesCache.get(node);
    }
    const {
      printer: {
        getCommentChildNodes,
        canAttachComment,
        getVisitorKeys: printerGetVisitorKeys
      },
      locStart,
      locEnd
    } = options;
    if (!canAttachComment) {
      return [];
    }
    const childNodes = ((getCommentChildNodes == null ? void 0 : getCommentChildNodes(node, options)) ?? [
      ...getChildren(node, {
        getVisitorKeys: create_get_visitor_keys_function_default(printerGetVisitorKeys)
      })
    ]).flatMap(
      (node2) => canAttachComment(node2) ? [node2] : getSortedChildNodes(node2, options)
    );
    childNodes.sort(
      (nodeA, nodeB) => locStart(nodeA) - locStart(nodeB) || locEnd(nodeA) - locEnd(nodeB)
    );
    childNodesCache.set(node, childNodes);
    return childNodes;
  }
  function decorateComment(node, comment, options, enclosingNode) {
    const { locStart, locEnd } = options;
    const commentStart = locStart(comment);
    const commentEnd = locEnd(comment);
    const childNodes = getSortedChildNodes(node, options);
    let precedingNode;
    let followingNode;
    let left = 0;
    let right = childNodes.length;
    while (left < right) {
      const middle = left + right >> 1;
      const child = childNodes[middle];
      const start = locStart(child);
      const end = locEnd(child);
      if (start <= commentStart && commentEnd <= end) {
        return decorateComment(child, comment, options, child);
      }
      if (end <= commentStart) {
        precedingNode = child;
        left = middle + 1;
        continue;
      }
      if (commentEnd <= start) {
        followingNode = child;
        right = middle;
        continue;
      }
      throw new Error("Comment location overlaps with node location");
    }
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "TemplateLiteral") {
      const { quasis } = enclosingNode;
      const commentIndex = findExpressionIndexForComment(
        quasis,
        comment,
        options
      );
      if (precedingNode && findExpressionIndexForComment(quasis, precedingNode, options) !== commentIndex) {
        precedingNode = null;
      }
      if (followingNode && findExpressionIndexForComment(quasis, followingNode, options) !== commentIndex) {
        followingNode = null;
      }
    }
    return { enclosingNode, precedingNode, followingNode };
  }
  var returnFalse = () => false;
  function attachComments(ast, options) {
    const { comments } = ast;
    delete ast.comments;
    if (!is_non_empty_array_default(comments) || !options.printer.canAttachComment) {
      return;
    }
    const tiesToBreak = [];
    const {
      locStart,
      locEnd,
      printer: {
        experimentalFeatures: {
          // TODO: Make this as default behavior
          avoidAstMutation = false
        } = {},
        handleComments = {}
      },
      originalText: text
    } = options;
    const {
      ownLine: handleOwnLineComment = returnFalse,
      endOfLine: handleEndOfLineComment = returnFalse,
      remaining: handleRemainingComment = returnFalse
    } = handleComments;
    const decoratedComments = comments.map((comment, index) => ({
      ...decorateComment(ast, comment, options),
      comment,
      text,
      options,
      ast,
      isLastComment: comments.length - 1 === index
    }));
    for (const [index, context] of decoratedComments.entries()) {
      const {
        comment,
        precedingNode,
        enclosingNode,
        followingNode,
        text: text2,
        options: options2,
        ast: ast2,
        isLastComment
      } = context;
      if (options2.parser === "json" || options2.parser === "json5" || options2.parser === "__js_expression" || options2.parser === "__ts_expression" || options2.parser === "__vue_expression" || options2.parser === "__vue_ts_expression") {
        if (locStart(comment) - locStart(ast2) <= 0) {
          addLeadingComment(ast2, comment);
          continue;
        }
        if (locEnd(comment) - locEnd(ast2) >= 0) {
          addTrailingComment(ast2, comment);
          continue;
        }
      }
      let args;
      if (avoidAstMutation) {
        args = [context];
      } else {
        comment.enclosingNode = enclosingNode;
        comment.precedingNode = precedingNode;
        comment.followingNode = followingNode;
        args = [comment, text2, options2, ast2, isLastComment];
      }
      if (isOwnLineComment(text2, options2, decoratedComments, index)) {
        comment.placement = "ownLine";
        if (handleOwnLineComment(...args)) {
        } else if (followingNode) {
          addLeadingComment(followingNode, comment);
        } else if (precedingNode) {
          addTrailingComment(precedingNode, comment);
        } else if (enclosingNode) {
          addDanglingComment(enclosingNode, comment);
        } else {
          addDanglingComment(ast2, comment);
        }
      } else if (isEndOfLineComment(text2, options2, decoratedComments, index)) {
        comment.placement = "endOfLine";
        if (handleEndOfLineComment(...args)) {
        } else if (precedingNode) {
          addTrailingComment(precedingNode, comment);
        } else if (followingNode) {
          addLeadingComment(followingNode, comment);
        } else if (enclosingNode) {
          addDanglingComment(enclosingNode, comment);
        } else {
          addDanglingComment(ast2, comment);
        }
      } else {
        comment.placement = "remaining";
        if (handleRemainingComment(...args)) {
        } else if (precedingNode && followingNode) {
          const tieCount = tiesToBreak.length;
          if (tieCount > 0) {
            const lastTie = tiesToBreak[tieCount - 1];
            if (lastTie.followingNode !== followingNode) {
              breakTies(tiesToBreak, options2);
            }
          }
          tiesToBreak.push(context);
        } else if (precedingNode) {
          addTrailingComment(precedingNode, comment);
        } else if (followingNode) {
          addLeadingComment(followingNode, comment);
        } else if (enclosingNode) {
          addDanglingComment(enclosingNode, comment);
        } else {
          addDanglingComment(ast2, comment);
        }
      }
    }
    breakTies(tiesToBreak, options);
    if (!avoidAstMutation) {
      for (const comment of comments) {
        delete comment.precedingNode;
        delete comment.enclosingNode;
        delete comment.followingNode;
      }
    }
  }
  var isAllEmptyAndNoLineBreak = (text) => !/[\S\n\u2028\u2029]/.test(text);
  function isOwnLineComment(text, options, decoratedComments, commentIndex) {
    const { comment, precedingNode } = decoratedComments[commentIndex];
    const { locStart, locEnd } = options;
    let start = locStart(comment);
    if (precedingNode) {
      for (let index = commentIndex - 1; index >= 0; index--) {
        const { comment: comment2, precedingNode: currentCommentPrecedingNode } = decoratedComments[index];
        if (currentCommentPrecedingNode !== precedingNode || !isAllEmptyAndNoLineBreak(text.slice(locEnd(comment2), start))) {
          break;
        }
        start = locStart(comment2);
      }
    }
    return has_newline_default(text, start, { backwards: true });
  }
  function isEndOfLineComment(text, options, decoratedComments, commentIndex) {
    const { comment, followingNode } = decoratedComments[commentIndex];
    const { locStart, locEnd } = options;
    let end = locEnd(comment);
    if (followingNode) {
      for (let index = commentIndex + 1; index < decoratedComments.length; index++) {
        const { comment: comment2, followingNode: currentCommentFollowingNode } = decoratedComments[index];
        if (currentCommentFollowingNode !== followingNode || !isAllEmptyAndNoLineBreak(text.slice(end, locStart(comment2)))) {
          break;
        }
        end = locEnd(comment2);
      }
    }
    return has_newline_default(text, end);
  }
  function breakTies(tiesToBreak, options) {
    var _a, _b;
    const tieCount = tiesToBreak.length;
    if (tieCount === 0) {
      return;
    }
    const { precedingNode, followingNode } = tiesToBreak[0];
    let gapEndPos = options.locStart(followingNode);
    let indexOfFirstLeadingComment;
    for (indexOfFirstLeadingComment = tieCount; indexOfFirstLeadingComment > 0; --indexOfFirstLeadingComment) {
      const {
        comment,
        precedingNode: currentCommentPrecedingNode,
        followingNode: currentCommentFollowingNode
      } = tiesToBreak[indexOfFirstLeadingComment - 1];
      assert_default.strictEqual(currentCommentPrecedingNode, precedingNode);
      assert_default.strictEqual(currentCommentFollowingNode, followingNode);
      const gap = options.originalText.slice(options.locEnd(comment), gapEndPos);
      if (((_b = (_a = options.printer).isGap) == null ? void 0 : _b.call(_a, gap, options)) ?? /^[\s(]*$/.test(gap)) {
        gapEndPos = options.locStart(comment);
      } else {
        break;
      }
    }
    for (const [i, { comment }] of tiesToBreak.entries()) {
      if (i < indexOfFirstLeadingComment) {
        addTrailingComment(precedingNode, comment);
      } else {
        addLeadingComment(followingNode, comment);
      }
    }
    for (const node of [precedingNode, followingNode]) {
      if (node.comments && node.comments.length > 1) {
        node.comments.sort((a, b) => options.locStart(a) - options.locStart(b));
      }
    }
    tiesToBreak.length = 0;
  }
  function findExpressionIndexForComment(quasis, comment, options) {
    const startPos = options.locStart(comment) - 1;
    for (let i = 1; i < quasis.length; ++i) {
      if (startPos < options.locStart(quasis[i])) {
        return i - 1;
      }
    }
    return 0;
  }

  // src/utils/is-previous-line-empty.js
  function isPreviousLineEmpty(text, startIndex) {
    let idx = startIndex - 1;
    idx = skipSpaces(text, idx, { backwards: true });
    idx = skip_newline_default(text, idx, { backwards: true });
    idx = skipSpaces(text, idx, { backwards: true });
    const idx2 = skip_newline_default(text, idx, { backwards: true });
    return idx !== idx2;
  }
  var is_previous_line_empty_default = isPreviousLineEmpty;

  // src/main/comments/print.js
  function printComment(path, options) {
    const comment = path.node;
    comment.printed = true;
    return options.printer.printComment(path, options);
  }
  function printLeadingComment(path, options) {
    var _a;
    const comment = path.node;
    const parts = [printComment(path, options)];
    const { printer, originalText, locStart, locEnd } = options;
    const isBlock = (_a = printer.isBlockComment) == null ? void 0 : _a.call(printer, comment);
    if (isBlock) {
      const lineBreak = has_newline_default(originalText, locEnd(comment)) ? has_newline_default(originalText, locStart(comment), {
        backwards: true
      }) ? hardline : line : " ";
      parts.push(lineBreak);
    } else {
      parts.push(hardline);
    }
    const index = skip_newline_default(
      originalText,
      skipSpaces(originalText, locEnd(comment))
    );
    if (index !== false && has_newline_default(originalText, index)) {
      parts.push(hardline);
    }
    return parts;
  }
  function printTrailingComment(path, options, previousComment) {
    var _a;
    const comment = path.node;
    const printed = printComment(path, options);
    const { printer, originalText, locStart } = options;
    const isBlock = (_a = printer.isBlockComment) == null ? void 0 : _a.call(printer, comment);
    if ((previousComment == null ? void 0 : previousComment.hasLineSuffix) && !(previousComment == null ? void 0 : previousComment.isBlock) || has_newline_default(originalText, locStart(comment), { backwards: true })) {
      const isLineBeforeEmpty = is_previous_line_empty_default(
        originalText,
        locStart(comment)
      );
      return {
        doc: lineSuffix([hardline, isLineBeforeEmpty ? hardline : "", printed]),
        isBlock,
        hasLineSuffix: true
      };
    }
    if (!isBlock || (previousComment == null ? void 0 : previousComment.hasLineSuffix)) {
      return {
        doc: [lineSuffix([" ", printed]), breakParent],
        isBlock,
        hasLineSuffix: true
      };
    }
    return { doc: [" ", printed], isBlock, hasLineSuffix: false };
  }
  function printCommentsSeparately(path, options) {
    const value = path.node;
    if (!value) {
      return {};
    }
    const ignored = options[Symbol.for("printedComments")];
    const comments = (value.comments || []).filter(
      (comment) => !ignored.has(comment)
    );
    if (comments.length === 0) {
      return { leading: "", trailing: "" };
    }
    const leadingParts = [];
    const trailingParts = [];
    let printedTrailingComment;
    path.each(() => {
      const comment = path.node;
      if (ignored == null ? void 0 : ignored.has(comment)) {
        return;
      }
      const { leading, trailing } = comment;
      if (leading) {
        leadingParts.push(printLeadingComment(path, options));
      } else if (trailing) {
        printedTrailingComment = printTrailingComment(
          path,
          options,
          printedTrailingComment
        );
        trailingParts.push(printedTrailingComment.doc);
      }
    }, "comments");
    return { leading: leadingParts, trailing: trailingParts };
  }
  function printComments(path, doc, options) {
    const { leading, trailing } = printCommentsSeparately(path, options);
    if (!leading && !trailing) {
      return doc;
    }
    return inheritLabel(doc, (doc2) => [leading, doc2, trailing]);
  }
  function ensureAllCommentsPrinted(options) {
    const {
      [Symbol.for("comments")]: comments,
      [Symbol.for("printedComments")]: printedComments
    } = options;
    for (const comment of comments) {
      if (!comment.printed && !printedComments.has(comment)) {
        throw new Error(
          'Comment "' + comment.value.trim() + '" was not printed. Please report this error!'
        );
      }
      delete comment.printed;
    }
  }

  // src/common/errors.js
  var ConfigError = class extends Error {
    name = "ConfigError";
  };
  var UndefinedParserError = class extends Error {
    name = "UndefinedParserError";
  };

  // src/main/core-options.evaluate.js
  var core_options_evaluate_default = {
    "cursorOffset": {
      "category": "Special",
      "type": "int",
      "default": -1,
      "range": {
        "start": -1,
        "end": Infinity,
        "step": 1
      },
      "description": "Print (to stderr) where a cursor at the given position would move to after formatting.\nThis option cannot be used with --range-start and --range-end.",
      "cliCategory": "Editor"
    },
    "endOfLine": {
      "category": "Global",
      "type": "choice",
      "default": "lf",
      "description": "Which end of line characters to apply.",
      "choices": [
        {
          "value": "lf",
          "description": "Line Feed only (\\n), common on Linux and macOS as well as inside git repos"
        },
        {
          "value": "crlf",
          "description": "Carriage Return + Line Feed characters (\\r\\n), common on Windows"
        },
        {
          "value": "cr",
          "description": "Carriage Return character only (\\r), used very rarely"
        },
        {
          "value": "auto",
          "description": "Maintain existing\n(mixed values within one file are normalised by looking at what's used after the first line)"
        }
      ]
    },
    "filepath": {
      "category": "Special",
      "type": "path",
      "description": "Specify the input filepath. This will be used to do parser inference.",
      "cliName": "stdin-filepath",
      "cliCategory": "Other",
      "cliDescription": "Path to the file to pretend that stdin comes from."
    },
    "insertPragma": {
      "category": "Special",
      "type": "boolean",
      "default": false,
      "description": "Insert @format pragma into file's first docblock comment.",
      "cliCategory": "Other"
    },
    "parser": {
      "category": "Global",
      "type": "choice",
      "default": void 0,
      "description": "Which parser to use.",
      "exception": (value) => typeof value === "string" || typeof value === "function",
      "choices": [
        {
          "value": "flow",
          "description": "Flow"
        },
        {
          "value": "babel",
          "description": "JavaScript"
        },
        {
          "value": "babel-flow",
          "description": "Flow"
        },
        {
          "value": "babel-ts",
          "description": "TypeScript"
        },
        {
          "value": "typescript",
          "description": "TypeScript"
        },
        {
          "value": "acorn",
          "description": "JavaScript"
        },
        {
          "value": "espree",
          "description": "JavaScript"
        },
        {
          "value": "meriyah",
          "description": "JavaScript"
        },
        {
          "value": "css",
          "description": "CSS"
        },
        {
          "value": "less",
          "description": "Less"
        },
        {
          "value": "scss",
          "description": "SCSS"
        },
        {
          "value": "json",
          "description": "JSON"
        },
        {
          "value": "json5",
          "description": "JSON5"
        },
        {
          "value": "json-stringify",
          "description": "JSON.stringify"
        },
        {
          "value": "graphql",
          "description": "GraphQL"
        },
        {
          "value": "markdown",
          "description": "Markdown"
        },
        {
          "value": "mdx",
          "description": "MDX"
        },
        {
          "value": "vue",
          "description": "Vue"
        },
        {
          "value": "yaml",
          "description": "YAML"
        },
        {
          "value": "glimmer",
          "description": "Ember / Handlebars"
        },
        {
          "value": "html",
          "description": "HTML"
        },
        {
          "value": "angular",
          "description": "Angular"
        },
        {
          "value": "lwc",
          "description": "Lightning Web Components"
        }
      ]
    },
    "plugins": {
      "type": "path",
      "array": true,
      "default": [
        {
          "value": []
        }
      ],
      "category": "Global",
      "description": "Add a plugin. Multiple plugins can be passed as separate `--plugin`s.",
      "exception": (value) => typeof value === "string" || typeof value === "object",
      "cliName": "plugin",
      "cliCategory": "Config"
    },
    "printWidth": {
      "category": "Global",
      "type": "int",
      "default": 80,
      "description": "The line length where Prettier will try wrap.",
      "range": {
        "start": 0,
        "end": Infinity,
        "step": 1
      }
    },
    "rangeEnd": {
      "category": "Special",
      "type": "int",
      "default": Infinity,
      "range": {
        "start": 0,
        "end": Infinity,
        "step": 1
      },
      "description": "Format code ending at a given character offset (exclusive).\nThe range will extend forwards to the end of the selected statement.\nThis option cannot be used with --cursor-offset.",
      "cliCategory": "Editor"
    },
    "rangeStart": {
      "category": "Special",
      "type": "int",
      "default": 0,
      "range": {
        "start": 0,
        "end": Infinity,
        "step": 1
      },
      "description": "Format code starting at a given character offset.\nThe range will extend backwards to the start of the first line containing the selected statement.\nThis option cannot be used with --cursor-offset.",
      "cliCategory": "Editor"
    },
    "requirePragma": {
      "category": "Special",
      "type": "boolean",
      "default": false,
      "description": "Require either '@prettier' or '@format' to be present in the file's first docblock comment\nin order for it to be formatted.",
      "cliCategory": "Other"
    },
    "tabWidth": {
      "type": "int",
      "category": "Global",
      "default": 2,
      "description": "Number of spaces per indentation level.",
      "range": {
        "start": 0,
        "end": Infinity,
        "step": 1
      }
    },
    "useTabs": {
      "category": "Global",
      "type": "boolean",
      "default": false,
      "description": "Indent with tabs instead of spaces."
    },
    "embeddedLanguageFormatting": {
      "category": "Global",
      "type": "choice",
      "default": "auto",
      "description": "Control how Prettier formats quoted code embedded in the file.",
      "choices": [
        {
          "value": "auto",
          "description": "Format embedded code if Prettier can automatically identify it."
        },
        {
          "value": "off",
          "description": "Never automatically format embedded code."
        }
      ]
    }
  };

  // src/main/support.js
  function getSupportInfo({
    plugins = [],
    showDeprecated = false
  } = {}) {
    const languages = plugins.flatMap((plugin) => plugin.languages ?? []);
    const options = [];
    for (const option of normalizeOptionSettings(Object.assign({}, ...plugins.map(({
      options: options2
    }) => options2), core_options_evaluate_default))) {
      if (!showDeprecated && option.deprecated) {
        continue;
      }
      if (Array.isArray(option.choices)) {
        if (!showDeprecated) {
          option.choices = option.choices.filter((choice) => !choice.deprecated);
        }
        if (option.name === "parser") {
          option.choices = [...option.choices, ...collectParsersFromLanguages(option.choices, languages, plugins)];
        }
      }
      option.pluginDefaults = Object.fromEntries(plugins.filter((plugin) => {
        var _a;
        return ((_a = plugin.defaultOptions) == null ? void 0 : _a[option.name]) !== void 0;
      }).map((plugin) => [plugin.name, plugin.defaultOptions[option.name]]));
      options.push(option);
    }
    return {
      languages,
      options
    };
  }
  function* collectParsersFromLanguages(parserChoices, languages, plugins) {
    const existingParsers = new Set(parserChoices.map((choice) => choice.value));
    for (const language of languages) {
      if (language.parsers) {
        for (const parserName of language.parsers) {
          if (!existingParsers.has(parserName)) {
            existingParsers.add(parserName);
            const plugin = plugins.find((plugin2) => plugin2.parsers && Object.prototype.hasOwnProperty.call(plugin2.parsers, parserName));
            let description = language.name;
            if (plugin == null ? void 0 : plugin.name) {
              description += ` (plugin: ${plugin.name})`;
            }
            yield {
              value: parserName,
              description
            };
          }
        }
      }
    }
  }
  function normalizeOptionSettings(settings) {
    const options = [];
    for (const [name, originalOption] of Object.entries(settings)) {
      const option = {
        name,
        ...originalOption
      };
      if (Array.isArray(option.default)) {
        option.default = at_default(
          /* isOptionalObject*/
          false,
          option.default,
          -1
        ).value;
      }
      options.push(option);
    }
    return options;
  }

  // src/utils/get-interpreter.js
  var get_interpreter_default = void 0;

  // src/utils/infer-parser.js
  var getFileBasename = (file) => file.split(/[/\\]/).pop();
  function getLanguageByFilename(languages, filename) {
    if (!filename) {
      return;
    }
    const basename = getFileBasename(filename).toLowerCase();
    return languages.find(
      (language) => {
        var _a, _b;
        return ((_a = language.extensions) == null ? void 0 : _a.some((extension) => basename.endsWith(extension))) || ((_b = language.filenames) == null ? void 0 : _b.some((name) => name.toLowerCase() === basename));
      }
    );
  }
  function getLanguageByName(languages, languageName) {
    if (!languageName) {
      return;
    }
    return languages.find(({ name }) => name.toLowerCase() === languageName) ?? languages.find(({ aliases }) => aliases == null ? void 0 : aliases.includes(languageName)) ?? languages.find(({ extensions }) => extensions == null ? void 0 : extensions.includes(`.${languageName}`));
  }
  function getLanguageByInterpreter(languages, file) {
    if (true) {
      return;
    }
    const interpreter = get_interpreter_default(file);
    if (!interpreter) {
      return;
    }
    return languages.find(
      (language) => {
        var _a;
        return (_a = language.interpreters) == null ? void 0 : _a.includes(interpreter);
      }
    );
  }
  function inferParser(options, fileInfo) {
    const languages = options.plugins.flatMap(
      (plugin) => (
        // @ts-expect-error -- Safe
        plugin.languages ?? []
      )
    );
    const language = getLanguageByName(languages, fileInfo.language) ?? getLanguageByFilename(languages, fileInfo.physicalFile) ?? getLanguageByFilename(languages, fileInfo.file) ?? getLanguageByInterpreter(languages, fileInfo.physicalFile);
    return language == null ? void 0 : language.parsers[0];
  }
  var infer_parser_default = inferParser;

  // node_modules/vnopts/lib/descriptors/api.js
  var apiDescriptor = {
    key: (key) => /^[$_a-zA-Z][$_a-zA-Z0-9]*$/.test(key) ? key : JSON.stringify(key),
    value(value) {
      if (value === null || typeof value !== "object") {
        return JSON.stringify(value);
      }
      if (Array.isArray(value)) {
        return `[${value.map((subValue) => apiDescriptor.value(subValue)).join(", ")}]`;
      }
      const keys = Object.keys(value);
      return keys.length === 0 ? "{}" : `{ ${keys.map((key) => `${apiDescriptor.key(key)}: ${apiDescriptor.value(value[key])}`).join(", ")} }`;
    },
    pair: ({ key, value }) => apiDescriptor.value({ [key]: value })
  };

  // node_modules/vnopts/lib/handlers/deprecated/common.js
  var import_chalk = __toESM(require_chalk(), 1);
  var commonDeprecatedHandler = (keyOrPair, redirectTo, { descriptor }) => {
    const messages = [
      `${import_chalk.default.yellow(typeof keyOrPair === "string" ? descriptor.key(keyOrPair) : descriptor.pair(keyOrPair))} is deprecated`
    ];
    if (redirectTo) {
      messages.push(`we now treat it as ${import_chalk.default.blue(typeof redirectTo === "string" ? descriptor.key(redirectTo) : descriptor.pair(redirectTo))}`);
    }
    return messages.join("; ") + ".";
  };

  // node_modules/vnopts/lib/handlers/invalid/common.js
  var import_chalk2 = __toESM(require_chalk(), 1);

  // node_modules/vnopts/lib/constants.js
  var VALUE_NOT_EXIST = Symbol.for("vnopts.VALUE_NOT_EXIST");
  var VALUE_UNCHANGED = Symbol.for("vnopts.VALUE_UNCHANGED");

  // node_modules/vnopts/lib/handlers/invalid/common.js
  var INDENTATION = " ".repeat(2);
  var commonInvalidHandler = (key, value, utils) => {
    const { text, list } = utils.normalizeExpectedResult(utils.schemas[key].expected(utils));
    const descriptions = [];
    if (text) {
      descriptions.push(getDescription(key, value, text, utils.descriptor));
    }
    if (list) {
      descriptions.push([getDescription(key, value, list.title, utils.descriptor)].concat(list.values.map((valueDescription) => getListDescription(valueDescription, utils.loggerPrintWidth))).join("\n"));
    }
    return chooseDescription(descriptions, utils.loggerPrintWidth);
  };
  function getDescription(key, value, expected, descriptor) {
    return [
      `Invalid ${import_chalk2.default.red(descriptor.key(key))} value.`,
      `Expected ${import_chalk2.default.blue(expected)},`,
      `but received ${value === VALUE_NOT_EXIST ? import_chalk2.default.gray("nothing") : import_chalk2.default.red(descriptor.value(value))}.`
    ].join(" ");
  }
  function getListDescription({ text, list }, printWidth) {
    const descriptions = [];
    if (text) {
      descriptions.push(`- ${import_chalk2.default.blue(text)}`);
    }
    if (list) {
      descriptions.push([`- ${import_chalk2.default.blue(list.title)}:`].concat(list.values.map((valueDescription) => getListDescription(valueDescription, printWidth - INDENTATION.length).replace(/^|\n/g, `$&${INDENTATION}`))).join("\n"));
    }
    return chooseDescription(descriptions, printWidth);
  }
  function chooseDescription(descriptions, printWidth) {
    if (descriptions.length === 1) {
      return descriptions[0];
    }
    const [firstDescription, secondDescription] = descriptions;
    const [firstWidth, secondWidth] = descriptions.map((description) => description.split("\n", 1)[0].length);
    return firstWidth > printWidth && firstWidth > secondWidth ? secondDescription : firstDescription;
  }

  // node_modules/vnopts/lib/handlers/unknown/leven.js
  var import_chalk3 = __toESM(require_chalk(), 1);

  // node_modules/leven/index.js
  var array = [];
  var characterCodeCache = [];
  function leven(first, second) {
    if (first === second) {
      return 0;
    }
    const swap = first;
    if (first.length > second.length) {
      first = second;
      second = swap;
    }
    let firstLength = first.length;
    let secondLength = second.length;
    while (firstLength > 0 && first.charCodeAt(~-firstLength) === second.charCodeAt(~-secondLength)) {
      firstLength--;
      secondLength--;
    }
    let start = 0;
    while (start < firstLength && first.charCodeAt(start) === second.charCodeAt(start)) {
      start++;
    }
    firstLength -= start;
    secondLength -= start;
    if (firstLength === 0) {
      return secondLength;
    }
    let bCharacterCode;
    let result;
    let temporary;
    let temporary2;
    let index = 0;
    let index2 = 0;
    while (index < firstLength) {
      characterCodeCache[index] = first.charCodeAt(start + index);
      array[index] = ++index;
    }
    while (index2 < secondLength) {
      bCharacterCode = second.charCodeAt(start + index2);
      temporary = index2++;
      result = index2;
      for (index = 0; index < firstLength; index++) {
        temporary2 = bCharacterCode === characterCodeCache[index] ? temporary : temporary + 1;
        temporary = array[index];
        result = array[index] = temporary > result ? temporary2 > result ? result + 1 : temporary2 : temporary2 > temporary ? temporary + 1 : temporary2;
      }
    }
    return result;
  }

  // node_modules/vnopts/lib/handlers/unknown/leven.js
  var levenUnknownHandler = (key, value, { descriptor, logger, schemas }) => {
    const messages = [
      `Ignored unknown option ${import_chalk3.default.yellow(descriptor.pair({ key, value }))}.`
    ];
    const suggestion = Object.keys(schemas).sort().find((knownKey) => leven(key, knownKey) < 3);
    if (suggestion) {
      messages.push(`Did you mean ${import_chalk3.default.blue(descriptor.key(suggestion))}?`);
    }
    logger.warn(messages.join(" "));
  };

  // node_modules/vnopts/lib/schema.js
  var HANDLER_KEYS = [
    "default",
    "expected",
    "validate",
    "deprecated",
    "forward",
    "redirect",
    "overlap",
    "preprocess",
    "postprocess"
  ];
  function createSchema(SchemaConstructor, parameters) {
    const schema = new SchemaConstructor(parameters);
    const subSchema = Object.create(schema);
    for (const handlerKey of HANDLER_KEYS) {
      if (handlerKey in parameters) {
        subSchema[handlerKey] = normalizeHandler(parameters[handlerKey], schema, Schema.prototype[handlerKey].length);
      }
    }
    return subSchema;
  }
  var Schema = class {
    static create(parameters) {
      return createSchema(this, parameters);
    }
    constructor(parameters) {
      this.name = parameters.name;
    }
    default(_utils) {
      return void 0;
    }
    // this is actually an abstract method but we need a placeholder to get `function.length`
    /* c8 ignore start */
    expected(_utils) {
      return "nothing";
    }
    /* c8 ignore stop */
    // this is actually an abstract method but we need a placeholder to get `function.length`
    /* c8 ignore start */
    validate(_value, _utils) {
      return false;
    }
    /* c8 ignore stop */
    deprecated(_value, _utils) {
      return false;
    }
    forward(_value, _utils) {
      return void 0;
    }
    redirect(_value, _utils) {
      return void 0;
    }
    overlap(currentValue, _newValue, _utils) {
      return currentValue;
    }
    preprocess(value, _utils) {
      return value;
    }
    postprocess(_value, _utils) {
      return VALUE_UNCHANGED;
    }
  };
  function normalizeHandler(handler, superSchema, handlerArgumentsLength) {
    return typeof handler === "function" ? (...args) => handler(...args.slice(0, handlerArgumentsLength - 1), superSchema, ...args.slice(handlerArgumentsLength - 1)) : () => handler;
  }

  // node_modules/vnopts/lib/schemas/alias.js
  var AliasSchema = class extends Schema {
    constructor(parameters) {
      super(parameters);
      this._sourceName = parameters.sourceName;
    }
    expected(utils) {
      return utils.schemas[this._sourceName].expected(utils);
    }
    validate(value, utils) {
      return utils.schemas[this._sourceName].validate(value, utils);
    }
    redirect(_value, _utils) {
      return this._sourceName;
    }
  };

  // node_modules/vnopts/lib/schemas/any.js
  var AnySchema = class extends Schema {
    expected() {
      return "anything";
    }
    validate() {
      return true;
    }
  };

  // node_modules/vnopts/lib/schemas/array.js
  var ArraySchema = class extends Schema {
    constructor({ valueSchema, name = valueSchema.name, ...handlers }) {
      super({ ...handlers, name });
      this._valueSchema = valueSchema;
    }
    expected(utils) {
      const { text, list } = utils.normalizeExpectedResult(this._valueSchema.expected(utils));
      return {
        text: text && `an array of ${text}`,
        list: list && {
          title: `an array of the following values`,
          values: [{ list }]
        }
      };
    }
    validate(value, utils) {
      if (!Array.isArray(value)) {
        return false;
      }
      const invalidValues = [];
      for (const subValue of value) {
        const subValidateResult = utils.normalizeValidateResult(this._valueSchema.validate(subValue, utils), subValue);
        if (subValidateResult !== true) {
          invalidValues.push(subValidateResult.value);
        }
      }
      return invalidValues.length === 0 ? true : { value: invalidValues };
    }
    deprecated(value, utils) {
      const deprecatedResult = [];
      for (const subValue of value) {
        const subDeprecatedResult = utils.normalizeDeprecatedResult(this._valueSchema.deprecated(subValue, utils), subValue);
        if (subDeprecatedResult !== false) {
          deprecatedResult.push(...subDeprecatedResult.map(({ value: deprecatedValue }) => ({
            value: [deprecatedValue]
          })));
        }
      }
      return deprecatedResult;
    }
    forward(value, utils) {
      const forwardResult = [];
      for (const subValue of value) {
        const subForwardResult = utils.normalizeForwardResult(this._valueSchema.forward(subValue, utils), subValue);
        forwardResult.push(...subForwardResult.map(wrapTransferResult));
      }
      return forwardResult;
    }
    redirect(value, utils) {
      const remain = [];
      const redirect = [];
      for (const subValue of value) {
        const subRedirectResult = utils.normalizeRedirectResult(this._valueSchema.redirect(subValue, utils), subValue);
        if ("remain" in subRedirectResult) {
          remain.push(subRedirectResult.remain);
        }
        redirect.push(...subRedirectResult.redirect.map(wrapTransferResult));
      }
      return remain.length === 0 ? { redirect } : { redirect, remain };
    }
    overlap(currentValue, newValue) {
      return currentValue.concat(newValue);
    }
  };
  function wrapTransferResult({ from, to }) {
    return { from: [from], to };
  }

  // node_modules/vnopts/lib/schemas/boolean.js
  var BooleanSchema = class extends Schema {
    expected() {
      return "true or false";
    }
    validate(value) {
      return typeof value === "boolean";
    }
  };

  // node_modules/vnopts/lib/utils.js
  function recordFromArray(array2, mainKey) {
    const record = /* @__PURE__ */ Object.create(null);
    for (const value of array2) {
      const key = value[mainKey];
      if (record[key]) {
        throw new Error(`Duplicate ${mainKey} ${JSON.stringify(key)}`);
      }
      record[key] = value;
    }
    return record;
  }
  function mapFromArray(array2, mainKey) {
    const map = /* @__PURE__ */ new Map();
    for (const value of array2) {
      const key = value[mainKey];
      if (map.has(key)) {
        throw new Error(`Duplicate ${mainKey} ${JSON.stringify(key)}`);
      }
      map.set(key, value);
    }
    return map;
  }
  function createAutoChecklist() {
    const map = /* @__PURE__ */ Object.create(null);
    return (id) => {
      const idString = JSON.stringify(id);
      if (map[idString]) {
        return true;
      }
      map[idString] = true;
      return false;
    };
  }
  function partition(array2, predicate) {
    const trueArray = [];
    const falseArray = [];
    for (const value of array2) {
      if (predicate(value)) {
        trueArray.push(value);
      } else {
        falseArray.push(value);
      }
    }
    return [trueArray, falseArray];
  }
  function isInt(value) {
    return value === Math.floor(value);
  }
  function comparePrimitive(a, b) {
    if (a === b) {
      return 0;
    }
    const typeofA = typeof a;
    const typeofB = typeof b;
    const orders = [
      "undefined",
      "object",
      "boolean",
      "number",
      "string"
    ];
    if (typeofA !== typeofB) {
      return orders.indexOf(typeofA) - orders.indexOf(typeofB);
    }
    if (typeofA !== "string") {
      return Number(a) - Number(b);
    }
    return a.localeCompare(b);
  }
  function normalizeInvalidHandler(invalidHandler) {
    return (...args) => {
      const errorMessageOrError = invalidHandler(...args);
      return typeof errorMessageOrError === "string" ? new Error(errorMessageOrError) : errorMessageOrError;
    };
  }
  function normalizeDefaultResult(result) {
    return result === void 0 ? {} : result;
  }
  function normalizeExpectedResult(result) {
    if (typeof result === "string") {
      return { text: result };
    }
    const { text, list } = result;
    assert2((text || list) !== void 0, "Unexpected `expected` result, there should be at least one field.");
    if (!list) {
      return { text };
    }
    return {
      text,
      list: {
        title: list.title,
        values: list.values.map(normalizeExpectedResult)
      }
    };
  }
  function normalizeValidateResult(result, value) {
    return result === true ? true : result === false ? { value } : result;
  }
  function normalizeDeprecatedResult(result, value, doNotNormalizeTrue = false) {
    return result === false ? false : result === true ? doNotNormalizeTrue ? true : [{ value }] : "value" in result ? [result] : result.length === 0 ? false : result;
  }
  function normalizeTransferResult(result, value) {
    return typeof result === "string" || "key" in result ? { from: value, to: result } : "from" in result ? { from: result.from, to: result.to } : { from: value, to: result.to };
  }
  function normalizeForwardResult(result, value) {
    return result === void 0 ? [] : Array.isArray(result) ? result.map((transferResult) => normalizeTransferResult(transferResult, value)) : [normalizeTransferResult(result, value)];
  }
  function normalizeRedirectResult(result, value) {
    const redirect = normalizeForwardResult(typeof result === "object" && "redirect" in result ? result.redirect : result, value);
    return redirect.length === 0 ? { remain: value, redirect } : typeof result === "object" && "remain" in result ? { remain: result.remain, redirect } : { redirect };
  }
  function assert2(isValid, message) {
    if (!isValid) {
      throw new Error(message);
    }
  }

  // node_modules/vnopts/lib/schemas/choice.js
  var ChoiceSchema = class extends Schema {
    constructor(parameters) {
      super(parameters);
      this._choices = mapFromArray(parameters.choices.map((choice) => choice && typeof choice === "object" ? choice : { value: choice }), "value");
    }
    expected({ descriptor }) {
      const choiceDescriptions = Array.from(this._choices.keys()).map((value) => this._choices.get(value)).filter(({ hidden }) => !hidden).map((choiceInfo) => choiceInfo.value).sort(comparePrimitive).map(descriptor.value);
      const head = choiceDescriptions.slice(0, -2);
      const tail = choiceDescriptions.slice(-2);
      const message = head.concat(tail.join(" or ")).join(", ");
      return {
        text: message,
        list: {
          title: "one of the following values",
          values: choiceDescriptions
        }
      };
    }
    validate(value) {
      return this._choices.has(value);
    }
    deprecated(value) {
      const choiceInfo = this._choices.get(value);
      return choiceInfo && choiceInfo.deprecated ? { value } : false;
    }
    forward(value) {
      const choiceInfo = this._choices.get(value);
      return choiceInfo ? choiceInfo.forward : void 0;
    }
    redirect(value) {
      const choiceInfo = this._choices.get(value);
      return choiceInfo ? choiceInfo.redirect : void 0;
    }
  };

  // node_modules/vnopts/lib/schemas/number.js
  var NumberSchema = class extends Schema {
    expected() {
      return "a number";
    }
    validate(value, _utils) {
      return typeof value === "number";
    }
  };

  // node_modules/vnopts/lib/schemas/integer.js
  var IntegerSchema = class extends NumberSchema {
    expected() {
      return "an integer";
    }
    validate(value, utils) {
      return utils.normalizeValidateResult(super.validate(value, utils), value) === true && isInt(value);
    }
  };

  // node_modules/vnopts/lib/schemas/string.js
  var StringSchema = class extends Schema {
    expected() {
      return "a string";
    }
    validate(value) {
      return typeof value === "string";
    }
  };

  // node_modules/vnopts/lib/defaults.js
  var defaultDescriptor = apiDescriptor;
  var defaultUnknownHandler = levenUnknownHandler;
  var defaultInvalidHandler = commonInvalidHandler;
  var defaultDeprecatedHandler = commonDeprecatedHandler;

  // node_modules/vnopts/lib/normalize.js
  var Normalizer = class {
    constructor(schemas, opts) {
      const { logger = console, loggerPrintWidth = 80, descriptor = defaultDescriptor, unknown = defaultUnknownHandler, invalid = defaultInvalidHandler, deprecated = defaultDeprecatedHandler, missing = () => false, required = () => false, preprocess = (x) => x, postprocess = () => VALUE_UNCHANGED } = opts || {};
      this._utils = {
        descriptor,
        logger: (
          /* c8 ignore next */
          logger || { warn: () => {
          } }
        ),
        loggerPrintWidth,
        schemas: recordFromArray(schemas, "name"),
        normalizeDefaultResult,
        normalizeExpectedResult,
        normalizeDeprecatedResult,
        normalizeForwardResult,
        normalizeRedirectResult,
        normalizeValidateResult
      };
      this._unknownHandler = unknown;
      this._invalidHandler = normalizeInvalidHandler(invalid);
      this._deprecatedHandler = deprecated;
      this._identifyMissing = (k, o) => !(k in o) || missing(k, o);
      this._identifyRequired = required;
      this._preprocess = preprocess;
      this._postprocess = postprocess;
      this.cleanHistory();
    }
    cleanHistory() {
      this._hasDeprecationWarned = createAutoChecklist();
    }
    normalize(options) {
      const newOptions = {};
      const preprocessed = this._preprocess(options, this._utils);
      const restOptionsArray = [preprocessed];
      const applyNormalization = () => {
        while (restOptionsArray.length !== 0) {
          const currentOptions = restOptionsArray.shift();
          const transferredOptionsArray = this._applyNormalization(currentOptions, newOptions);
          restOptionsArray.push(...transferredOptionsArray);
        }
      };
      applyNormalization();
      for (const key of Object.keys(this._utils.schemas)) {
        const schema = this._utils.schemas[key];
        if (!(key in newOptions)) {
          const defaultResult = normalizeDefaultResult(schema.default(this._utils));
          if ("value" in defaultResult) {
            restOptionsArray.push({ [key]: defaultResult.value });
          }
        }
      }
      applyNormalization();
      for (const key of Object.keys(this._utils.schemas)) {
        if (!(key in newOptions)) {
          continue;
        }
        const schema = this._utils.schemas[key];
        const value = newOptions[key];
        const newValue = schema.postprocess(value, this._utils);
        if (newValue === VALUE_UNCHANGED) {
          continue;
        }
        this._applyValidation(newValue, key, schema);
        newOptions[key] = newValue;
      }
      this._applyPostprocess(newOptions);
      this._applyRequiredCheck(newOptions);
      return newOptions;
    }
    _applyNormalization(options, newOptions) {
      const transferredOptionsArray = [];
      const { knownKeys, unknownKeys } = this._partitionOptionKeys(options);
      for (const key of knownKeys) {
        const schema = this._utils.schemas[key];
        const value = schema.preprocess(options[key], this._utils);
        this._applyValidation(value, key, schema);
        const appendTransferredOptions = ({ from, to }) => {
          transferredOptionsArray.push(typeof to === "string" ? { [to]: from } : { [to.key]: to.value });
        };
        const warnDeprecated = ({ value: currentValue, redirectTo }) => {
          const deprecatedResult = normalizeDeprecatedResult(
            schema.deprecated(currentValue, this._utils),
            value,
            /* doNotNormalizeTrue */
            true
          );
          if (deprecatedResult === false) {
            return;
          }
          if (deprecatedResult === true) {
            if (!this._hasDeprecationWarned(key)) {
              this._utils.logger.warn(this._deprecatedHandler(key, redirectTo, this._utils));
            }
          } else {
            for (const { value: deprecatedValue } of deprecatedResult) {
              const pair = { key, value: deprecatedValue };
              if (!this._hasDeprecationWarned(pair)) {
                const redirectToPair = typeof redirectTo === "string" ? { key: redirectTo, value: deprecatedValue } : redirectTo;
                this._utils.logger.warn(this._deprecatedHandler(pair, redirectToPair, this._utils));
              }
            }
          }
        };
        const forwardResult = normalizeForwardResult(schema.forward(value, this._utils), value);
        forwardResult.forEach(appendTransferredOptions);
        const redirectResult = normalizeRedirectResult(schema.redirect(value, this._utils), value);
        redirectResult.redirect.forEach(appendTransferredOptions);
        if ("remain" in redirectResult) {
          const remainingValue = redirectResult.remain;
          newOptions[key] = key in newOptions ? schema.overlap(newOptions[key], remainingValue, this._utils) : remainingValue;
          warnDeprecated({ value: remainingValue });
        }
        for (const { from, to } of redirectResult.redirect) {
          warnDeprecated({ value: from, redirectTo: to });
        }
      }
      for (const key of unknownKeys) {
        const value = options[key];
        this._applyUnknownHandler(key, value, newOptions, (knownResultKey, knownResultValue) => {
          transferredOptionsArray.push({ [knownResultKey]: knownResultValue });
        });
      }
      return transferredOptionsArray;
    }
    _applyRequiredCheck(options) {
      for (const key of Object.keys(this._utils.schemas)) {
        if (this._identifyMissing(key, options)) {
          if (this._identifyRequired(key)) {
            throw this._invalidHandler(key, VALUE_NOT_EXIST, this._utils);
          }
        }
      }
    }
    _partitionOptionKeys(options) {
      const [knownKeys, unknownKeys] = partition(Object.keys(options).filter((key) => !this._identifyMissing(key, options)), (key) => key in this._utils.schemas);
      return { knownKeys, unknownKeys };
    }
    _applyValidation(value, key, schema) {
      const validateResult = normalizeValidateResult(schema.validate(value, this._utils), value);
      if (validateResult !== true) {
        throw this._invalidHandler(key, validateResult.value, this._utils);
      }
    }
    _applyUnknownHandler(key, value, newOptions, knownResultHandler) {
      const unknownResult = this._unknownHandler(key, value, this._utils);
      if (!unknownResult) {
        return;
      }
      for (const resultKey of Object.keys(unknownResult)) {
        if (this._identifyMissing(resultKey, unknownResult)) {
          continue;
        }
        const resultValue = unknownResult[resultKey];
        if (resultKey in this._utils.schemas) {
          knownResultHandler(resultKey, resultValue);
        } else {
          newOptions[resultKey] = resultValue;
        }
      }
    }
    _applyPostprocess(options) {
      const postprocessed = this._postprocess(options, this._utils);
      if (postprocessed === VALUE_UNCHANGED) {
        return;
      }
      if (postprocessed.delete) {
        for (const deleteKey of postprocessed.delete) {
          delete options[deleteKey];
        }
      }
      if (postprocessed.override) {
        const { knownKeys, unknownKeys } = this._partitionOptionKeys(postprocessed.override);
        for (const key of knownKeys) {
          const value = postprocessed.override[key];
          this._applyValidation(value, key, this._utils.schemas[key]);
          options[key] = value;
        }
        for (const key of unknownKeys) {
          const value = postprocessed.override[key];
          this._applyUnknownHandler(key, value, options, (knownResultKey, knownResultValue) => {
            const schema = this._utils.schemas[knownResultKey];
            this._applyValidation(knownResultValue, knownResultKey, schema);
            options[knownResultKey] = knownResultValue;
          });
        }
      }
    }
  };

  // src/main/normalize-options.js
  var hasDeprecationWarned;
  function normalizeOptions(options, optionInfos, {
    logger = false,
    isCLI = false,
    passThrough = false,
    FlagSchema,
    descriptor
  } = {}) {
    if (isCLI) {
      if (!FlagSchema) {
        throw new Error("'FlagSchema' option is required.");
      }
      if (!descriptor) {
        throw new Error("'descriptor' option is required.");
      }
    } else {
      descriptor = apiDescriptor;
    }
    const unknown = !passThrough ? (key, value, options2) => {
      const {
        _,
        ...schemas2
      } = options2.schemas;
      return levenUnknownHandler(key, value, {
        ...options2,
        schemas: schemas2
      });
    } : Array.isArray(passThrough) ? (key, value) => !passThrough.includes(key) ? void 0 : {
      [key]: value
    } : (key, value) => ({
      [key]: value
    });
    const schemas = optionInfosToSchemas(optionInfos, {
      isCLI,
      FlagSchema
    });
    const normalizer = new Normalizer(schemas, {
      logger,
      unknown,
      descriptor
    });
    const shouldSuppressDuplicateDeprecationWarnings = logger !== false;
    if (shouldSuppressDuplicateDeprecationWarnings && hasDeprecationWarned) {
      normalizer._hasDeprecationWarned = hasDeprecationWarned;
    }
    const normalized = normalizer.normalize(options);
    if (shouldSuppressDuplicateDeprecationWarnings) {
      hasDeprecationWarned = normalizer._hasDeprecationWarned;
    }
    return normalized;
  }
  function optionInfosToSchemas(optionInfos, {
    isCLI,
    FlagSchema
  }) {
    const schemas = [];
    if (isCLI) {
      schemas.push(AnySchema.create({
        name: "_"
      }));
    }
    for (const optionInfo of optionInfos) {
      schemas.push(optionInfoToSchema(optionInfo, {
        isCLI,
        optionInfos,
        FlagSchema
      }));
      if (optionInfo.alias && isCLI) {
        schemas.push(AliasSchema.create({
          // @ts-expect-error
          name: optionInfo.alias,
          sourceName: optionInfo.name
        }));
      }
    }
    return schemas;
  }
  function optionInfoToSchema(optionInfo, {
    isCLI,
    optionInfos,
    FlagSchema
  }) {
    const {
      name
    } = optionInfo;
    const parameters = {
      name
    };
    let SchemaConstructor;
    const handlers = {};
    switch (optionInfo.type) {
      case "int":
        SchemaConstructor = IntegerSchema;
        if (isCLI) {
          parameters.preprocess = Number;
        }
        break;
      case "string":
        SchemaConstructor = StringSchema;
        break;
      case "choice":
        SchemaConstructor = ChoiceSchema;
        parameters.choices = optionInfo.choices.map((choiceInfo) => (choiceInfo == null ? void 0 : choiceInfo.redirect) ? {
          ...choiceInfo,
          redirect: {
            to: {
              key: optionInfo.name,
              value: choiceInfo.redirect
            }
          }
        } : choiceInfo);
        break;
      case "boolean":
        SchemaConstructor = BooleanSchema;
        break;
      case "flag":
        SchemaConstructor = FlagSchema;
        parameters.flags = optionInfos.flatMap((optionInfo2) => [optionInfo2.alias, optionInfo2.description && optionInfo2.name, optionInfo2.oppositeDescription && `no-${optionInfo2.name}`].filter(Boolean));
        break;
      case "path":
        SchemaConstructor = StringSchema;
        break;
      default:
        throw new Error(`Unexpected type ${optionInfo.type}`);
    }
    if (optionInfo.exception) {
      parameters.validate = (value, schema, utils) => optionInfo.exception(value) || schema.validate(value, utils);
    } else {
      parameters.validate = (value, schema, utils) => value === void 0 || schema.validate(value, utils);
    }
    if (optionInfo.redirect) {
      handlers.redirect = (value) => !value ? void 0 : {
        to: {
          key: optionInfo.redirect.option,
          value: optionInfo.redirect.value
        }
      };
    }
    if (optionInfo.deprecated) {
      handlers.deprecated = true;
    }
    if (isCLI && !optionInfo.array) {
      const originalPreprocess = parameters.preprocess || ((x) => x);
      parameters.preprocess = (value, schema, utils) => schema.preprocess(originalPreprocess(Array.isArray(value) ? at_default(
        /* isOptionalObject*/
        false,
        value,
        -1
      ) : value), utils);
    }
    return optionInfo.array ? ArraySchema.create({
      ...isCLI ? {
        preprocess: (v) => Array.isArray(v) ? v : [v]
      } : {},
      ...handlers,
      // @ts-expect-error
      valueSchema: SchemaConstructor.create(parameters)
    }) : SchemaConstructor.create({
      ...parameters,
      ...handlers
    });
  }
  var normalize_options_default = normalizeOptions;

  // src/main/parser-and-printer.js
  function getParserPluginByParserName(plugins, parserName) {
    if (!parserName) {
      throw new Error("parserName is required.");
    }
    for (let index = plugins.length - 1; index >= 0; index--) {
      const plugin = plugins[index];
      if (plugin.parsers && Object.prototype.hasOwnProperty.call(plugin.parsers, parserName)) {
        return plugin;
      }
    }
    let message = `Couldn't resolve parser "${parserName}".`;
    if (true) {
      message += " Plugins must be explicitly added to the standalone bundle.";
    }
    throw new ConfigError(message);
  }
  function getPrinterPluginByAstFormat(plugins, astFormat) {
    if (!astFormat) {
      throw new Error("astFormat is required.");
    }
    for (let index = plugins.length - 1; index >= 0; index--) {
      const plugin = plugins[index];
      if (plugin.printers && Object.prototype.hasOwnProperty.call(plugin.printers, astFormat)) {
        return plugin;
      }
    }
    let message = `Couldn't find plugin for AST format "${astFormat}".`;
    if (true) {
      message += " Plugins must be explicitly added to the standalone bundle.";
    }
    throw new ConfigError(message);
  }
  function resolveParser({
    plugins,
    parser
  }) {
    const plugin = getParserPluginByParserName(plugins, parser);
    return initParser(plugin, parser);
  }
  function initParser(plugin, parserName) {
    const parserOrParserInitFunction = plugin.parsers[parserName];
    return typeof parserOrParserInitFunction === "function" ? parserOrParserInitFunction() : parserOrParserInitFunction;
  }
  function initPrinter(plugin, astFormat) {
    const printerOrPrinterInitFunction = plugin.printers[astFormat];
    return typeof printerOrPrinterInitFunction === "function" ? printerOrPrinterInitFunction() : printerOrPrinterInitFunction;
  }

  // src/main/normalize-format-options.js
  var formatOptionsHiddenDefaults = {
    astFormat: "estree",
    printer: {},
    originalText: void 0,
    locStart: null,
    locEnd: null
  };
  function normalizeFormatOptions(options, opts = {}) {
    var _a;
    const rawOptions = { ...options };
    if (!rawOptions.parser) {
      if (!rawOptions.filepath) {
        throw new UndefinedParserError(
          "No parser and no file path given, couldn't infer a parser."
        );
      } else {
        rawOptions.parser = infer_parser_default(rawOptions, {
          physicalFile: rawOptions.filepath
        });
        if (!rawOptions.parser) {
          throw new UndefinedParserError(
            `No parser could be inferred for file "${rawOptions.filepath}".`
          );
        }
      }
    }
    const supportOptions = getSupportInfo({
      plugins: options.plugins,
      showDeprecated: true
    }).options;
    const defaults = {
      ...formatOptionsHiddenDefaults,
      ...Object.fromEntries(
        supportOptions.filter((optionInfo) => optionInfo.default !== void 0).map((option) => [option.name, option.default])
      )
    };
    const parserPlugin = getParserPluginByParserName(
      rawOptions.plugins,
      rawOptions.parser
    );
    const parser = initParser(parserPlugin, rawOptions.parser);
    rawOptions.astFormat = parser.astFormat;
    rawOptions.locEnd = parser.locEnd;
    rawOptions.locStart = parser.locStart;
    const printerPlugin = ((_a = parserPlugin.printers) == null ? void 0 : _a[parser.astFormat]) ? parserPlugin : getPrinterPluginByAstFormat(rawOptions.plugins, parser.astFormat);
    const printer = initPrinter(printerPlugin, parser.astFormat);
    rawOptions.printer = printer;
    const pluginDefaults = printerPlugin.defaultOptions ? Object.fromEntries(
      Object.entries(printerPlugin.defaultOptions).filter(
        ([, value]) => value !== void 0
      )
    ) : {};
    const mixedDefaults = { ...defaults, ...pluginDefaults };
    for (const [k, value] of Object.entries(mixedDefaults)) {
      if (rawOptions[k] === null || rawOptions[k] === void 0) {
        rawOptions[k] = value;
      }
    }
    if (rawOptions.parser === "json") {
      rawOptions.trailingComma = "none";
    }
    return normalize_options_default(rawOptions, supportOptions, {
      passThrough: Object.keys(formatOptionsHiddenDefaults),
      ...opts
    });
  }
  var normalize_format_options_default = normalizeFormatOptions;

  // src/main/parse.js
  var import_code_frame = __toESM(require_lib(), 1);
  function parse(originalText, options) {
    const parser = resolveParser(options);
    const text = parser.preprocess ? parser.preprocess(originalText, options) : originalText;
    options.originalText = text;
    let ast;
    try {
      ast = parser.parse(
        text,
        options,
        // TODO: remove the third argument in v4
        // The duplicated argument is passed as intended, see #10156
        options
      );
    } catch (error) {
      handleParseError(error, originalText);
    }
    return { text, ast };
  }
  function handleParseError(error, text) {
    const { loc } = error;
    if (loc) {
      const codeFrame = (0, import_code_frame.codeFrameColumns)(text, loc, { highlightCode: true });
      error.message += "\n" + codeFrame;
      error.codeFrame = codeFrame;
      throw error;
    }
    throw error;
  }
  var parse_default = parse;

  // src/main/multiparser.js
  function printEmbeddedLanguages(path, genericPrint, options, printAstToDoc2, embeds) {
    const {
      embeddedLanguageFormatting,
      printer: {
        embed,
        hasPrettierIgnore = () => false,
        getVisitorKeys: printerGetVisitorKeys
      }
    } = options;
    if (!embed || embeddedLanguageFormatting !== "auto") {
      return;
    }
    if (embed.length > 2) {
      throw new Error(
        "printer.embed has too many parameters. The API changed in Prettier v3. Please update your plugin. See https://prettier.io/docs/en/plugins.html#optional-embed"
      );
    }
    const getVisitorKeys = create_get_visitor_keys_function_default(
      embed.getVisitorKeys ?? printerGetVisitorKeys
    );
    const embedCallResults = [];
    recurse();
    const originalPathStack = path.stack;
    for (const { print, node, pathStack } of embedCallResults) {
      try {
        path.stack = pathStack;
        const doc = print(textToDocForEmbed, genericPrint, path, options);
        if (doc) {
          embeds.set(node, doc);
        }
      } catch (error) {
        if (globalThis.PRETTIER_DEBUG) {
          throw error;
        }
      }
    }
    path.stack = originalPathStack;
    function textToDocForEmbed(text, partialNextOptions) {
      return textToDoc(text, partialNextOptions, options, printAstToDoc2);
    }
    function recurse() {
      const { node } = path;
      if (node === null || typeof node !== "object" || hasPrettierIgnore(path)) {
        return;
      }
      for (const key of getVisitorKeys(node)) {
        if (Array.isArray(node[key])) {
          path.each(recurse, key);
        } else {
          path.call(recurse, key);
        }
      }
      const result = embed(path, options);
      if (!result) {
        return;
      }
      if (typeof result === "function") {
        embedCallResults.push({
          print: result,
          node,
          pathStack: [...path.stack]
        });
        return;
      }
      if (false) {
        throw new Error(
          "`embed` should return an async function instead of Promise."
        );
      }
      embeds.set(node, result);
    }
  }
  function textToDoc(text, partialNextOptions, parentOptions, printAstToDoc2) {
    const options = normalize_format_options_default(
      {
        ...parentOptions,
        ...partialNextOptions,
        parentParser: parentOptions.parser,
        originalText: text
      },
      { passThrough: true }
    );
    const { ast } = parse_default(text, options);
    const doc = printAstToDoc2(ast, options);
    return stripTrailingHardline(doc);
  }

  // src/main/create-print-pre-check-function.js
  function createPrintPreCheckFunction(options) {
    if (true) {
      return () => {
      };
    }
    const getVisitorKeys = create_get_visitor_keys_function_default(
      options.printer.getVisitorKeys
    );
    return function(path) {
      if (path.isRoot) {
        return;
      }
      const { key, parent } = path;
      const visitorKeys = getVisitorKeys(parent);
      if (visitorKeys.includes(key)) {
        return;
      }
      throw Object.assign(new Error("Calling `print()` on non-node object."), {
        parentNode: parent,
        allowedProperties: visitorKeys,
        printingProperty: key,
        printingValue: path.node,
        pathStack: path.stack.length > 5 ? ["...", ...path.stack.slice(-5)] : [...path.stack]
      });
    };
  }
  var create_print_pre_check_function_default = createPrintPreCheckFunction;

  // src/main/print-ignored.js
  function printIgnored(path, options) {
    const {
      originalText,
      [Symbol.for("comments")]: comments,
      locStart,
      locEnd,
      [Symbol.for("printedComments")]: printedComments
    } = options;
    const { node } = path;
    const start = locStart(node);
    const end = locEnd(node);
    for (const comment of comments) {
      if (locStart(comment) >= start && locEnd(comment) <= end) {
        printedComments.add(comment);
      }
    }
    return originalText.slice(start, end);
  }
  var print_ignored_default = printIgnored;

  // src/main/ast-to-doc.js
  function printAstToDoc(ast, options) {
    ({ ast } = prepareToPrint(ast, options));
    const cache = /* @__PURE__ */ new Map();
    const path = new ast_path_default(ast);
    const ensurePrintingNode = create_print_pre_check_function_default(options);
    const embeds = /* @__PURE__ */ new Map();
    printEmbeddedLanguages(path, mainPrint, options, printAstToDoc, embeds);
    const doc = callPluginPrintFunction(
      path,
      options,
      mainPrint,
      void 0,
      embeds
    );
    ensureAllCommentsPrinted(options);
    return doc;
    function mainPrint(selector, args) {
      if (selector === void 0 || selector === path) {
        return mainPrintInternal(args);
      }
      if (Array.isArray(selector)) {
        return path.call(() => mainPrintInternal(args), ...selector);
      }
      return path.call(() => mainPrintInternal(args), selector);
    }
    function mainPrintInternal(args) {
      ensurePrintingNode(path);
      const value = path.node;
      if (value === void 0 || value === null) {
        return "";
      }
      const shouldCache = value && typeof value === "object" && args === void 0;
      if (shouldCache && cache.has(value)) {
        return cache.get(value);
      }
      const doc2 = callPluginPrintFunction(path, options, mainPrint, args, embeds);
      if (shouldCache) {
        cache.set(value, doc2);
      }
      return doc2;
    }
  }
  function callPluginPrintFunction(path, options, printPath, args, embeds) {
    var _a;
    const { node } = path;
    const { printer } = options;
    let doc;
    if ((_a = printer.hasPrettierIgnore) == null ? void 0 : _a.call(printer, path)) {
      doc = print_ignored_default(path, options);
    } else if (embeds.has(node)) {
      doc = embeds.get(node);
    } else {
      doc = printer.print(path, options, printPath, args);
    }
    if (node === options.cursorNode) {
      doc = inheritLabel(doc, (doc2) => [cursor, doc2, cursor]);
    }
    if (printer.printComment && (!printer.willPrintOwnComments || !printer.willPrintOwnComments(path, options))) {
      doc = printComments(path, doc, options);
    }
    return doc;
  }
  function prepareToPrint(ast, options) {
    const comments = ast.comments ?? [];
    options[Symbol.for("comments")] = comments;
    options[Symbol.for("tokens")] = ast.tokens ?? [];
    options[Symbol.for("printedComments")] = /* @__PURE__ */ new Set();
    attachComments(ast, options);
    const {
      printer: { preprocess }
    } = options;
    ast = preprocess ? preprocess(ast, options) : ast;
    return { ast, comments };
  }
  return __toCommonJS(ast_to_doc_exports);
});