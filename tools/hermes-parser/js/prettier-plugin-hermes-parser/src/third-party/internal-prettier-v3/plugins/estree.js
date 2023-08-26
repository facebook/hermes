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
    root.prettierPlugins.estree = interopModuleDefault();
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
  var __accessCheck = (obj, member, msg) => {
    if (!member.has(obj))
      throw TypeError("Cannot " + msg);
  };
  var __privateGet = (obj, member, getter) => {
    __accessCheck(obj, member, "read from private field");
    return getter ? getter.call(obj) : member.get(obj);
  };
  var __privateAdd = (obj, member, value) => {
    if (member.has(obj))
      throw TypeError("Cannot add the same private member more than once");
    member instanceof WeakSet ? member.add(obj) : member.set(obj, value);
  };
  var __privateSet = (obj, member, value, setter) => {
    __accessCheck(obj, member, "write to private field");
    setter ? setter.call(obj, value) : member.set(obj, value);
    return value;
  };

  // node_modules/jest-docblock/build/index.js
  var require_build = __commonJS({
    "node_modules/jest-docblock/build/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.extract = extract2;
      exports.parse = parse;
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
      function parse(docblock) {
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

  // src/plugins/estree.js
  var estree_exports = {};
  __export(estree_exports, {
    languages: () => languages,
    options: () => options_default,
    printers: () => printers
  });

  // src/language-js/printer.js
  var printer_exports = {};
  __export(printer_exports, {
    canAttachComment: () => canAttachComment,
    embed: () => embed_default,
    experimentalFeatures: () => experimentalFeatures,
    getCommentChildNodes: () => getCommentChildNodes,
    getVisitorKeys: () => get_visitor_keys_default,
    handleComments: () => handle_comments_exports,
    insertPragma: () => insertPragma,
    isBlockComment: () => is_block_comment_default,
    isGap: () => isGap,
    massageAstNode: () => clean_default,
    print: () => print_default,
    printComment: () => printComment2,
    willPrintOwnComments: () => willPrintOwnComments
  });

  // src/main/print-ignored.js
  function printIgnored(path, options2) {
    const {
      originalText,
      [Symbol.for("comments")]: comments,
      locStart: locStart2,
      locEnd: locEnd2,
      [Symbol.for("printedComments")]: printedComments
    } = options2;
    const { node } = path;
    const start = locStart2(node);
    const end = locEnd2(node);
    for (const comment of comments) {
      if (locStart2(comment) >= start && locEnd2(comment) <= end) {
        printedComments.add(comment);
      }
    }
    return originalText.slice(start, end);
  }
  var print_ignored_default = printIgnored;

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

  // src/document/utils/traverse-doc.js
  var traverseDocOnExitStackMarker = {};
  function traverseDoc(doc, onEnter, onExit, shouldTraverseConditionalGroups) {
    const docsStack = [doc];
    while (docsStack.length > 0) {
      const doc2 = docsStack.pop();
      if (doc2 === traverseDocOnExitStackMarker) {
        onExit(docsStack.pop());
        continue;
      }
      if (onExit) {
        docsStack.push(doc2, traverseDocOnExitStackMarker);
      }
      const docType = get_doc_type_default(doc2);
      if (!docType) {
        throw new invalid_doc_error_default(doc2);
      }
      if ((onEnter == null ? void 0 : onEnter(doc2)) === false) {
        continue;
      }
      switch (docType) {
        case DOC_TYPE_ARRAY:
        case DOC_TYPE_FILL: {
          const parts = docType === DOC_TYPE_ARRAY ? doc2 : doc2.parts;
          for (let ic = parts.length, i = ic - 1; i >= 0; --i) {
            docsStack.push(parts[i]);
          }
          break;
        }
        case DOC_TYPE_IF_BREAK:
          docsStack.push(doc2.flatContents, doc2.breakContents);
          break;
        case DOC_TYPE_GROUP:
          if (shouldTraverseConditionalGroups && doc2.expandedStates) {
            for (let ic = doc2.expandedStates.length, i = ic - 1; i >= 0; --i) {
              docsStack.push(doc2.expandedStates[i]);
            }
          } else {
            docsStack.push(doc2.contents);
          }
          break;
        case DOC_TYPE_ALIGN:
        case DOC_TYPE_INDENT:
        case DOC_TYPE_INDENT_IF_BREAK:
        case DOC_TYPE_LABEL:
        case DOC_TYPE_LINE_SUFFIX:
          docsStack.push(doc2.contents);
          break;
        case DOC_TYPE_STRING:
        case DOC_TYPE_CURSOR:
        case DOC_TYPE_TRIM:
        case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
        case DOC_TYPE_LINE:
        case DOC_TYPE_BREAK_PARENT:
          break;
        default:
          throw new invalid_doc_error_default(doc2);
      }
    }
  }
  var traverse_doc_default = traverseDoc;

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
  function dedentToRoot(contents) {
    return align(Number.NEGATIVE_INFINITY, contents);
  }
  function dedent(contents) {
    return align(-1, contents);
  }
  function conditionalGroup(states, opts) {
    return group(states[0], { ...opts, expandedStates: states });
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
  function indentIfBreak(contents, opts) {
    assertDoc(contents);
    return {
      type: DOC_TYPE_INDENT_IF_BREAK,
      contents,
      groupId: opts.groupId,
      negate: opts.negate
    };
  }
  function lineSuffix(contents) {
    assertDoc(contents);
    return { type: DOC_TYPE_LINE_SUFFIX, contents };
  }
  var lineSuffixBoundary = { type: DOC_TYPE_LINE_SUFFIX_BOUNDARY };
  var breakParent = { type: DOC_TYPE_BREAK_PARENT };
  var hardlineWithoutBreakParent = { type: DOC_TYPE_LINE, hard: true };
  var literallineWithoutBreakParent = {
    type: DOC_TYPE_LINE,
    hard: true,
    literal: true
  };
  var line = { type: DOC_TYPE_LINE };
  var softline = { type: DOC_TYPE_LINE, soft: true };
  var hardline = [hardlineWithoutBreakParent, breakParent];
  var literalline = [literallineWithoutBreakParent, breakParent];
  var cursor = { type: DOC_TYPE_CURSOR };
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
  function addAlignmentToDoc(doc, size, tabWidth) {
    assertDoc(doc);
    let aligned = doc;
    if (size > 0) {
      for (let i = 0; i < Math.floor(size / tabWidth); ++i) {
        aligned = indent(aligned);
      }
      aligned = align(size % tabWidth, aligned);
      aligned = align(Number.NEGATIVE_INFINITY, aligned);
    }
    return aligned;
  }
  function label(label2, contents) {
    assertDoc(contents);
    return label2 ? { type: DOC_TYPE_LABEL, label: label2, contents } : contents;
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
  function findInDoc(doc, fn, defaultValue) {
    let result = defaultValue;
    let shouldSkipFurtherProcessing = false;
    function findInDocOnEnterFn(doc2) {
      if (shouldSkipFurtherProcessing) {
        return false;
      }
      const maybeResult = fn(doc2);
      if (maybeResult !== void 0) {
        shouldSkipFurtherProcessing = true;
        result = maybeResult;
      }
    }
    traverse_doc_default(doc, findInDocOnEnterFn);
    return result;
  }
  function willBreakFn(doc) {
    if (doc.type === DOC_TYPE_GROUP && doc.break) {
      return true;
    }
    if (doc.type === DOC_TYPE_LINE && doc.hard) {
      return true;
    }
    if (doc.type === DOC_TYPE_BREAK_PARENT) {
      return true;
    }
  }
  function willBreak(doc) {
    return findInDoc(doc, willBreakFn, false);
  }
  function breakParentGroup(groupStack) {
    if (groupStack.length > 0) {
      const parentGroup = at_default(
        /* isOptionalObject*/
        false,
        groupStack,
        -1
      );
      if (!parentGroup.expandedStates && !parentGroup.break) {
        parentGroup.break = "propagated";
      }
    }
    return null;
  }
  function propagateBreaks(doc) {
    const alreadyVisitedSet = /* @__PURE__ */ new Set();
    const groupStack = [];
    function propagateBreaksOnEnterFn(doc2) {
      if (doc2.type === DOC_TYPE_BREAK_PARENT) {
        breakParentGroup(groupStack);
      }
      if (doc2.type === DOC_TYPE_GROUP) {
        groupStack.push(doc2);
        if (alreadyVisitedSet.has(doc2)) {
          return false;
        }
        alreadyVisitedSet.add(doc2);
      }
    }
    function propagateBreaksOnExitFn(doc2) {
      if (doc2.type === DOC_TYPE_GROUP) {
        const group2 = groupStack.pop();
        if (group2.break) {
          breakParentGroup(groupStack);
        }
      }
    }
    traverse_doc_default(
      doc,
      propagateBreaksOnEnterFn,
      propagateBreaksOnExitFn,
      /* shouldTraverseConditionalGroups */
      true
    );
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
  function replaceEndOfLine(doc, replacement = literalline) {
    return mapDoc(doc, (currentDoc) => typeof currentDoc === "string" ? join(replacement, currentDoc.split("\n")) : currentDoc);
  }
  function canBreakFn(doc) {
    if (doc.type === DOC_TYPE_LINE) {
      return true;
    }
  }
  function canBreak(doc) {
    return findInDoc(doc, canBreakFn, false);
  }
  function inheritLabel(doc, fn) {
    return doc.type === DOC_TYPE_LABEL ? {
      ...doc,
      contents: fn(doc.contents)
    } : fn(doc);
  }

  // src/utils/is-non-empty-array.js
  function isNonEmptyArray(object) {
    return Array.isArray(object) && object.length > 0;
  }
  var is_non_empty_array_default = isNonEmptyArray;

  // node_modules/@prettier/is-es5-identifier-name/dist/index.js
  var regexp_default = /^[\$A-Z_a-z\xAA\xB5\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0370-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386\u0388-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u048A-\u052F\u0531-\u0556\u0559\u0561-\u0587\u05D0-\u05EA\u05F0-\u05F2\u0620-\u064A\u066E\u066F\u0671-\u06D3\u06D5\u06E5\u06E6\u06EE\u06EF\u06FA-\u06FC\u06FF\u0710\u0712-\u072F\u074D-\u07A5\u07B1\u07CA-\u07EA\u07F4\u07F5\u07FA\u0800-\u0815\u081A\u0824\u0828\u0840-\u0858\u08A0-\u08B4\u08B6-\u08BD\u0904-\u0939\u093D\u0950\u0958-\u0961\u0971-\u0980\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BD\u09CE\u09DC\u09DD\u09DF-\u09E1\u09F0\u09F1\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A59-\u0A5C\u0A5E\u0A72-\u0A74\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABD\u0AD0\u0AE0\u0AE1\u0AF9\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3D\u0B5C\u0B5D\u0B5F-\u0B61\u0B71\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BD0\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D\u0C58-\u0C5A\u0C60\u0C61\u0C80\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBD\u0CDE\u0CE0\u0CE1\u0CF1\u0CF2\u0D05-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D\u0D4E\u0D54-\u0D56\u0D5F-\u0D61\u0D7A-\u0D7F\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0E01-\u0E30\u0E32\u0E33\u0E40-\u0E46\u0E81\u0E82\u0E84\u0E87\u0E88\u0E8A\u0E8D\u0E94-\u0E97\u0E99-\u0E9F\u0EA1-\u0EA3\u0EA5\u0EA7\u0EAA\u0EAB\u0EAD-\u0EB0\u0EB2\u0EB3\u0EBD\u0EC0-\u0EC4\u0EC6\u0EDC-\u0EDF\u0F00\u0F40-\u0F47\u0F49-\u0F6C\u0F88-\u0F8C\u1000-\u102A\u103F\u1050-\u1055\u105A-\u105D\u1061\u1065\u1066\u106E-\u1070\u1075-\u1081\u108E\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u170C\u170E-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176C\u176E-\u1770\u1780-\u17B3\u17D7\u17DC\u1820-\u1877\u1880-\u1884\u1887-\u18A8\u18AA\u18B0-\u18F5\u1900-\u191E\u1950-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u1A00-\u1A16\u1A20-\u1A54\u1AA7\u1B05-\u1B33\u1B45-\u1B4B\u1B83-\u1BA0\u1BAE\u1BAF\u1BBA-\u1BE5\u1C00-\u1C23\u1C4D-\u1C4F\u1C5A-\u1C7D\u1C80-\u1C88\u1CE9-\u1CEC\u1CEE-\u1CF1\u1CF5\u1CF6\u1D00-\u1DBF\u1E00-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u2071\u207F\u2090-\u209C\u2102\u2107\u210A-\u2113\u2115\u2119-\u211D\u2124\u2126\u2128\u212A-\u212D\u212F-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2C2E\u2C30-\u2C5E\u2C60-\u2CE4\u2CEB-\u2CEE\u2CF2\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D80-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u2E2F\u3005-\u3007\u3021-\u3029\u3031-\u3035\u3038-\u303C\u3041-\u3096\u309D-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312D\u3131-\u318E\u31A0-\u31BA\u31F0-\u31FF\u3400-\u4DB5\u4E00-\u9FD5\uA000-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA61F\uA62A\uA62B\uA640-\uA66E\uA67F-\uA69D\uA6A0-\uA6EF\uA717-\uA71F\uA722-\uA788\uA78B-\uA7AE\uA7B0-\uA7B7\uA7F7-\uA801\uA803-\uA805\uA807-\uA80A\uA80C-\uA822\uA840-\uA873\uA882-\uA8B3\uA8F2-\uA8F7\uA8FB\uA8FD\uA90A-\uA925\uA930-\uA946\uA960-\uA97C\uA984-\uA9B2\uA9CF\uA9E0-\uA9E4\uA9E6-\uA9EF\uA9FA-\uA9FE\uAA00-\uAA28\uAA40-\uAA42\uAA44-\uAA4B\uAA60-\uAA76\uAA7A\uAA7E-\uAAAF\uAAB1\uAAB5\uAAB6\uAAB9-\uAABD\uAAC0\uAAC2\uAADB-\uAADD\uAAE0-\uAAEA\uAAF2-\uAAF4\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB65\uAB70-\uABE2\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D\uFB1F-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE70-\uFE74\uFE76-\uFEFC\uFF21-\uFF3A\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC][\$0-9A-Z_a-z\xAA\xB5\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0300-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386\u0388-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u0483-\u0487\u048A-\u052F\u0531-\u0556\u0559\u0561-\u0587\u0591-\u05BD\u05BF\u05C1\u05C2\u05C4\u05C5\u05C7\u05D0-\u05EA\u05F0-\u05F2\u0610-\u061A\u0620-\u0669\u066E-\u06D3\u06D5-\u06DC\u06DF-\u06E8\u06EA-\u06FC\u06FF\u0710-\u074A\u074D-\u07B1\u07C0-\u07F5\u07FA\u0800-\u082D\u0840-\u085B\u08A0-\u08B4\u08B6-\u08BD\u08D4-\u08E1\u08E3-\u0963\u0966-\u096F\u0971-\u0983\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BC-\u09C4\u09C7\u09C8\u09CB-\u09CE\u09D7\u09DC\u09DD\u09DF-\u09E3\u09E6-\u09F1\u0A01-\u0A03\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A3C\u0A3E-\u0A42\u0A47\u0A48\u0A4B-\u0A4D\u0A51\u0A59-\u0A5C\u0A5E\u0A66-\u0A75\u0A81-\u0A83\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABC-\u0AC5\u0AC7-\u0AC9\u0ACB-\u0ACD\u0AD0\u0AE0-\u0AE3\u0AE6-\u0AEF\u0AF9\u0B01-\u0B03\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3C-\u0B44\u0B47\u0B48\u0B4B-\u0B4D\u0B56\u0B57\u0B5C\u0B5D\u0B5F-\u0B63\u0B66-\u0B6F\u0B71\u0B82\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BBE-\u0BC2\u0BC6-\u0BC8\u0BCA-\u0BCD\u0BD0\u0BD7\u0BE6-\u0BEF\u0C00-\u0C03\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D-\u0C44\u0C46-\u0C48\u0C4A-\u0C4D\u0C55\u0C56\u0C58-\u0C5A\u0C60-\u0C63\u0C66-\u0C6F\u0C80-\u0C83\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBC-\u0CC4\u0CC6-\u0CC8\u0CCA-\u0CCD\u0CD5\u0CD6\u0CDE\u0CE0-\u0CE3\u0CE6-\u0CEF\u0CF1\u0CF2\u0D01-\u0D03\u0D05-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D-\u0D44\u0D46-\u0D48\u0D4A-\u0D4E\u0D54-\u0D57\u0D5F-\u0D63\u0D66-\u0D6F\u0D7A-\u0D7F\u0D82\u0D83\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0DCA\u0DCF-\u0DD4\u0DD6\u0DD8-\u0DDF\u0DE6-\u0DEF\u0DF2\u0DF3\u0E01-\u0E3A\u0E40-\u0E4E\u0E50-\u0E59\u0E81\u0E82\u0E84\u0E87\u0E88\u0E8A\u0E8D\u0E94-\u0E97\u0E99-\u0E9F\u0EA1-\u0EA3\u0EA5\u0EA7\u0EAA\u0EAB\u0EAD-\u0EB9\u0EBB-\u0EBD\u0EC0-\u0EC4\u0EC6\u0EC8-\u0ECD\u0ED0-\u0ED9\u0EDC-\u0EDF\u0F00\u0F18\u0F19\u0F20-\u0F29\u0F35\u0F37\u0F39\u0F3E-\u0F47\u0F49-\u0F6C\u0F71-\u0F84\u0F86-\u0F97\u0F99-\u0FBC\u0FC6\u1000-\u1049\u1050-\u109D\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u135D-\u135F\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u170C\u170E-\u1714\u1720-\u1734\u1740-\u1753\u1760-\u176C\u176E-\u1770\u1772\u1773\u1780-\u17D3\u17D7\u17DC\u17DD\u17E0-\u17E9\u180B-\u180D\u1810-\u1819\u1820-\u1877\u1880-\u18AA\u18B0-\u18F5\u1900-\u191E\u1920-\u192B\u1930-\u193B\u1946-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u19D0-\u19D9\u1A00-\u1A1B\u1A20-\u1A5E\u1A60-\u1A7C\u1A7F-\u1A89\u1A90-\u1A99\u1AA7\u1AB0-\u1ABD\u1B00-\u1B4B\u1B50-\u1B59\u1B6B-\u1B73\u1B80-\u1BF3\u1C00-\u1C37\u1C40-\u1C49\u1C4D-\u1C7D\u1C80-\u1C88\u1CD0-\u1CD2\u1CD4-\u1CF6\u1CF8\u1CF9\u1D00-\u1DF5\u1DFB-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u200C\u200D\u203F\u2040\u2054\u2071\u207F\u2090-\u209C\u20D0-\u20DC\u20E1\u20E5-\u20F0\u2102\u2107\u210A-\u2113\u2115\u2119-\u211D\u2124\u2126\u2128\u212A-\u212D\u212F-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2C2E\u2C30-\u2C5E\u2C60-\u2CE4\u2CEB-\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D7F-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u2DE0-\u2DFF\u2E2F\u3005-\u3007\u3021-\u302F\u3031-\u3035\u3038-\u303C\u3041-\u3096\u3099\u309A\u309D-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312D\u3131-\u318E\u31A0-\u31BA\u31F0-\u31FF\u3400-\u4DB5\u4E00-\u9FD5\uA000-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA62B\uA640-\uA66F\uA674-\uA67D\uA67F-\uA6F1\uA717-\uA71F\uA722-\uA788\uA78B-\uA7AE\uA7B0-\uA7B7\uA7F7-\uA827\uA840-\uA873\uA880-\uA8C5\uA8D0-\uA8D9\uA8E0-\uA8F7\uA8FB\uA8FD\uA900-\uA92D\uA930-\uA953\uA960-\uA97C\uA980-\uA9C0\uA9CF-\uA9D9\uA9E0-\uA9FE\uAA00-\uAA36\uAA40-\uAA4D\uAA50-\uAA59\uAA60-\uAA76\uAA7A-\uAAC2\uAADB-\uAADD\uAAE0-\uAAEF\uAAF2-\uAAF6\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB65\uAB70-\uABEA\uABEC\uABED\uABF0-\uABF9\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE00-\uFE0F\uFE20-\uFE2F\uFE33\uFE34\uFE4D-\uFE4F\uFE70-\uFE74\uFE76-\uFEFC\uFF10-\uFF19\uFF21-\uFF3A\uFF3F\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC]*$/;
  var isEs5IdentifierName = (id) => regexp_default.test(id);
  var src_default = isEs5IdentifierName;

  // src/utils/is-object.js
  function isObject(object) {
    return object !== null && typeof object === "object";
  }
  var is_object_default = isObject;

  // src/utils/ast-utils.js
  function* getChildren(node, options2) {
    const { getVisitorKeys: getVisitorKeys3, filter = () => true } = options2;
    const isMatchedNode = (node2) => is_object_default(node2) && filter(node2);
    for (const key of getVisitorKeys3(node)) {
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
  function* getDescendants(node, options2) {
    const queue = [node];
    for (let index = 0; index < queue.length; index++) {
      const node2 = queue[index];
      for (const child of getChildren(node2, options2)) {
        yield child;
        queue.push(child);
      }
    }
  }
  function hasDescendant(node, { getVisitorKeys: getVisitorKeys3, predicate }) {
    for (const descendant of getDescendants(node, { getVisitorKeys: getVisitorKeys3 })) {
      if (predicate(descendant)) {
        return true;
      }
    }
    return false;
  }

  // src/utils/skip.js
  function skip(characters) {
    return (text, startIndex, options2) => {
      const backwards = Boolean(options2 == null ? void 0 : options2.backwards);
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

  // node_modules/emoji-regex/index.mjs
  var emoji_regex_default = () => {
    return /[#*0-9]\uFE0F?\u20E3|[\xA9\xAE\u203C\u2049\u2122\u2139\u2194-\u2199\u21A9\u21AA\u231A\u231B\u2328\u23CF\u23ED-\u23EF\u23F1\u23F2\u23F8-\u23FA\u24C2\u25AA\u25AB\u25B6\u25C0\u25FB\u25FC\u25FE\u2600-\u2604\u260E\u2611\u2614\u2615\u2618\u2620\u2622\u2623\u2626\u262A\u262E\u262F\u2638-\u263A\u2640\u2642\u2648-\u2653\u265F\u2660\u2663\u2665\u2666\u2668\u267B\u267E\u267F\u2692\u2694-\u2697\u2699\u269B\u269C\u26A0\u26A7\u26AA\u26B0\u26B1\u26BD\u26BE\u26C4\u26C8\u26CF\u26D1\u26D3\u26E9\u26F0-\u26F5\u26F7\u26F8\u26FA\u2702\u2708\u2709\u270F\u2712\u2714\u2716\u271D\u2721\u2733\u2734\u2744\u2747\u2757\u2763\u27A1\u2934\u2935\u2B05-\u2B07\u2B1B\u2B1C\u2B55\u3030\u303D\u3297\u3299]\uFE0F?|[\u261D\u270C\u270D](?:\uFE0F|\uD83C[\uDFFB-\uDFFF])?|[\u270A\u270B](?:\uD83C[\uDFFB-\uDFFF])?|[\u23E9-\u23EC\u23F0\u23F3\u25FD\u2693\u26A1\u26AB\u26C5\u26CE\u26D4\u26EA\u26FD\u2705\u2728\u274C\u274E\u2753-\u2755\u2795-\u2797\u27B0\u27BF\u2B50]|\u26F9(?:\uFE0F|\uD83C[\uDFFB-\uDFFF])?(?:\u200D[\u2640\u2642]\uFE0F?)?|\u2764\uFE0F?(?:\u200D(?:\uD83D\uDD25|\uD83E\uDE79))?|\uD83C(?:[\uDC04\uDD70\uDD71\uDD7E\uDD7F\uDE02\uDE37\uDF21\uDF24-\uDF2C\uDF36\uDF7D\uDF96\uDF97\uDF99-\uDF9B\uDF9E\uDF9F\uDFCD\uDFCE\uDFD4-\uDFDF\uDFF5\uDFF7]\uFE0F?|[\uDF85\uDFC2\uDFC7](?:\uD83C[\uDFFB-\uDFFF])?|[\uDFC3\uDFC4\uDFCA](?:\uD83C[\uDFFB-\uDFFF])?(?:\u200D[\u2640\u2642]\uFE0F?)?|[\uDFCB\uDFCC](?:\uFE0F|\uD83C[\uDFFB-\uDFFF])?(?:\u200D[\u2640\u2642]\uFE0F?)?|[\uDCCF\uDD8E\uDD91-\uDD9A\uDE01\uDE1A\uDE2F\uDE32-\uDE36\uDE38-\uDE3A\uDE50\uDE51\uDF00-\uDF20\uDF2D-\uDF35\uDF37-\uDF7C\uDF7E-\uDF84\uDF86-\uDF93\uDFA0-\uDFC1\uDFC5\uDFC6\uDFC8\uDFC9\uDFCF-\uDFD3\uDFE0-\uDFF0\uDFF8-\uDFFF]|\uDDE6\uD83C[\uDDE8-\uDDEC\uDDEE\uDDF1\uDDF2\uDDF4\uDDF6-\uDDFA\uDDFC\uDDFD\uDDFF]|\uDDE7\uD83C[\uDDE6\uDDE7\uDDE9-\uDDEF\uDDF1-\uDDF4\uDDF6-\uDDF9\uDDFB\uDDFC\uDDFE\uDDFF]|\uDDE8\uD83C[\uDDE6\uDDE8\uDDE9\uDDEB-\uDDEE\uDDF0-\uDDF5\uDDF7\uDDFA-\uDDFF]|\uDDE9\uD83C[\uDDEA\uDDEC\uDDEF\uDDF0\uDDF2\uDDF4\uDDFF]|\uDDEA\uD83C[\uDDE6\uDDE8\uDDEA\uDDEC\uDDED\uDDF7-\uDDFA]|\uDDEB\uD83C[\uDDEE-\uDDF0\uDDF2\uDDF4\uDDF7]|\uDDEC\uD83C[\uDDE6\uDDE7\uDDE9-\uDDEE\uDDF1-\uDDF3\uDDF5-\uDDFA\uDDFC\uDDFE]|\uDDED\uD83C[\uDDF0\uDDF2\uDDF3\uDDF7\uDDF9\uDDFA]|\uDDEE\uD83C[\uDDE8-\uDDEA\uDDF1-\uDDF4\uDDF6-\uDDF9]|\uDDEF\uD83C[\uDDEA\uDDF2\uDDF4\uDDF5]|\uDDF0\uD83C[\uDDEA\uDDEC-\uDDEE\uDDF2\uDDF3\uDDF5\uDDF7\uDDFC\uDDFE\uDDFF]|\uDDF1\uD83C[\uDDE6-\uDDE8\uDDEE\uDDF0\uDDF7-\uDDFB\uDDFE]|\uDDF2\uD83C[\uDDE6\uDDE8-\uDDED\uDDF0-\uDDFF]|\uDDF3\uD83C[\uDDE6\uDDE8\uDDEA-\uDDEC\uDDEE\uDDF1\uDDF4\uDDF5\uDDF7\uDDFA\uDDFF]|\uDDF4\uD83C\uDDF2|\uDDF5\uD83C[\uDDE6\uDDEA-\uDDED\uDDF0-\uDDF3\uDDF7-\uDDF9\uDDFC\uDDFE]|\uDDF6\uD83C\uDDE6|\uDDF7\uD83C[\uDDEA\uDDF4\uDDF8\uDDFA\uDDFC]|\uDDF8\uD83C[\uDDE6-\uDDEA\uDDEC-\uDDF4\uDDF7-\uDDF9\uDDFB\uDDFD-\uDDFF]|\uDDF9\uD83C[\uDDE6\uDDE8\uDDE9\uDDEB-\uDDED\uDDEF-\uDDF4\uDDF7\uDDF9\uDDFB\uDDFC\uDDFF]|\uDDFA\uD83C[\uDDE6\uDDEC\uDDF2\uDDF3\uDDF8\uDDFE\uDDFF]|\uDDFB\uD83C[\uDDE6\uDDE8\uDDEA\uDDEC\uDDEE\uDDF3\uDDFA]|\uDDFC\uD83C[\uDDEB\uDDF8]|\uDDFD\uD83C\uDDF0|\uDDFE\uD83C[\uDDEA\uDDF9]|\uDDFF\uD83C[\uDDE6\uDDF2\uDDFC]|\uDFF3\uFE0F?(?:\u200D(?:\u26A7\uFE0F?|\uD83C\uDF08))?|\uDFF4(?:\u200D\u2620\uFE0F?|\uDB40\uDC67\uDB40\uDC62\uDB40(?:\uDC65\uDB40\uDC6E\uDB40\uDC67|\uDC73\uDB40\uDC63\uDB40\uDC74|\uDC77\uDB40\uDC6C\uDB40\uDC73)\uDB40\uDC7F)?)|\uD83D(?:[\uDC08\uDC26](?:\u200D\u2B1B)?|[\uDC3F\uDCFD\uDD49\uDD4A\uDD6F\uDD70\uDD73\uDD76-\uDD79\uDD87\uDD8A-\uDD8D\uDDA5\uDDA8\uDDB1\uDDB2\uDDBC\uDDC2-\uDDC4\uDDD1-\uDDD3\uDDDC-\uDDDE\uDDE1\uDDE3\uDDE8\uDDEF\uDDF3\uDDFA\uDECB\uDECD-\uDECF\uDEE0-\uDEE5\uDEE9\uDEF0\uDEF3]\uFE0F?|[\uDC42\uDC43\uDC46-\uDC50\uDC66\uDC67\uDC6B-\uDC6D\uDC72\uDC74-\uDC76\uDC78\uDC7C\uDC83\uDC85\uDC8F\uDC91\uDCAA\uDD7A\uDD95\uDD96\uDE4C\uDE4F\uDEC0\uDECC](?:\uD83C[\uDFFB-\uDFFF])?|[\uDC6E\uDC70\uDC71\uDC73\uDC77\uDC81\uDC82\uDC86\uDC87\uDE45-\uDE47\uDE4B\uDE4D\uDE4E\uDEA3\uDEB4-\uDEB6](?:\uD83C[\uDFFB-\uDFFF])?(?:\u200D[\u2640\u2642]\uFE0F?)?|[\uDD74\uDD90](?:\uFE0F|\uD83C[\uDFFB-\uDFFF])?|[\uDC00-\uDC07\uDC09-\uDC14\uDC16-\uDC25\uDC27-\uDC3A\uDC3C-\uDC3E\uDC40\uDC44\uDC45\uDC51-\uDC65\uDC6A\uDC79-\uDC7B\uDC7D-\uDC80\uDC84\uDC88-\uDC8E\uDC90\uDC92-\uDCA9\uDCAB-\uDCFC\uDCFF-\uDD3D\uDD4B-\uDD4E\uDD50-\uDD67\uDDA4\uDDFB-\uDE2D\uDE2F-\uDE34\uDE37-\uDE44\uDE48-\uDE4A\uDE80-\uDEA2\uDEA4-\uDEB3\uDEB7-\uDEBF\uDEC1-\uDEC5\uDED0-\uDED2\uDED5-\uDED7\uDEDC-\uDEDF\uDEEB\uDEEC\uDEF4-\uDEFC\uDFE0-\uDFEB\uDFF0]|\uDC15(?:\u200D\uD83E\uDDBA)?|\uDC3B(?:\u200D\u2744\uFE0F?)?|\uDC41\uFE0F?(?:\u200D\uD83D\uDDE8\uFE0F?)?|\uDC68(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?\uDC68|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D(?:[\uDC68\uDC69]\u200D\uD83D(?:\uDC66(?:\u200D\uD83D\uDC66)?|\uDC67(?:\u200D\uD83D[\uDC66\uDC67])?)|[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uDC66(?:\u200D\uD83D\uDC66)?|\uDC67(?:\u200D\uD83D[\uDC66\uDC67])?)|\uD83E[\uDDAF-\uDDB3\uDDBC\uDDBD])|\uD83C(?:\uDFFB(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?\uDC68\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D\uDC68\uD83C[\uDFFC-\uDFFF])))?|\uDFFC(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?\uDC68\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D\uDC68\uD83C[\uDFFB\uDFFD-\uDFFF])))?|\uDFFD(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?\uDC68\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D\uDC68\uD83C[\uDFFB\uDFFC\uDFFE\uDFFF])))?|\uDFFE(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?\uDC68\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D\uDC68\uD83C[\uDFFB-\uDFFD\uDFFF])))?|\uDFFF(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?\uDC68\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D\uDC68\uD83C[\uDFFB-\uDFFE])))?))?|\uDC69(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:\uDC8B\u200D\uD83D)?[\uDC68\uDC69]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D(?:[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uDC66(?:\u200D\uD83D\uDC66)?|\uDC67(?:\u200D\uD83D[\uDC66\uDC67])?|\uDC69\u200D\uD83D(?:\uDC66(?:\u200D\uD83D\uDC66)?|\uDC67(?:\u200D\uD83D[\uDC66\uDC67])?))|\uD83E[\uDDAF-\uDDB3\uDDBC\uDDBD])|\uD83C(?:\uDFFB(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:[\uDC68\uDC69]|\uDC8B\u200D\uD83D[\uDC68\uDC69])\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D[\uDC68\uDC69]\uD83C[\uDFFC-\uDFFF])))?|\uDFFC(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:[\uDC68\uDC69]|\uDC8B\u200D\uD83D[\uDC68\uDC69])\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D[\uDC68\uDC69]\uD83C[\uDFFB\uDFFD-\uDFFF])))?|\uDFFD(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:[\uDC68\uDC69]|\uDC8B\u200D\uD83D[\uDC68\uDC69])\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D[\uDC68\uDC69]\uD83C[\uDFFB\uDFFC\uDFFE\uDFFF])))?|\uDFFE(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:[\uDC68\uDC69]|\uDC8B\u200D\uD83D[\uDC68\uDC69])\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D[\uDC68\uDC69]\uD83C[\uDFFB-\uDFFD\uDFFF])))?|\uDFFF(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D\uD83D(?:[\uDC68\uDC69]|\uDC8B\u200D\uD83D[\uDC68\uDC69])\uD83C[\uDFFB-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83D[\uDC68\uDC69]\uD83C[\uDFFB-\uDFFE])))?))?|\uDC6F(?:\u200D[\u2640\u2642]\uFE0F?)?|\uDD75(?:\uFE0F|\uD83C[\uDFFB-\uDFFF])?(?:\u200D[\u2640\u2642]\uFE0F?)?|\uDE2E(?:\u200D\uD83D\uDCA8)?|\uDE35(?:\u200D\uD83D\uDCAB)?|\uDE36(?:\u200D\uD83C\uDF2B\uFE0F?)?)|\uD83E(?:[\uDD0C\uDD0F\uDD18-\uDD1F\uDD30-\uDD34\uDD36\uDD77\uDDB5\uDDB6\uDDBB\uDDD2\uDDD3\uDDD5\uDEC3-\uDEC5\uDEF0\uDEF2-\uDEF8](?:\uD83C[\uDFFB-\uDFFF])?|[\uDD26\uDD35\uDD37-\uDD39\uDD3D\uDD3E\uDDB8\uDDB9\uDDCD-\uDDCF\uDDD4\uDDD6-\uDDDD](?:\uD83C[\uDFFB-\uDFFF])?(?:\u200D[\u2640\u2642]\uFE0F?)?|[\uDDDE\uDDDF](?:\u200D[\u2640\u2642]\uFE0F?)?|[\uDD0D\uDD0E\uDD10-\uDD17\uDD20-\uDD25\uDD27-\uDD2F\uDD3A\uDD3F-\uDD45\uDD47-\uDD76\uDD78-\uDDB4\uDDB7\uDDBA\uDDBC-\uDDCC\uDDD0\uDDE0-\uDDFF\uDE70-\uDE7C\uDE80-\uDE88\uDE90-\uDEBD\uDEBF-\uDEC2\uDECE-\uDEDB\uDEE0-\uDEE8]|\uDD3C(?:\u200D[\u2640\u2642]\uFE0F?|\uD83C[\uDFFB-\uDFFF])?|\uDDD1(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\uD83C[\uDF3E\uDF73\uDF7C\uDF84\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83E\uDDD1))|\uD83C(?:\uDFFB(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D(?:\uD83D\uDC8B\u200D)?\uD83E\uDDD1\uD83C[\uDFFC-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF84\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83E\uDDD1\uD83C[\uDFFB-\uDFFF])))?|\uDFFC(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D(?:\uD83D\uDC8B\u200D)?\uD83E\uDDD1\uD83C[\uDFFB\uDFFD-\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF84\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83E\uDDD1\uD83C[\uDFFB-\uDFFF])))?|\uDFFD(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D(?:\uD83D\uDC8B\u200D)?\uD83E\uDDD1\uD83C[\uDFFB\uDFFC\uDFFE\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF84\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83E\uDDD1\uD83C[\uDFFB-\uDFFF])))?|\uDFFE(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D(?:\uD83D\uDC8B\u200D)?\uD83E\uDDD1\uD83C[\uDFFB-\uDFFD\uDFFF]|\uD83C[\uDF3E\uDF73\uDF7C\uDF84\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83E\uDDD1\uD83C[\uDFFB-\uDFFF])))?|\uDFFF(?:\u200D(?:[\u2695\u2696\u2708]\uFE0F?|\u2764\uFE0F?\u200D(?:\uD83D\uDC8B\u200D)?\uD83E\uDDD1\uD83C[\uDFFB-\uDFFE]|\uD83C[\uDF3E\uDF73\uDF7C\uDF84\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\uD83E(?:[\uDDAF-\uDDB3\uDDBC\uDDBD]|\uDD1D\u200D\uD83E\uDDD1\uD83C[\uDFFB-\uDFFF])))?))?|\uDEF1(?:\uD83C(?:\uDFFB(?:\u200D\uD83E\uDEF2\uD83C[\uDFFC-\uDFFF])?|\uDFFC(?:\u200D\uD83E\uDEF2\uD83C[\uDFFB\uDFFD-\uDFFF])?|\uDFFD(?:\u200D\uD83E\uDEF2\uD83C[\uDFFB\uDFFC\uDFFE\uDFFF])?|\uDFFE(?:\u200D\uD83E\uDEF2\uD83C[\uDFFB-\uDFFD\uDFFF])?|\uDFFF(?:\u200D\uD83E\uDEF2\uD83C[\uDFFB-\uDFFE])?))?)/g;
  };

  // node_modules/eastasianwidth/eastasianwidth.js
  var eastasianwidth_default = {
    eastAsianWidth(character) {
      var x = character.charCodeAt(0);
      var y = character.length == 2 ? character.charCodeAt(1) : 0;
      var codePoint = x;
      if (55296 <= x && x <= 56319 && 56320 <= y && y <= 57343) {
        x &= 1023;
        y &= 1023;
        codePoint = x << 10 | y;
        codePoint += 65536;
      }
      if (12288 == codePoint || 65281 <= codePoint && codePoint <= 65376 || 65504 <= codePoint && codePoint <= 65510) {
        return "F";
      }
      if (4352 <= codePoint && codePoint <= 4447 || 4515 <= codePoint && codePoint <= 4519 || 4602 <= codePoint && codePoint <= 4607 || 9001 <= codePoint && codePoint <= 9002 || 11904 <= codePoint && codePoint <= 11929 || 11931 <= codePoint && codePoint <= 12019 || 12032 <= codePoint && codePoint <= 12245 || 12272 <= codePoint && codePoint <= 12283 || 12289 <= codePoint && codePoint <= 12350 || 12353 <= codePoint && codePoint <= 12438 || 12441 <= codePoint && codePoint <= 12543 || 12549 <= codePoint && codePoint <= 12589 || 12593 <= codePoint && codePoint <= 12686 || 12688 <= codePoint && codePoint <= 12730 || 12736 <= codePoint && codePoint <= 12771 || 12784 <= codePoint && codePoint <= 12830 || 12832 <= codePoint && codePoint <= 12871 || 12880 <= codePoint && codePoint <= 13054 || 13056 <= codePoint && codePoint <= 19903 || 19968 <= codePoint && codePoint <= 42124 || 42128 <= codePoint && codePoint <= 42182 || 43360 <= codePoint && codePoint <= 43388 || 44032 <= codePoint && codePoint <= 55203 || 55216 <= codePoint && codePoint <= 55238 || 55243 <= codePoint && codePoint <= 55291 || 63744 <= codePoint && codePoint <= 64255 || 65040 <= codePoint && codePoint <= 65049 || 65072 <= codePoint && codePoint <= 65106 || 65108 <= codePoint && codePoint <= 65126 || 65128 <= codePoint && codePoint <= 65131 || 110592 <= codePoint && codePoint <= 110593 || 127488 <= codePoint && codePoint <= 127490 || 127504 <= codePoint && codePoint <= 127546 || 127552 <= codePoint && codePoint <= 127560 || 127568 <= codePoint && codePoint <= 127569 || 131072 <= codePoint && codePoint <= 194367 || 177984 <= codePoint && codePoint <= 196605 || 196608 <= codePoint && codePoint <= 262141) {
        return "W";
      }
      return "N";
    }
  };

  // src/utils/get-string-width.js
  var notAsciiRegex = /[^\x20-\x7F]/;
  function getStringWidth(text) {
    if (!text) {
      return 0;
    }
    if (!notAsciiRegex.test(text)) {
      return text.length;
    }
    text = text.replace(emoji_regex_default(), "  ");
    let width = 0;
    for (const character of text) {
      const codePoint = character.codePointAt(0);
      if (codePoint <= 31 || codePoint >= 127 && codePoint <= 159) {
        continue;
      }
      if (codePoint >= 768 && codePoint <= 879) {
        continue;
      }
      const code = eastasianwidth_default.eastAsianWidth(character);
      width += code === "F" || code === "W" ? 2 : 1;
    }
    return width;
  }
  var get_string_width_default = getStringWidth;

  // src/language-js/loc.js
  function locStart(node) {
    var _a;
    const start = node.range ? node.range[0] : node.start;
    const decorators = ((_a = node.declaration) == null ? void 0 : _a.decorators) ?? node.decorators;
    if (is_non_empty_array_default(decorators)) {
      return Math.min(locStart(decorators[0]), start);
    }
    return start;
  }
  function locEnd(node) {
    return node.range ? node.range[1] : node.end;
  }
  function hasSameLocStart(nodeA, nodeB) {
    const nodeAStart = locStart(nodeA);
    return Number.isInteger(nodeAStart) && nodeAStart === locStart(nodeB);
  }
  function hasSameLocEnd(nodeA, nodeB) {
    const nodeAEnd = locEnd(nodeA);
    return Number.isInteger(nodeAEnd) && nodeAEnd === locEnd(nodeB);
  }
  function hasSameLoc(nodeA, nodeB) {
    return hasSameLocStart(nodeA, nodeB) && hasSameLocEnd(nodeA, nodeB);
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
    function getVisitorKeys3(node) {
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
    return getVisitorKeys3;
  }
  var create_get_visitor_keys_default = createGetVisitorKeys;

  // src/language-js/traverse/visitor-keys.evaluate.js
  var visitor_keys_evaluate_default = {
    "ArrayExpression": [
      "elements"
    ],
    "AssignmentExpression": [
      "left",
      "right"
    ],
    "BinaryExpression": [
      "left",
      "right"
    ],
    "InterpreterDirective": [],
    "Directive": [
      "value"
    ],
    "DirectiveLiteral": [],
    "BlockStatement": [
      "directives",
      "body"
    ],
    "BreakStatement": [
      "label"
    ],
    "CallExpression": [
      "callee",
      "arguments",
      "typeParameters",
      "typeArguments"
    ],
    "CatchClause": [
      "param",
      "body"
    ],
    "ConditionalExpression": [
      "test",
      "consequent",
      "alternate"
    ],
    "ContinueStatement": [
      "label"
    ],
    "DebuggerStatement": [],
    "DoWhileStatement": [
      "test",
      "body"
    ],
    "EmptyStatement": [],
    "ExpressionStatement": [
      "expression"
    ],
    "File": [
      "program"
    ],
    "ForInStatement": [
      "left",
      "right",
      "body"
    ],
    "ForStatement": [
      "init",
      "test",
      "update",
      "body"
    ],
    "FunctionDeclaration": [
      "id",
      "params",
      "body",
      "returnType",
      "typeParameters",
      "predicate"
    ],
    "FunctionExpression": [
      "id",
      "params",
      "body",
      "returnType",
      "typeParameters"
    ],
    "Identifier": [
      "typeAnnotation",
      "decorators"
    ],
    "IfStatement": [
      "test",
      "consequent",
      "alternate"
    ],
    "LabeledStatement": [
      "label",
      "body"
    ],
    "StringLiteral": [],
    "NumericLiteral": [],
    "NullLiteral": [],
    "BooleanLiteral": [],
    "RegExpLiteral": [],
    "LogicalExpression": [
      "left",
      "right"
    ],
    "MemberExpression": [
      "object",
      "property"
    ],
    "NewExpression": [
      "callee",
      "arguments",
      "typeParameters",
      "typeArguments"
    ],
    "Program": [
      "directives",
      "body"
    ],
    "ObjectExpression": [
      "properties"
    ],
    "ObjectMethod": [
      "key",
      "params",
      "body",
      "decorators",
      "returnType",
      "typeParameters"
    ],
    "ObjectProperty": [
      "key",
      "value",
      "decorators"
    ],
    "RestElement": [
      "argument",
      "typeAnnotation",
      "decorators"
    ],
    "ReturnStatement": [
      "argument"
    ],
    "SequenceExpression": [
      "expressions"
    ],
    "ParenthesizedExpression": [
      "expression"
    ],
    "SwitchCase": [
      "test",
      "consequent"
    ],
    "SwitchStatement": [
      "discriminant",
      "cases"
    ],
    "ThisExpression": [],
    "ThrowStatement": [
      "argument"
    ],
    "TryStatement": [
      "block",
      "handler",
      "finalizer"
    ],
    "UnaryExpression": [
      "argument"
    ],
    "UpdateExpression": [
      "argument"
    ],
    "VariableDeclaration": [
      "declarations"
    ],
    "VariableDeclarator": [
      "id",
      "init"
    ],
    "WhileStatement": [
      "test",
      "body"
    ],
    "WithStatement": [
      "object",
      "body"
    ],
    "AssignmentPattern": [
      "left",
      "right",
      "decorators",
      "typeAnnotation"
    ],
    "ArrayPattern": [
      "elements",
      "typeAnnotation",
      "decorators"
    ],
    "ArrowFunctionExpression": [
      "params",
      "body",
      "returnType",
      "typeParameters",
      "predicate"
    ],
    "ClassBody": [
      "body"
    ],
    "ClassExpression": [
      "id",
      "body",
      "superClass",
      "mixins",
      "typeParameters",
      "superTypeParameters",
      "implements",
      "decorators",
      "superTypeArguments"
    ],
    "ClassDeclaration": [
      "id",
      "body",
      "superClass",
      "mixins",
      "typeParameters",
      "superTypeParameters",
      "implements",
      "decorators",
      "superTypeArguments"
    ],
    "ExportAllDeclaration": [
      "source",
      "attributes",
      "assertions",
      "exported"
    ],
    "ExportDefaultDeclaration": [
      "declaration"
    ],
    "ExportNamedDeclaration": [
      "declaration",
      "specifiers",
      "source",
      "attributes",
      "assertions"
    ],
    "ExportSpecifier": [
      "local",
      "exported"
    ],
    "ForOfStatement": [
      "left",
      "right",
      "body"
    ],
    "ImportDeclaration": [
      "specifiers",
      "source",
      "attributes",
      "assertions"
    ],
    "ImportDefaultSpecifier": [
      "local"
    ],
    "ImportNamespaceSpecifier": [
      "local"
    ],
    "ImportSpecifier": [
      "local",
      "imported"
    ],
    "MetaProperty": [
      "meta",
      "property"
    ],
    "ClassMethod": [
      "key",
      "params",
      "body",
      "decorators",
      "returnType",
      "typeParameters"
    ],
    "ObjectPattern": [
      "properties",
      "typeAnnotation",
      "decorators"
    ],
    "SpreadElement": [
      "argument"
    ],
    "Super": [],
    "TaggedTemplateExpression": [
      "tag",
      "quasi",
      "typeParameters",
      "typeArguments"
    ],
    "TemplateElement": [],
    "TemplateLiteral": [
      "quasis",
      "expressions"
    ],
    "YieldExpression": [
      "argument"
    ],
    "AwaitExpression": [
      "argument"
    ],
    "Import": [],
    "BigIntLiteral": [],
    "ExportNamespaceSpecifier": [
      "exported"
    ],
    "OptionalMemberExpression": [
      "object",
      "property"
    ],
    "OptionalCallExpression": [
      "callee",
      "arguments",
      "typeParameters",
      "typeArguments"
    ],
    "ClassProperty": [
      "key",
      "value",
      "typeAnnotation",
      "decorators",
      "variance"
    ],
    "ClassAccessorProperty": [
      "key",
      "value",
      "typeAnnotation",
      "decorators"
    ],
    "ClassPrivateProperty": [
      "key",
      "value",
      "decorators",
      "typeAnnotation",
      "variance"
    ],
    "ClassPrivateMethod": [
      "key",
      "params",
      "body",
      "decorators",
      "returnType",
      "typeParameters"
    ],
    "PrivateName": [
      "id"
    ],
    "StaticBlock": [
      "body"
    ],
    "AnyTypeAnnotation": [],
    "ArrayTypeAnnotation": [
      "elementType"
    ],
    "BooleanTypeAnnotation": [],
    "BooleanLiteralTypeAnnotation": [],
    "NullLiteralTypeAnnotation": [],
    "ClassImplements": [
      "id",
      "typeParameters"
    ],
    "DeclareClass": [
      "id",
      "typeParameters",
      "extends",
      "mixins",
      "implements",
      "body"
    ],
    "DeclareFunction": [
      "id",
      "predicate"
    ],
    "DeclareInterface": [
      "id",
      "typeParameters",
      "extends",
      "body"
    ],
    "DeclareModule": [
      "id",
      "body"
    ],
    "DeclareModuleExports": [
      "typeAnnotation"
    ],
    "DeclareTypeAlias": [
      "id",
      "typeParameters",
      "right"
    ],
    "DeclareOpaqueType": [
      "id",
      "typeParameters",
      "supertype"
    ],
    "DeclareVariable": [
      "id"
    ],
    "DeclareExportDeclaration": [
      "declaration",
      "specifiers",
      "source"
    ],
    "DeclareExportAllDeclaration": [
      "source"
    ],
    "DeclaredPredicate": [
      "value"
    ],
    "ExistsTypeAnnotation": [],
    "FunctionTypeAnnotation": [
      "typeParameters",
      "params",
      "rest",
      "returnType",
      "this"
    ],
    "FunctionTypeParam": [
      "name",
      "typeAnnotation"
    ],
    "GenericTypeAnnotation": [
      "id",
      "typeParameters"
    ],
    "InferredPredicate": [],
    "InterfaceExtends": [
      "id",
      "typeParameters"
    ],
    "InterfaceDeclaration": [
      "id",
      "typeParameters",
      "extends",
      "body"
    ],
    "InterfaceTypeAnnotation": [
      "extends",
      "body"
    ],
    "IntersectionTypeAnnotation": [
      "types"
    ],
    "MixedTypeAnnotation": [],
    "EmptyTypeAnnotation": [],
    "NullableTypeAnnotation": [
      "typeAnnotation"
    ],
    "NumberLiteralTypeAnnotation": [],
    "NumberTypeAnnotation": [],
    "ObjectTypeAnnotation": [
      "properties",
      "indexers",
      "callProperties",
      "internalSlots"
    ],
    "ObjectTypeInternalSlot": [
      "id",
      "value",
      "optional",
      "static",
      "method"
    ],
    "ObjectTypeCallProperty": [
      "value"
    ],
    "ObjectTypeIndexer": [
      "id",
      "key",
      "value",
      "variance"
    ],
    "ObjectTypeProperty": [
      "key",
      "value",
      "variance"
    ],
    "ObjectTypeSpreadProperty": [
      "argument"
    ],
    "OpaqueType": [
      "id",
      "typeParameters",
      "supertype",
      "impltype"
    ],
    "QualifiedTypeIdentifier": [
      "id",
      "qualification"
    ],
    "StringLiteralTypeAnnotation": [],
    "StringTypeAnnotation": [],
    "SymbolTypeAnnotation": [],
    "ThisTypeAnnotation": [],
    "TupleTypeAnnotation": [
      "types",
      "elementTypes"
    ],
    "TypeofTypeAnnotation": [
      "argument"
    ],
    "TypeAlias": [
      "id",
      "typeParameters",
      "right"
    ],
    "TypeAnnotation": [
      "typeAnnotation"
    ],
    "TypeCastExpression": [
      "expression",
      "typeAnnotation"
    ],
    "TypeParameter": [
      "bound",
      "default",
      "variance"
    ],
    "TypeParameterDeclaration": [
      "params"
    ],
    "TypeParameterInstantiation": [
      "params"
    ],
    "UnionTypeAnnotation": [
      "types"
    ],
    "Variance": [],
    "VoidTypeAnnotation": [],
    "EnumDeclaration": [
      "id",
      "body"
    ],
    "EnumBooleanBody": [
      "members"
    ],
    "EnumNumberBody": [
      "members"
    ],
    "EnumStringBody": [
      "members"
    ],
    "EnumSymbolBody": [
      "members"
    ],
    "EnumBooleanMember": [
      "id",
      "init"
    ],
    "EnumNumberMember": [
      "id",
      "init"
    ],
    "EnumStringMember": [
      "id",
      "init"
    ],
    "EnumDefaultedMember": [
      "id"
    ],
    "IndexedAccessType": [
      "objectType",
      "indexType"
    ],
    "OptionalIndexedAccessType": [
      "objectType",
      "indexType"
    ],
    "JSXAttribute": [
      "name",
      "value"
    ],
    "JSXClosingElement": [
      "name"
    ],
    "JSXElement": [
      "openingElement",
      "children",
      "closingElement"
    ],
    "JSXEmptyExpression": [],
    "JSXExpressionContainer": [
      "expression"
    ],
    "JSXSpreadChild": [
      "expression"
    ],
    "JSXIdentifier": [],
    "JSXMemberExpression": [
      "object",
      "property"
    ],
    "JSXNamespacedName": [
      "namespace",
      "name"
    ],
    "JSXOpeningElement": [
      "name",
      "attributes",
      "typeArguments",
      "typeParameters"
    ],
    "JSXSpreadAttribute": [
      "argument"
    ],
    "JSXText": [],
    "JSXFragment": [
      "openingFragment",
      "children",
      "closingFragment"
    ],
    "JSXOpeningFragment": [],
    "JSXClosingFragment": [],
    "Noop": [],
    "Placeholder": [],
    "V8IntrinsicIdentifier": [],
    "ArgumentPlaceholder": [],
    "BindExpression": [
      "object",
      "callee"
    ],
    "ImportAttribute": [
      "key",
      "value"
    ],
    "Decorator": [
      "expression"
    ],
    "DoExpression": [
      "body"
    ],
    "ExportDefaultSpecifier": [
      "exported"
    ],
    "RecordExpression": [
      "properties"
    ],
    "TupleExpression": [
      "elements"
    ],
    "DecimalLiteral": [],
    "ModuleExpression": [
      "body"
    ],
    "TopicReference": [],
    "PipelineTopicExpression": [
      "expression"
    ],
    "PipelineBareFunction": [
      "callee"
    ],
    "PipelinePrimaryTopicReference": [],
    "TSParameterProperty": [
      "parameter",
      "decorators"
    ],
    "TSDeclareFunction": [
      "id",
      "typeParameters",
      "params",
      "returnType",
      "body"
    ],
    "TSDeclareMethod": [
      "decorators",
      "key",
      "typeParameters",
      "params",
      "returnType"
    ],
    "TSQualifiedName": [
      "left",
      "right"
    ],
    "TSCallSignatureDeclaration": [
      "typeParameters",
      "parameters",
      "typeAnnotation",
      "params",
      "returnType"
    ],
    "TSConstructSignatureDeclaration": [
      "typeParameters",
      "parameters",
      "typeAnnotation",
      "params",
      "returnType"
    ],
    "TSPropertySignature": [
      "key",
      "typeAnnotation"
    ],
    "TSMethodSignature": [
      "key",
      "typeParameters",
      "parameters",
      "typeAnnotation",
      "params",
      "returnType"
    ],
    "TSIndexSignature": [
      "parameters",
      "typeAnnotation"
    ],
    "TSAnyKeyword": [],
    "TSBooleanKeyword": [],
    "TSBigIntKeyword": [],
    "TSIntrinsicKeyword": [],
    "TSNeverKeyword": [],
    "TSNullKeyword": [],
    "TSNumberKeyword": [],
    "TSObjectKeyword": [],
    "TSStringKeyword": [],
    "TSSymbolKeyword": [],
    "TSUndefinedKeyword": [],
    "TSUnknownKeyword": [],
    "TSVoidKeyword": [],
    "TSThisType": [],
    "TSFunctionType": [
      "typeParameters",
      "parameters",
      "typeAnnotation",
      "params",
      "returnType"
    ],
    "TSConstructorType": [
      "typeParameters",
      "parameters",
      "typeAnnotation",
      "params",
      "returnType"
    ],
    "TSTypeReference": [
      "typeName",
      "typeParameters",
      "typeArguments"
    ],
    "TSTypePredicate": [
      "parameterName",
      "typeAnnotation"
    ],
    "TSTypeQuery": [
      "exprName",
      "typeParameters",
      "typeArguments"
    ],
    "TSTypeLiteral": [
      "members"
    ],
    "TSArrayType": [
      "elementType"
    ],
    "TSTupleType": [
      "elementTypes"
    ],
    "TSOptionalType": [
      "typeAnnotation"
    ],
    "TSRestType": [
      "typeAnnotation"
    ],
    "TSNamedTupleMember": [
      "label",
      "elementType"
    ],
    "TSUnionType": [
      "types"
    ],
    "TSIntersectionType": [
      "types"
    ],
    "TSConditionalType": [
      "checkType",
      "extendsType",
      "trueType",
      "falseType"
    ],
    "TSInferType": [
      "typeParameter"
    ],
    "TSParenthesizedType": [
      "typeAnnotation"
    ],
    "TSTypeOperator": [
      "typeAnnotation"
    ],
    "TSIndexedAccessType": [
      "objectType",
      "indexType"
    ],
    "TSMappedType": [
      "typeParameter",
      "typeAnnotation",
      "nameType"
    ],
    "TSLiteralType": [
      "literal"
    ],
    "TSExpressionWithTypeArguments": [
      "expression",
      "typeParameters"
    ],
    "TSInterfaceDeclaration": [
      "id",
      "typeParameters",
      "extends",
      "body"
    ],
    "TSInterfaceBody": [
      "body"
    ],
    "TSTypeAliasDeclaration": [
      "id",
      "typeParameters",
      "typeAnnotation"
    ],
    "TSInstantiationExpression": [
      "expression",
      "typeParameters",
      "typeArguments"
    ],
    "TSAsExpression": [
      "expression",
      "typeAnnotation"
    ],
    "TSSatisfiesExpression": [
      "expression",
      "typeAnnotation"
    ],
    "TSTypeAssertion": [
      "typeAnnotation",
      "expression"
    ],
    "TSEnumDeclaration": [
      "id",
      "members"
    ],
    "TSEnumMember": [
      "id",
      "initializer"
    ],
    "TSModuleDeclaration": [
      "id",
      "body"
    ],
    "TSModuleBlock": [
      "body"
    ],
    "TSImportType": [
      "argument",
      "qualifier",
      "typeParameters",
      "typeArguments"
    ],
    "TSImportEqualsDeclaration": [
      "id",
      "moduleReference"
    ],
    "TSExternalModuleReference": [
      "expression"
    ],
    "TSNonNullExpression": [
      "expression"
    ],
    "TSExportAssignment": [
      "expression"
    ],
    "TSNamespaceExportDeclaration": [
      "id"
    ],
    "TSTypeAnnotation": [
      "typeAnnotation"
    ],
    "TSTypeParameterInstantiation": [
      "params"
    ],
    "TSTypeParameterDeclaration": [
      "params"
    ],
    "TSTypeParameter": [
      "constraint",
      "default",
      "name"
    ],
    "ChainExpression": [
      "expression"
    ],
    "ExperimentalRestProperty": [
      "argument"
    ],
    "ExperimentalSpreadProperty": [
      "argument"
    ],
    "ImportExpression": [
      "source",
      "attributes"
    ],
    "Literal": [],
    "MethodDefinition": [
      "decorators",
      "key",
      "value"
    ],
    "PrivateIdentifier": [],
    "Property": [
      "key",
      "value"
    ],
    "PropertyDefinition": [
      "decorators",
      "key",
      "typeAnnotation",
      "value",
      "variance"
    ],
    "AccessorProperty": [
      "decorators",
      "key",
      "typeAnnotation",
      "value"
    ],
    "TSAbstractAccessorProperty": [
      "decorators",
      "key",
      "typeAnnotation"
    ],
    "TSAbstractKeyword": [],
    "TSAbstractMethodDefinition": [
      "key",
      "value"
    ],
    "TSAbstractPropertyDefinition": [
      "decorators",
      "key",
      "typeAnnotation"
    ],
    "TSAsyncKeyword": [],
    "TSClassImplements": [
      "expression",
      "typeArguments",
      "typeParameters"
    ],
    "TSDeclareKeyword": [],
    "TSEmptyBodyFunctionExpression": [
      "id",
      "typeParameters",
      "params",
      "returnType"
    ],
    "TSExportKeyword": [],
    "TSInterfaceHeritage": [
      "expression",
      "typeArguments",
      "typeParameters"
    ],
    "TSPrivateKeyword": [],
    "TSProtectedKeyword": [],
    "TSPublicKeyword": [],
    "TSReadonlyKeyword": [],
    "TSStaticKeyword": [],
    "TSTemplateLiteralType": [
      "quasis",
      "types"
    ],
    "BigIntLiteralTypeAnnotation": [],
    "BigIntTypeAnnotation": [],
    "ComponentDeclaration": [
      "id",
      "params",
      "body",
      "typeParameters",
      "rendersType"
    ],
    "ComponentParameter": [
      "name",
      "local"
    ],
    "ComponentTypeAnnotation": [
      "params",
      "rest",
      "typeParameters",
      "rendersType"
    ],
    "ComponentTypeParameter": [
      "name",
      "typeAnnotation"
    ],
    "ConditionalTypeAnnotation": [
      "checkType",
      "extendsType",
      "trueType",
      "falseType"
    ],
    "DeclareComponent": [
      "id",
      "params",
      "rest",
      "typeParameters",
      "rendersType"
    ],
    "DeclareEnum": [
      "id",
      "body"
    ],
    "InferTypeAnnotation": [
      "typeParameter"
    ],
    "KeyofTypeAnnotation": [
      "argument"
    ],
    "ObjectTypeMappedTypeProperty": [
      "keyTparam",
      "propType",
      "sourceType",
      "variance"
    ],
    "QualifiedTypeofIdentifier": [
      "qualification",
      "id"
    ],
    "TupleTypeLabeledElement": [
      "label",
      "elementType",
      "variance"
    ],
    "TupleTypeSpreadElement": [
      "label",
      "typeAnnotation"
    ],
    "TypeOperator": [
      "typeAnnotation"
    ],
    "TypePredicate": [
      "parameterName",
      "typeAnnotation",
      "asserts"
    ],
    "NGRoot": [
      "node"
    ],
    "NGPipeExpression": [
      "left",
      "right",
      "arguments"
    ],
    "NGChainedExpression": [
      "expressions"
    ],
    "NGEmptyExpression": [],
    "NGMicrosyntax": [
      "body"
    ],
    "NGMicrosyntaxKey": [],
    "NGMicrosyntaxExpression": [
      "expression",
      "alias"
    ],
    "NGMicrosyntaxKeyedExpression": [
      "key",
      "expression"
    ],
    "NGMicrosyntaxLet": [
      "key",
      "value"
    ],
    "NGMicrosyntaxAs": [
      "key",
      "alias"
    ],
    "JsExpressionRoot": [
      "node"
    ],
    "JsonRoot": [
      "node"
    ],
    "TSJSDocAllType": [],
    "TSJSDocUnknownType": [],
    "TSJSDocNullableType": [
      "typeAnnotation"
    ],
    "TSJSDocNonNullableType": [
      "typeAnnotation"
    ],
    "NeverTypeAnnotation": [],
    "UndefinedTypeAnnotation": [],
    "UnknownTypeAnnotation": []
  };

  // src/language-js/traverse/get-visitor-keys.js
  var getVisitorKeys = create_get_visitor_keys_default(visitor_keys_evaluate_default);
  var get_visitor_keys_default = getVisitorKeys;

  // src/language-js/utils/create-type-check-function.js
  function createTypeCheckFunction(types) {
    types = new Set(types);
    return (node) => types.has(node == null ? void 0 : node.type);
  }
  var create_type_check_function_default = createTypeCheckFunction;

  // src/language-js/utils/is-block-comment.js
  var isBlockComment = create_type_check_function_default([
    "Block",
    "CommentBlock",
    // `meriyah`
    "MultiLine"
  ]);
  var is_block_comment_default = isBlockComment;

  // src/language-js/utils/is-node-matches.js
  function isNodeMatchesNameOrPath(node, nameOrPath) {
    const names = nameOrPath.split(".");
    for (let index = names.length - 1; index >= 0; index--) {
      const name = names[index];
      if (index === 0) {
        return node.type === "Identifier" && node.name === name;
      }
      if (node.type !== "MemberExpression" || node.optional || node.computed || node.property.type !== "Identifier" || node.property.name !== name) {
        return false;
      }
      node = node.object;
    }
  }
  function isNodeMatches(node, nameOrPaths) {
    return nameOrPaths.some(
      (nameOrPath) => isNodeMatchesNameOrPath(node, nameOrPath)
    );
  }
  var is_node_matches_default = isNodeMatches;

  // src/language-js/utils/is-flow-keyword-type.js
  var isFlowKeywordType = create_type_check_function_default([
    "AnyTypeAnnotation",
    "ThisTypeAnnotation",
    "NumberTypeAnnotation",
    "VoidTypeAnnotation",
    "BooleanTypeAnnotation",
    "BigIntTypeAnnotation",
    "SymbolTypeAnnotation",
    "StringTypeAnnotation",
    "NeverTypeAnnotation",
    "UndefinedTypeAnnotation",
    "UnknownTypeAnnotation",
    // FLow only
    "EmptyTypeAnnotation",
    "MixedTypeAnnotation"
  ]);
  var is_flow_keyword_type_default = isFlowKeywordType;

  // src/language-js/utils/is-ts-keyword-type.js
  function isTsKeywordType({ type }) {
    return type.startsWith("TS") && type.endsWith("Keyword");
  }
  var is_ts_keyword_type_default = isTsKeywordType;

  // src/language-js/utils/index.js
  function hasNode(node, predicate) {
    return predicate(node) || hasDescendant(node, {
      getVisitorKeys: get_visitor_keys_default,
      predicate
    });
  }
  function hasNakedLeftSide(node) {
    return node.type === "AssignmentExpression" || node.type === "BinaryExpression" || node.type === "LogicalExpression" || node.type === "NGPipeExpression" || node.type === "ConditionalExpression" || isCallExpression(node) || isMemberExpression(node) || node.type === "SequenceExpression" || node.type === "TaggedTemplateExpression" || node.type === "BindExpression" || node.type === "UpdateExpression" && !node.prefix || isTSTypeExpression(node) || node.type === "TSNonNullExpression" || node.type === "ChainExpression";
  }
  function getLeftSide(node) {
    if (node.expressions) {
      return node.expressions[0];
    }
    return node.left ?? node.test ?? node.callee ?? node.object ?? node.tag ?? node.argument ?? node.expression;
  }
  function getLeftSidePathName(node) {
    if (node.expressions) {
      return ["expressions", 0];
    }
    if (node.left) {
      return ["left"];
    }
    if (node.test) {
      return ["test"];
    }
    if (node.object) {
      return ["object"];
    }
    if (node.callee) {
      return ["callee"];
    }
    if (node.tag) {
      return ["tag"];
    }
    if (node.argument) {
      return ["argument"];
    }
    if (node.expression) {
      return ["expression"];
    }
    throw new Error("Unexpected node has no left side.");
  }
  var isLineComment = create_type_check_function_default([
    "Line",
    "CommentLine",
    // `meriyah` has `SingleLine`, `HashbangComment`, `HTMLOpen`, and `HTMLClose`
    "SingleLine",
    "HashbangComment",
    "HTMLOpen",
    "HTMLClose",
    // `espree`
    "Hashbang",
    // Babel hashbang
    "InterpreterDirective"
  ]);
  var isExportDeclaration = create_type_check_function_default(["ExportDefaultDeclaration", "DeclareExportDeclaration", "ExportNamedDeclaration", "ExportAllDeclaration", "DeclareExportAllDeclaration"]);
  var isArrayOrTupleExpression = create_type_check_function_default(["ArrayExpression", "TupleExpression"]);
  var isObjectOrRecordExpression = create_type_check_function_default(["ObjectExpression", "RecordExpression"]);
  function isNumericLiteral(node) {
    return node.type === "NumericLiteral" || node.type === "Literal" && typeof node.value === "number";
  }
  function isSignedNumericLiteral(node) {
    return node.type === "UnaryExpression" && (node.operator === "+" || node.operator === "-") && isNumericLiteral(node.argument);
  }
  function isStringLiteral(node) {
    return node.type === "StringLiteral" || node.type === "Literal" && typeof node.value === "string";
  }
  function isRegExpLiteral(node) {
    return node.type === "RegExpLiteral" || node.type === "Literal" && Boolean(node.regex);
  }
  var isObjectType = create_type_check_function_default(["ObjectTypeAnnotation", "TSTypeLiteral", "TSMappedType"]);
  var isFunctionOrArrowExpression = create_type_check_function_default(["FunctionExpression", "ArrowFunctionExpression"]);
  function isFunctionOrArrowExpressionWithBody(node) {
    return node.type === "FunctionExpression" || node.type === "ArrowFunctionExpression" && node.body.type === "BlockStatement";
  }
  function isAngularTestWrapper(node) {
    return isCallExpression(node) && node.callee.type === "Identifier" && ["async", "inject", "fakeAsync", "waitForAsync"].includes(node.callee.name);
  }
  var isJsxElement = create_type_check_function_default(["JSXElement", "JSXFragment"]);
  function isGetterOrSetter(node) {
    return node.kind === "get" || node.kind === "set";
  }
  function isFunctionNotation(node) {
    return isGetterOrSetter(node) || hasSameLocStart(node, node.value);
  }
  function isObjectTypePropertyAFunction(node) {
    return (node.type === "ObjectTypeProperty" || node.type === "ObjectTypeInternalSlot") && node.value.type === "FunctionTypeAnnotation" && !node.static && !isFunctionNotation(node);
  }
  function isTypeAnnotationAFunction(node) {
    return (node.type === "TypeAnnotation" || node.type === "TSTypeAnnotation") && node.typeAnnotation.type === "FunctionTypeAnnotation" && !node.static && !hasSameLocStart(node, node.typeAnnotation);
  }
  var isBinaryish = create_type_check_function_default(["BinaryExpression", "LogicalExpression", "NGPipeExpression"]);
  function isMemberish(node) {
    return isMemberExpression(node) || node.type === "BindExpression" && Boolean(node.object);
  }
  var isSimpleTypeAnnotation = create_type_check_function_default([
    "TSThisType",
    // literals
    "NullLiteralTypeAnnotation",
    "BooleanLiteralTypeAnnotation",
    "StringLiteralTypeAnnotation",
    "BigIntLiteralTypeAnnotation",
    "NumberLiteralTypeAnnotation",
    "TSLiteralType",
    "TSTemplateLiteralType"
  ]);
  function isSimpleType(node) {
    return is_ts_keyword_type_default(node) || is_flow_keyword_type_default(node) || isSimpleTypeAnnotation(node) || (node.type === "GenericTypeAnnotation" || node.type === "TSTypeReference") && !node.typeParameters;
  }
  function isUnitTestSetUp(node) {
    const unitTestSetUpRe = /^(?:before|after)(?:Each|All)$/;
    return node.callee.type === "Identifier" && node.arguments.length === 1 && unitTestSetUpRe.test(node.callee.name);
  }
  var testCallCalleePatterns = ["it", "it.only", "it.skip", "describe", "describe.only", "describe.skip", "test", "test.only", "test.skip", "test.step", "test.describe", "test.describe.only", "test.describe.parallel", "test.describe.parallel.only", "test.describe.serial", "test.describe.serial.only", "skip", "xit", "xdescribe", "xtest", "fit", "fdescribe", "ftest"];
  function isTestCallCallee(node) {
    return is_node_matches_default(node, testCallCalleePatterns);
  }
  function isTestCall(node, parent) {
    if (node.type !== "CallExpression") {
      return false;
    }
    if (node.arguments.length === 1) {
      if (isAngularTestWrapper(node) && parent && isTestCall(parent)) {
        return isFunctionOrArrowExpression(node.arguments[0]);
      }
      if (isUnitTestSetUp(node)) {
        return isAngularTestWrapper(node.arguments[0]);
      }
    } else if ((node.arguments.length === 2 || node.arguments.length === 3) && (node.arguments[0].type === "TemplateLiteral" || isStringLiteral(node.arguments[0])) && isTestCallCallee(node.callee)) {
      if (node.arguments[2] && !isNumericLiteral(node.arguments[2])) {
        return false;
      }
      return (node.arguments.length === 2 ? isFunctionOrArrowExpression(node.arguments[1]) : isFunctionOrArrowExpressionWithBody(node.arguments[1]) && getFunctionParameters(node.arguments[1]).length <= 1) || isAngularTestWrapper(node.arguments[1]);
    }
    return false;
  }
  var isCallExpression = create_type_check_function_default(["CallExpression", "OptionalCallExpression"]);
  var isMemberExpression = create_type_check_function_default(["MemberExpression", "OptionalMemberExpression"]);
  function isSimpleTemplateLiteral(node) {
    let expressionsKey = "expressions";
    if (node.type === "TSTemplateLiteralType") {
      expressionsKey = "types";
    }
    const expressions = node[expressionsKey];
    if (expressions.length === 0) {
      return false;
    }
    return expressions.every((expr) => {
      if (hasComment(expr)) {
        return false;
      }
      if (expr.type === "Identifier" || expr.type === "ThisExpression") {
        return true;
      }
      if (expr.type === "ChainExpression") {
        expr = expr.expression;
      }
      if (isMemberExpression(expr)) {
        let head = expr;
        while (isMemberExpression(head)) {
          if (head.property.type !== "Identifier" && head.property.type !== "Literal" && head.property.type !== "StringLiteral" && head.property.type !== "NumericLiteral") {
            return false;
          }
          head = head.object;
          if (hasComment(head)) {
            return false;
          }
        }
        if (head.type === "Identifier" || head.type === "ThisExpression") {
          return true;
        }
        return false;
      }
      return false;
    });
  }
  function hasLeadingOwnLineComment(text, node) {
    if (isJsxElement(node)) {
      return hasNodeIgnoreComment(node);
    }
    return hasComment(node, CommentCheckFlags.Leading, (comment) => has_newline_default(text, locEnd(comment)));
  }
  function isStringPropSafeToUnquote(node, options2) {
    return options2.parser !== "json" && isStringLiteral(node.key) && rawText(node.key).slice(1, -1) === node.key.value && (src_default(node.key.value) && // With `--strictPropertyInitialization`, TS treats properties with quoted names differently than unquoted ones.
    // See https://github.com/microsoft/TypeScript/pull/20075
    !(options2.parser === "babel-ts" && node.type === "ClassProperty" || options2.parser === "typescript" && node.type === "PropertyDefinition") || isSimpleNumber(node.key.value) && String(Number(node.key.value)) === node.key.value && (options2.parser === "babel" || options2.parser === "acorn" || options2.parser === "espree" || options2.parser === "meriyah" || options2.parser === "__babel_estree"));
  }
  function isSimpleNumber(numberString) {
    return /^(?:\d+|\d+\.\d+)$/.test(numberString);
  }
  function templateLiteralHasNewLines(template) {
    return template.quasis.some((quasi) => quasi.value.raw.includes("\n"));
  }
  function isTemplateOnItsOwnLine(node, text) {
    return (node.type === "TemplateLiteral" && templateLiteralHasNewLines(node) || node.type === "TaggedTemplateExpression" && templateLiteralHasNewLines(node.quasi)) && !has_newline_default(text, locStart(node), {
      backwards: true
    });
  }
  function needsHardlineAfterDanglingComment(node) {
    if (!hasComment(node)) {
      return false;
    }
    const lastDanglingComment = at_default(
      /* isOptionalObject*/
      false,
      getComments(node, CommentCheckFlags.Dangling),
      -1
    );
    return lastDanglingComment && !is_block_comment_default(lastDanglingComment);
  }
  function isFunctionCompositionArgs(args) {
    if (args.length <= 1) {
      return false;
    }
    let count = 0;
    for (const arg of args) {
      if (isFunctionOrArrowExpression(arg)) {
        count += 1;
        if (count > 1) {
          return true;
        }
      } else if (isCallExpression(arg)) {
        for (const childArg of arg.arguments) {
          if (isFunctionOrArrowExpression(childArg)) {
            return true;
          }
        }
      }
    }
    return false;
  }
  function isLongCurriedCallExpression(path) {
    const {
      node,
      parent,
      key
    } = path;
    return key === "callee" && isCallExpression(node) && isCallExpression(parent) && parent.arguments.length > 0 && node.arguments.length > parent.arguments.length;
  }
  var simpleCallArgumentUnaryOperators = /* @__PURE__ */ new Set(["!", "-", "+", "~"]);
  function isSimpleCallArgument(node, depth = 2) {
    if (depth <= 0) {
      return false;
    }
    const isChildSimple = (child) => isSimpleCallArgument(child, depth - 1);
    if (isRegExpLiteral(node)) {
      return get_string_width_default(node.pattern ?? node.regex.pattern) <= 5;
    }
    if (node.type === "Literal" || node.type === "BigIntLiteral" || node.type === "DecimalLiteral" || node.type === "BooleanLiteral" || node.type === "NullLiteral" || node.type === "NumericLiteral" || node.type === "StringLiteral" || node.type === "Identifier" || node.type === "ThisExpression" || node.type === "Super" || node.type === "PrivateName" || node.type === "PrivateIdentifier" || node.type === "ArgumentPlaceholder" || node.type === "Import") {
      return true;
    }
    if (node.type === "TemplateLiteral") {
      return node.quasis.every((element) => !element.value.raw.includes("\n")) && node.expressions.every(isChildSimple);
    }
    if (isObjectOrRecordExpression(node)) {
      return node.properties.every((p) => !p.computed && (p.shorthand || p.value && isChildSimple(p.value)));
    }
    if (isArrayOrTupleExpression(node)) {
      return node.elements.every((x) => x === null || isChildSimple(x));
    }
    if (isCallLikeExpression(node)) {
      if (node.type === "ImportExpression" || isSimpleCallArgument(node.callee, depth)) {
        const args = getCallArguments(node);
        return args.length <= depth && args.every(isChildSimple);
      }
      return false;
    }
    if (isMemberExpression(node)) {
      return isSimpleCallArgument(node.object, depth) && isSimpleCallArgument(node.property, depth);
    }
    if (node.type === "UnaryExpression" && simpleCallArgumentUnaryOperators.has(node.operator) || node.type === "UpdateExpression") {
      return isSimpleCallArgument(node.argument, depth);
    }
    if (node.type === "TSNonNullExpression") {
      return isSimpleCallArgument(node.expression, depth);
    }
    return false;
  }
  function rawText(node) {
    var _a;
    return ((_a = node.extra) == null ? void 0 : _a.raw) ?? node.raw;
  }
  function identity(x) {
    return x;
  }
  function shouldPrintComma(options2, level = "es5") {
    return options2.trailingComma === "es5" && level === "es5" || options2.trailingComma === "all" && (level === "all" || level === "es5");
  }
  function startsWithNoLookaheadToken(node, predicate) {
    switch (node.type) {
      case "BinaryExpression":
      case "LogicalExpression":
      case "AssignmentExpression":
      case "NGPipeExpression":
        return startsWithNoLookaheadToken(node.left, predicate);
      case "MemberExpression":
      case "OptionalMemberExpression":
        return startsWithNoLookaheadToken(node.object, predicate);
      case "TaggedTemplateExpression":
        if (node.tag.type === "FunctionExpression") {
          return false;
        }
        return startsWithNoLookaheadToken(node.tag, predicate);
      case "CallExpression":
      case "OptionalCallExpression":
        if (node.callee.type === "FunctionExpression") {
          return false;
        }
        return startsWithNoLookaheadToken(node.callee, predicate);
      case "ConditionalExpression":
        return startsWithNoLookaheadToken(node.test, predicate);
      case "UpdateExpression":
        return !node.prefix && startsWithNoLookaheadToken(node.argument, predicate);
      case "BindExpression":
        return node.object && startsWithNoLookaheadToken(node.object, predicate);
      case "SequenceExpression":
        return startsWithNoLookaheadToken(node.expressions[0], predicate);
      case "ChainExpression":
      case "TSSatisfiesExpression":
      case "TSAsExpression":
      case "TSNonNullExpression":
        return startsWithNoLookaheadToken(node.expression, predicate);
      default:
        return predicate(node);
    }
  }
  var equalityOperators = {
    "==": true,
    "!=": true,
    "===": true,
    "!==": true
  };
  var multiplicativeOperators = {
    "*": true,
    "/": true,
    "%": true
  };
  var bitshiftOperators = {
    ">>": true,
    ">>>": true,
    "<<": true
  };
  function shouldFlatten(parentOp, nodeOp) {
    if (getPrecedence(nodeOp) !== getPrecedence(parentOp)) {
      return false;
    }
    if (parentOp === "**") {
      return false;
    }
    if (equalityOperators[parentOp] && equalityOperators[nodeOp]) {
      return false;
    }
    if (nodeOp === "%" && multiplicativeOperators[parentOp] || parentOp === "%" && multiplicativeOperators[nodeOp]) {
      return false;
    }
    if (nodeOp !== parentOp && multiplicativeOperators[nodeOp] && multiplicativeOperators[parentOp]) {
      return false;
    }
    if (bitshiftOperators[parentOp] && bitshiftOperators[nodeOp]) {
      return false;
    }
    return true;
  }
  var PRECEDENCE = new Map([["|>"], ["??"], ["||"], ["&&"], ["|"], ["^"], ["&"], ["==", "===", "!=", "!=="], ["<", ">", "<=", ">=", "in", "instanceof"], [">>", "<<", ">>>"], ["+", "-"], ["*", "/", "%"], ["**"]].flatMap((operators, index) => operators.map((operator) => [operator, index])));
  function getPrecedence(operator) {
    return PRECEDENCE.get(operator);
  }
  function isBitwiseOperator(operator) {
    return Boolean(bitshiftOperators[operator]) || operator === "|" || operator === "^" || operator === "&";
  }
  function hasRestParameter(node) {
    var _a;
    if (node.rest) {
      return true;
    }
    const parameters = getFunctionParameters(node);
    return ((_a = at_default(
      /* isOptionalObject*/
      false,
      parameters,
      -1
    )) == null ? void 0 : _a.type) === "RestElement";
  }
  var functionParametersCache = /* @__PURE__ */ new WeakMap();
  function getFunctionParameters(node) {
    if (functionParametersCache.has(node)) {
      return functionParametersCache.get(node);
    }
    const parameters = [];
    if (node.this) {
      parameters.push(node.this);
    }
    if (Array.isArray(node.parameters)) {
      parameters.push(...node.parameters);
    } else if (Array.isArray(node.params)) {
      parameters.push(...node.params);
    }
    if (node.rest) {
      parameters.push(node.rest);
    }
    functionParametersCache.set(node, parameters);
    return parameters;
  }
  function iterateFunctionParametersPath(path, iteratee) {
    const {
      node
    } = path;
    let index = 0;
    const callback = (childPath) => iteratee(childPath, index++);
    if (node.this) {
      path.call(callback, "this");
    }
    if (Array.isArray(node.parameters)) {
      path.each(callback, "parameters");
    } else if (Array.isArray(node.params)) {
      path.each(callback, "params");
    }
    if (node.rest) {
      path.call(callback, "rest");
    }
  }
  var callArgumentsCache = /* @__PURE__ */ new WeakMap();
  function getCallArguments(node) {
    if (callArgumentsCache.has(node)) {
      return callArgumentsCache.get(node);
    }
    let args = node.arguments;
    if (node.type === "ImportExpression") {
      args = [node.source];
      if (node.attributes) {
        args.push(node.attributes);
      }
    }
    callArgumentsCache.set(node, args);
    return args;
  }
  function iterateCallArgumentsPath(path, iteratee) {
    const {
      node
    } = path;
    if (node.type === "ImportExpression") {
      path.call((sourcePath) => iteratee(sourcePath, 0), "source");
      if (node.attributes) {
        path.call((sourcePath) => iteratee(sourcePath, 1), "attributes");
      }
    } else {
      path.each(iteratee, "arguments");
    }
  }
  function getCallArgumentSelector(node, index) {
    if (node.type === "ImportExpression") {
      if (index === 0 || index === (node.attributes ? -2 : -1)) {
        return "source";
      }
      if (node.attributes && (index === 1 || index === -1)) {
        return "attributes";
      }
      throw new RangeError("Invalid argument index");
    }
    if (index < 0) {
      index = node.arguments.length + index;
    }
    if (index < 0 || index >= node.arguments.length) {
      throw new RangeError("Invalid argument index");
    }
    return ["arguments", index];
  }
  function isPrettierIgnoreComment(comment) {
    return comment.value.trim() === "prettier-ignore" && !comment.unignore;
  }
  function hasNodeIgnoreComment(node) {
    return (node == null ? void 0 : node.prettierIgnore) || hasComment(node, CommentCheckFlags.PrettierIgnore);
  }
  var CommentCheckFlags = {
    /** Check comment is a leading comment */
    Leading: 1 << 1,
    /** Check comment is a trailing comment */
    Trailing: 1 << 2,
    /** Check comment is a dangling comment */
    Dangling: 1 << 3,
    /** Check comment is a block comment */
    Block: 1 << 4,
    /** Check comment is a line comment */
    Line: 1 << 5,
    /** Check comment is a `prettier-ignore` comment */
    PrettierIgnore: 1 << 6,
    /** Check comment is the first attached comment */
    First: 1 << 7,
    /** Check comment is the last attached comment */
    Last: 1 << 8
  };
  var getCommentTestFunction = (flags, fn) => {
    if (typeof flags === "function") {
      fn = flags;
      flags = 0;
    }
    if (flags || fn) {
      return (comment, index, comments) => !(flags & CommentCheckFlags.Leading && !comment.leading || flags & CommentCheckFlags.Trailing && !comment.trailing || flags & CommentCheckFlags.Dangling && (comment.leading || comment.trailing) || flags & CommentCheckFlags.Block && !is_block_comment_default(comment) || flags & CommentCheckFlags.Line && !isLineComment(comment) || flags & CommentCheckFlags.First && index !== 0 || flags & CommentCheckFlags.Last && index !== comments.length - 1 || flags & CommentCheckFlags.PrettierIgnore && !isPrettierIgnoreComment(comment) || fn && !fn(comment));
    }
  };
  function hasComment(node, flags, fn) {
    if (!is_non_empty_array_default(node == null ? void 0 : node.comments)) {
      return false;
    }
    const test = getCommentTestFunction(flags, fn);
    return test ? node.comments.some(test) : true;
  }
  function getComments(node, flags, fn) {
    if (!Array.isArray(node == null ? void 0 : node.comments)) {
      return [];
    }
    const test = getCommentTestFunction(flags, fn);
    return test ? node.comments.filter(test) : node.comments;
  }
  var isNextLineEmpty2 = (node, {
    originalText
  }) => is_next_line_empty_default(originalText, locEnd(node));
  function isCallLikeExpression(node) {
    return isCallExpression(node) || node.type === "NewExpression" || node.type === "ImportExpression";
  }
  function isObjectProperty(node) {
    return node && (node.type === "ObjectProperty" || node.type === "Property" && !node.method && node.kind === "init");
  }
  var markerForIfWithoutBlockAndSameLineComment = Symbol("ifWithoutBlockAndSameLineComment");
  var isTSTypeExpression = create_type_check_function_default(["TSAsExpression", "TSSatisfiesExpression"]);

  // src/language-js/needs-parens.js
  function needsParens(path, options2) {
    var _a, _b, _c, _d, _e, _f, _g;
    if (path.isRoot) {
      return false;
    }
    const { node, key, parent } = path;
    if (options2.__isInHtmlInterpolation && !options2.bracketSpacing && endsWithRightBracket(node) && isFollowedByRightBracket(path)) {
      return true;
    }
    if (isStatement(node)) {
      return false;
    }
    if (node.type === "Identifier") {
      if (((_a = node.extra) == null ? void 0 : _a.parenthesized) && /^PRETTIER_HTML_PLACEHOLDER_\d+_\d+_IN_JS$/.test(node.name)) {
        return true;
      }
      if (key === "left" && (node.name === "async" && !parent.await || node.name === "let") && parent.type === "ForOfStatement") {
        return true;
      }
      if (node.name === "let") {
        const expression = (_b = path.findAncestor(
          (node2) => node2.type === "ForOfStatement"
        )) == null ? void 0 : _b.left;
        if (expression && startsWithNoLookaheadToken(
          expression,
          (leftmostNode) => leftmostNode === node
        )) {
          return true;
        }
      }
      if (key === "object" && node.name === "let" && parent.type === "MemberExpression" && parent.computed && !parent.optional) {
        const statement = path.findAncestor(
          (node2) => node2.type === "ExpressionStatement" || node2.type === "ForStatement" || node2.type === "ForInStatement"
        );
        const expression = !statement ? void 0 : statement.type === "ExpressionStatement" ? statement.expression : statement.type === "ForStatement" ? statement.init : statement.left;
        if (expression && startsWithNoLookaheadToken(
          expression,
          (leftmostNode) => leftmostNode === node
        )) {
          return true;
        }
      }
      return false;
    }
    if (node.type === "ObjectExpression" || node.type === "FunctionExpression" || node.type === "ClassExpression" || node.type === "DoExpression") {
      const expression = (_c = path.findAncestor(
        (node2) => node2.type === "ExpressionStatement"
      )) == null ? void 0 : _c.expression;
      if (expression && startsWithNoLookaheadToken(
        expression,
        (leftmostNode) => leftmostNode === node
      )) {
        return true;
      }
    }
    if (node.type === "ObjectExpression") {
      const arrowFunctionBody = (_d = path.findAncestor(
        (node2) => node2.type === "ArrowFunctionExpression"
      )) == null ? void 0 : _d.body;
      if (arrowFunctionBody && arrowFunctionBody.type !== "SequenceExpression" && // these have parens added anyway
      arrowFunctionBody.type !== "AssignmentExpression" && startsWithNoLookaheadToken(
        arrowFunctionBody,
        (leftmostNode) => leftmostNode === node
      )) {
        return true;
      }
    }
    switch (parent.type) {
      case "ParenthesizedExpression":
        return false;
      case "ClassDeclaration":
      case "ClassExpression":
        if (key === "superClass" && (node.type === "ArrowFunctionExpression" || node.type === "AssignmentExpression" || node.type === "AwaitExpression" || node.type === "BinaryExpression" || node.type === "ConditionalExpression" || node.type === "LogicalExpression" || node.type === "NewExpression" || node.type === "ObjectExpression" || node.type === "SequenceExpression" || node.type === "TaggedTemplateExpression" || node.type === "UnaryExpression" || node.type === "UpdateExpression" || node.type === "YieldExpression" || node.type === "TSNonNullExpression" || node.type === "ClassExpression" && is_non_empty_array_default(node.decorators))) {
          return true;
        }
        break;
      case "ExportDefaultDeclaration":
        return (
          // `export default function` or `export default class` can't be followed by
          // anything after. So an expression like `export default (function(){}).toString()`
          // needs to be followed by a parentheses
          shouldWrapFunctionForExportDefault(path, options2) || // `export default (foo, bar)` also needs parentheses
          node.type === "SequenceExpression"
        );
      case "Decorator":
        if (key === "expression") {
          if (isMemberExpression(node) && node.computed) {
            return true;
          }
          let hasCallExpression = false;
          let hasMemberExpression = false;
          let current = node;
          while (current) {
            switch (current.type) {
              case "MemberExpression":
                hasMemberExpression = true;
                current = current.object;
                break;
              case "CallExpression":
                if (
                  /** @(x().y) */
                  hasMemberExpression || /** @(x().y()) */
                  hasCallExpression
                ) {
                  return options2.parser !== "typescript";
                }
                hasCallExpression = true;
                current = current.callee;
                break;
              case "Identifier":
                return false;
              case "TaggedTemplateExpression":
                return options2.parser !== "typescript";
              default:
                return true;
            }
          }
          return true;
        }
        break;
      case "TypeAnnotation":
        if (path.match(
          void 0,
          void 0,
          (node2, key2) => key2 === "returnType" && node2.type === "ArrowFunctionExpression"
        ) && includesFunctionTypeInObjectType(node)) {
          return true;
        }
        break;
    }
    switch (node.type) {
      case "UpdateExpression":
        if (parent.type === "UnaryExpression") {
          return node.prefix && (node.operator === "++" && parent.operator === "+" || node.operator === "--" && parent.operator === "-");
        }
      case "UnaryExpression":
        switch (parent.type) {
          case "UnaryExpression":
            return node.operator === parent.operator && (node.operator === "+" || node.operator === "-");
          case "BindExpression":
            return true;
          case "MemberExpression":
          case "OptionalMemberExpression":
            return key === "object";
          case "TaggedTemplateExpression":
            return true;
          case "NewExpression":
          case "CallExpression":
          case "OptionalCallExpression":
            return key === "callee";
          case "BinaryExpression":
            return key === "left" && parent.operator === "**";
          case "TSNonNullExpression":
            return true;
          default:
            return false;
        }
      case "BinaryExpression":
        if (parent.type === "UpdateExpression") {
          return true;
        }
        if (node.operator === "in" && isPathInForStatementInitializer(path)) {
          return true;
        }
        if (node.operator === "|>" && ((_e = node.extra) == null ? void 0 : _e.parenthesized)) {
          const grandParent = path.grandparent;
          if (grandParent.type === "BinaryExpression" && grandParent.operator === "|>") {
            return true;
          }
        }
      case "TSTypeAssertion":
      case "TSAsExpression":
      case "TSSatisfiesExpression":
      case "LogicalExpression":
        switch (parent.type) {
          case "TSAsExpression":
          case "TSSatisfiesExpression":
            return !isTSTypeExpression(node);
          case "ConditionalExpression":
            return isTSTypeExpression(node);
          case "CallExpression":
          case "NewExpression":
          case "OptionalCallExpression":
            return key === "callee";
          case "ClassExpression":
          case "ClassDeclaration":
            return key === "superClass";
          case "TSTypeAssertion":
          case "TaggedTemplateExpression":
          case "UnaryExpression":
          case "JSXSpreadAttribute":
          case "SpreadElement":
          case "BindExpression":
          case "AwaitExpression":
          case "TSNonNullExpression":
          case "UpdateExpression":
            return true;
          case "MemberExpression":
          case "OptionalMemberExpression":
            return key === "object";
          case "AssignmentExpression":
          case "AssignmentPattern":
            return key === "left" && (node.type === "TSTypeAssertion" || isTSTypeExpression(node));
          case "LogicalExpression":
            if (node.type === "LogicalExpression") {
              return parent.operator !== node.operator;
            }
          case "BinaryExpression": {
            const { operator, type } = node;
            if (!operator && type !== "TSTypeAssertion") {
              return true;
            }
            const precedence = getPrecedence(operator);
            const parentOperator = parent.operator;
            const parentPrecedence = getPrecedence(parentOperator);
            if (parentPrecedence > precedence) {
              return true;
            }
            if (key === "right" && parentPrecedence === precedence) {
              return true;
            }
            if (parentPrecedence === precedence && !shouldFlatten(parentOperator, operator)) {
              return true;
            }
            if (parentPrecedence < precedence && operator === "%") {
              return parentOperator === "+" || parentOperator === "-";
            }
            if (isBitwiseOperator(parentOperator)) {
              return true;
            }
            return false;
          }
          default:
            return false;
        }
      case "SequenceExpression":
        switch (parent.type) {
          case "ReturnStatement":
            return false;
          case "ForStatement":
            return false;
          case "ExpressionStatement":
            return key !== "expression";
          case "ArrowFunctionExpression":
            return key !== "body";
          default:
            return true;
        }
      case "YieldExpression":
        if (parent.type === "AwaitExpression") {
          return true;
        }
      case "AwaitExpression":
        switch (parent.type) {
          case "TaggedTemplateExpression":
          case "UnaryExpression":
          case "LogicalExpression":
          case "SpreadElement":
          case "TSAsExpression":
          case "TSSatisfiesExpression":
          case "TSNonNullExpression":
          case "BindExpression":
            return true;
          case "MemberExpression":
          case "OptionalMemberExpression":
            return key === "object";
          case "NewExpression":
          case "CallExpression":
          case "OptionalCallExpression":
            return key === "callee";
          case "ConditionalExpression":
            return key === "test";
          case "BinaryExpression":
            if (!node.argument && parent.operator === "|>") {
              return false;
            }
            return true;
          default:
            return false;
        }
      case "TSFunctionType":
        if (path.match(
          (node2) => node2.type === "TSFunctionType",
          (node2, key2) => key2 === "typeAnnotation" && node2.type === "TSTypeAnnotation",
          (node2, key2) => key2 === "returnType" && node2.type === "ArrowFunctionExpression"
        )) {
          return true;
        }
      case "TSConditionalType":
      case "TSConstructorType":
        if (key === "extendsType" && parent.type === "TSConditionalType") {
          if (node.type === "TSConditionalType") {
            return true;
          }
          let { typeAnnotation } = node.returnType || node.typeAnnotation;
          if (typeAnnotation.type === "TSTypePredicate" && typeAnnotation.typeAnnotation) {
            typeAnnotation = typeAnnotation.typeAnnotation.typeAnnotation;
          }
          if (typeAnnotation.type === "TSInferType" && typeAnnotation.typeParameter.constraint) {
            return true;
          }
        }
        if (key === "checkType" && parent.type === "TSConditionalType") {
          return true;
        }
      case "TSUnionType":
      case "TSIntersectionType":
        if ((parent.type === "TSUnionType" || parent.type === "TSIntersectionType") && parent.types.length > 1 && (!node.types || node.types.length > 1)) {
          return true;
        }
      case "TSInferType":
        if (node.type === "TSInferType" && parent.type === "TSRestType") {
          return false;
        }
      case "TSTypeOperator":
        return parent.type === "TSArrayType" || parent.type === "TSOptionalType" || parent.type === "TSRestType" || key === "objectType" && parent.type === "TSIndexedAccessType" || parent.type === "TSTypeOperator" || parent.type === "TSTypeAnnotation" && path.grandparent.type.startsWith("TSJSDoc");
      case "TSTypeQuery":
        return key === "objectType" && parent.type === "TSIndexedAccessType" || key === "elementType" && parent.type === "TSArrayType";
      case "TypeOperator":
        return parent.type === "ArrayTypeAnnotation" || parent.type === "NullableTypeAnnotation" || key === "objectType" && (parent.type === "IndexedAccessType" || parent.type === "OptionalIndexedAccessType") || parent.type === "TypeOperator";
      case "TypeofTypeAnnotation":
        return key === "objectType" && (parent.type === "IndexedAccessType" || parent.type === "OptionalIndexedAccessType") || key === "elementType" && parent.type === "ArrayTypeAnnotation";
      case "ArrayTypeAnnotation":
        return parent.type === "NullableTypeAnnotation";
      case "IntersectionTypeAnnotation":
      case "UnionTypeAnnotation":
        return parent.type === "TypeOperator" || parent.type === "ArrayTypeAnnotation" || parent.type === "NullableTypeAnnotation" || parent.type === "IntersectionTypeAnnotation" || parent.type === "UnionTypeAnnotation" || key === "objectType" && (parent.type === "IndexedAccessType" || parent.type === "OptionalIndexedAccessType") || path.match(
          void 0,
          (node2, key2) => node2.type === "TypeAnnotation",
          (node2, key2) => key2 === "rendersType" && (node2.type === "ComponentDeclaration" || node2.type === "ComponentTypeAnnotation" || node2.type === "DeclareComponent")
        );
      case "InferTypeAnnotation":
      case "NullableTypeAnnotation":
        return parent.type === "ArrayTypeAnnotation" || key === "objectType" && (parent.type === "IndexedAccessType" || parent.type === "OptionalIndexedAccessType");
      case "FunctionTypeAnnotation": {
        if (path.match(
          void 0,
          (node2, key2) => key2 === "typeAnnotation" && node2.type === "TypeAnnotation",
          (node2, key2) => key2 === "returnType" && node2.type === "ArrowFunctionExpression"
        )) {
          return true;
        }
        if (path.match(
          void 0,
          (node2, key2) => key2 === "typeAnnotation" && node2.type === "TypePredicate",
          (node2, key2) => key2 === "typeAnnotation" && node2.type === "TypeAnnotation",
          (node2, key2) => key2 === "returnType" && node2.type === "ArrowFunctionExpression"
        )) {
          return true;
        }
        const ancestor = parent.type === "NullableTypeAnnotation" ? path.grandparent : parent;
        return ancestor.type === "UnionTypeAnnotation" || ancestor.type === "IntersectionTypeAnnotation" || ancestor.type === "ArrayTypeAnnotation" || key === "objectType" && (ancestor.type === "IndexedAccessType" || ancestor.type === "OptionalIndexedAccessType") || key === "checkType" && parent.type === "ConditionalTypeAnnotation" || key === "extendsType" && parent.type === "ConditionalTypeAnnotation" && node.returnType.type === "InferTypeAnnotation" && node.returnType.typeParameter.bound || // We should check ancestor's parent to know whether the parentheses
        // are really needed, but since ??T doesn't make sense this check
        // will almost never be true.
        ancestor.type === "NullableTypeAnnotation" || // See #5283
        parent.type === "FunctionTypeParam" && parent.name === null && getFunctionParameters(node).some(
          (param) => {
            var _a2;
            return ((_a2 = param.typeAnnotation) == null ? void 0 : _a2.type) === "NullableTypeAnnotation";
          }
        );
      }
      case "ConditionalTypeAnnotation":
        if (key === "extendsType" && parent.type === "ConditionalTypeAnnotation" && node.type === "ConditionalTypeAnnotation") {
          return true;
        }
        if (key === "checkType" && parent.type === "ConditionalTypeAnnotation") {
          return true;
        }
      case "OptionalIndexedAccessType":
        return key === "objectType" && parent.type === "IndexedAccessType";
      case "StringLiteral":
      case "NumericLiteral":
      case "Literal":
        if (typeof node.value === "string" && parent.type === "ExpressionStatement" && !parent.directive) {
          const grandParent = path.grandparent;
          return grandParent.type === "Program" || grandParent.type === "BlockStatement";
        }
        return key === "object" && parent.type === "MemberExpression" && typeof node.value === "number";
      case "AssignmentExpression": {
        const grandParent = path.grandparent;
        if (key === "body" && parent.type === "ArrowFunctionExpression") {
          return true;
        }
        if (key === "key" && (parent.type === "ClassProperty" || parent.type === "PropertyDefinition") && parent.computed) {
          return false;
        }
        if ((key === "init" || key === "update") && parent.type === "ForStatement") {
          return false;
        }
        if (parent.type === "ExpressionStatement") {
          return node.left.type === "ObjectPattern";
        }
        if (key === "key" && parent.type === "TSPropertySignature") {
          return false;
        }
        if (parent.type === "AssignmentExpression") {
          return false;
        }
        if (parent.type === "SequenceExpression" && grandParent.type === "ForStatement" && (grandParent.init === parent || grandParent.update === parent)) {
          return false;
        }
        if (key === "value" && parent.type === "Property" && grandParent.type === "ObjectPattern" && grandParent.properties.includes(parent)) {
          return false;
        }
        if (parent.type === "NGChainedExpression") {
          return false;
        }
        return true;
      }
      case "ConditionalExpression":
        switch (parent.type) {
          case "TaggedTemplateExpression":
          case "UnaryExpression":
          case "SpreadElement":
          case "BinaryExpression":
          case "LogicalExpression":
          case "NGPipeExpression":
          case "ExportDefaultDeclaration":
          case "AwaitExpression":
          case "JSXSpreadAttribute":
          case "TSTypeAssertion":
          case "TypeCastExpression":
          case "TSAsExpression":
          case "TSSatisfiesExpression":
          case "TSNonNullExpression":
            return true;
          case "NewExpression":
          case "CallExpression":
          case "OptionalCallExpression":
            return key === "callee";
          case "ConditionalExpression":
            return key === "test";
          case "MemberExpression":
          case "OptionalMemberExpression":
            return key === "object";
          default:
            return false;
        }
      case "FunctionExpression":
        switch (parent.type) {
          case "NewExpression":
          case "CallExpression":
          case "OptionalCallExpression":
            return key === "callee";
          case "TaggedTemplateExpression":
            return true;
          default:
            return false;
        }
      case "ArrowFunctionExpression":
        switch (parent.type) {
          case "BinaryExpression":
            return parent.operator !== "|>" || ((_f = node.extra) == null ? void 0 : _f.parenthesized);
          case "NewExpression":
          case "CallExpression":
          case "OptionalCallExpression":
            return key === "callee";
          case "MemberExpression":
          case "OptionalMemberExpression":
            return key === "object";
          case "TSAsExpression":
          case "TSSatisfiesExpression":
          case "TSNonNullExpression":
          case "BindExpression":
          case "TaggedTemplateExpression":
          case "UnaryExpression":
          case "LogicalExpression":
          case "AwaitExpression":
          case "TSTypeAssertion":
            return true;
          case "ConditionalExpression":
            return key === "test";
          default:
            return false;
        }
      case "ClassExpression":
        switch (parent.type) {
          case "NewExpression":
            return key === "callee";
          default:
            return false;
        }
      case "OptionalMemberExpression":
      case "OptionalCallExpression":
      case "CallExpression":
      case "MemberExpression":
        if (shouldAddParenthesesToChainElement(path)) {
          return true;
        }
      case "TaggedTemplateExpression":
      case "TSNonNullExpression":
        if (key === "callee" && (parent.type === "BindExpression" || parent.type === "NewExpression")) {
          let object = node;
          while (object) {
            switch (object.type) {
              case "CallExpression":
              case "OptionalCallExpression":
                return true;
              case "MemberExpression":
              case "OptionalMemberExpression":
              case "BindExpression":
                object = object.object;
                break;
              case "TaggedTemplateExpression":
                object = object.tag;
                break;
              case "TSNonNullExpression":
                object = object.expression;
                break;
              default:
                return false;
            }
          }
        }
        return false;
      case "BindExpression":
        return key === "callee" && (parent.type === "BindExpression" || parent.type === "NewExpression") || key === "object" && isMemberExpression(parent);
      case "NGPipeExpression":
        if (parent.type === "NGRoot" || parent.type === "NGMicrosyntaxExpression" || parent.type === "ObjectProperty" && // Preserve parens for compatibility with AngularJS expressions
        !((_g = node.extra) == null ? void 0 : _g.parenthesized) || isArrayOrTupleExpression(parent) || key === "arguments" && isCallExpression(parent) || key === "right" && parent.type === "NGPipeExpression" || key === "property" && parent.type === "MemberExpression" || parent.type === "AssignmentExpression") {
          return false;
        }
        return true;
      case "JSXFragment":
      case "JSXElement":
        return key === "callee" || key === "left" && parent.type === "BinaryExpression" && parent.operator === "<" || !isArrayOrTupleExpression(parent) && parent.type !== "ArrowFunctionExpression" && parent.type !== "AssignmentExpression" && parent.type !== "AssignmentPattern" && parent.type !== "BinaryExpression" && parent.type !== "NewExpression" && parent.type !== "ConditionalExpression" && parent.type !== "ExpressionStatement" && parent.type !== "JsExpressionRoot" && parent.type !== "JSXAttribute" && parent.type !== "JSXElement" && parent.type !== "JSXExpressionContainer" && parent.type !== "JSXFragment" && parent.type !== "LogicalExpression" && !isCallExpression(parent) && !isObjectProperty(parent) && parent.type !== "ReturnStatement" && parent.type !== "ThrowStatement" && parent.type !== "TypeCastExpression" && parent.type !== "VariableDeclarator" && parent.type !== "YieldExpression";
      case "TSInstantiationExpression":
        return key === "object" && isMemberExpression(parent);
    }
    return false;
  }
  var isStatement = create_type_check_function_default([
    "BlockStatement",
    "BreakStatement",
    "ComponentDeclaration",
    "ClassBody",
    "ClassDeclaration",
    "ClassMethod",
    "ClassProperty",
    "PropertyDefinition",
    "ClassPrivateProperty",
    "ContinueStatement",
    "DebuggerStatement",
    "DeclareComponent",
    "DeclareClass",
    "DeclareExportAllDeclaration",
    "DeclareExportDeclaration",
    "DeclareFunction",
    "DeclareInterface",
    "DeclareModule",
    "DeclareModuleExports",
    "DeclareVariable",
    "DeclareEnum",
    "DoWhileStatement",
    "EnumDeclaration",
    "ExportAllDeclaration",
    "ExportDefaultDeclaration",
    "ExportNamedDeclaration",
    "ExpressionStatement",
    "ForInStatement",
    "ForOfStatement",
    "ForStatement",
    "FunctionDeclaration",
    "IfStatement",
    "ImportDeclaration",
    "InterfaceDeclaration",
    "LabeledStatement",
    "MethodDefinition",
    "ReturnStatement",
    "SwitchStatement",
    "ThrowStatement",
    "TryStatement",
    "TSDeclareFunction",
    "TSEnumDeclaration",
    "TSImportEqualsDeclaration",
    "TSInterfaceDeclaration",
    "TSModuleDeclaration",
    "TSNamespaceExportDeclaration",
    "TypeAlias",
    "VariableDeclaration",
    "WhileStatement",
    "WithStatement"
  ]);
  function isPathInForStatementInitializer(path) {
    let i = 0;
    let { node } = path;
    while (node) {
      const parent = path.getParentNode(i++);
      if ((parent == null ? void 0 : parent.type) === "ForStatement" && parent.init === node) {
        return true;
      }
      node = parent;
    }
    return false;
  }
  function includesFunctionTypeInObjectType(node) {
    return hasNode(
      node,
      (node2) => node2.type === "ObjectTypeAnnotation" && hasNode(node2, (node3) => node3.type === "FunctionTypeAnnotation")
    );
  }
  function endsWithRightBracket(node) {
    return isObjectOrRecordExpression(node);
  }
  function isFollowedByRightBracket(path) {
    const { parent, key } = path;
    switch (parent.type) {
      case "NGPipeExpression":
        if (key === "arguments" && path.isLast) {
          return path.callParent(isFollowedByRightBracket);
        }
        break;
      case "ObjectProperty":
        if (key === "value") {
          return path.callParent(() => path.key === "properties" && path.isLast);
        }
        break;
      case "BinaryExpression":
      case "LogicalExpression":
        if (key === "right") {
          return path.callParent(isFollowedByRightBracket);
        }
        break;
      case "ConditionalExpression":
        if (key === "alternate") {
          return path.callParent(isFollowedByRightBracket);
        }
        break;
      case "UnaryExpression":
        if (parent.prefix) {
          return path.callParent(isFollowedByRightBracket);
        }
        break;
    }
    return false;
  }
  function shouldWrapFunctionForExportDefault(path, options2) {
    const { node, parent } = path;
    if (node.type === "FunctionExpression" || node.type === "ClassExpression") {
      return parent.type === "ExportDefaultDeclaration" || // in some cases the function is already wrapped
      // (e.g. `export default (function() {})();`)
      // in this case we don't need to add extra parens
      !needsParens(path, options2);
    }
    if (!hasNakedLeftSide(node) || parent.type !== "ExportDefaultDeclaration" && needsParens(path, options2)) {
      return false;
    }
    return path.call(
      () => shouldWrapFunctionForExportDefault(path, options2),
      ...getLeftSidePathName(node)
    );
  }
  function shouldAddParenthesesToChainElement(path) {
    const { node, parent, grandparent, key } = path;
    if ((node.type === "OptionalMemberExpression" || node.type === "OptionalCallExpression") && (key === "object" && parent.type === "MemberExpression" || key === "callee" && (parent.type === "CallExpression" || parent.type === "NewExpression") || parent.type === "TSNonNullExpression" && grandparent.type === "MemberExpression" && grandparent.object === parent)) {
      return true;
    }
    if (path.match(
      () => node.type === "CallExpression" || node.type === "MemberExpression",
      (node2, name) => name === "expression" && node2.type === "ChainExpression"
    ) && (path.match(
      void 0,
      void 0,
      (node2, name) => name === "callee" && (node2.type === "CallExpression" && !node2.optional || node2.type === "NewExpression") || name === "object" && node2.type === "MemberExpression" && !node2.optional
    ) || path.match(
      void 0,
      void 0,
      (node2, name) => name === "expression" && node2.type === "TSNonNullExpression",
      (node2, name) => name === "object" && node2.type === "MemberExpression"
    ))) {
      return true;
    }
    if (path.match(
      () => node.type === "CallExpression" || node.type === "MemberExpression",
      (node2, name) => name === "expression" && node2.type === "TSNonNullExpression",
      (node2, name) => name === "expression" && node2.type === "ChainExpression",
      (node2, name) => name === "object" && node2.type === "MemberExpression"
    )) {
      return true;
    }
    return false;
  }
  var needs_parens_default = needsParens;

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
  var returnTrue = () => true;
  function printComment(path, options2) {
    const comment = path.node;
    comment.printed = true;
    return options2.printer.printComment(path, options2);
  }
  function printLeadingComment(path, options2) {
    var _a;
    const comment = path.node;
    const parts = [printComment(path, options2)];
    const { printer, originalText, locStart: locStart2, locEnd: locEnd2 } = options2;
    const isBlock = (_a = printer.isBlockComment) == null ? void 0 : _a.call(printer, comment);
    if (isBlock) {
      const lineBreak = has_newline_default(originalText, locEnd2(comment)) ? has_newline_default(originalText, locStart2(comment), {
        backwards: true
      }) ? hardline : line : " ";
      parts.push(lineBreak);
    } else {
      parts.push(hardline);
    }
    const index = skip_newline_default(
      originalText,
      skipSpaces(originalText, locEnd2(comment))
    );
    if (index !== false && has_newline_default(originalText, index)) {
      parts.push(hardline);
    }
    return parts;
  }
  function printTrailingComment(path, options2, previousComment) {
    var _a;
    const comment = path.node;
    const printed = printComment(path, options2);
    const { printer, originalText, locStart: locStart2 } = options2;
    const isBlock = (_a = printer.isBlockComment) == null ? void 0 : _a.call(printer, comment);
    if ((previousComment == null ? void 0 : previousComment.hasLineSuffix) && !(previousComment == null ? void 0 : previousComment.isBlock) || has_newline_default(originalText, locStart2(comment), { backwards: true })) {
      const isLineBeforeEmpty = is_previous_line_empty_default(
        originalText,
        locStart2(comment)
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
  function printDanglingComments(path, options2, danglingCommentsPrintOptions = {}) {
    const { node } = path;
    if (!is_non_empty_array_default(node == null ? void 0 : node.comments)) {
      return "";
    }
    const {
      indent: shouldIndent = false,
      marker,
      filter = returnTrue
    } = danglingCommentsPrintOptions;
    const parts = [];
    path.each(({ node: comment }) => {
      if (comment.leading || comment.trailing || comment.marker !== marker || !filter(comment)) {
        return;
      }
      parts.push(printComment(path, options2));
    }, "comments");
    if (parts.length === 0) {
      return "";
    }
    const doc = join(hardline, parts);
    return shouldIndent ? indent([hardline, doc]) : doc;
  }
  function printCommentsSeparately(path, options2) {
    const value = path.node;
    if (!value) {
      return {};
    }
    const ignored = options2[Symbol.for("printedComments")];
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
        leadingParts.push(printLeadingComment(path, options2));
      } else if (trailing) {
        printedTrailingComment = printTrailingComment(
          path,
          options2,
          printedTrailingComment
        );
        trailingParts.push(printedTrailingComment.doc);
      }
    }, "comments");
    return { leading: leadingParts, trailing: trailingParts };
  }
  function printComments(path, doc, options2) {
    const { leading, trailing } = printCommentsSeparately(path, options2);
    if (!leading && !trailing) {
      return doc;
    }
    return inheritLabel(doc, (doc2) => [leading, doc2, trailing]);
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

  // node_modules/escape-string-regexp/index.js
  function escapeStringRegexp(string) {
    if (typeof string !== "string") {
      throw new TypeError("Expected a string");
    }
    return string.replace(/[|\\{}()[\]^$+*?.]/g, "\\$&").replace(/-/g, "\\x2d");
  }

  // src/utils/whitespace-utils.js
  var _whitespaceCharacters;
  var WhitespaceUtils = class {
    constructor(whitespaceCharacters) {
      __privateAdd(this, _whitespaceCharacters, void 0);
      __privateSet(this, _whitespaceCharacters, new Set(whitespaceCharacters));
      if (false) {
        throw new TypeError(`Invalid characters: ${JSON.stringify(whitespaceCharacters)}`);
      }
    }
    getLeadingWhitespaceCount(text) {
      const whitespaceCharacters = __privateGet(this, _whitespaceCharacters);
      let count = 0;
      for (let index = 0; index < text.length && whitespaceCharacters.has(text.charAt(index)); index++) {
        count++;
      }
      return count;
    }
    getTrailingWhitespaceCount(text) {
      const whitespaceCharacters = __privateGet(this, _whitespaceCharacters);
      let count = 0;
      for (let index = text.length - 1; index >= 0 && whitespaceCharacters.has(text.charAt(index)); index--) {
        count++;
      }
      return count;
    }
    getLeadingWhitespace(text) {
      const count = this.getLeadingWhitespaceCount(text);
      return text.slice(0, count);
    }
    getTrailingWhitespace(text) {
      const count = this.getTrailingWhitespaceCount(text);
      return text.slice(text.length - count);
    }
    hasLeadingWhitespace(text) {
      return __privateGet(this, _whitespaceCharacters).has(text.charAt(0));
    }
    hasTrailingWhitespace(text) {
      return __privateGet(this, _whitespaceCharacters).has(at_default(
        /* isOptionalObject*/
        false,
        text,
        -1
      ));
    }
    trimStart(text) {
      const count = this.getLeadingWhitespaceCount(text);
      return text.slice(count);
    }
    trimEnd(text) {
      const count = this.getTrailingWhitespaceCount(text);
      return text.slice(0, text.length - count);
    }
    trim(text) {
      return this.trimEnd(this.trimStart(text));
    }
    split(text, captureWhitespace = false) {
      const pattern = `[${escapeStringRegexp([...__privateGet(this, _whitespaceCharacters)].join(""))}]+`;
      const regexp = new RegExp(captureWhitespace ? `(${pattern})` : pattern);
      return text.split(regexp);
    }
    hasWhitespaceCharacter(text) {
      const whitespaceCharacters = __privateGet(this, _whitespaceCharacters);
      return Array.prototype.some.call(text, (character) => whitespaceCharacters.has(character));
    }
    hasNonWhitespaceCharacter(text) {
      const whitespaceCharacters = __privateGet(this, _whitespaceCharacters);
      return Array.prototype.some.call(text, (character) => !whitespaceCharacters.has(character));
    }
    isWhitespaceOnly(text) {
      const whitespaceCharacters = __privateGet(this, _whitespaceCharacters);
      return Array.prototype.every.call(text, (character) => whitespaceCharacters.has(character));
    }
  };
  _whitespaceCharacters = new WeakMap();
  var whitespace_utils_default = WhitespaceUtils;

  // src/language-js/comments/handle-comments.js
  var handle_comments_exports = {};
  __export(handle_comments_exports, {
    endOfLine: () => handleEndOfLineComment,
    ownLine: () => handleOwnLineComment,
    remaining: () => handleRemainingComment
  });

  // src/utils/get-next-non-space-non-comment-character-index.js
  function getNextNonSpaceNonCommentCharacterIndex(text, startIndex) {
    let oldIdx = null;
    let nextIdx = startIndex;
    while (nextIdx !== oldIdx) {
      oldIdx = nextIdx;
      nextIdx = skipSpaces(text, nextIdx);
      nextIdx = skip_inline_comment_default(text, nextIdx);
      nextIdx = skip_trailing_comment_default(text, nextIdx);
      nextIdx = skip_newline_default(text, nextIdx);
    }
    return nextIdx;
  }
  var get_next_non_space_non_comment_character_index_default = getNextNonSpaceNonCommentCharacterIndex;

  // src/utils/get-next-non-space-non-comment-character.js
  function getNextNonSpaceNonCommentCharacter(text, startIndex) {
    const index = get_next_non_space_non_comment_character_index_default(text, startIndex);
    return index === false ? "" : text.charAt(index);
  }
  var get_next_non_space_non_comment_character_default = getNextNonSpaceNonCommentCharacter;

  // src/utils/has-newline-in-range.js
  function hasNewlineInRange(text, startIndex, endIndex) {
    for (let i = startIndex; i < endIndex; ++i) {
      if (text.charAt(i) === "\n") {
        return true;
      }
    }
    return false;
  }
  var has_newline_in_range_default = hasNewlineInRange;

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

  // src/language-js/utils/is-type-cast-comment.js
  function isTypeCastComment(comment) {
    return is_block_comment_default(comment) && comment.value[0] === "*" && // TypeScript expects the type to be enclosed in curly brackets, however
    // Closure Compiler accepts types in parens and even without any delimiters at all.
    // That's why we just search for "@type" and "@satisfies".
    /@(?:type|satisfies)\b/.test(comment.value);
  }
  var is_type_cast_comment_default = isTypeCastComment;

  // src/language-js/comments/handle-comments.js
  function handleOwnLineComment(context) {
    return [handleIgnoreComments, handleLastFunctionArgComments, handleLastComponentArgComments, handleMemberExpressionComments, handleIfStatementComments, handleWhileComments, handleTryStatementComments, handleClassComments, handleForComments, handleUnionTypeComments, handleOnlyComments, handleModuleSpecifiersComments, handleAssignmentPatternComments, handleMethodNameComments, handleLabeledStatementComments, handleBreakAndContinueStatementComments].some((fn) => fn(context));
  }
  function handleEndOfLineComment(context) {
    return [handleClosureTypeCastComments, handleLastFunctionArgComments, handleConditionalExpressionComments, handleModuleSpecifiersComments, handleIfStatementComments, handleWhileComments, handleTryStatementComments, handleClassComments, handleLabeledStatementComments, handleCallExpressionComments, handlePropertyComments, handleOnlyComments, handleVariableDeclaratorComments, handleBreakAndContinueStatementComments, handleSwitchDefaultCaseComments].some((fn) => fn(context));
  }
  function handleRemainingComment(context) {
    return [handleIgnoreComments, handleIfStatementComments, handleWhileComments, handleObjectPropertyAssignment, handleCommentInEmptyParens, handleMethodNameComments, handleOnlyComments, handleCommentAfterArrowParams, handleFunctionNameComments, handleTSMappedTypeComments, handleBreakAndContinueStatementComments, handleTSFunctionTrailingComments].some((fn) => fn(context));
  }
  function addBlockStatementFirstComment(node, comment) {
    const firstNonEmptyNode = (node.body || node.properties).find(({
      type
    }) => type !== "EmptyStatement");
    if (firstNonEmptyNode) {
      addLeadingComment(firstNonEmptyNode, comment);
    } else {
      addDanglingComment(node, comment);
    }
  }
  function addBlockOrNotComment(node, comment) {
    if (node.type === "BlockStatement") {
      addBlockStatementFirstComment(node, comment);
    } else {
      addLeadingComment(node, comment);
    }
  }
  function handleClosureTypeCastComments({
    comment,
    followingNode
  }) {
    if (followingNode && is_type_cast_comment_default(comment)) {
      addLeadingComment(followingNode, comment);
      return true;
    }
    return false;
  }
  function handleIfStatementComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode,
    text
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) !== "IfStatement" || !followingNode) {
      return false;
    }
    const nextCharacter = get_next_non_space_non_comment_character_default(text, locEnd(comment));
    if (nextCharacter === ")") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if (precedingNode === enclosingNode.consequent && followingNode === enclosingNode.alternate) {
      if (precedingNode.type === "BlockStatement") {
        addTrailingComment(precedingNode, comment);
      } else {
        const isSingleLineComment = comment.type === "SingleLine" || comment.loc.start.line === comment.loc.end.line;
        const isSameLineComment = comment.loc.start.line === precedingNode.loc.start.line;
        if (isSingleLineComment && isSameLineComment) {
          addDanglingComment(precedingNode, comment, precedingNode.type === "ExpressionStatement" ? markerForIfWithoutBlockAndSameLineComment : void 0);
        } else {
          addDanglingComment(enclosingNode, comment);
        }
      }
      return true;
    }
    if (followingNode.type === "BlockStatement") {
      addBlockStatementFirstComment(followingNode, comment);
      return true;
    }
    if (followingNode.type === "IfStatement") {
      addBlockOrNotComment(followingNode.consequent, comment);
      return true;
    }
    if (enclosingNode.consequent === followingNode) {
      addLeadingComment(followingNode, comment);
      return true;
    }
    return false;
  }
  function handleWhileComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode,
    text
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) !== "WhileStatement" || !followingNode) {
      return false;
    }
    const nextCharacter = get_next_non_space_non_comment_character_default(text, locEnd(comment));
    if (nextCharacter === ")") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if (followingNode.type === "BlockStatement") {
      addBlockStatementFirstComment(followingNode, comment);
      return true;
    }
    if (enclosingNode.body === followingNode) {
      addLeadingComment(followingNode, comment);
      return true;
    }
    return false;
  }
  function handleTryStatementComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) !== "TryStatement" && (enclosingNode == null ? void 0 : enclosingNode.type) !== "CatchClause" || !followingNode) {
      return false;
    }
    if (enclosingNode.type === "CatchClause" && precedingNode) {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if (followingNode.type === "BlockStatement") {
      addBlockStatementFirstComment(followingNode, comment);
      return true;
    }
    if (followingNode.type === "TryStatement") {
      addBlockOrNotComment(followingNode.finalizer, comment);
      return true;
    }
    if (followingNode.type === "CatchClause") {
      addBlockOrNotComment(followingNode.body, comment);
      return true;
    }
    return false;
  }
  function handleMemberExpressionComments({
    comment,
    enclosingNode,
    followingNode
  }) {
    if (isMemberExpression(enclosingNode) && (followingNode == null ? void 0 : followingNode.type) === "Identifier") {
      addLeadingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleConditionalExpressionComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode,
    text
  }) {
    const isSameLineAsPrecedingNode = precedingNode && !has_newline_in_range_default(text, locEnd(precedingNode), locStart(comment));
    if ((!precedingNode || !isSameLineAsPrecedingNode) && ((enclosingNode == null ? void 0 : enclosingNode.type) === "ConditionalExpression" || (enclosingNode == null ? void 0 : enclosingNode.type) === "TSConditionalType") && followingNode) {
      addLeadingComment(followingNode, comment);
      return true;
    }
    return false;
  }
  function handleObjectPropertyAssignment({
    comment,
    precedingNode,
    enclosingNode
  }) {
    if (isObjectProperty(enclosingNode) && enclosingNode.shorthand && enclosingNode.key === precedingNode && enclosingNode.value.type === "AssignmentPattern") {
      addTrailingComment(enclosingNode.value.left, comment);
      return true;
    }
    return false;
  }
  var classLikeNodeTypes = /* @__PURE__ */ new Set(["ClassDeclaration", "ClassExpression", "DeclareClass", "DeclareInterface", "InterfaceDeclaration", "TSInterfaceDeclaration"]);
  function handleClassComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode
  }) {
    if (classLikeNodeTypes.has(enclosingNode == null ? void 0 : enclosingNode.type)) {
      if (is_non_empty_array_default(enclosingNode.decorators) && !((followingNode == null ? void 0 : followingNode.type) === "Decorator")) {
        addTrailingComment(at_default(
          /* isOptionalObject*/
          false,
          enclosingNode.decorators,
          -1
        ), comment);
        return true;
      }
      if (enclosingNode.body && followingNode === enclosingNode.body) {
        addBlockStatementFirstComment(enclosingNode.body, comment);
        return true;
      }
      if (followingNode) {
        if (enclosingNode.superClass && followingNode === enclosingNode.superClass && precedingNode && (precedingNode === enclosingNode.id || precedingNode === enclosingNode.typeParameters)) {
          addTrailingComment(precedingNode, comment);
          return true;
        }
        for (const prop of ["implements", "extends", "mixins"]) {
          if (enclosingNode[prop] && followingNode === enclosingNode[prop][0]) {
            if (precedingNode && (precedingNode === enclosingNode.id || precedingNode === enclosingNode.typeParameters || precedingNode === enclosingNode.superClass)) {
              addTrailingComment(precedingNode, comment);
            } else {
              addDanglingComment(enclosingNode, comment, prop);
            }
            return true;
          }
        }
      }
    }
    return false;
  }
  var propertyLikeNodeTypes = /* @__PURE__ */ new Set(["ClassMethod", "ClassProperty", "PropertyDefinition", "TSAbstractPropertyDefinition", "TSAbstractMethodDefinition", "TSDeclareMethod", "MethodDefinition", "ClassAccessorProperty", "AccessorProperty", "TSAbstractAccessorProperty"]);
  function handleMethodNameComments({
    comment,
    precedingNode,
    enclosingNode,
    text
  }) {
    if (enclosingNode && precedingNode && get_next_non_space_non_comment_character_default(text, locEnd(comment)) === "(" && // "MethodDefinition" is handled in getCommentChildNodes
    (enclosingNode.type === "Property" || enclosingNode.type === "TSDeclareMethod" || enclosingNode.type === "TSAbstractMethodDefinition") && precedingNode.type === "Identifier" && enclosingNode.key === precedingNode && // special Property case: { key: /*comment*/(value) };
    // comment should be attached to value instead of key
    get_next_non_space_non_comment_character_default(text, locEnd(precedingNode)) !== ":") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if ((precedingNode == null ? void 0 : precedingNode.type) === "Decorator" && propertyLikeNodeTypes.has(enclosingNode == null ? void 0 : enclosingNode.type)) {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    return false;
  }
  var functionLikeNodeTypes = /* @__PURE__ */ new Set(["FunctionDeclaration", "FunctionExpression", "ClassMethod", "MethodDefinition", "ObjectMethod"]);
  function handleFunctionNameComments({
    comment,
    precedingNode,
    enclosingNode,
    text
  }) {
    if (get_next_non_space_non_comment_character_default(text, locEnd(comment)) !== "(") {
      return false;
    }
    if (precedingNode && functionLikeNodeTypes.has(enclosingNode == null ? void 0 : enclosingNode.type)) {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    return false;
  }
  function handleCommentAfterArrowParams({
    comment,
    enclosingNode,
    text
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) !== "ArrowFunctionExpression") {
      return false;
    }
    const index = get_next_non_space_non_comment_character_index_default(text, locEnd(comment));
    if (index !== false && text.slice(index, index + 2) === "=>") {
      addDanglingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleCommentInEmptyParens({
    comment,
    enclosingNode,
    text
  }) {
    if (get_next_non_space_non_comment_character_default(text, locEnd(comment)) !== ")") {
      return false;
    }
    if (enclosingNode && (isRealFunctionLikeNode(enclosingNode) && getFunctionParameters(enclosingNode).length === 0 || isCallLikeExpression(enclosingNode) && getCallArguments(enclosingNode).length === 0)) {
      addDanglingComment(enclosingNode, comment);
      return true;
    }
    if (((enclosingNode == null ? void 0 : enclosingNode.type) === "MethodDefinition" || (enclosingNode == null ? void 0 : enclosingNode.type) === "TSAbstractMethodDefinition") && getFunctionParameters(enclosingNode.value).length === 0) {
      addDanglingComment(enclosingNode.value, comment);
      return true;
    }
    return false;
  }
  function handleLastComponentArgComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode,
    text
  }) {
    if ((precedingNode == null ? void 0 : precedingNode.type) === "ComponentTypeParameter" && ((enclosingNode == null ? void 0 : enclosingNode.type) === "DeclareComponent" || (enclosingNode == null ? void 0 : enclosingNode.type) === "ComponentTypeAnnotation") && (followingNode == null ? void 0 : followingNode.type) !== "ComponentTypeParameter") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if (((precedingNode == null ? void 0 : precedingNode.type) === "ComponentParameter" || (precedingNode == null ? void 0 : precedingNode.type) === "RestElement") && (enclosingNode == null ? void 0 : enclosingNode.type) === "ComponentDeclaration" && get_next_non_space_non_comment_character_default(text, locEnd(comment)) === ")") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    return false;
  }
  function handleLastFunctionArgComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode,
    text
  }) {
    if ((precedingNode == null ? void 0 : precedingNode.type) === "FunctionTypeParam" && (enclosingNode == null ? void 0 : enclosingNode.type) === "FunctionTypeAnnotation" && (followingNode == null ? void 0 : followingNode.type) !== "FunctionTypeParam") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if (((precedingNode == null ? void 0 : precedingNode.type) === "Identifier" || (precedingNode == null ? void 0 : precedingNode.type) === "AssignmentPattern" || (precedingNode == null ? void 0 : precedingNode.type) === "ObjectPattern" || (precedingNode == null ? void 0 : precedingNode.type) === "ArrayPattern" || (precedingNode == null ? void 0 : precedingNode.type) === "RestElement") && enclosingNode && isRealFunctionLikeNode(enclosingNode) && get_next_non_space_non_comment_character_default(text, locEnd(comment)) === ")") {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "FunctionDeclaration" && (followingNode == null ? void 0 : followingNode.type) === "BlockStatement") {
      const functionParamRightParenIndex = (() => {
        const parameters = getFunctionParameters(enclosingNode);
        if (parameters.length > 0) {
          return get_next_non_space_non_comment_character_index_default(text, locEnd(at_default(
            /* isOptionalObject*/
            false,
            parameters,
            -1
          )));
        }
        const functionParamLeftParenIndex = get_next_non_space_non_comment_character_index_default(text, locEnd(enclosingNode.id));
        return functionParamLeftParenIndex !== false && get_next_non_space_non_comment_character_index_default(text, functionParamLeftParenIndex + 1);
      })();
      if (locStart(comment) > functionParamRightParenIndex) {
        addBlockStatementFirstComment(followingNode, comment);
        return true;
      }
    }
    return false;
  }
  function handleLabeledStatementComments({
    comment,
    enclosingNode
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "LabeledStatement") {
      addLeadingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleBreakAndContinueStatementComments({
    comment,
    enclosingNode
  }) {
    if (((enclosingNode == null ? void 0 : enclosingNode.type) === "ContinueStatement" || (enclosingNode == null ? void 0 : enclosingNode.type) === "BreakStatement") && !enclosingNode.label) {
      addTrailingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleCallExpressionComments({
    comment,
    precedingNode,
    enclosingNode
  }) {
    if (isCallExpression(enclosingNode) && precedingNode && enclosingNode.callee === precedingNode && enclosingNode.arguments.length > 0) {
      addLeadingComment(enclosingNode.arguments[0], comment);
      return true;
    }
    return false;
  }
  function handleUnionTypeComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "UnionTypeAnnotation" || (enclosingNode == null ? void 0 : enclosingNode.type) === "TSUnionType") {
      if (isPrettierIgnoreComment(comment)) {
        followingNode.prettierIgnore = true;
        comment.unignore = true;
      }
      if (precedingNode) {
        addTrailingComment(precedingNode, comment);
        return true;
      }
      return false;
    }
    if (((followingNode == null ? void 0 : followingNode.type) === "UnionTypeAnnotation" || (followingNode == null ? void 0 : followingNode.type) === "TSUnionType") && isPrettierIgnoreComment(comment)) {
      followingNode.types[0].prettierIgnore = true;
      comment.unignore = true;
    }
    return false;
  }
  function handlePropertyComments({
    comment,
    enclosingNode
  }) {
    if (isObjectProperty(enclosingNode)) {
      addLeadingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleOnlyComments({
    comment,
    enclosingNode,
    followingNode,
    ast,
    isLastComment
  }) {
    var _a;
    if (((_a = ast == null ? void 0 : ast.body) == null ? void 0 : _a.length) === 0) {
      if (isLastComment) {
        addDanglingComment(ast, comment);
      } else {
        addLeadingComment(ast, comment);
      }
      return true;
    }
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "Program" && enclosingNode.body.length === 0 && !is_non_empty_array_default(enclosingNode.directives)) {
      if (isLastComment) {
        addDanglingComment(enclosingNode, comment);
      } else {
        addLeadingComment(enclosingNode, comment);
      }
      return true;
    }
    if ((followingNode == null ? void 0 : followingNode.type) === "Program" && followingNode.body.length === 0 && (enclosingNode == null ? void 0 : enclosingNode.type) === "ModuleExpression") {
      addDanglingComment(followingNode, comment);
      return true;
    }
    return false;
  }
  function handleForComments({
    comment,
    enclosingNode
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "ForInStatement" || (enclosingNode == null ? void 0 : enclosingNode.type) === "ForOfStatement") {
      addLeadingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleModuleSpecifiersComments({
    comment,
    precedingNode,
    enclosingNode,
    text
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "ImportSpecifier" || (enclosingNode == null ? void 0 : enclosingNode.type) === "ExportSpecifier") {
      addLeadingComment(enclosingNode, comment);
      return true;
    }
    const isImportDeclaration = (precedingNode == null ? void 0 : precedingNode.type) === "ImportSpecifier" && (enclosingNode == null ? void 0 : enclosingNode.type) === "ImportDeclaration";
    const isExportDeclaration2 = (precedingNode == null ? void 0 : precedingNode.type) === "ExportSpecifier" && (enclosingNode == null ? void 0 : enclosingNode.type) === "ExportNamedDeclaration";
    if ((isImportDeclaration || isExportDeclaration2) && has_newline_default(text, locEnd(comment))) {
      addTrailingComment(precedingNode, comment);
      return true;
    }
    return false;
  }
  function handleAssignmentPatternComments({
    comment,
    enclosingNode
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) === "AssignmentPattern") {
      addLeadingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  var assignmentLikeNodeTypes = /* @__PURE__ */ new Set(["VariableDeclarator", "AssignmentExpression", "TypeAlias", "TSTypeAliasDeclaration"]);
  var complexExprNodeTypes = /* @__PURE__ */ new Set(["ObjectExpression", "RecordExpression", "ArrayExpression", "TupleExpression", "TemplateLiteral", "TaggedTemplateExpression", "ObjectTypeAnnotation", "TSTypeLiteral"]);
  function handleVariableDeclaratorComments({
    comment,
    enclosingNode,
    followingNode
  }) {
    if (assignmentLikeNodeTypes.has(enclosingNode == null ? void 0 : enclosingNode.type) && followingNode && (complexExprNodeTypes.has(followingNode.type) || is_block_comment_default(comment))) {
      addLeadingComment(followingNode, comment);
      return true;
    }
    return false;
  }
  function handleTSFunctionTrailingComments({
    comment,
    enclosingNode,
    followingNode,
    text
  }) {
    if (!followingNode && ((enclosingNode == null ? void 0 : enclosingNode.type) === "TSMethodSignature" || (enclosingNode == null ? void 0 : enclosingNode.type) === "TSDeclareFunction" || (enclosingNode == null ? void 0 : enclosingNode.type) === "TSAbstractMethodDefinition") && get_next_non_space_non_comment_character_default(text, locEnd(comment)) === ";") {
      addTrailingComment(enclosingNode, comment);
      return true;
    }
    return false;
  }
  function handleIgnoreComments({
    comment,
    enclosingNode,
    followingNode
  }) {
    if (isPrettierIgnoreComment(comment) && (enclosingNode == null ? void 0 : enclosingNode.type) === "TSMappedType" && (followingNode == null ? void 0 : followingNode.type) === "TSTypeParameter" && followingNode.constraint) {
      enclosingNode.prettierIgnore = true;
      comment.unignore = true;
      return true;
    }
  }
  function handleTSMappedTypeComments({
    comment,
    precedingNode,
    enclosingNode,
    followingNode
  }) {
    if ((enclosingNode == null ? void 0 : enclosingNode.type) !== "TSMappedType") {
      return false;
    }
    if ((followingNode == null ? void 0 : followingNode.type) === "TSTypeParameter" && followingNode.name) {
      addLeadingComment(followingNode.name, comment);
      return true;
    }
    if ((precedingNode == null ? void 0 : precedingNode.type) === "TSTypeParameter" && precedingNode.constraint) {
      addTrailingComment(precedingNode.constraint, comment);
      return true;
    }
    return false;
  }
  function handleSwitchDefaultCaseComments({
    comment,
    enclosingNode,
    followingNode
  }) {
    if (!enclosingNode || enclosingNode.type !== "SwitchCase" || enclosingNode.test || !followingNode || followingNode !== enclosingNode.consequent[0]) {
      return false;
    }
    if (followingNode.type === "BlockStatement" && isLineComment(comment)) {
      addBlockStatementFirstComment(followingNode, comment);
    } else {
      addDanglingComment(enclosingNode, comment);
    }
    return true;
  }
  var isRealFunctionLikeNode = create_type_check_function_default(["ArrowFunctionExpression", "FunctionExpression", "FunctionDeclaration", "ObjectMethod", "ClassMethod", "TSDeclareFunction", "TSCallSignatureDeclaration", "TSConstructSignatureDeclaration", "TSMethodSignature", "TSConstructorType", "TSFunctionType", "TSDeclareMethod"]);

  // src/language-js/utils/is-indentable-block-comment.js
  function isIndentableBlockComment(comment) {
    const lines = `*${comment.value}*`.split("\n");
    return lines.length > 1 && lines.every((line2) => line2.trimStart()[0] === "*");
  }
  var is_indentable_block_comment_default = isIndentableBlockComment;

  // src/language-js/print/comment.js
  function printComment2(commentPath, options2) {
    const comment = commentPath.node;
    if (isLineComment(comment)) {
      return options2.originalText.slice(locStart(comment), locEnd(comment)).trimEnd();
    }
    if (is_block_comment_default(comment)) {
      if (is_indentable_block_comment_default(comment)) {
        return printIndentableBlockComment(comment);
      }
      return ["/*", replaceEndOfLine(comment.value), "*/"];
    }
    throw new Error("Not a comment: " + JSON.stringify(comment));
  }
  function printIndentableBlockComment(comment) {
    const lines = comment.value.split("\n");
    return [
      "/*",
      join(
        hardline,
        lines.map(
          (line2, index) => index === 0 ? line2.trimEnd() : " " + (index < lines.length - 1 ? line2.trim() : line2.trimStart())
        )
      ),
      "*/"
    ];
  }

  // src/language-js/comments/printer-methods.js
  var nodeTypesCanNotAttachComment = /* @__PURE__ */ new Set([
    "EmptyStatement",
    "TemplateElement",
    // In ESTree `import` is a token, `import("foo")`
    "Import",
    // There is no similar node in Babel AST
    // ```ts
    // class Foo {
    //   bar();
    //      ^^^ TSEmptyBodyFunctionExpression
    // }
    // ```
    "TSEmptyBodyFunctionExpression",
    // There is no similar node in Babel AST, `a?.b`
    "ChainExpression"
  ]);
  function canAttachComment(node) {
    return !nodeTypesCanNotAttachComment.has(node.type);
  }
  function getCommentChildNodes(node, options2) {
    var _a;
    if ((options2.parser === "typescript" || options2.parser === "flow" || options2.parser === "acorn" || options2.parser === "espree" || options2.parser === "meriyah" || options2.parser === "__babel_estree") && node.type === "MethodDefinition" && ((_a = node.value) == null ? void 0 : _a.type) === "FunctionExpression" && getFunctionParameters(node.value).length === 0 && !node.value.returnType && !is_non_empty_array_default(node.value.typeParameters) && node.value.body) {
      return [...node.decorators || [], node.key, node.value.body];
    }
  }
  function willPrintOwnComments(path) {
    const {
      node,
      parent
    } = path;
    return (isJsxElement(node) || parent && (parent.type === "JSXSpreadAttribute" || parent.type === "JSXSpreadChild" || parent.type === "UnionTypeAnnotation" || parent.type === "TSUnionType" || (parent.type === "ClassDeclaration" || parent.type === "ClassExpression") && parent.superClass === node)) && (!hasNodeIgnoreComment(node) || parent.type === "UnionTypeAnnotation" || parent.type === "TSUnionType");
  }
  function isGap(text, {
    parser
  }) {
    if (parser === "flow" || parser === "babel-flow") {
      text = string_replace_all_default(
        /* isOptionalObject*/
        false,
        text,
        /[\s(]/g,
        ""
      );
      return text === "" || text === "/*" || text === "/*::";
    }
  }

  // src/language-js/print/jsx.js
  var jsxWhitespaceUtils = new whitespace_utils_default(" \n\r	");
  var isEmptyStringOrAnyLine = (doc) => doc === "" || doc === line || doc === hardline || doc === softline;
  function printJsxElementInternal(path, options2, print3) {
    var _a, _b, _c;
    const {
      node
    } = path;
    if (node.type === "JSXElement" && isEmptyJsxElement(node)) {
      return [print3("openingElement"), print3("closingElement")];
    }
    const openingLines = node.type === "JSXElement" ? print3("openingElement") : print3("openingFragment");
    const closingLines = node.type === "JSXElement" ? print3("closingElement") : print3("closingFragment");
    if (node.children.length === 1 && node.children[0].type === "JSXExpressionContainer" && (node.children[0].expression.type === "TemplateLiteral" || node.children[0].expression.type === "TaggedTemplateExpression")) {
      return [openingLines, ...path.map(print3, "children"), closingLines];
    }
    node.children = node.children.map((child) => {
      if (isJsxWhitespaceExpression(child)) {
        return {
          type: "JSXText",
          value: " ",
          raw: " "
        };
      }
      return child;
    });
    const containsTag = node.children.some(isJsxElement);
    const containsMultipleExpressions = node.children.filter((child) => child.type === "JSXExpressionContainer").length > 1;
    const containsMultipleAttributes = node.type === "JSXElement" && node.openingElement.attributes.length > 1;
    let forcedBreak = willBreak(openingLines) || containsTag || containsMultipleAttributes || containsMultipleExpressions;
    const isMdxBlock = path.parent.rootMarker === "mdx";
    const rawJsxWhitespace = options2.singleQuote ? "{' '}" : '{" "}';
    const jsxWhitespace = isMdxBlock ? " " : ifBreak([rawJsxWhitespace, softline], " ");
    const isFacebookTranslationTag = ((_b = (_a = node.openingElement) == null ? void 0 : _a.name) == null ? void 0 : _b.name) === "fbt";
    const children = printJsxChildren(path, options2, print3, jsxWhitespace, isFacebookTranslationTag);
    const containsText = node.children.some((child) => isMeaningfulJsxText(child));
    for (let i = children.length - 2; i >= 0; i--) {
      const isPairOfEmptyStrings = children[i] === "" && children[i + 1] === "";
      const isPairOfHardlines = children[i] === hardline && children[i + 1] === "" && children[i + 2] === hardline;
      const isLineFollowedByJsxWhitespace = (children[i] === softline || children[i] === hardline) && children[i + 1] === "" && children[i + 2] === jsxWhitespace;
      const isJsxWhitespaceFollowedByLine = children[i] === jsxWhitespace && children[i + 1] === "" && (children[i + 2] === softline || children[i + 2] === hardline);
      const isDoubleJsxWhitespace = children[i] === jsxWhitespace && children[i + 1] === "" && children[i + 2] === jsxWhitespace;
      const isPairOfHardOrSoftLines = children[i] === softline && children[i + 1] === "" && children[i + 2] === hardline || children[i] === hardline && children[i + 1] === "" && children[i + 2] === softline;
      if (isPairOfHardlines && containsText || isPairOfEmptyStrings || isLineFollowedByJsxWhitespace || isDoubleJsxWhitespace || isPairOfHardOrSoftLines) {
        children.splice(i, 2);
      } else if (isJsxWhitespaceFollowedByLine) {
        children.splice(i + 1, 2);
      }
    }
    while (children.length > 0 && isEmptyStringOrAnyLine(at_default(
      /* isOptionalObject*/
      false,
      children,
      -1
    ))) {
      children.pop();
    }
    while (children.length > 1 && isEmptyStringOrAnyLine(children[0]) && isEmptyStringOrAnyLine(children[1])) {
      children.shift();
      children.shift();
    }
    const multilineChildren = [];
    for (const [i, child] of children.entries()) {
      if (child === jsxWhitespace) {
        if (i === 1 && children[i - 1] === "") {
          if (children.length === 2) {
            multilineChildren.push(rawJsxWhitespace);
            continue;
          }
          multilineChildren.push([rawJsxWhitespace, hardline]);
          continue;
        } else if (i === children.length - 1) {
          multilineChildren.push(rawJsxWhitespace);
          continue;
        } else if (children[i - 1] === "" && children[i - 2] === hardline) {
          multilineChildren.push(rawJsxWhitespace);
          continue;
        }
      }
      multilineChildren.push(child);
      if (willBreak(child)) {
        forcedBreak = true;
      }
    }
    let content = containsText ? fill(multilineChildren) : group(multilineChildren, {
      shouldBreak: true
    });
    if (((_c = options2.cursorNode) == null ? void 0 : _c.type) === "JSXText" && node.children.includes(options2.cursorNode)) {
      content = [cursor, content, cursor];
    }
    if (isMdxBlock) {
      return content;
    }
    const multiLineElem = group([openingLines, indent([hardline, content]), hardline, closingLines]);
    if (forcedBreak) {
      return multiLineElem;
    }
    return conditionalGroup([group([openingLines, ...children, closingLines]), multiLineElem]);
  }
  function printJsxChildren(path, options2, print3, jsxWhitespace, isFacebookTranslationTag) {
    const parts = [];
    path.each(({
      node,
      next
    }) => {
      if (node.type === "JSXText") {
        const text = rawText(node);
        if (isMeaningfulJsxText(node)) {
          const words = jsxWhitespaceUtils.split(
            text,
            /* captureWhitespace */
            true
          );
          if (words[0] === "") {
            parts.push("");
            words.shift();
            if (/\n/.test(words[0])) {
              parts.push(separatorWithWhitespace(isFacebookTranslationTag, words[1], node, next));
            } else {
              parts.push(jsxWhitespace);
            }
            words.shift();
          }
          let endWhitespace;
          if (at_default(
            /* isOptionalObject*/
            false,
            words,
            -1
          ) === "") {
            words.pop();
            endWhitespace = words.pop();
          }
          if (words.length === 0) {
            return;
          }
          for (const [i, word] of words.entries()) {
            if (i % 2 === 1) {
              parts.push(line);
            } else {
              parts.push(word);
            }
          }
          if (endWhitespace !== void 0) {
            if (/\n/.test(endWhitespace)) {
              parts.push(separatorWithWhitespace(isFacebookTranslationTag, at_default(
                /* isOptionalObject*/
                false,
                parts,
                -1
              ), node, next));
            } else {
              parts.push(jsxWhitespace);
            }
          } else {
            parts.push(separatorNoWhitespace(isFacebookTranslationTag, at_default(
              /* isOptionalObject*/
              false,
              parts,
              -1
            ), node, next));
          }
        } else if (/\n/.test(text)) {
          if (text.match(/\n/g).length > 1) {
            parts.push("", hardline);
          }
        } else {
          parts.push("", jsxWhitespace);
        }
      } else {
        const printedChild = print3();
        parts.push(printedChild);
        const directlyFollowedByMeaningfulText = next && isMeaningfulJsxText(next);
        if (directlyFollowedByMeaningfulText) {
          const trimmed = jsxWhitespaceUtils.trim(rawText(next));
          const [firstWord] = jsxWhitespaceUtils.split(trimmed);
          parts.push(separatorNoWhitespace(isFacebookTranslationTag, firstWord, node, next));
        } else {
          parts.push(hardline);
        }
      }
    }, "children");
    return parts;
  }
  function separatorNoWhitespace(isFacebookTranslationTag, child, childNode, nextNode) {
    if (isFacebookTranslationTag) {
      return "";
    }
    if (childNode.type === "JSXElement" && !childNode.closingElement || (nextNode == null ? void 0 : nextNode.type) === "JSXElement" && !nextNode.closingElement) {
      return child.length === 1 ? softline : hardline;
    }
    return softline;
  }
  function separatorWithWhitespace(isFacebookTranslationTag, child, childNode, nextNode) {
    if (isFacebookTranslationTag) {
      return hardline;
    }
    if (child.length === 1) {
      return childNode.type === "JSXElement" && !childNode.closingElement || (nextNode == null ? void 0 : nextNode.type) === "JSXElement" && !nextNode.closingElement ? hardline : softline;
    }
    return hardline;
  }
  var NO_WRAP_PARENTS = /* @__PURE__ */ new Set(["ArrayExpression", "TupleExpression", "JSXAttribute", "JSXElement", "JSXExpressionContainer", "JSXFragment", "ExpressionStatement", "CallExpression", "OptionalCallExpression", "ConditionalExpression", "JsExpressionRoot"]);
  function maybeWrapJsxElementInParens(path, elem, options2) {
    const {
      parent
    } = path;
    if (NO_WRAP_PARENTS.has(parent.type)) {
      return elem;
    }
    const shouldBreak = path.match(void 0, (node) => node.type === "ArrowFunctionExpression", isCallExpression, (node) => node.type === "JSXExpressionContainer");
    const needsParens2 = needs_parens_default(path, options2);
    return group([needsParens2 ? "" : ifBreak("("), indent([softline, elem]), softline, needsParens2 ? "" : ifBreak(")")], {
      shouldBreak
    });
  }
  function printJsxAttribute(path, options2, print3) {
    const {
      node
    } = path;
    const parts = [];
    parts.push(print3("name"));
    if (node.value) {
      let res;
      if (isStringLiteral(node.value)) {
        const raw = rawText(node.value);
        let final = string_replace_all_default(
          /* isOptionalObject*/
          false,
          string_replace_all_default(
            /* isOptionalObject*/
            false,
            raw.slice(1, -1),
            "&apos;",
            "'"
          ),
          "&quot;",
          '"'
        );
        const quote = get_preferred_quote_default(final, options2.jsxSingleQuote);
        final = quote === '"' ? string_replace_all_default(
          /* isOptionalObject*/
          false,
          final,
          '"',
          "&quot;"
        ) : string_replace_all_default(
          /* isOptionalObject*/
          false,
          final,
          "'",
          "&apos;"
        );
        res = path.call(() => printComments(path, replaceEndOfLine(quote + final + quote), options2), "value");
      } else {
        res = print3("value");
      }
      parts.push("=", res);
    }
    return parts;
  }
  function printJsxExpressionContainer(path, options2, print3) {
    const {
      node
    } = path;
    const shouldInline = (node2, parent) => node2.type === "JSXEmptyExpression" || !hasComment(node2) && (isArrayOrTupleExpression(node2) || isObjectOrRecordExpression(node2) || node2.type === "ArrowFunctionExpression" || node2.type === "AwaitExpression" && (shouldInline(node2.argument, node2) || node2.argument.type === "JSXElement") || isCallExpression(node2) || node2.type === "ChainExpression" && isCallExpression(node2.expression) || node2.type === "FunctionExpression" || node2.type === "TemplateLiteral" || node2.type === "TaggedTemplateExpression" || node2.type === "DoExpression" || isJsxElement(parent) && (node2.type === "ConditionalExpression" || isBinaryish(node2)));
    if (shouldInline(node.expression, path.parent)) {
      return group(["{", print3("expression"), lineSuffixBoundary, "}"]);
    }
    return group(["{", indent([softline, print3("expression")]), softline, lineSuffixBoundary, "}"]);
  }
  function printJsxOpeningElement(path, options2, print3) {
    var _a, _b;
    const {
      node
    } = path;
    const nameHasComments = hasComment(node.name) || hasComment(node.typeParameters);
    if (node.selfClosing && node.attributes.length === 0 && !nameHasComments) {
      return ["<", print3("name"), print3("typeParameters"), " />"];
    }
    if (((_a = node.attributes) == null ? void 0 : _a.length) === 1 && node.attributes[0].value && isStringLiteral(node.attributes[0].value) && !node.attributes[0].value.value.includes("\n") && // We should break for the following cases:
    // <div
    //   // comment
    //   attr="value"
    // >
    // <div
    //   attr="value"
    //   // comment
    // >
    !nameHasComments && !hasComment(node.attributes[0])) {
      return group(["<", print3("name"), print3("typeParameters"), " ", ...path.map(print3, "attributes"), node.selfClosing ? " />" : ">"]);
    }
    const shouldBreak = (_b = node.attributes) == null ? void 0 : _b.some((attr) => attr.value && isStringLiteral(attr.value) && attr.value.value.includes("\n"));
    const attributeLine = options2.singleAttributePerLine && node.attributes.length > 1 ? hardline : line;
    return group(["<", print3("name"), print3("typeParameters"), indent(path.map(() => [attributeLine, print3()], "attributes")), ...printEndOfOpeningTag(node, options2, nameHasComments)], {
      shouldBreak
    });
  }
  function printEndOfOpeningTag(node, options2, nameHasComments) {
    if (node.selfClosing) {
      return [line, "/>"];
    }
    const bracketSameLine = shouldPrintBracketSameLine(node, options2, nameHasComments);
    if (bracketSameLine) {
      return [">"];
    }
    return [softline, ">"];
  }
  function shouldPrintBracketSameLine(node, options2, nameHasComments) {
    const lastAttrHasTrailingComments = node.attributes.length > 0 && hasComment(at_default(
      /* isOptionalObject*/
      false,
      node.attributes,
      -1
    ), CommentCheckFlags.Trailing);
    return (
      // Simple tags (no attributes and no comment in tag name) should be
      // kept unbroken regardless of `bracketSameLine`.
      // jsxBracketSameLine is deprecated in favour of bracketSameLine,
      // but is still needed for backwards compatibility.
      node.attributes.length === 0 && !nameHasComments || (options2.bracketSameLine || options2.jsxBracketSameLine) && // We should print the bracket in a new line for the following cases:
      // <div
      //   // comment
      // >
      // <div
      //   attr // comment
      // >
      (!nameHasComments || node.attributes.length > 0) && !lastAttrHasTrailingComments
    );
  }
  function printJsxClosingElement(path, options2, print3) {
    const {
      node
    } = path;
    const parts = [];
    parts.push("</");
    const printed = print3("name");
    if (hasComment(node.name, CommentCheckFlags.Leading | CommentCheckFlags.Line)) {
      parts.push(indent([hardline, printed]), hardline);
    } else if (hasComment(node.name, CommentCheckFlags.Leading | CommentCheckFlags.Block)) {
      parts.push(" ", printed);
    } else {
      parts.push(printed);
    }
    parts.push(">");
    return parts;
  }
  function printJsxOpeningClosingFragment(path, options2) {
    const {
      node
    } = path;
    const nodeHasComment = hasComment(node);
    const hasOwnLineComment = hasComment(node, CommentCheckFlags.Line);
    const isOpeningFragment = node.type === "JSXOpeningFragment";
    return [isOpeningFragment ? "<" : "</", indent([hasOwnLineComment ? hardline : nodeHasComment && !isOpeningFragment ? " " : "", printDanglingComments(path, options2)]), hasOwnLineComment ? hardline : "", ">"];
  }
  function printJsxElement(path, options2, print3) {
    const elem = printComments(path, printJsxElementInternal(path, options2, print3), options2);
    return maybeWrapJsxElementInParens(path, elem, options2);
  }
  function printJsxEmptyExpression(path, options2) {
    const {
      node
    } = path;
    const requiresHardline = hasComment(node, CommentCheckFlags.Line);
    return [printDanglingComments(path, options2, {
      indent: requiresHardline
    }), requiresHardline ? hardline : ""];
  }
  function printJsxSpreadAttributeOrChild(path, options2, print3) {
    const {
      node
    } = path;
    return ["{", path.call(({
      node: node2
    }) => {
      const printed = ["...", print3()];
      if (!hasComment(node2) || !willPrintOwnComments(path)) {
        return printed;
      }
      return [indent([softline, printComments(path, printed, options2)]), softline];
    }, node.type === "JSXSpreadAttribute" ? "argument" : "expression"), "}"];
  }
  function printJsx(path, options2, print3) {
    const {
      node
    } = path;
    if (!node.type.startsWith("JSX")) {
      return;
    }
    switch (node.type) {
      case "JSXAttribute":
        return printJsxAttribute(path, options2, print3);
      case "JSXIdentifier":
        return node.name;
      case "JSXNamespacedName":
        return join(":", [print3("namespace"), print3("name")]);
      case "JSXMemberExpression":
        return join(".", [print3("object"), print3("property")]);
      case "JSXSpreadAttribute":
      case "JSXSpreadChild":
        return printJsxSpreadAttributeOrChild(path, options2, print3);
      case "JSXExpressionContainer":
        return printJsxExpressionContainer(path, options2, print3);
      case "JSXFragment":
      case "JSXElement":
        return printJsxElement(path, options2, print3);
      case "JSXOpeningElement":
        return printJsxOpeningElement(path, options2, print3);
      case "JSXClosingElement":
        return printJsxClosingElement(path, options2, print3);
      case "JSXOpeningFragment":
      case "JSXClosingFragment":
        return printJsxOpeningClosingFragment(
          path,
          options2
          /*, print*/
        );
      case "JSXEmptyExpression":
        return printJsxEmptyExpression(
          path,
          options2
          /*, print*/
        );
      case "JSXText":
        throw new Error("JSXText should be handled by JSXElement");
      default:
        throw new unexpected_node_error_default(node, "JSX");
    }
  }
  function isEmptyJsxElement(node) {
    if (node.children.length === 0) {
      return true;
    }
    if (node.children.length > 1) {
      return false;
    }
    const child = node.children[0];
    return child.type === "JSXText" && !isMeaningfulJsxText(child);
  }
  function isMeaningfulJsxText(node) {
    return node.type === "JSXText" && (jsxWhitespaceUtils.hasNonWhitespaceCharacter(rawText(node)) || !/\n/.test(rawText(node)));
  }
  function isJsxWhitespaceExpression(node) {
    return node.type === "JSXExpressionContainer" && isStringLiteral(node.expression) && node.expression.value === " " && !hasComment(node.expression);
  }
  function hasJsxIgnoreComment(path) {
    const {
      node,
      parent
    } = path;
    if (!isJsxElement(node) || !isJsxElement(parent)) {
      return false;
    }
    const {
      index,
      siblings
    } = path;
    let prevSibling;
    for (let i = index; i > 0; i--) {
      const candidate = siblings[i - 1];
      if (candidate.type === "JSXText" && !isMeaningfulJsxText(candidate)) {
        continue;
      }
      prevSibling = candidate;
      break;
    }
    return (prevSibling == null ? void 0 : prevSibling.type) === "JSXExpressionContainer" && prevSibling.expression.type === "JSXEmptyExpression" && hasNodeIgnoreComment(prevSibling.expression);
  }

  // src/language-js/utils/is-ignored.js
  function isIgnored(path) {
    return hasNodeIgnoreComment(path.node) || hasJsxIgnoreComment(path);
  }
  var is_ignored_default = isIgnored;

  // src/language-js/print/binaryish.js
  var uid = 0;
  function printBinaryishExpression(path, options2, print3) {
    var _a;
    const {
      node,
      parent,
      grandparent,
      key
    } = path;
    const isInsideParenthesis = key !== "body" && (parent.type === "IfStatement" || parent.type === "WhileStatement" || parent.type === "SwitchStatement" || parent.type === "DoWhileStatement");
    const isHackPipeline = node.operator === "|>" && ((_a = path.root.extra) == null ? void 0 : _a.__isUsingHackPipeline);
    const parts = printBinaryishExpressions(
      path,
      print3,
      options2,
      /* isNested */
      false,
      isInsideParenthesis
    );
    if (isInsideParenthesis) {
      return parts;
    }
    if (isHackPipeline) {
      return group(parts);
    }
    if (isCallExpression(parent) && parent.callee === node || parent.type === "UnaryExpression" || isMemberExpression(parent) && !parent.computed) {
      return group([indent([softline, ...parts]), softline]);
    }
    const shouldNotIndent = parent.type === "ReturnStatement" || parent.type === "ThrowStatement" || parent.type === "JSXExpressionContainer" && grandparent.type === "JSXAttribute" || node.operator !== "|" && parent.type === "JsExpressionRoot" || node.type !== "NGPipeExpression" && (parent.type === "NGRoot" && options2.parser === "__ng_binding" || parent.type === "NGMicrosyntaxExpression" && grandparent.type === "NGMicrosyntax" && grandparent.body.length === 1) || node === parent.body && parent.type === "ArrowFunctionExpression" || node !== parent.body && parent.type === "ForStatement" || parent.type === "ConditionalExpression" && grandparent.type !== "ReturnStatement" && grandparent.type !== "ThrowStatement" && !isCallExpression(grandparent) || parent.type === "TemplateLiteral";
    const shouldIndentIfInlining = parent.type === "AssignmentExpression" || parent.type === "VariableDeclarator" || parent.type === "ClassProperty" || parent.type === "PropertyDefinition" || parent.type === "TSAbstractPropertyDefinition" || parent.type === "ClassPrivateProperty" || isObjectProperty(parent);
    const samePrecedenceSubExpression = isBinaryish(node.left) && shouldFlatten(node.operator, node.left.operator);
    if (shouldNotIndent || shouldInlineLogicalExpression(node) && !samePrecedenceSubExpression || !shouldInlineLogicalExpression(node) && shouldIndentIfInlining) {
      return group(parts);
    }
    if (parts.length === 0) {
      return "";
    }
    const hasJsx = isJsxElement(node.right);
    const firstGroupIndex = parts.findIndex((part) => typeof part !== "string" && !Array.isArray(part) && part.type === DOC_TYPE_GROUP);
    const headParts = parts.slice(0, firstGroupIndex === -1 ? 1 : firstGroupIndex + 1);
    const rest = parts.slice(headParts.length, hasJsx ? -1 : void 0);
    const groupId = Symbol("logicalChain-" + ++uid);
    const chain = group([
      // Don't include the initial expression in the indentation
      // level. The first item is guaranteed to be the first
      // left-most expression.
      ...headParts,
      indent(rest)
    ], {
      id: groupId
    });
    if (!hasJsx) {
      return chain;
    }
    const jsxPart = at_default(
      /* isOptionalObject*/
      false,
      parts,
      -1
    );
    return group([chain, indentIfBreak(jsxPart, {
      groupId
    })]);
  }
  function printBinaryishExpressions(path, print3, options2, isNested, isInsideParenthesis) {
    var _a;
    const {
      node
    } = path;
    if (!isBinaryish(node)) {
      return [group(print3())];
    }
    let parts = [];
    if (shouldFlatten(node.operator, node.left.operator)) {
      parts = path.call((left) => printBinaryishExpressions(
        left,
        print3,
        options2,
        /* isNested */
        true,
        isInsideParenthesis
      ), "left");
    } else {
      parts.push(group(print3("left")));
    }
    const shouldInline = shouldInlineLogicalExpression(node);
    const lineBeforeOperator = (node.operator === "|>" || node.type === "NGPipeExpression" || isVueFilterSequenceExpression(path, options2)) && !hasLeadingOwnLineComment(options2.originalText, node.right);
    const operator = node.type === "NGPipeExpression" ? "|" : node.operator;
    const rightSuffix = node.type === "NGPipeExpression" && node.arguments.length > 0 ? group(indent([softline, ": ", join([line, ": "], path.map(() => align(2, group(print3())), "arguments"))])) : "";
    let right;
    if (shouldInline) {
      right = [operator, " ", print3("right"), rightSuffix];
    } else {
      const isHackPipeline = operator === "|>" && ((_a = path.root.extra) == null ? void 0 : _a.__isUsingHackPipeline);
      const rightContent = isHackPipeline ? path.call((left) => printBinaryishExpressions(
        left,
        print3,
        options2,
        /* isNested */
        true,
        isInsideParenthesis
      ), "right") : print3("right");
      right = [lineBeforeOperator ? line : "", operator, lineBeforeOperator ? " " : line, rightContent, rightSuffix];
    }
    const {
      parent
    } = path;
    const shouldBreak = hasComment(node.left, CommentCheckFlags.Trailing | CommentCheckFlags.Line);
    const shouldGroup = shouldBreak || !(isInsideParenthesis && node.type === "LogicalExpression") && parent.type !== node.type && node.left.type !== node.type && node.right.type !== node.type;
    parts.push(lineBeforeOperator ? "" : " ", shouldGroup ? group(right, {
      shouldBreak
    }) : right);
    if (isNested && hasComment(node)) {
      const printed = cleanDoc(printComments(path, parts, options2));
      if (Array.isArray(printed) || printed.type === DOC_TYPE_FILL) {
        return getDocParts(printed);
      }
      return [printed];
    }
    return parts;
  }
  function shouldInlineLogicalExpression(node) {
    if (node.type !== "LogicalExpression") {
      return false;
    }
    if (isObjectOrRecordExpression(node.right) && node.right.properties.length > 0) {
      return true;
    }
    if (isArrayOrTupleExpression(node.right) && node.right.elements.length > 0) {
      return true;
    }
    if (isJsxElement(node.right)) {
      return true;
    }
    return false;
  }
  var isBitwiseOrExpression = (node) => node.type === "BinaryExpression" && node.operator === "|";
  function isVueFilterSequenceExpression(path, options2) {
    return (options2.parser === "__vue_expression" || options2.parser === "__vue_ts_expression") && isBitwiseOrExpression(path.node) && !path.hasAncestor((node) => !isBitwiseOrExpression(node) && node.type !== "JsExpressionRoot");
  }

  // src/common/errors.js
  var ArgExpansionBailout = class extends Error {
    name = "ArgExpansionBailout";
  };

  // src/language-js/print/array.js
  function printEmptyArrayElements(path, options2, openBracket, closeBracket) {
    const {
      node
    } = path;
    if (!hasComment(node, CommentCheckFlags.Dangling)) {
      return [openBracket, closeBracket];
    }
    return group([openBracket, printDanglingComments(path, options2, {
      indent: true
    }), softline, closeBracket]);
  }
  function printArray(path, options2, print3) {
    const {
      node
    } = path;
    const parts = [];
    const openBracket = node.type === "TupleExpression" ? "#[" : "[";
    const closeBracket = "]";
    const elementsProperty = (
      // TODO: Remove `types` when babel changes AST of `TupleTypeAnnotation`
      node.type === "TupleTypeAnnotation" && node.types ? "types" : node.type === "TSTupleType" || node.type === "TupleTypeAnnotation" ? "elementTypes" : "elements"
    );
    const elements = node[elementsProperty];
    if (elements.length === 0) {
      parts.push(printEmptyArrayElements(path, options2, openBracket, closeBracket));
    } else {
      const lastElem = at_default(
        /* isOptionalObject*/
        false,
        elements,
        -1
      );
      const canHaveTrailingComma = (lastElem == null ? void 0 : lastElem.type) !== "RestElement";
      const needsForcedTrailingComma = lastElem === null;
      const groupId = Symbol("array");
      const shouldBreak = !options2.__inJestEach && elements.length > 1 && elements.every((element, i, elements2) => {
        const elementType = element == null ? void 0 : element.type;
        if (!isArrayOrTupleExpression(element) && !isObjectOrRecordExpression(element)) {
          return false;
        }
        const nextElement = elements2[i + 1];
        if (nextElement && elementType !== nextElement.type) {
          return false;
        }
        const itemsKey = isArrayOrTupleExpression(element) ? "elements" : "properties";
        return element[itemsKey] && element[itemsKey].length > 1;
      });
      const shouldUseConciseFormatting = isConciselyPrintedArray(node, options2);
      const trailingComma = !canHaveTrailingComma ? "" : needsForcedTrailingComma ? "," : !shouldPrintComma(options2) ? "" : shouldUseConciseFormatting ? ifBreak(",", "", {
        groupId
      }) : ifBreak(",");
      parts.push(group([openBracket, indent([softline, shouldUseConciseFormatting ? printArrayElementsConcisely(path, options2, print3, trailingComma) : [printArrayElements(path, options2, elementsProperty, print3), trailingComma], printDanglingComments(path, options2)]), softline, closeBracket], {
        shouldBreak,
        id: groupId
      }));
    }
    parts.push(printOptionalToken(path), printTypeAnnotationProperty(path, print3));
    return parts;
  }
  function isConciselyPrintedArray(node, options2) {
    return isArrayOrTupleExpression(node) && node.elements.length > 1 && node.elements.every((element) => element && (isNumericLiteral(element) || isSignedNumericLiteral(element) && !hasComment(element.argument)) && !hasComment(element, CommentCheckFlags.Trailing | CommentCheckFlags.Line, (comment) => !has_newline_default(options2.originalText, locStart(comment), {
      backwards: true
    })));
  }
  function isLineAfterElementEmpty({
    node
  }, {
    originalText: text
  }) {
    let currentIdx = locEnd(node);
    if (currentIdx === locStart(node)) {
      return false;
    }
    const length = text.length;
    while (currentIdx < length) {
      if (text[currentIdx] === ",") {
        break;
      }
      currentIdx = skip_inline_comment_default(text, skip_trailing_comment_default(text, currentIdx + 1));
    }
    return is_next_line_empty_default(text, currentIdx);
  }
  function printArrayElements(path, options2, elementsProperty, print3) {
    const parts = [];
    path.each(({
      node,
      isLast
    }) => {
      parts.push(node ? group(print3()) : "");
      if (!isLast) {
        parts.push([",", line, node && isLineAfterElementEmpty(path, options2) ? softline : ""]);
      }
    }, elementsProperty);
    return parts;
  }
  function printArrayElementsConcisely(path, options2, print3, trailingComma) {
    const parts = [];
    path.each(({
      isLast,
      next
    }) => {
      parts.push([print3(), isLast ? trailingComma : ","]);
      if (!isLast) {
        parts.push(isLineAfterElementEmpty(path, options2) ? [hardline, hardline] : hasComment(next, CommentCheckFlags.Leading | CommentCheckFlags.Line) ? hardline : line);
      }
    }, "elements");
    return fill(parts);
  }

  // src/language-js/print/call-arguments.js
  function printCallArguments(path, options2, print3) {
    const {
      node
    } = path;
    const args = getCallArguments(node);
    if (args.length === 0) {
      return ["(", printDanglingComments(path, options2), ")"];
    }
    if (isReactHookCallWithDepsArray(args)) {
      return ["(", print3(["arguments", 0]), ", ", print3(["arguments", 1]), ")"];
    }
    let anyArgEmptyLine = false;
    const lastArgIndex = args.length - 1;
    const printedArguments = [];
    iterateCallArgumentsPath(path, ({
      node: arg
    }, index) => {
      let argDoc = print3();
      if (index === lastArgIndex) {
      } else if (isNextLineEmpty2(arg, options2)) {
        anyArgEmptyLine = true;
        argDoc = [argDoc, ",", hardline, hardline];
      } else {
        argDoc = [argDoc, ",", line];
      }
      printedArguments.push(argDoc);
    });
    const isDynamicImport = node.type === "ImportExpression" || node.callee.type === "Import";
    const maybeTrailingComma = !isDynamicImport && shouldPrintComma(options2, "all") ? "," : "";
    function allArgsBrokenOut() {
      return group(["(", indent([line, ...printedArguments]), maybeTrailingComma, line, ")"], {
        shouldBreak: true
      });
    }
    if (anyArgEmptyLine || path.parent.type !== "Decorator" && isFunctionCompositionArgs(args)) {
      return allArgsBrokenOut();
    }
    if (shouldExpandFirstArg(args)) {
      const tailArgs = printedArguments.slice(1);
      if (tailArgs.some(willBreak)) {
        return allArgsBrokenOut();
      }
      let firstArg;
      try {
        firstArg = print3(getCallArgumentSelector(node, 0), {
          expandFirstArg: true
        });
      } catch (caught) {
        if (caught instanceof ArgExpansionBailout) {
          return allArgsBrokenOut();
        }
        throw caught;
      }
      if (willBreak(firstArg)) {
        return [breakParent, conditionalGroup([["(", group(firstArg, {
          shouldBreak: true
        }), ", ", ...tailArgs, ")"], allArgsBrokenOut()])];
      }
      return conditionalGroup([["(", firstArg, ", ", ...tailArgs, ")"], ["(", group(firstArg, {
        shouldBreak: true
      }), ", ", ...tailArgs, ")"], allArgsBrokenOut()]);
    }
    if (shouldExpandLastArg(args, printedArguments, options2)) {
      const headArgs = printedArguments.slice(0, -1);
      if (headArgs.some(willBreak)) {
        return allArgsBrokenOut();
      }
      let lastArg;
      try {
        lastArg = print3(getCallArgumentSelector(node, -1), {
          expandLastArg: true
        });
      } catch (caught) {
        if (caught instanceof ArgExpansionBailout) {
          return allArgsBrokenOut();
        }
        throw caught;
      }
      if (willBreak(lastArg)) {
        return [breakParent, conditionalGroup([["(", ...headArgs, group(lastArg, {
          shouldBreak: true
        }), ")"], allArgsBrokenOut()])];
      }
      return conditionalGroup([["(", ...headArgs, lastArg, ")"], ["(", ...headArgs, group(lastArg, {
        shouldBreak: true
      }), ")"], allArgsBrokenOut()]);
    }
    const contents = ["(", indent([softline, ...printedArguments]), ifBreak(maybeTrailingComma), softline, ")"];
    if (isLongCurriedCallExpression(path)) {
      return contents;
    }
    return group(contents, {
      shouldBreak: printedArguments.some(willBreak) || anyArgEmptyLine
    });
  }
  function couldExpandArg(arg, arrowChainRecursion = false) {
    return isObjectOrRecordExpression(arg) && (arg.properties.length > 0 || hasComment(arg)) || isArrayOrTupleExpression(arg) && (arg.elements.length > 0 || hasComment(arg)) || arg.type === "TSTypeAssertion" && couldExpandArg(arg.expression) || isTSTypeExpression(arg) && couldExpandArg(arg.expression) || arg.type === "FunctionExpression" || arg.type === "ArrowFunctionExpression" && // we want to avoid breaking inside composite return types but not simple keywords
    // https://github.com/prettier/prettier/issues/4070
    // export class Thing implements OtherThing {
    //   do: (type: Type) => Provider<Prop> = memoize(
    //     (type: ObjectType): Provider<Opts> => {}
    //   );
    // }
    // https://github.com/prettier/prettier/issues/6099
    // app.get("/", (req, res): void => {
    //   res.send("Hello World!");
    // });
    (!arg.returnType || !arg.returnType.typeAnnotation || arg.returnType.typeAnnotation.type !== "TSTypeReference" || // https://github.com/prettier/prettier/issues/7542
    isNonEmptyBlockStatement(arg.body)) && (arg.body.type === "BlockStatement" || arg.body.type === "ArrowFunctionExpression" && couldExpandArg(arg.body, true) || isObjectOrRecordExpression(arg.body) || isArrayOrTupleExpression(arg.body) || !arrowChainRecursion && (isCallExpression(arg.body) || arg.body.type === "ConditionalExpression") || isJsxElement(arg.body)) || arg.type === "DoExpression" || arg.type === "ModuleExpression";
  }
  function shouldExpandLastArg(args, argDocs, options2) {
    var _a, _b;
    const lastArg = at_default(
      /* isOptionalObject*/
      false,
      args,
      -1
    );
    if (args.length === 1) {
      const lastArgDoc = at_default(
        /* isOptionalObject*/
        false,
        argDocs,
        -1
      );
      if (((_a = lastArgDoc.label) == null ? void 0 : _a.embed) && ((_b = lastArgDoc.label) == null ? void 0 : _b.hug) !== false) {
        return true;
      }
    }
    const penultimateArg = at_default(
      /* isOptionalObject*/
      false,
      args,
      -2
    );
    return !hasComment(lastArg, CommentCheckFlags.Leading) && !hasComment(lastArg, CommentCheckFlags.Trailing) && couldExpandArg(lastArg) && // If the last two arguments are of the same type,
    // disable last element expansion.
    (!penultimateArg || penultimateArg.type !== lastArg.type) && // useMemo(() => func(), [foo, bar, baz])
    (args.length !== 2 || penultimateArg.type !== "ArrowFunctionExpression" || !isArrayOrTupleExpression(lastArg)) && !(args.length > 1 && isConciselyPrintedArray(lastArg, options2));
  }
  function shouldExpandFirstArg(args) {
    if (args.length !== 2) {
      return false;
    }
    const [firstArg, secondArg] = args;
    if (firstArg.type === "ModuleExpression" && isTypeModuleObjectExpression(secondArg)) {
      return true;
    }
    return !hasComment(firstArg) && (firstArg.type === "FunctionExpression" || firstArg.type === "ArrowFunctionExpression" && firstArg.body.type === "BlockStatement") && secondArg.type !== "FunctionExpression" && secondArg.type !== "ArrowFunctionExpression" && secondArg.type !== "ConditionalExpression" && isHopefullyShortCallArgument(secondArg) && !couldExpandArg(secondArg);
  }
  function isHopefullyShortCallArgument(node) {
    var _a;
    if (node.type === "ParenthesizedExpression") {
      return isHopefullyShortCallArgument(node.expression);
    }
    if (isTSTypeExpression(node) || node.type === "TypeCastExpression") {
      let {
        typeAnnotation
      } = node;
      if (typeAnnotation.type === "TypeAnnotation") {
        typeAnnotation = typeAnnotation.typeAnnotation;
      }
      if (typeAnnotation.type === "TSArrayType") {
        typeAnnotation = typeAnnotation.elementType;
        if (typeAnnotation.type === "TSArrayType") {
          typeAnnotation = typeAnnotation.elementType;
        }
      }
      if ((typeAnnotation.type === "GenericTypeAnnotation" || typeAnnotation.type === "TSTypeReference") && ((_a = typeAnnotation.typeParameters) == null ? void 0 : _a.params.length) === 1) {
        typeAnnotation = typeAnnotation.typeParameters.params[0];
      }
      return isSimpleType(typeAnnotation) && isSimpleCallArgument(node.expression, 1);
    }
    if (isCallLikeExpression(node) && getCallArguments(node).length > 1) {
      return false;
    }
    if (isBinaryish(node)) {
      return isSimpleCallArgument(node.left, 1) && isSimpleCallArgument(node.right, 1);
    }
    return isRegExpLiteral(node) || isSimpleCallArgument(node);
  }
  function isReactHookCallWithDepsArray(args) {
    return args.length === 2 && args[0].type === "ArrowFunctionExpression" && getFunctionParameters(args[0]).length === 0 && args[0].body.type === "BlockStatement" && args[1].type === "ArrayExpression" && !args.some((arg) => hasComment(arg));
  }
  function isNonEmptyBlockStatement(node) {
    return node.type === "BlockStatement" && (node.body.some((node2) => node2.type !== "EmptyStatement") || hasComment(node, CommentCheckFlags.Dangling));
  }
  function isTypeModuleObjectExpression(node) {
    return node.type === "ObjectExpression" && node.properties.length === 1 && isObjectProperty(node.properties[0]) && node.properties[0].key.type === "Identifier" && node.properties[0].key.name === "type" && isStringLiteral(node.properties[0].value) && node.properties[0].value.value === "module";
  }
  var call_arguments_default = printCallArguments;

  // src/language-js/print/member.js
  function printMemberExpression(path, options2, print3) {
    var _a;
    const objectDoc = print3("object");
    const lookupDoc = printMemberLookup(path, options2, print3);
    const { node, parent } = path;
    const firstNonMemberParent = path.findAncestor(
      (node2) => !(isMemberExpression(node2) || node2.type === "TSNonNullExpression")
    );
    const shouldInline = firstNonMemberParent && (firstNonMemberParent.type === "NewExpression" || firstNonMemberParent.type === "BindExpression" || firstNonMemberParent.type === "AssignmentExpression" && firstNonMemberParent.left.type !== "Identifier") || node.computed || node.object.type === "Identifier" && node.property.type === "Identifier" && !isMemberExpression(parent) || (parent.type === "AssignmentExpression" || parent.type === "VariableDeclarator") && (isCallExpression(node.object) && node.object.arguments.length > 0 || node.object.type === "TSNonNullExpression" && isCallExpression(node.object.expression) && node.object.expression.arguments.length > 0 || ((_a = objectDoc.label) == null ? void 0 : _a.memberChain));
    return label(objectDoc.label, [
      objectDoc,
      shouldInline ? lookupDoc : group(indent([softline, lookupDoc]))
    ]);
  }
  function printMemberLookup(path, options2, print3) {
    const property = print3("property");
    const { node } = path;
    const optional = printOptionalToken(path);
    if (!node.computed) {
      return [optional, ".", property];
    }
    if (!node.property || isNumericLiteral(node.property)) {
      return [optional, "[", property, "]"];
    }
    return group([optional, "[", indent([softline, property]), softline, "]"]);
  }

  // src/language-js/print/member-chain.js
  function printMemberChain(path, options2, print3) {
    const {
      parent
    } = path;
    const isExpressionStatement = !parent || parent.type === "ExpressionStatement";
    const printedNodes = [];
    function shouldInsertEmptyLineAfter(node2) {
      const {
        originalText
      } = options2;
      const nextCharIndex = get_next_non_space_non_comment_character_index_default(originalText, locEnd(node2));
      const nextChar = originalText.charAt(nextCharIndex);
      if (nextChar === ")") {
        return nextCharIndex !== false && is_next_line_empty_default(originalText, nextCharIndex + 1);
      }
      return isNextLineEmpty2(node2, options2);
    }
    function rec(path2) {
      const {
        node: node2
      } = path2;
      if (isCallExpression(node2) && (isMemberish(node2.callee) || isCallExpression(node2.callee))) {
        printedNodes.unshift({
          node: node2,
          printed: [printComments(path2, [printOptionalToken(path2), printFunctionTypeParameters(path2, options2, print3), call_arguments_default(path2, options2, print3)], options2), shouldInsertEmptyLineAfter(node2) ? hardline : ""]
        });
        path2.call((callee) => rec(callee), "callee");
      } else if (isMemberish(node2)) {
        printedNodes.unshift({
          node: node2,
          needsParens: needs_parens_default(path2, options2),
          printed: printComments(path2, isMemberExpression(node2) ? printMemberLookup(path2, options2, print3) : printBindExpressionCallee(path2, options2, print3), options2)
        });
        path2.call((object) => rec(object), "object");
      } else if (node2.type === "TSNonNullExpression") {
        printedNodes.unshift({
          node: node2,
          printed: printComments(path2, "!", options2)
        });
        path2.call((expression) => rec(expression), "expression");
      } else {
        printedNodes.unshift({
          node: node2,
          printed: print3()
        });
      }
    }
    const {
      node
    } = path;
    printedNodes.unshift({
      node,
      printed: [printOptionalToken(path), printFunctionTypeParameters(path, options2, print3), call_arguments_default(path, options2, print3)]
    });
    if (node.callee) {
      path.call((callee) => rec(callee), "callee");
    }
    const groups = [];
    let currentGroup = [printedNodes[0]];
    let i = 1;
    for (; i < printedNodes.length; ++i) {
      if (printedNodes[i].node.type === "TSNonNullExpression" || isCallExpression(printedNodes[i].node) || isMemberExpression(printedNodes[i].node) && printedNodes[i].node.computed && isNumericLiteral(printedNodes[i].node.property)) {
        currentGroup.push(printedNodes[i]);
      } else {
        break;
      }
    }
    if (!isCallExpression(printedNodes[0].node)) {
      for (; i + 1 < printedNodes.length; ++i) {
        if (isMemberish(printedNodes[i].node) && isMemberish(printedNodes[i + 1].node)) {
          currentGroup.push(printedNodes[i]);
        } else {
          break;
        }
      }
    }
    groups.push(currentGroup);
    currentGroup = [];
    let hasSeenCallExpression = false;
    for (; i < printedNodes.length; ++i) {
      if (hasSeenCallExpression && isMemberish(printedNodes[i].node)) {
        if (printedNodes[i].node.computed && isNumericLiteral(printedNodes[i].node.property)) {
          currentGroup.push(printedNodes[i]);
          continue;
        }
        groups.push(currentGroup);
        currentGroup = [];
        hasSeenCallExpression = false;
      }
      if (isCallExpression(printedNodes[i].node) || printedNodes[i].node.type === "ImportExpression") {
        hasSeenCallExpression = true;
      }
      currentGroup.push(printedNodes[i]);
      if (hasComment(printedNodes[i].node, CommentCheckFlags.Trailing)) {
        groups.push(currentGroup);
        currentGroup = [];
        hasSeenCallExpression = false;
      }
    }
    if (currentGroup.length > 0) {
      groups.push(currentGroup);
    }
    function isFactory(name) {
      return /^[A-Z]|^[$_]+$/.test(name);
    }
    function isShort(name) {
      return name.length <= options2.tabWidth;
    }
    function shouldNotWrap(groups2) {
      var _a;
      const hasComputed = (_a = groups2[1][0]) == null ? void 0 : _a.node.computed;
      if (groups2[0].length === 1) {
        const firstNode = groups2[0][0].node;
        return firstNode.type === "ThisExpression" || firstNode.type === "Identifier" && (isFactory(firstNode.name) || isExpressionStatement && isShort(firstNode.name) || hasComputed);
      }
      const lastNode = at_default(
        /* isOptionalObject*/
        false,
        groups2[0],
        -1
      ).node;
      return isMemberExpression(lastNode) && lastNode.property.type === "Identifier" && (isFactory(lastNode.property.name) || hasComputed);
    }
    const shouldMerge = groups.length >= 2 && !hasComment(groups[1][0].node) && shouldNotWrap(groups);
    function printGroup(printedGroup) {
      const printed = printedGroup.map((tuple) => tuple.printed);
      if (printedGroup.length > 0 && at_default(
        /* isOptionalObject*/
        false,
        printedGroup,
        -1
      ).needsParens) {
        return ["(", ...printed, ")"];
      }
      return printed;
    }
    function printIndentedGroup(groups2) {
      if (groups2.length === 0) {
        return "";
      }
      return indent(group([hardline, join(hardline, groups2.map(printGroup))]));
    }
    const printedGroups = groups.map(printGroup);
    const oneLine = printedGroups;
    const cutoff = shouldMerge ? 3 : 2;
    const flatGroups = groups.flat();
    const nodeHasComment = flatGroups.slice(1, -1).some((node2) => hasComment(node2.node, CommentCheckFlags.Leading)) || flatGroups.slice(0, -1).some((node2) => hasComment(node2.node, CommentCheckFlags.Trailing)) || groups[cutoff] && hasComment(groups[cutoff][0].node, CommentCheckFlags.Leading);
    if (groups.length <= cutoff && !nodeHasComment) {
      if (isLongCurriedCallExpression(path)) {
        return oneLine;
      }
      return group(oneLine);
    }
    const lastNodeBeforeIndent = at_default(
      /* isOptionalObject*/
      false,
      groups[shouldMerge ? 1 : 0],
      -1
    ).node;
    const shouldHaveEmptyLineBeforeIndent = !isCallExpression(lastNodeBeforeIndent) && shouldInsertEmptyLineAfter(lastNodeBeforeIndent);
    const expanded = [printGroup(groups[0]), shouldMerge ? groups.slice(1, 2).map(printGroup) : "", shouldHaveEmptyLineBeforeIndent ? hardline : "", printIndentedGroup(groups.slice(shouldMerge ? 2 : 1))];
    const callExpressions = printedNodes.map(({
      node: node2
    }) => node2).filter(isCallExpression);
    function lastGroupWillBreakAndOtherCallsHaveFunctionArguments() {
      const lastGroupNode = at_default(
        /* isOptionalObject*/
        false,
        at_default(
          /* isOptionalObject*/
          false,
          groups,
          -1
        ),
        -1
      ).node;
      const lastGroupDoc = at_default(
        /* isOptionalObject*/
        false,
        printedGroups,
        -1
      );
      return isCallExpression(lastGroupNode) && willBreak(lastGroupDoc) && callExpressions.slice(0, -1).some((node2) => node2.arguments.some(isFunctionOrArrowExpression));
    }
    let result;
    if (nodeHasComment || callExpressions.length > 2 && callExpressions.some((expr) => !expr.arguments.every((arg) => isSimpleCallArgument(arg))) || printedGroups.slice(0, -1).some(willBreak) || lastGroupWillBreakAndOtherCallsHaveFunctionArguments()) {
      result = group(expanded);
    } else {
      result = [
        // We only need to check `oneLine` because if `expanded` is chosen
        // that means that the parent group has already been broken
        // naturally
        willBreak(oneLine) || shouldHaveEmptyLineBeforeIndent ? breakParent : "",
        conditionalGroup([oneLine, expanded])
      ];
    }
    return label({
      memberChain: true
    }, result);
  }
  var member_chain_default = printMemberChain;

  // src/language-js/print/call-expression.js
  function printCallExpression(path, options2, print3) {
    var _a;
    const { node, parent } = path;
    const isNew = node.type === "NewExpression";
    const isDynamicImport = node.type === "ImportExpression";
    const optional = printOptionalToken(path);
    const args = getCallArguments(node);
    const isTemplateLiteralSingleArg = args.length === 1 && isTemplateOnItsOwnLine(args[0], options2.originalText);
    if (isTemplateLiteralSingleArg || // Dangling comments are not handled, all these special cases should have arguments #9668
    args.length > 0 && !isNew && !isDynamicImport && // We want to keep CommonJS- and AMD-style require calls, and AMD-style
    // define calls, as a unit.
    // e.g. `define(["some/lib"], (lib) => {`
    (isCommonsJsOrAmdCall(node, parent) || // Keep test declarations on a single line
    // e.g. `it('long name', () => {`
    isTestCall(node, parent))) {
      const printed = [];
      iterateCallArgumentsPath(path, () => {
        printed.push(print3());
      });
      if (!(isTemplateLiteralSingleArg && ((_a = printed[0].label) == null ? void 0 : _a.embed))) {
        return [
          isNew ? "new " : "",
          print3("callee"),
          optional,
          printFunctionTypeParameters(path, options2, print3),
          "(",
          join(", ", printed),
          ")"
        ];
      }
    }
    if (!isDynamicImport && !isNew && isMemberish(node.callee) && !path.call((path2) => needs_parens_default(path2, options2), "callee")) {
      return member_chain_default(path, options2, print3);
    }
    const contents = [
      isNew ? "new " : "",
      isDynamicImport ? "import" : print3("callee"),
      optional,
      printFunctionTypeParameters(path, options2, print3),
      call_arguments_default(path, options2, print3)
    ];
    if (isDynamicImport || isCallExpression(node.callee)) {
      return group(contents);
    }
    return contents;
  }
  function isCommonsJsOrAmdCall(node, parentNode) {
    if (node.callee.type !== "Identifier") {
      return false;
    }
    if (node.callee.name === "require") {
      const args = getCallArguments(node);
      return args.length === 1 && isStringLiteral(args[0]) || args.length > 1;
    }
    if (node.callee.name === "define") {
      const args = getCallArguments(node);
      return parentNode.type === "ExpressionStatement" && (args.length === 1 || args.length === 2 && args[0].type === "ArrayExpression" || args.length === 3 && isStringLiteral(args[0]) && args[1].type === "ArrayExpression");
    }
    return false;
  }

  // src/utils/make-string.js
  function makeString(rawText2, enclosingQuote, unescapeUnnecessaryEscapes) {
    const otherQuote = enclosingQuote === '"' ? "'" : '"';
    const regex = /\\(.)|(["'])/gs;
    const raw = string_replace_all_default(
      /* isOptionalObject*/
      false,
      rawText2,
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

  // src/utils/print-number.js
  function printNumber(rawNumber) {
    return rawNumber.toLowerCase().replace(/^([+-]?[\d.]+e)(?:\+|(-))?0*(?=\d)/, "$1$2").replace(/^([+-]?[\d.]+)e[+-]?0+$/, "$1").replace(/^([+-])?\./, "$10.").replace(/(\.\d+?)0+(?=e|$)/, "$1").replace(/\.(?=e|$)/, "");
  }
  var print_number_default = printNumber;

  // src/language-js/print/literal.js
  function printLiteral(path, options2) {
    const { node } = path;
    switch (node.type) {
      case "RegExpLiteral":
        return printRegex(node);
      case "BigIntLiteral":
        return printBigInt(node.extra.raw);
      case "NumericLiteral":
        return print_number_default(node.extra.raw);
      case "StringLiteral":
        return replaceEndOfLine(print_string_default(node.extra.raw, options2));
      case "NullLiteral":
        return "null";
      case "BooleanLiteral":
        return String(node.value);
      case "DecimalLiteral":
        return print_number_default(node.value) + "m";
      case "DirectiveLiteral":
        return printDirective(node.extra.raw, options2);
      case "Literal": {
        if (node.regex) {
          return printRegex(node.regex);
        }
        if (node.bigint) {
          return printBigInt(node.raw);
        }
        if (node.decimal) {
          return print_number_default(node.decimal) + "m";
        }
        const { value } = node;
        if (typeof value === "number") {
          return print_number_default(node.raw);
        }
        if (typeof value === "string") {
          return isDirective(path) ? printDirective(node.raw, options2) : replaceEndOfLine(print_string_default(node.raw, options2));
        }
        return String(value);
      }
    }
  }
  function isDirective(path) {
    if (path.key !== "expression") {
      return;
    }
    const { parent } = path;
    return parent.type === "ExpressionStatement" && parent.directive;
  }
  function printBigInt(raw) {
    return raw.toLowerCase();
  }
  function printRegex({ pattern, flags }) {
    flags = [...flags].sort().join("");
    return `/${pattern}/${flags}`;
  }
  function printDirective(rawText2, options2) {
    const rawContent = rawText2.slice(1, -1);
    if (rawContent.includes('"') || rawContent.includes("'")) {
      return rawText2;
    }
    const enclosingQuote = options2.singleQuote ? "'" : '"';
    return enclosingQuote + rawContent + enclosingQuote;
  }
  var isLiteral = create_type_check_function_default([
    "Literal",
    // Babel, flow uses `BigIntLiteral` too
    "BigIntLiteral",
    "BooleanLiteral",
    "DecimalLiteral",
    "DirectiveLiteral",
    "NullLiteral",
    "NumericLiteral",
    "RegExpLiteral",
    "StringLiteral"
  ]);

  // src/language-js/print/assignment.js
  function printAssignment(path, options2, print3, leftDoc, operator, rightPropertyName) {
    const layout = chooseLayout(path, options2, print3, leftDoc, rightPropertyName);
    const rightDoc = rightPropertyName ? print3(rightPropertyName, { assignmentLayout: layout }) : "";
    switch (layout) {
      case "break-after-operator":
        return group([group(leftDoc), operator, group(indent([line, rightDoc]))]);
      case "never-break-after-operator":
        return group([group(leftDoc), operator, " ", rightDoc]);
      case "fluid": {
        const groupId = Symbol("assignment");
        return group([
          group(leftDoc),
          operator,
          group(indent(line), { id: groupId }),
          lineSuffixBoundary,
          indentIfBreak(rightDoc, { groupId })
        ]);
      }
      case "break-lhs":
        return group([leftDoc, operator, " ", group(rightDoc)]);
      case "chain":
        return [group(leftDoc), operator, line, rightDoc];
      case "chain-tail":
        return [group(leftDoc), operator, indent([line, rightDoc])];
      case "chain-tail-arrow-chain":
        return [group(leftDoc), operator, rightDoc];
      case "only-left":
        return leftDoc;
    }
  }
  function printAssignmentExpression(path, options2, print3) {
    const { node } = path;
    return printAssignment(
      path,
      options2,
      print3,
      print3("left"),
      [" ", node.operator],
      "right"
    );
  }
  function printVariableDeclarator(path, options2, print3) {
    return printAssignment(path, options2, print3, print3("id"), " =", "init");
  }
  function chooseLayout(path, options2, print3, leftDoc, rightPropertyName) {
    const { node } = path;
    const rightNode = node[rightPropertyName];
    if (!rightNode) {
      return "only-left";
    }
    const isTail = !isAssignment(rightNode);
    const shouldUseChainFormatting = path.match(
      isAssignment,
      isAssignmentOrVariableDeclarator,
      (node2) => !isTail || node2.type !== "ExpressionStatement" && node2.type !== "VariableDeclaration"
    );
    if (shouldUseChainFormatting) {
      return !isTail ? "chain" : rightNode.type === "ArrowFunctionExpression" && rightNode.body.type === "ArrowFunctionExpression" ? "chain-tail-arrow-chain" : "chain-tail";
    }
    const isHeadOfLongChain = !isTail && isAssignment(rightNode.right);
    if (isHeadOfLongChain || hasLeadingOwnLineComment(options2.originalText, rightNode)) {
      return "break-after-operator";
    }
    if (rightNode.type === "CallExpression" && rightNode.callee.name === "require" || // do not put values on a separate line from the key in json
    options2.parser === "json5" || options2.parser === "json") {
      return "never-break-after-operator";
    }
    if (isComplexDestructuring(node) || isComplexTypeAliasParams(node) || hasComplexTypeAnnotation(node) || isArrowFunctionVariableDeclarator(node) && canBreak(leftDoc)) {
      return "break-lhs";
    }
    const hasShortKey = isObjectPropertyWithShortKey(node, leftDoc, options2);
    if (path.call(
      () => shouldBreakAfterOperator(path, options2, print3, hasShortKey),
      rightPropertyName
    )) {
      return "break-after-operator";
    }
    if (hasShortKey || rightNode.type === "TemplateLiteral" || rightNode.type === "TaggedTemplateExpression" || rightNode.type === "BooleanLiteral" || isNumericLiteral(rightNode) || rightNode.type === "ClassExpression") {
      return "never-break-after-operator";
    }
    return "fluid";
  }
  function shouldBreakAfterOperator(path, options2, print3, hasShortKey) {
    const rightNode = path.node;
    if (isBinaryish(rightNode) && !shouldInlineLogicalExpression(rightNode)) {
      return true;
    }
    switch (rightNode.type) {
      case "StringLiteralTypeAnnotation":
      case "SequenceExpression":
        return true;
      case "ConditionalExpression": {
        const { test } = rightNode;
        return isBinaryish(test) && !shouldInlineLogicalExpression(test);
      }
      case "ClassExpression":
        return is_non_empty_array_default(rightNode.decorators);
    }
    if (hasShortKey) {
      return false;
    }
    let node = rightNode;
    const propertiesForPath = [];
    for (; ; ) {
      if (node.type === "UnaryExpression" || node.type === "AwaitExpression" || node.type === "YieldExpression" && node.argument !== null) {
        node = node.argument;
        propertiesForPath.push("argument");
      } else if (node.type === "TSNonNullExpression") {
        node = node.expression;
        propertiesForPath.push("expression");
      } else {
        break;
      }
    }
    if (isStringLiteral(node) || path.call(
      () => isPoorlyBreakableMemberOrCallChain(path, options2, print3),
      ...propertiesForPath
    )) {
      return true;
    }
    return false;
  }
  function isComplexDestructuring(node) {
    if (isAssignmentOrVariableDeclarator(node)) {
      const leftNode = node.left || node.id;
      return leftNode.type === "ObjectPattern" && leftNode.properties.length > 2 && leftNode.properties.some(
        (property) => {
          var _a;
          return isObjectProperty(property) && (!property.shorthand || ((_a = property.value) == null ? void 0 : _a.type) === "AssignmentPattern");
        }
      );
    }
    return false;
  }
  function isAssignment(node) {
    return node.type === "AssignmentExpression";
  }
  function isAssignmentOrVariableDeclarator(node) {
    return isAssignment(node) || node.type === "VariableDeclarator";
  }
  function isComplexTypeAliasParams(node) {
    const typeParams = getTypeParametersFromTypeAlias(node);
    if (is_non_empty_array_default(typeParams)) {
      const constraintPropertyName = node.type === "TSTypeAliasDeclaration" ? "constraint" : "bound";
      if (typeParams.length > 1 && typeParams.some((param) => param[constraintPropertyName] || param.default)) {
        return true;
      }
    }
    return false;
  }
  var isTypeAlias = create_type_check_function_default([
    "TSTypeAliasDeclaration",
    "TypeAlias"
  ]);
  function getTypeParametersFromTypeAlias(node) {
    var _a;
    if (isTypeAlias(node)) {
      return (_a = node.typeParameters) == null ? void 0 : _a.params;
    }
  }
  function hasComplexTypeAnnotation(node) {
    if (node.type !== "VariableDeclarator") {
      return false;
    }
    const { typeAnnotation } = node.id;
    if (!typeAnnotation || !typeAnnotation.typeAnnotation) {
      return false;
    }
    const typeParams = getTypeParametersFromTypeReference(
      typeAnnotation.typeAnnotation
    );
    return is_non_empty_array_default(typeParams) && typeParams.length > 1 && typeParams.some(
      (param) => is_non_empty_array_default(getTypeParametersFromTypeReference(param)) || param.type === "TSConditionalType"
    );
  }
  function isArrowFunctionVariableDeclarator(node) {
    var _a;
    return node.type === "VariableDeclarator" && ((_a = node.init) == null ? void 0 : _a.type) === "ArrowFunctionExpression";
  }
  var isTypeReference = create_type_check_function_default([
    "TSTypeReference",
    "GenericTypeAnnotation"
  ]);
  function getTypeParametersFromTypeReference(node) {
    var _a;
    if (isTypeReference(node)) {
      return (_a = node.typeParameters) == null ? void 0 : _a.params;
    }
  }
  function isPoorlyBreakableMemberOrCallChain(path, options2, print3, deep = false) {
    var _a;
    const { node } = path;
    const goDeeper = () => isPoorlyBreakableMemberOrCallChain(path, options2, print3, true);
    if (node.type === "ChainExpression" || node.type === "TSNonNullExpression") {
      return path.call(goDeeper, "expression");
    }
    if (isCallExpression(node)) {
      const doc = printCallExpression(path, options2, print3);
      if ((_a = doc.label) == null ? void 0 : _a.memberChain) {
        return false;
      }
      const args = getCallArguments(node);
      const isPoorlyBreakableCall = args.length === 0 || args.length === 1 && isLoneShortArgument(args[0], options2);
      if (!isPoorlyBreakableCall) {
        return false;
      }
      if (isCallExpressionWithComplexTypeArguments(node, print3)) {
        return false;
      }
      return path.call(goDeeper, "callee");
    }
    if (isMemberExpression(node)) {
      return path.call(goDeeper, "object");
    }
    return deep && (node.type === "Identifier" || node.type === "ThisExpression");
  }
  var LONE_SHORT_ARGUMENT_THRESHOLD_RATE = 0.25;
  function isLoneShortArgument(node, { printWidth }) {
    if (hasComment(node)) {
      return false;
    }
    const threshold = printWidth * LONE_SHORT_ARGUMENT_THRESHOLD_RATE;
    if (node.type === "ThisExpression" || node.type === "Identifier" && node.name.length <= threshold || isSignedNumericLiteral(node) && !hasComment(node.argument)) {
      return true;
    }
    const regexpPattern = node.type === "Literal" && "regex" in node && node.regex.pattern || node.type === "RegExpLiteral" && node.pattern;
    if (regexpPattern) {
      return regexpPattern.length <= threshold;
    }
    if (isStringLiteral(node)) {
      return rawText(node).length <= threshold;
    }
    if (node.type === "TemplateLiteral") {
      return node.expressions.length === 0 && node.quasis[0].value.raw.length <= threshold && !node.quasis[0].value.raw.includes("\n");
    }
    return isLiteral(node);
  }
  function isObjectPropertyWithShortKey(node, keyDoc, options2) {
    if (!isObjectProperty(node)) {
      return false;
    }
    keyDoc = cleanDoc(keyDoc);
    const MIN_OVERLAP_FOR_BREAK = 3;
    return typeof keyDoc === "string" && get_string_width_default(keyDoc) < options2.tabWidth + MIN_OVERLAP_FOR_BREAK;
  }
  function isCallExpressionWithComplexTypeArguments(node, print3) {
    const typeArgs = getTypeArgumentsFromCallExpression(node);
    if (is_non_empty_array_default(typeArgs)) {
      if (typeArgs.length > 1) {
        return true;
      }
      if (typeArgs.length === 1) {
        const firstArg = typeArgs[0];
        if (firstArg.type === "TSUnionType" || firstArg.type === "UnionTypeAnnotation" || firstArg.type === "TSIntersectionType" || firstArg.type === "IntersectionTypeAnnotation" || firstArg.type === "TSTypeLiteral" || firstArg.type === "ObjectTypeAnnotation") {
          return true;
        }
      }
      const typeArgsKeyName = node.typeParameters ? "typeParameters" : "typeArguments";
      if (willBreak(print3(typeArgsKeyName))) {
        return true;
      }
    }
    return false;
  }
  function getTypeArgumentsFromCallExpression(node) {
    var _a;
    return (_a = node.typeParameters ?? node.typeArguments) == null ? void 0 : _a.params;
  }

  // src/language-js/print/function-parameters.js
  function printFunctionParameters(path, print3, options2, expandArg, printTypeParams) {
    const functionNode = path.node;
    const parameters = getFunctionParameters(functionNode);
    const typeParams = printTypeParams ? printFunctionTypeParameters(path, options2, print3) : "";
    if (parameters.length === 0) {
      return [
        typeParams,
        "(",
        printDanglingComments(path, options2, {
          filter: (comment) => get_next_non_space_non_comment_character_default(
            options2.originalText,
            locEnd(comment)
          ) === ")"
        }),
        ")"
      ];
    }
    const { parent } = path;
    const isParametersInTestCall = isTestCall(parent);
    const shouldHugParameters = shouldHugTheOnlyFunctionParameter(functionNode);
    const printed = [];
    iterateFunctionParametersPath(path, (parameterPath, index) => {
      const isLastParameter = index === parameters.length - 1;
      if (isLastParameter && functionNode.rest) {
        printed.push("...");
      }
      printed.push(print3());
      if (isLastParameter) {
        return;
      }
      printed.push(",");
      if (isParametersInTestCall || shouldHugParameters) {
        printed.push(" ");
      } else if (isNextLineEmpty2(parameters[index], options2)) {
        printed.push(hardline, hardline);
      } else {
        printed.push(line);
      }
    });
    if (expandArg && !isDecoratedFunction(path)) {
      if (willBreak(typeParams) || willBreak(printed)) {
        throw new ArgExpansionBailout();
      }
      return group([removeLines(typeParams), "(", removeLines(printed), ")"]);
    }
    const hasNotParameterDecorator = parameters.every(
      (node) => !is_non_empty_array_default(node.decorators)
    );
    if (shouldHugParameters && hasNotParameterDecorator) {
      return [typeParams, "(", ...printed, ")"];
    }
    if (isParametersInTestCall) {
      return [typeParams, "(", ...printed, ")"];
    }
    const isFlowShorthandWithOneArg = (isObjectTypePropertyAFunction(parent) || isTypeAnnotationAFunction(parent) || parent.type === "TypeAlias" || parent.type === "UnionTypeAnnotation" || parent.type === "TSUnionType" || parent.type === "IntersectionTypeAnnotation" || parent.type === "FunctionTypeAnnotation" && parent.returnType === functionNode) && parameters.length === 1 && parameters[0].name === null && // `type q = (this: string) => void;`
    functionNode.this !== parameters[0] && parameters[0].typeAnnotation && functionNode.typeParameters === null && isSimpleType(parameters[0].typeAnnotation) && !functionNode.rest;
    if (isFlowShorthandWithOneArg) {
      if (options2.arrowParens === "always") {
        return ["(", ...printed, ")"];
      }
      return printed;
    }
    return [
      typeParams,
      "(",
      indent([softline, ...printed]),
      ifBreak(
        !hasRestParameter(functionNode) && shouldPrintComma(options2, "all") ? "," : ""
      ),
      softline,
      ")"
    ];
  }
  function shouldHugTheOnlyFunctionParameter(node) {
    if (!node) {
      return false;
    }
    const parameters = getFunctionParameters(node);
    if (parameters.length !== 1) {
      return false;
    }
    const [parameter] = parameters;
    return !hasComment(parameter) && (parameter.type === "ObjectPattern" || parameter.type === "ArrayPattern" || parameter.type === "Identifier" && parameter.typeAnnotation && (parameter.typeAnnotation.type === "TypeAnnotation" || parameter.typeAnnotation.type === "TSTypeAnnotation") && isObjectType(parameter.typeAnnotation.typeAnnotation) || parameter.type === "FunctionTypeParam" && isObjectType(parameter.typeAnnotation) && parameter !== node.rest || parameter.type === "AssignmentPattern" && (parameter.left.type === "ObjectPattern" || parameter.left.type === "ArrayPattern") && (parameter.right.type === "Identifier" || isObjectOrRecordExpression(parameter.right) && parameter.right.properties.length === 0 || isArrayOrTupleExpression(parameter.right) && parameter.right.elements.length === 0));
  }
  function getReturnTypeNode(functionNode) {
    let returnTypeNode;
    if (functionNode.returnType) {
      returnTypeNode = functionNode.returnType;
      if (returnTypeNode.typeAnnotation) {
        returnTypeNode = returnTypeNode.typeAnnotation;
      }
    } else if (functionNode.typeAnnotation) {
      returnTypeNode = functionNode.typeAnnotation;
    }
    return returnTypeNode;
  }
  function shouldGroupFunctionParameters(functionNode, returnTypeDoc) {
    var _a;
    const returnTypeNode = getReturnTypeNode(functionNode);
    if (!returnTypeNode) {
      return false;
    }
    const typeParameters = (_a = functionNode.typeParameters) == null ? void 0 : _a.params;
    if (typeParameters) {
      if (typeParameters.length > 1) {
        return false;
      }
      if (typeParameters.length === 1) {
        const typeParameter = typeParameters[0];
        if (typeParameter.constraint || typeParameter.default) {
          return false;
        }
      }
    }
    return getFunctionParameters(functionNode).length === 1 && (isObjectType(returnTypeNode) || willBreak(returnTypeDoc));
  }
  function isDecoratedFunction(path) {
    return path.match(
      (node) => node.type === "ArrowFunctionExpression" && node.body.type === "BlockStatement",
      (node, name) => {
        if (node.type === "CallExpression" && name === "arguments" && node.arguments.length === 1 && node.callee.type === "CallExpression") {
          const decorator = node.callee.callee;
          return decorator.type === "Identifier" || decorator.type === "MemberExpression" && !decorator.computed && decorator.object.type === "Identifier" && decorator.property.type === "Identifier";
        }
        return false;
      },
      (node, name) => node.type === "VariableDeclarator" && name === "init" || node.type === "ExportDefaultDeclaration" && name === "declaration" || node.type === "TSExportAssignment" && name === "expression" || node.type === "AssignmentExpression" && name === "right" && node.left.type === "MemberExpression" && node.left.object.type === "Identifier" && node.left.object.name === "module" && node.left.property.type === "Identifier" && node.left.property.name === "exports",
      (node) => node.type !== "VariableDeclaration" || node.kind === "const" && node.declarations.length === 1
    );
  }
  function shouldBreakFunctionParameters(functionNode) {
    const parameters = getFunctionParameters(functionNode);
    return parameters.length > 1 && parameters.some((parameter) => parameter.type === "TSParameterProperty");
  }

  // src/language-js/print/type-annotation.js
  function shouldHugType(node) {
    if (isSimpleType(node) || isObjectType(node)) {
      return true;
    }
    if (node.type === "UnionTypeAnnotation" || node.type === "TSUnionType") {
      const voidCount = node.types.filter(
        (node2) => node2.type === "VoidTypeAnnotation" || node2.type === "TSVoidKeyword" || node2.type === "NullLiteralTypeAnnotation" || node2.type === "TSNullKeyword"
      ).length;
      const hasObject = node.types.some(
        (node2) => node2.type === "ObjectTypeAnnotation" || node2.type === "TSTypeLiteral" || // This is a bit aggressive but captures Array<{x}>
        node2.type === "GenericTypeAnnotation" || node2.type === "TSTypeReference"
      );
      const hasComments = node.types.some((node2) => hasComment(node2));
      if (node.types.length - 1 === voidCount && hasObject && !hasComments) {
        return true;
      }
    }
    return false;
  }
  function printOpaqueType(path, options2, print3) {
    const semi = options2.semi ? ";" : "";
    const { node } = path;
    const parts = [
      printDeclareToken(path),
      "opaque type ",
      print3("id"),
      print3("typeParameters")
    ];
    if (node.supertype) {
      parts.push(": ", print3("supertype"));
    }
    if (node.impltype) {
      parts.push(" = ", print3("impltype"));
    }
    parts.push(semi);
    return parts;
  }
  function printTypeAlias(path, options2, print3) {
    const semi = options2.semi ? ";" : "";
    const { node } = path;
    const parts = [printDeclareToken(path)];
    parts.push("type ", print3("id"), print3("typeParameters"));
    const rightPropertyName = node.type === "TSTypeAliasDeclaration" ? "typeAnnotation" : "right";
    return [
      printAssignment(path, options2, print3, parts, " =", rightPropertyName),
      semi
    ];
  }
  function printIntersectionType(path, options2, print3) {
    let wasIndented = false;
    return group(
      path.map(({ isFirst, previous, node, index }) => {
        const doc = print3();
        if (isFirst) {
          return doc;
        }
        const currentIsObjectType = isObjectType(node);
        const previousIsObjectType = isObjectType(previous);
        if (previousIsObjectType && currentIsObjectType) {
          return [" & ", wasIndented ? indent(doc) : doc];
        }
        if (!previousIsObjectType && !currentIsObjectType) {
          return indent([" &", line, doc]);
        }
        if (index > 1) {
          wasIndented = true;
        }
        return [" & ", index > 1 ? indent(doc) : doc];
      }, "types")
    );
  }
  function printUnionType(path, options2, print3) {
    const { node } = path;
    const { parent } = path;
    const shouldIndent = parent.type !== "TypeParameterInstantiation" && parent.type !== "TSTypeParameterInstantiation" && parent.type !== "GenericTypeAnnotation" && parent.type !== "TSTypeReference" && parent.type !== "TSTypeAssertion" && parent.type !== "TupleTypeAnnotation" && parent.type !== "TSTupleType" && !(parent.type === "FunctionTypeParam" && !parent.name && path.grandparent.this !== parent) && !((parent.type === "TypeAlias" || parent.type === "VariableDeclarator" || parent.type === "TSTypeAliasDeclaration") && hasLeadingOwnLineComment(options2.originalText, node));
    const shouldHug = shouldHugType(node);
    const printed = path.map((typePath) => {
      let printedType = print3();
      if (!shouldHug) {
        printedType = align(2, printedType);
      }
      return printComments(typePath, printedType, options2);
    }, "types");
    if (shouldHug) {
      return join(" | ", printed);
    }
    const shouldAddStartLine = shouldIndent && !hasLeadingOwnLineComment(options2.originalText, node);
    const code = [
      ifBreak([shouldAddStartLine ? line : "", "| "]),
      join([line, "| "], printed)
    ];
    if (needs_parens_default(path, options2)) {
      return group([indent(code), softline]);
    }
    if (parent.type === "TupleTypeAnnotation" || parent.type === "TSTupleType") {
      const elementTypes = parent[
        // TODO: Remove `types` when babel changes AST of `TupleTypeAnnotation`
        parent.type === "TupleTypeAnnotation" && parent.types ? "types" : "elementTypes"
      ];
      if (elementTypes.length > 1) {
        return group([
          indent([ifBreak(["(", softline]), code]),
          softline,
          ifBreak(")")
        ]);
      }
    }
    return group(shouldIndent ? indent(code) : code);
  }
  function isFlowArrowFunctionTypeAnnotation(path) {
    var _a;
    const { node, parent } = path;
    return node.type === "FunctionTypeAnnotation" && (isObjectTypePropertyAFunction(parent) || !((parent.type === "ObjectTypeProperty" || parent.type === "ObjectTypeInternalSlot") && !parent.variance && !parent.optional && hasSameLocStart(parent, node) || parent.type === "ObjectTypeCallProperty" || ((_a = path.getParentNode(2)) == null ? void 0 : _a.type) === "DeclareFunction"));
  }
  function printFunctionType(path, options2, print3) {
    const { node } = path;
    const parts = [
      // `TSConstructorType` only
      printAbstractToken(path)
    ];
    if (node.type === "TSConstructorType" || node.type === "TSConstructSignatureDeclaration") {
      parts.push("new ");
    }
    let parametersDoc = printFunctionParameters(
      path,
      print3,
      options2,
      /* expandArg */
      false,
      /* printTypeParams */
      true
    );
    const returnTypeDoc = [];
    if (node.type === "FunctionTypeAnnotation") {
      returnTypeDoc.push(
        isFlowArrowFunctionTypeAnnotation(path) ? " => " : ": ",
        print3("returnType")
      );
    } else {
      returnTypeDoc.push(
        printTypeAnnotationProperty(
          path,
          print3,
          node.returnType ? "returnType" : "typeAnnotation"
        )
      );
    }
    if (shouldGroupFunctionParameters(node, returnTypeDoc)) {
      parametersDoc = group(parametersDoc);
    }
    parts.push(parametersDoc, returnTypeDoc);
    return group(parts);
  }
  function printIndexedAccessType(path, options2, print3) {
    return [
      print3("objectType"),
      printOptionalToken(path),
      "[",
      print3("indexType"),
      "]"
    ];
  }
  function printInferType(path, options2, print3) {
    return ["infer ", print3("typeParameter")];
  }
  function printJSDocType(path, print3, token) {
    const { node } = path;
    return [
      node.postfix ? "" : token,
      printTypeAnnotationProperty(path, print3),
      node.postfix ? token : ""
    ];
  }
  function printRestType(path, options2, print3) {
    const { node } = path;
    return [
      "...",
      ...node.type === "TupleTypeSpreadElement" && node.label ? [print3("label"), ": "] : [],
      print3("typeAnnotation")
    ];
  }
  function printNamedTupleMember(path, options2, print3) {
    const { node } = path;
    return [
      // `TupleTypeLabeledElement` only
      node.variance ? print3("variance") : "",
      print3("label"),
      node.optional ? "?" : "",
      ": ",
      print3("elementType")
    ];
  }
  var typeAnnotationNodesCheckedLeadingComments = /* @__PURE__ */ new WeakSet();
  function printTypeAnnotationProperty(path, print3, propertyName = "typeAnnotation") {
    const {
      node: { [propertyName]: typeAnnotation }
    } = path;
    if (!typeAnnotation) {
      return "";
    }
    let shouldPrintLeadingSpace = false;
    if (typeAnnotation.type === "TSTypeAnnotation" || typeAnnotation.type === "TypeAnnotation") {
      const firstToken = path.call(getTypeAnnotationFirstToken, propertyName);
      if (firstToken === "=>" || firstToken === ":" && hasComment(typeAnnotation, CommentCheckFlags.Leading)) {
        shouldPrintLeadingSpace = true;
      }
      typeAnnotationNodesCheckedLeadingComments.add(typeAnnotation);
    }
    return shouldPrintLeadingSpace ? [" ", print3(propertyName)] : print3(propertyName);
  }
  var getTypeAnnotationFirstToken = (path) => {
    if (
      // TypeScript
      path.match(
        (node) => node.type === "TSTypeAnnotation",
        (node, key) => (key === "returnType" || key === "typeAnnotation") && (node.type === "TSFunctionType" || node.type === "TSConstructorType")
      )
    ) {
      return "=>";
    }
    if (
      // TypeScript
      path.match(
        (node) => node.type === "TSTypeAnnotation",
        (node, key) => key === "typeAnnotation" && (node.type === "TSJSDocNullableType" || node.type === "TSJSDocNonNullableType" || node.type === "TSTypePredicate")
      ) || /*
          Flow
      
          ```js
          declare function foo(): void;
                              ^^^^^^^^ `TypeAnnotation`
          ```
          */
      path.match(
        (node) => node.type === "TypeAnnotation",
        (node, key) => key === "typeAnnotation" && node.type === "Identifier",
        (node, key) => key === "id" && node.type === "DeclareFunction"
      ) || /*
          Flow
      
          ```js
          type A = () => infer R extends string;
                                         ^^^^^^ `TypeAnnotation`
          ```
          */
      path.match(
        (node) => node.type === "TypeAnnotation",
        (node, key) => key === "bound" && node.type === "TypeParameter" && node.usesExtendsBound
      )
    ) {
      return "";
    }
    return ":";
  };
  function printTypeAnnotation(path, options2, print3) {
    if (false) {
      const { node } = path;
      if (!typeAnnotationNodesCheckedLeadingComments.has(node)) {
        throw Object.assign(
          new Error(
            `'${node.type}' should be printed by '${printTypeAnnotationProperty.name}' function.`
          ),
          { parentNode: path.parent, propertyName: path.key }
        );
      }
    }
    const token = getTypeAnnotationFirstToken(path);
    return token ? [token, " ", print3("typeAnnotation")] : print3("typeAnnotation");
  }
  function printArrayType(print3) {
    return [print3("elementType"), "[]"];
  }
  function printTypeQuery({ node }, print3) {
    return [
      "typeof ",
      ...node.type === "TSTypeQuery" ? [print3("exprName"), print3("typeParameters")] : [print3("argument")]
    ];
  }
  function printTypePredicate(path, print3) {
    const { node } = path;
    return [
      node.asserts ? "asserts " : "",
      print3("parameterName"),
      node.typeAnnotation ? [" is ", printTypeAnnotationProperty(path, print3)] : ""
    ];
  }

  // src/language-js/print/misc.js
  function printOptionalToken(path) {
    const { node } = path;
    if (!node.optional || // It's an optional computed method parsed by typescript-estree.
    // "?" is printed in `printMethod`.
    node.type === "Identifier" && node === path.parent.key) {
      return "";
    }
    if (isCallExpression(node) || isMemberExpression(node) && node.computed || node.type === "OptionalIndexedAccessType") {
      return "?.";
    }
    return "?";
  }
  function printDefiniteToken(path) {
    return path.node.definite || path.match(
      void 0,
      (node, name) => name === "id" && node.type === "VariableDeclarator" && node.definite
    ) ? "!" : "";
  }
  var flowDeclareNodeTypes = /* @__PURE__ */ new Set([
    "DeclareClass",
    "DeclareComponent",
    "DeclareFunction",
    "DeclareVariable",
    "DeclareExportDeclaration",
    "DeclareExportAllDeclaration",
    "DeclareOpaqueType",
    "DeclareTypeAlias",
    "DeclareEnum",
    "DeclareInterface"
  ]);
  function printDeclareToken(path) {
    const { node } = path;
    return (
      // TypeScript
      node.declare || // Flow
      flowDeclareNodeTypes.has(node.type) && path.parent.type !== "DeclareExportDeclaration" ? "declare " : ""
    );
  }
  var tsAbstractNodeTypes = /* @__PURE__ */ new Set([
    "TSAbstractMethodDefinition",
    "TSAbstractPropertyDefinition",
    "TSAbstractAccessorProperty"
  ]);
  function printAbstractToken({ node }) {
    return node.abstract || tsAbstractNodeTypes.has(node.type) ? "abstract " : "";
  }
  function printFunctionTypeParameters(path, options2, print3) {
    const fun = path.node;
    if (fun.typeArguments) {
      return print3("typeArguments");
    }
    if (fun.typeParameters) {
      return print3("typeParameters");
    }
    return "";
  }
  function printBindExpressionCallee(path, options2, print3) {
    return ["::", print3("callee")];
  }
  function adjustClause(node, clause, forceSpace) {
    if (node.type === "EmptyStatement") {
      return ";";
    }
    if (node.type === "BlockStatement" || forceSpace) {
      return [" ", clause];
    }
    return indent([line, clause]);
  }
  function printRestSpread(path, print3) {
    return ["...", print3("argument"), printTypeAnnotationProperty(path, print3)];
  }
  function printTypeScriptAccessibilityToken(node) {
    return node.accessibility ? node.accessibility + " " : "";
  }

  // src/language-js/print/decorators.js
  function printClassMemberDecorators(path, options2, print3) {
    const { node } = path;
    return group([
      join(line, path.map(print3, "decorators")),
      hasNewlineBetweenOrAfterDecorators(node, options2) ? hardline : line
    ]);
  }
  function printDecoratorsBeforeExport(path, options2, print3) {
    if (!hasDecoratorsBeforeExport(path.node)) {
      return "";
    }
    return [
      join(hardline, path.map(print3, "declaration", "decorators")),
      hardline
    ];
  }
  function printDecorators(path, options2, print3) {
    const { node, parent } = path;
    const { decorators } = node;
    if (!is_non_empty_array_default(decorators) || // If the parent node is an export declaration and the decorator
    // was written before the export, the export will be responsible
    // for printing the decorators.
    hasDecoratorsBeforeExport(parent) || // Decorators already printed in ignored node
    is_ignored_default(path)) {
      return "";
    }
    const shouldBreak = node.type === "ClassExpression" || node.type === "ClassDeclaration" || hasNewlineBetweenOrAfterDecorators(node, options2);
    return [
      path.key === "declaration" && isExportDeclaration(parent) ? hardline : shouldBreak ? breakParent : "",
      join(line, path.map(print3, "decorators")),
      line
    ];
  }
  function hasNewlineBetweenOrAfterDecorators(node, options2) {
    return node.decorators.some(
      (decorator) => has_newline_default(options2.originalText, locEnd(decorator))
    );
  }
  function hasDecoratorsBeforeExport(node) {
    var _a;
    if (node.type !== "ExportDefaultDeclaration" && node.type !== "ExportNamedDeclaration" && node.type !== "DeclareExportDeclaration") {
      return false;
    }
    const decorators = (_a = node.declaration) == null ? void 0 : _a.decorators;
    return is_non_empty_array_default(decorators) && hasSameLocStart(node, decorators[0]);
  }

  // src/language-js/print/module.js
  function printImportDeclaration(path, options2, print3) {
    const { node } = path;
    return [
      "import",
      node.module ? " module" : "",
      printImportKind(node),
      printModuleSpecifiers(path, options2, print3),
      printModuleSource(path, options2, print3),
      printImportAttributes(path, options2, print3),
      options2.semi ? ";" : ""
    ];
  }
  var isDefaultExport = (node) => node.type === "ExportDefaultDeclaration" || node.type === "DeclareExportDeclaration" && node.default;
  function printExportDeclaration(path, options2, print3) {
    const { node } = path;
    const parts = [
      printDecoratorsBeforeExport(path, options2, print3),
      printDeclareToken(path),
      "export",
      isDefaultExport(node) ? " default" : ""
    ];
    const { declaration, exported } = node;
    if (hasComment(node, CommentCheckFlags.Dangling)) {
      parts.push(" ", printDanglingComments(path, options2));
      if (needsHardlineAfterDanglingComment(node)) {
        parts.push(hardline);
      }
    }
    if (declaration) {
      parts.push(" ", print3("declaration"));
    } else {
      parts.push(printExportKind(node));
      if (node.type === "ExportAllDeclaration" || node.type === "DeclareExportAllDeclaration") {
        parts.push(" *");
        if (exported) {
          parts.push(" as ", print3("exported"));
        }
      } else {
        parts.push(printModuleSpecifiers(path, options2, print3));
      }
      parts.push(
        printModuleSource(path, options2, print3),
        printImportAttributes(path, options2, print3)
      );
    }
    parts.push(printSemicolonAfterExportDeclaration(node, options2));
    return parts;
  }
  var shouldOmitSemicolon = create_type_check_function_default([
    "ComponentDeclaration",
    "ClassDeclaration",
    "FunctionDeclaration",
    "TSInterfaceDeclaration",
    "DeclareComponent",
    "DeclareClass",
    "DeclareFunction",
    "TSDeclareFunction",
    "EnumDeclaration"
  ]);
  function printSemicolonAfterExportDeclaration(node, options2) {
    if (options2.semi && (!node.declaration || isDefaultExport(node) && !shouldOmitSemicolon(node.declaration))) {
      return ";";
    }
    return "";
  }
  function printImportOrExportKind(kind, spaceBeforeKind = true) {
    return kind && kind !== "value" ? `${spaceBeforeKind ? " " : ""}${kind}${spaceBeforeKind ? "" : " "}` : "";
  }
  function printImportKind(node, spaceBeforeKind) {
    return printImportOrExportKind(node.importKind, spaceBeforeKind);
  }
  function printExportKind(node) {
    return printImportOrExportKind(node.exportKind);
  }
  function printModuleSource(path, options2, print3) {
    const { node } = path;
    if (!node.source) {
      return "";
    }
    const parts = [];
    if (!shouldNotPrintSpecifiers(node, options2)) {
      parts.push(" from");
    }
    parts.push(" ", print3("source"));
    return parts;
  }
  function printModuleSpecifiers(path, options2, print3) {
    const { node } = path;
    if (shouldNotPrintSpecifiers(node, options2)) {
      return "";
    }
    const parts = [" "];
    if (is_non_empty_array_default(node.specifiers)) {
      const standaloneSpecifiers = [];
      const groupedSpecifiers = [];
      path.each(() => {
        const specifierType = path.node.type;
        if (specifierType === "ExportNamespaceSpecifier" || specifierType === "ExportDefaultSpecifier" || specifierType === "ImportNamespaceSpecifier" || specifierType === "ImportDefaultSpecifier") {
          standaloneSpecifiers.push(print3());
        } else if (specifierType === "ExportSpecifier" || specifierType === "ImportSpecifier") {
          groupedSpecifiers.push(print3());
        } else {
          throw new unexpected_node_error_default(node, "specifier");
        }
      }, "specifiers");
      parts.push(join(", ", standaloneSpecifiers));
      if (groupedSpecifiers.length > 0) {
        if (standaloneSpecifiers.length > 0) {
          parts.push(", ");
        }
        const canBreak2 = groupedSpecifiers.length > 1 || standaloneSpecifiers.length > 0 || node.specifiers.some((node2) => hasComment(node2));
        if (canBreak2) {
          parts.push(
            group([
              "{",
              indent([
                options2.bracketSpacing ? line : softline,
                join([",", line], groupedSpecifiers)
              ]),
              ifBreak(shouldPrintComma(options2) ? "," : ""),
              options2.bracketSpacing ? line : softline,
              "}"
            ])
          );
        } else {
          parts.push([
            "{",
            options2.bracketSpacing ? " " : "",
            ...groupedSpecifiers,
            options2.bracketSpacing ? " " : "",
            "}"
          ]);
        }
      }
    } else {
      parts.push("{}");
    }
    return parts;
  }
  function shouldNotPrintSpecifiers(node, options2) {
    const { type, importKind, source, specifiers } = node;
    if (type !== "ImportDeclaration" || is_non_empty_array_default(specifiers) || importKind === "type") {
      return false;
    }
    return !/{\s*}/.test(
      options2.originalText.slice(locStart(node), locStart(source))
    );
  }
  function printImportAttributes(path, options2, print3) {
    var _a;
    const { node } = path;
    const property = is_non_empty_array_default(node.attributes) ? "attributes" : is_non_empty_array_default(node.assertions) ? "assertions" : void 0;
    if (!property) {
      return "";
    }
    const keyword = property === "assertions" || ((_a = node.extra) == null ? void 0 : _a.deprecatedAssertSyntax) ? "assert" : "with";
    return [
      ` ${keyword} {`,
      options2.bracketSpacing ? " " : "",
      join(", ", path.map(print3, property)),
      options2.bracketSpacing ? " " : "",
      "}"
    ];
  }
  function printModuleSpecifier(path, options2, print3) {
    const { node } = path;
    const { type } = node;
    const isImportSpecifier = type.startsWith("Import");
    const leftSideProperty = isImportSpecifier ? "imported" : "local";
    const rightSideProperty = isImportSpecifier ? "local" : "exported";
    const leftSideNode = node[leftSideProperty];
    const rightSideNode = node[rightSideProperty];
    let left = "";
    let right = "";
    if (type === "ExportNamespaceSpecifier" || type === "ImportNamespaceSpecifier") {
      left = "*";
    } else if (leftSideNode) {
      left = print3(leftSideProperty);
    }
    if (rightSideNode && !isShorthandSpecifier(node)) {
      right = print3(rightSideProperty);
    }
    return [
      printImportOrExportKind(
        type === "ImportSpecifier" ? node.importKind : node.exportKind,
        /* spaceBeforeKind */
        false
      ),
      left,
      left && right ? " as " : "",
      right
    ];
  }
  function isShorthandSpecifier(specifier) {
    if (specifier.type !== "ImportSpecifier" && specifier.type !== "ExportSpecifier") {
      return false;
    }
    const {
      local,
      [specifier.type === "ImportSpecifier" ? "imported" : "exported"]: importedOrExported
    } = specifier;
    if (local.type !== importedOrExported.type || !hasSameLoc(local, importedOrExported)) {
      return false;
    }
    if (isStringLiteral(local)) {
      return local.value === importedOrExported.value && rawText(local) === rawText(importedOrExported);
    }
    switch (local.type) {
      case "Identifier":
        return local.name === importedOrExported.name;
      default:
        return false;
    }
  }

  // src/language-js/print/ternary.js
  function conditionalExpressionChainContainsJsx(node) {
    const conditionalExpressions = [node];
    for (let index = 0; index < conditionalExpressions.length; index++) {
      const conditionalExpression = conditionalExpressions[index];
      for (const property of ["test", "consequent", "alternate"]) {
        const node2 = conditionalExpression[property];
        if (isJsxElement(node2)) {
          return true;
        }
        if (node2.type === "ConditionalExpression") {
          conditionalExpressions.push(node2);
        }
      }
    }
    return false;
  }
  function printTernaryTest(path, options2, print3) {
    const { node } = path;
    const isConditionalExpression = node.type === "ConditionalExpression";
    const alternateNodePropertyName = isConditionalExpression ? "alternate" : "falseType";
    const { parent } = path;
    const printed = isConditionalExpression ? print3("test") : [print3("checkType"), " ", "extends", " ", print3("extendsType")];
    if (parent.type === node.type && parent[alternateNodePropertyName] === node) {
      return align(2, printed);
    }
    return printed;
  }
  var ancestorNameMap = /* @__PURE__ */ new Map([
    ["AssignmentExpression", "right"],
    ["VariableDeclarator", "init"],
    ["ReturnStatement", "argument"],
    ["ThrowStatement", "argument"],
    ["UnaryExpression", "argument"],
    ["YieldExpression", "argument"]
  ]);
  function shouldExtraIndentForConditionalExpression(path) {
    const { node } = path;
    if (node.type !== "ConditionalExpression") {
      return false;
    }
    let parent;
    let child = node;
    for (let ancestorCount = 0; !parent; ancestorCount++) {
      const node2 = path.getParentNode(ancestorCount);
      if (node2.type === "ChainExpression" && node2.expression === child || isCallExpression(node2) && node2.callee === child || isMemberExpression(node2) && node2.object === child || node2.type === "TSNonNullExpression" && node2.expression === child) {
        child = node2;
        continue;
      }
      if (node2.type === "NewExpression" && node2.callee === child || isTSTypeExpression(node2) && node2.expression === child) {
        parent = path.getParentNode(ancestorCount + 1);
        child = node2;
      } else {
        parent = node2;
      }
    }
    if (child === node) {
      return false;
    }
    return parent[ancestorNameMap.get(parent.type)] === child;
  }
  function printTernary(path, options2, print3) {
    const { node } = path;
    const isConditionalExpression = node.type === "ConditionalExpression";
    const consequentNodePropertyName = isConditionalExpression ? "consequent" : "trueType";
    const alternateNodePropertyName = isConditionalExpression ? "alternate" : "falseType";
    const testNodePropertyNames = isConditionalExpression ? ["test"] : ["checkType", "extendsType"];
    const consequentNode = node[consequentNodePropertyName];
    const alternateNode = node[alternateNodePropertyName];
    const parts = [];
    let jsxMode = false;
    const { parent } = path;
    const isParentTest = parent.type === node.type && testNodePropertyNames.some((prop) => parent[prop] === node);
    let forceNoIndent = parent.type === node.type && !isParentTest;
    let currentParent;
    let previousParent;
    let i = 0;
    do {
      previousParent = currentParent || node;
      currentParent = path.getParentNode(i);
      i++;
    } while (currentParent && currentParent.type === node.type && testNodePropertyNames.every(
      (prop) => currentParent[prop] !== previousParent
    ));
    const firstNonConditionalParent = currentParent || parent;
    const lastConditionalParent = previousParent;
    if (isConditionalExpression && (isJsxElement(node[testNodePropertyNames[0]]) || isJsxElement(consequentNode) || isJsxElement(alternateNode) || conditionalExpressionChainContainsJsx(lastConditionalParent))) {
      jsxMode = true;
      forceNoIndent = true;
      const wrap = (doc) => [
        ifBreak("("),
        indent([softline, doc]),
        softline,
        ifBreak(")")
      ];
      const isNil = (node2) => node2.type === "NullLiteral" || node2.type === "Literal" && node2.value === null || node2.type === "Identifier" && node2.name === "undefined";
      parts.push(
        " ? ",
        isNil(consequentNode) ? print3(consequentNodePropertyName) : wrap(print3(consequentNodePropertyName)),
        " : ",
        alternateNode.type === node.type || isNil(alternateNode) ? print3(alternateNodePropertyName) : wrap(print3(alternateNodePropertyName))
      );
    } else {
      const part = [
        line,
        "? ",
        consequentNode.type === node.type ? ifBreak("", "(") : "",
        align(2, print3(consequentNodePropertyName)),
        consequentNode.type === node.type ? ifBreak("", ")") : "",
        line,
        ": ",
        alternateNode.type === node.type ? print3(alternateNodePropertyName) : align(2, print3(alternateNodePropertyName))
      ];
      parts.push(
        parent.type !== node.type || parent[alternateNodePropertyName] === node || isParentTest ? part : options2.useTabs ? dedent(indent(part)) : align(Math.max(0, options2.tabWidth - 2), part)
      );
    }
    const shouldBreak = [
      consequentNodePropertyName,
      alternateNodePropertyName,
      ...testNodePropertyNames
    ].some(
      (property) => hasComment(
        node[property],
        (comment) => is_block_comment_default(comment) && has_newline_in_range_default(
          options2.originalText,
          locStart(comment),
          locEnd(comment)
        )
      )
    );
    const maybeGroup = (doc) => parent === firstNonConditionalParent ? group(doc, { shouldBreak }) : shouldBreak ? [doc, breakParent] : doc;
    const breakClosingParen = !jsxMode && (isMemberExpression(parent) || parent.type === "NGPipeExpression" && parent.left === node) && !parent.computed;
    const shouldExtraIndent = shouldExtraIndentForConditionalExpression(path);
    const result = maybeGroup([
      printTernaryTest(path, options2, print3),
      forceNoIndent ? parts : indent(parts),
      isConditionalExpression && breakClosingParen && !shouldExtraIndent ? softline : ""
    ]);
    return isParentTest || shouldExtraIndent ? group([indent([softline, result]), softline]) : result;
  }

  // src/utils/get-alignment-size.js
  function getAlignmentSize(text, tabWidth, startIndex = 0) {
    let size = 0;
    for (let i = startIndex; i < text.length; ++i) {
      if (text[i] === "	") {
        size = size + tabWidth - size % tabWidth;
      } else {
        size++;
      }
    }
    return size;
  }
  var get_alignment_size_default = getAlignmentSize;

  // src/utils/get-indent-size.js
  function getIndentSize(value, tabWidth) {
    const lastNewlineIndex = value.lastIndexOf("\n");
    if (lastNewlineIndex === -1) {
      return 0;
    }
    return get_alignment_size_default(
      // All the leading whitespaces
      value.slice(lastNewlineIndex + 1).match(/^[\t ]*/)[0],
      tabWidth
    );
  }
  var get_indent_size_default = getIndentSize;

  // src/common/end-of-line.js
  function convertEndOfLineToChars(value) {
    switch (value) {
      case "cr":
        return "\r";
      case "crlf":
        return "\r\n";
      default:
        return "\n";
    }
  }

  // src/document/printer.js
  var MODE_BREAK = Symbol("MODE_BREAK");
  var MODE_FLAT = Symbol("MODE_FLAT");
  var CURSOR_PLACEHOLDER = Symbol("cursor");
  function rootIndent() {
    return {
      value: "",
      length: 0,
      queue: []
    };
  }
  function makeIndent(ind, options2) {
    return generateInd(ind, {
      type: "indent"
    }, options2);
  }
  function makeAlign(indent2, widthOrDoc, options2) {
    if (widthOrDoc === Number.NEGATIVE_INFINITY) {
      return indent2.root || rootIndent();
    }
    if (widthOrDoc < 0) {
      return generateInd(indent2, {
        type: "dedent"
      }, options2);
    }
    if (!widthOrDoc) {
      return indent2;
    }
    if (widthOrDoc.type === "root") {
      return {
        ...indent2,
        root: indent2
      };
    }
    const alignType = typeof widthOrDoc === "string" ? "stringAlign" : "numberAlign";
    return generateInd(indent2, {
      type: alignType,
      n: widthOrDoc
    }, options2);
  }
  function generateInd(ind, newPart, options2) {
    const queue = newPart.type === "dedent" ? ind.queue.slice(0, -1) : [...ind.queue, newPart];
    let value = "";
    let length = 0;
    let lastTabs = 0;
    let lastSpaces = 0;
    for (const part of queue) {
      switch (part.type) {
        case "indent":
          flush();
          if (options2.useTabs) {
            addTabs(1);
          } else {
            addSpaces(options2.tabWidth);
          }
          break;
        case "stringAlign":
          flush();
          value += part.n;
          length += part.n.length;
          break;
        case "numberAlign":
          lastTabs += 1;
          lastSpaces += part.n;
          break;
        default:
          throw new Error(`Unexpected type '${part.type}'`);
      }
    }
    flushSpaces();
    return {
      ...ind,
      value,
      length,
      queue
    };
    function addTabs(count) {
      value += "	".repeat(count);
      length += options2.tabWidth * count;
    }
    function addSpaces(count) {
      value += " ".repeat(count);
      length += count;
    }
    function flush() {
      if (options2.useTabs) {
        flushTabs();
      } else {
        flushSpaces();
      }
    }
    function flushTabs() {
      if (lastTabs > 0) {
        addTabs(lastTabs);
      }
      resetLast();
    }
    function flushSpaces() {
      if (lastSpaces > 0) {
        addSpaces(lastSpaces);
      }
      resetLast();
    }
    function resetLast() {
      lastTabs = 0;
      lastSpaces = 0;
    }
  }
  function trim(out) {
    let trimCount = 0;
    let cursorCount = 0;
    let outIndex = out.length;
    outer:
      while (outIndex--) {
        const last = out[outIndex];
        if (last === CURSOR_PLACEHOLDER) {
          cursorCount++;
          continue;
        }
        if (false) {
          throw new Error(`Unexpected value in trim: '${typeof last}'`);
        }
        for (let charIndex = last.length - 1; charIndex >= 0; charIndex--) {
          const char = last[charIndex];
          if (char === " " || char === "	") {
            trimCount++;
          } else {
            out[outIndex] = last.slice(0, charIndex + 1);
            break outer;
          }
        }
      }
    if (trimCount > 0 || cursorCount > 0) {
      out.length = outIndex + 1;
      while (cursorCount-- > 0) {
        out.push(CURSOR_PLACEHOLDER);
      }
    }
    return trimCount;
  }
  function fits(next, restCommands, width, hasLineSuffix, groupModeMap, mustBeFlat) {
    if (width === Number.POSITIVE_INFINITY) {
      return true;
    }
    let restIdx = restCommands.length;
    const cmds = [next];
    const out = [];
    while (width >= 0) {
      if (cmds.length === 0) {
        if (restIdx === 0) {
          return true;
        }
        cmds.push(restCommands[--restIdx]);
        continue;
      }
      const {
        mode,
        doc
      } = cmds.pop();
      switch (get_doc_type_default(doc)) {
        case DOC_TYPE_STRING:
          out.push(doc);
          width -= get_string_width_default(doc);
          break;
        case DOC_TYPE_ARRAY:
        case DOC_TYPE_FILL: {
          const parts = getDocParts(doc);
          for (let i = parts.length - 1; i >= 0; i--) {
            cmds.push({
              mode,
              doc: parts[i]
            });
          }
          break;
        }
        case DOC_TYPE_INDENT:
        case DOC_TYPE_ALIGN:
        case DOC_TYPE_INDENT_IF_BREAK:
        case DOC_TYPE_LABEL:
          cmds.push({
            mode,
            doc: doc.contents
          });
          break;
        case DOC_TYPE_TRIM:
          width += trim(out);
          break;
        case DOC_TYPE_GROUP: {
          if (mustBeFlat && doc.break) {
            return false;
          }
          const groupMode = doc.break ? MODE_BREAK : mode;
          const contents = doc.expandedStates && groupMode === MODE_BREAK ? at_default(
            /* isOptionalObject*/
            false,
            doc.expandedStates,
            -1
          ) : doc.contents;
          cmds.push({
            mode: groupMode,
            doc: contents
          });
          break;
        }
        case DOC_TYPE_IF_BREAK: {
          const groupMode = doc.groupId ? groupModeMap[doc.groupId] || MODE_FLAT : mode;
          const contents = groupMode === MODE_BREAK ? doc.breakContents : doc.flatContents;
          if (contents) {
            cmds.push({
              mode,
              doc: contents
            });
          }
          break;
        }
        case DOC_TYPE_LINE:
          if (mode === MODE_BREAK || doc.hard) {
            return true;
          }
          if (!doc.soft) {
            out.push(" ");
            width--;
          }
          break;
        case DOC_TYPE_LINE_SUFFIX:
          hasLineSuffix = true;
          break;
        case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
          if (hasLineSuffix) {
            return false;
          }
          break;
      }
    }
    return false;
  }
  function printDocToString(doc, options2) {
    const groupModeMap = {};
    const width = options2.printWidth;
    const newLine = convertEndOfLineToChars(options2.endOfLine);
    let pos = 0;
    const cmds = [{
      ind: rootIndent(),
      mode: MODE_BREAK,
      doc
    }];
    const out = [];
    let shouldRemeasure = false;
    const lineSuffix2 = [];
    let printedCursorCount = 0;
    propagateBreaks(doc);
    while (cmds.length > 0) {
      const {
        ind,
        mode,
        doc: doc2
      } = cmds.pop();
      switch (get_doc_type_default(doc2)) {
        case DOC_TYPE_STRING: {
          const formatted = newLine !== "\n" ? string_replace_all_default(
            /* isOptionalObject*/
            false,
            doc2,
            "\n",
            newLine
          ) : doc2;
          out.push(formatted);
          if (cmds.length > 0) {
            pos += get_string_width_default(formatted);
          }
          break;
        }
        case DOC_TYPE_ARRAY:
          for (let i = doc2.length - 1; i >= 0; i--) {
            cmds.push({
              ind,
              mode,
              doc: doc2[i]
            });
          }
          break;
        case DOC_TYPE_CURSOR:
          if (printedCursorCount >= 2) {
            throw new Error("There are too many 'cursor' in doc.");
          }
          out.push(CURSOR_PLACEHOLDER);
          printedCursorCount++;
          break;
        case DOC_TYPE_INDENT:
          cmds.push({
            ind: makeIndent(ind, options2),
            mode,
            doc: doc2.contents
          });
          break;
        case DOC_TYPE_ALIGN:
          cmds.push({
            ind: makeAlign(ind, doc2.n, options2),
            mode,
            doc: doc2.contents
          });
          break;
        case DOC_TYPE_TRIM:
          pos -= trim(out);
          break;
        case DOC_TYPE_GROUP:
          switch (mode) {
            case MODE_FLAT:
              if (!shouldRemeasure) {
                cmds.push({
                  ind,
                  mode: doc2.break ? MODE_BREAK : MODE_FLAT,
                  doc: doc2.contents
                });
                break;
              }
            case MODE_BREAK: {
              shouldRemeasure = false;
              const next = {
                ind,
                mode: MODE_FLAT,
                doc: doc2.contents
              };
              const rem = width - pos;
              const hasLineSuffix = lineSuffix2.length > 0;
              if (!doc2.break && fits(next, cmds, rem, hasLineSuffix, groupModeMap)) {
                cmds.push(next);
              } else {
                if (doc2.expandedStates) {
                  const mostExpanded = at_default(
                    /* isOptionalObject*/
                    false,
                    doc2.expandedStates,
                    -1
                  );
                  if (doc2.break) {
                    cmds.push({
                      ind,
                      mode: MODE_BREAK,
                      doc: mostExpanded
                    });
                    break;
                  } else {
                    for (let i = 1; i < doc2.expandedStates.length + 1; i++) {
                      if (i >= doc2.expandedStates.length) {
                        cmds.push({
                          ind,
                          mode: MODE_BREAK,
                          doc: mostExpanded
                        });
                        break;
                      } else {
                        const state = doc2.expandedStates[i];
                        const cmd = {
                          ind,
                          mode: MODE_FLAT,
                          doc: state
                        };
                        if (fits(cmd, cmds, rem, hasLineSuffix, groupModeMap)) {
                          cmds.push(cmd);
                          break;
                        }
                      }
                    }
                  }
                } else {
                  cmds.push({
                    ind,
                    mode: MODE_BREAK,
                    doc: doc2.contents
                  });
                }
              }
              break;
            }
          }
          if (doc2.id) {
            groupModeMap[doc2.id] = at_default(
              /* isOptionalObject*/
              false,
              cmds,
              -1
            ).mode;
          }
          break;
        case DOC_TYPE_FILL: {
          const rem = width - pos;
          const {
            parts
          } = doc2;
          if (parts.length === 0) {
            break;
          }
          const [content, whitespace] = parts;
          const contentFlatCmd = {
            ind,
            mode: MODE_FLAT,
            doc: content
          };
          const contentBreakCmd = {
            ind,
            mode: MODE_BREAK,
            doc: content
          };
          const contentFits = fits(contentFlatCmd, [], rem, lineSuffix2.length > 0, groupModeMap, true);
          if (parts.length === 1) {
            if (contentFits) {
              cmds.push(contentFlatCmd);
            } else {
              cmds.push(contentBreakCmd);
            }
            break;
          }
          const whitespaceFlatCmd = {
            ind,
            mode: MODE_FLAT,
            doc: whitespace
          };
          const whitespaceBreakCmd = {
            ind,
            mode: MODE_BREAK,
            doc: whitespace
          };
          if (parts.length === 2) {
            if (contentFits) {
              cmds.push(whitespaceFlatCmd, contentFlatCmd);
            } else {
              cmds.push(whitespaceBreakCmd, contentBreakCmd);
            }
            break;
          }
          parts.splice(0, 2);
          const remainingCmd = {
            ind,
            mode,
            doc: fill(parts)
          };
          const secondContent = parts[0];
          const firstAndSecondContentFlatCmd = {
            ind,
            mode: MODE_FLAT,
            doc: [content, whitespace, secondContent]
          };
          const firstAndSecondContentFits = fits(firstAndSecondContentFlatCmd, [], rem, lineSuffix2.length > 0, groupModeMap, true);
          if (firstAndSecondContentFits) {
            cmds.push(remainingCmd, whitespaceFlatCmd, contentFlatCmd);
          } else if (contentFits) {
            cmds.push(remainingCmd, whitespaceBreakCmd, contentFlatCmd);
          } else {
            cmds.push(remainingCmd, whitespaceBreakCmd, contentBreakCmd);
          }
          break;
        }
        case DOC_TYPE_IF_BREAK:
        case DOC_TYPE_INDENT_IF_BREAK: {
          const groupMode = doc2.groupId ? groupModeMap[doc2.groupId] : mode;
          if (groupMode === MODE_BREAK) {
            const breakContents = doc2.type === DOC_TYPE_IF_BREAK ? doc2.breakContents : doc2.negate ? doc2.contents : indent(doc2.contents);
            if (breakContents) {
              cmds.push({
                ind,
                mode,
                doc: breakContents
              });
            }
          }
          if (groupMode === MODE_FLAT) {
            const flatContents = doc2.type === DOC_TYPE_IF_BREAK ? doc2.flatContents : doc2.negate ? indent(doc2.contents) : doc2.contents;
            if (flatContents) {
              cmds.push({
                ind,
                mode,
                doc: flatContents
              });
            }
          }
          break;
        }
        case DOC_TYPE_LINE_SUFFIX:
          lineSuffix2.push({
            ind,
            mode,
            doc: doc2.contents
          });
          break;
        case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
          if (lineSuffix2.length > 0) {
            cmds.push({
              ind,
              mode,
              doc: hardlineWithoutBreakParent
            });
          }
          break;
        case DOC_TYPE_LINE:
          switch (mode) {
            case MODE_FLAT:
              if (!doc2.hard) {
                if (!doc2.soft) {
                  out.push(" ");
                  pos += 1;
                }
                break;
              } else {
                shouldRemeasure = true;
              }
            case MODE_BREAK:
              if (lineSuffix2.length > 0) {
                cmds.push({
                  ind,
                  mode,
                  doc: doc2
                }, ...lineSuffix2.reverse());
                lineSuffix2.length = 0;
                break;
              }
              if (doc2.literal) {
                if (ind.root) {
                  out.push(newLine, ind.root.value);
                  pos = ind.root.length;
                } else {
                  out.push(newLine);
                  pos = 0;
                }
              } else {
                pos -= trim(out);
                out.push(newLine + ind.value);
                pos = ind.length;
              }
              break;
          }
          break;
        case DOC_TYPE_LABEL:
          cmds.push({
            ind,
            mode,
            doc: doc2.contents
          });
          break;
        case DOC_TYPE_BREAK_PARENT:
          break;
        default:
          throw new invalid_doc_error_default(doc2);
      }
      if (cmds.length === 0 && lineSuffix2.length > 0) {
        cmds.push(...lineSuffix2.reverse());
        lineSuffix2.length = 0;
      }
    }
    const cursorPlaceholderIndex = out.indexOf(CURSOR_PLACEHOLDER);
    if (cursorPlaceholderIndex !== -1) {
      const otherCursorPlaceholderIndex = out.indexOf(CURSOR_PLACEHOLDER, cursorPlaceholderIndex + 1);
      const beforeCursor = out.slice(0, cursorPlaceholderIndex).join("");
      const aroundCursor = out.slice(cursorPlaceholderIndex + 1, otherCursorPlaceholderIndex).join("");
      const afterCursor = out.slice(otherCursorPlaceholderIndex + 1).join("");
      return {
        formatted: beforeCursor + aroundCursor + afterCursor,
        cursorNodeStart: beforeCursor.length,
        cursorNodeText: aroundCursor
      };
    }
    return {
      formatted: out.join("")
    };
  }

  // src/language-js/print/template-literal.js
  function printTemplateLiteral(path, print3, options2) {
    const {
      node
    } = path;
    const isTemplateLiteral = node.type === "TemplateLiteral";
    if (isTemplateLiteral && isJestEachTemplateLiteral(path)) {
      const printed = printJestEachTemplateLiteral(path, options2, print3);
      if (printed) {
        return printed;
      }
    }
    let expressionsKey = "expressions";
    if (node.type === "TSTemplateLiteralType") {
      expressionsKey = "types";
    }
    const parts = [];
    let expressionDocs = path.map(print3, expressionsKey);
    const isSimple = isSimpleTemplateLiteral(node);
    if (isSimple) {
      expressionDocs = expressionDocs.map((doc) => printDocToString(doc, {
        ...options2,
        printWidth: Number.POSITIVE_INFINITY
      }).formatted);
    }
    parts.push(lineSuffixBoundary, "`");
    let previousQuasiIndentSize = 0;
    path.each(({
      index,
      node: quasi
    }) => {
      parts.push(print3());
      if (quasi.tail) {
        return;
      }
      const {
        tabWidth
      } = options2;
      const text = quasi.value.raw;
      const indentSize = text.includes("\n") ? get_indent_size_default(text, tabWidth) : previousQuasiIndentSize;
      previousQuasiIndentSize = indentSize;
      let expressionDoc = expressionDocs[index];
      if (!isSimple) {
        const expression = node[expressionsKey][index];
        if (hasComment(expression) || isMemberExpression(expression) || expression.type === "ConditionalExpression" || expression.type === "SequenceExpression" || isTSTypeExpression(expression) || isBinaryish(expression)) {
          expressionDoc = [indent([softline, expressionDoc]), softline];
        }
      }
      const aligned = indentSize === 0 && text.endsWith("\n") ? align(Number.NEGATIVE_INFINITY, expressionDoc) : addAlignmentToDoc(expressionDoc, indentSize, tabWidth);
      parts.push(group(["${", aligned, lineSuffixBoundary, "}"]));
    }, "quasis");
    parts.push("`");
    return parts;
  }
  function printTaggedTemplateLiteral(print3) {
    const quasiDoc = print3("quasi");
    return label(quasiDoc.label && {
      tagged: true,
      ...quasiDoc.label
    }, [print3("tag"), print3("typeParameters"), lineSuffixBoundary, quasiDoc]);
  }
  function printJestEachTemplateLiteral(path, options2, print3) {
    const {
      node
    } = path;
    const headerNames = node.quasis[0].value.raw.trim().split(/\s*\|\s*/);
    if (headerNames.length > 1 || headerNames.some((headerName) => headerName.length > 0)) {
      options2.__inJestEach = true;
      const expressions = path.map(print3, "expressions");
      options2.__inJestEach = false;
      const parts = [];
      const stringifiedExpressions = expressions.map((doc) => "${" + printDocToString(doc, {
        ...options2,
        printWidth: Number.POSITIVE_INFINITY,
        endOfLine: "lf"
      }).formatted + "}");
      const tableBody = [{
        hasLineBreak: false,
        cells: []
      }];
      for (let i = 1; i < node.quasis.length; i++) {
        const row = at_default(
          /* isOptionalObject*/
          false,
          tableBody,
          -1
        );
        const correspondingExpression = stringifiedExpressions[i - 1];
        row.cells.push(correspondingExpression);
        if (correspondingExpression.includes("\n")) {
          row.hasLineBreak = true;
        }
        if (node.quasis[i].value.raw.includes("\n")) {
          tableBody.push({
            hasLineBreak: false,
            cells: []
          });
        }
      }
      const maxColumnCount = Math.max(headerNames.length, ...tableBody.map((row) => row.cells.length));
      const maxColumnWidths = Array.from({
        length: maxColumnCount
      }).fill(0);
      const table = [{
        cells: headerNames
      }, ...tableBody.filter((row) => row.cells.length > 0)];
      for (const {
        cells
      } of table.filter((row) => !row.hasLineBreak)) {
        for (const [index, cell] of cells.entries()) {
          maxColumnWidths[index] = Math.max(maxColumnWidths[index], get_string_width_default(cell));
        }
      }
      parts.push(lineSuffixBoundary, "`", indent([hardline, join(hardline, table.map((row) => join(" | ", row.cells.map((cell, index) => row.hasLineBreak ? cell : cell + " ".repeat(maxColumnWidths[index] - get_string_width_default(cell))))))]), hardline, "`");
      return parts;
    }
  }
  function printTemplateExpression(path, print3) {
    const {
      node
    } = path;
    let printed = print3();
    if (hasComment(node)) {
      printed = group([indent([softline, printed]), softline]);
    }
    return ["${", printed, lineSuffixBoundary, "}"];
  }
  function printTemplateExpressions(path, print3) {
    return path.map((path2) => printTemplateExpression(path2, print3), "expressions");
  }
  function escapeTemplateCharacters(doc, raw) {
    return mapDoc(doc, (currentDoc) => {
      if (typeof currentDoc === "string") {
        return raw ? string_replace_all_default(
          /* isOptionalObject*/
          false,
          currentDoc,
          /(\\*)`/g,
          "$1$1\\`"
        ) : uncookTemplateElementValue(currentDoc);
      }
      return currentDoc;
    });
  }
  function uncookTemplateElementValue(cookedValue) {
    return string_replace_all_default(
      /* isOptionalObject*/
      false,
      cookedValue,
      /([\\`]|\${)/g,
      "\\$1"
    );
  }
  function isJestEachTemplateLiteral({
    node,
    parent
  }) {
    const jestEachTriggerRegex = /^[fx]?(?:describe|it|test)$/;
    return parent.type === "TaggedTemplateExpression" && parent.quasi === node && parent.tag.type === "MemberExpression" && parent.tag.property.type === "Identifier" && parent.tag.property.name === "each" && (parent.tag.object.type === "Identifier" && jestEachTriggerRegex.test(parent.tag.object.name) || parent.tag.object.type === "MemberExpression" && parent.tag.object.property.type === "Identifier" && (parent.tag.object.property.name === "only" || parent.tag.object.property.name === "skip") && parent.tag.object.object.type === "Identifier" && jestEachTriggerRegex.test(parent.tag.object.object.name));
  }

  // src/utils/create-group-id-mapper.js
  function createGroupIdMapper(description) {
    const groupIds = /* @__PURE__ */ new WeakMap();
    return function(node) {
      if (!groupIds.has(node)) {
        groupIds.set(node, Symbol(description));
      }
      return groupIds.get(node);
    };
  }
  var create_group_id_mapper_default = createGroupIdMapper;

  // src/language-js/print/mapped-type.js
  function printFlowMappedTypeOptionalModifier(optional) {
    switch (optional) {
      case null:
        return "";
      case "PlusOptional":
        return "+?";
      case "MinusOptional":
        return "-?";
      case "Optional":
        return "?";
    }
  }
  function printFlowMappedTypeProperty(path, options2, print3) {
    const { node } = path;
    return group([
      node.variance ? print3("variance") : "",
      "[",
      indent([print3("keyTparam"), " in ", print3("sourceType")]),
      "]",
      printFlowMappedTypeOptionalModifier(node.optional),
      ": ",
      print3("propType")
    ]);
  }
  function printTypeScriptMappedTypeModifier(tokenNode, keyword) {
    if (tokenNode === "+" || tokenNode === "-") {
      return tokenNode + keyword;
    }
    return keyword;
  }
  function printTypescriptMappedType(path, options2, print3) {
    const { node } = path;
    const shouldBreak = has_newline_in_range_default(
      options2.originalText,
      locStart(node),
      // Ideally, this should be the next token after `{`, but there is no node starts with it.
      locStart(node.typeParameter)
    );
    return group(
      [
        "{",
        indent([
          options2.bracketSpacing ? line : softline,
          group([
            print3("typeParameter"),
            node.optional ? printTypeScriptMappedTypeModifier(node.optional, "?") : "",
            node.typeAnnotation ? ": " : "",
            print3("typeAnnotation")
          ]),
          options2.semi ? ifBreak(";") : ""
        ]),
        printDanglingComments(path, options2),
        options2.bracketSpacing ? line : softline,
        "}"
      ],
      { shouldBreak }
    );
  }

  // src/language-js/print/type-parameters.js
  var getTypeParametersGroupId = create_group_id_mapper_default("typeParameters");
  function shouldForceTrailingComma(path, options2, paramsKey) {
    const { node } = path;
    return getFunctionParameters(node).length === 1 && node.type.startsWith("TS") && !node[paramsKey][0].constraint && path.parent.type === "ArrowFunctionExpression" && !(options2.filepath && /\.ts$/.test(options2.filepath));
  }
  function printTypeParameters(path, options2, print3, paramsKey) {
    const { node } = path;
    if (!node[paramsKey]) {
      return "";
    }
    if (!Array.isArray(node[paramsKey])) {
      return print3(paramsKey);
    }
    const grandparent = path.getNode(2);
    const isParameterInTestCall = grandparent && isTestCall(grandparent);
    const isArrowFunctionVariable = path.match(
      (node2) => !(node2[paramsKey].length === 1 && isObjectType(node2[paramsKey][0])),
      void 0,
      (node2, name) => name === "typeAnnotation",
      (node2) => node2.type === "Identifier",
      isArrowFunctionVariableDeclarator
    );
    const shouldInline = node[paramsKey].length === 0 || !isArrowFunctionVariable && (isParameterInTestCall || node[paramsKey].length === 1 && (node[paramsKey][0].type === "NullableTypeAnnotation" || shouldHugType(node[paramsKey][0])));
    if (shouldInline) {
      return [
        "<",
        join(", ", path.map(print3, paramsKey)),
        printDanglingCommentsForInline(path, options2),
        ">"
      ];
    }
    const trailingComma = node.type === "TSTypeParameterInstantiation" ? "" : shouldForceTrailingComma(path, options2, paramsKey) ? "," : shouldPrintComma(options2) ? ifBreak(",") : "";
    return group(
      [
        "<",
        indent([softline, join([",", line], path.map(print3, paramsKey))]),
        trailingComma,
        softline,
        ">"
      ],
      { id: getTypeParametersGroupId(node) }
    );
  }
  function printDanglingCommentsForInline(path, options2) {
    const { node } = path;
    if (!hasComment(node, CommentCheckFlags.Dangling)) {
      return "";
    }
    const hasOnlyBlockComments = !hasComment(node, CommentCheckFlags.Line);
    const printed = printDanglingComments(path, options2, {
      indent: !hasOnlyBlockComments
    });
    if (hasOnlyBlockComments) {
      return printed;
    }
    return [printed, hardline];
  }
  function printTypeParameter(path, options2, print3) {
    const { node, parent } = path;
    const parts = [node.type === "TSTypeParameter" && node.const ? "const " : ""];
    const name = node.type === "TSTypeParameter" ? print3("name") : node.name;
    if (parent.type === "TSMappedType") {
      if (parent.readonly) {
        parts.push(
          printTypeScriptMappedTypeModifier(parent.readonly, "readonly"),
          " "
        );
      }
      parts.push("[", name);
      if (node.constraint) {
        parts.push(" in ", print3("constraint"));
      }
      if (parent.nameType) {
        parts.push(
          " as ",
          path.callParent(() => print3("nameType"))
        );
      }
      parts.push("]");
      return parts;
    }
    if (node.variance) {
      parts.push(print3("variance"));
    }
    if (node.in) {
      parts.push("in ");
    }
    if (node.out) {
      parts.push("out ");
    }
    parts.push(name);
    if (node.bound) {
      if (node.usesExtendsBound) {
        parts.push(" extends ");
      }
      parts.push(printTypeAnnotationProperty(path, print3, "bound"));
    }
    if (node.constraint) {
      const groupId = Symbol("constraint");
      parts.push(
        " extends",
        group(indent(line), { id: groupId }),
        lineSuffixBoundary,
        indentIfBreak(print3("constraint"), { groupId })
      );
    }
    if (node.default) {
      parts.push(" = ", print3("default"));
    }
    return group(parts);
  }

  // scripts/build/shims/assert.js
  var assert = new Proxy(() => {
  }, { get: () => assert });
  var assert_default = assert;

  // src/language-js/print/property.js
  var needsQuoteProps = /* @__PURE__ */ new WeakMap();
  function printPropertyKey(path, options2, print3) {
    const { node } = path;
    if (node.computed) {
      return ["[", print3("key"), "]"];
    }
    const { parent } = path;
    const { key } = node;
    if (options2.quoteProps === "consistent" && !needsQuoteProps.has(parent)) {
      const objectHasStringProp = (parent.properties || parent.body || parent.members).some(
        (prop) => !prop.computed && prop.key && isStringLiteral(prop.key) && !isStringPropSafeToUnquote(prop, options2)
      );
      needsQuoteProps.set(parent, objectHasStringProp);
    }
    if ((key.type === "Identifier" || isNumericLiteral(key) && isSimpleNumber(print_number_default(rawText(key))) && // Avoid converting 999999999999999999999 to 1e+21, 0.99999999999999999 to 1 and 1.0 to 1.
    String(key.value) === print_number_default(rawText(key)) && // Quoting number keys is safe in JS and Flow, but not in TypeScript (as
    // mentioned in `isStringPropSafeToUnquote`).
    !(options2.parser === "typescript" || options2.parser === "babel-ts")) && (options2.parser === "json" || options2.quoteProps === "consistent" && needsQuoteProps.get(parent))) {
      const prop = print_string_default(
        JSON.stringify(
          key.type === "Identifier" ? key.name : key.value.toString()
        ),
        options2
      );
      return path.call((keyPath) => printComments(keyPath, prop, options2), "key");
    }
    if (isStringPropSafeToUnquote(node, options2) && (options2.quoteProps === "as-needed" || options2.quoteProps === "consistent" && !needsQuoteProps.get(parent))) {
      return path.call(
        (keyPath) => printComments(
          keyPath,
          /^\d/.test(key.value) ? print_number_default(key.value) : key.value,
          options2
        ),
        "key"
      );
    }
    return print3("key");
  }
  function printProperty(path, options2, print3) {
    const { node } = path;
    if (node.shorthand) {
      return print3("value");
    }
    return printAssignment(
      path,
      options2,
      print3,
      printPropertyKey(path, options2, print3),
      ":",
      "value"
    );
  }

  // src/language-js/print/function.js
  var isMethod = (node) => node.type === "ObjectMethod" || node.type === "ClassMethod" || node.type === "ClassPrivateMethod" || node.type === "MethodDefinition" || node.type === "TSAbstractMethodDefinition" || node.type === "TSDeclareMethod" || (node.type === "Property" || node.type === "ObjectProperty") && (node.method || node.kind === "get" || node.kind === "set");
  var isMethodValue = (path) => path.node.type === "FunctionExpression" && path.key === "value" && isMethod(path.parent);
  function printFunction(path, print3, options2, args) {
    if (isMethodValue(path)) {
      return printMethodValue(path, options2, print3);
    }
    const { node } = path;
    let expandArg = false;
    if ((node.type === "FunctionDeclaration" || node.type === "FunctionExpression") && (args == null ? void 0 : args.expandLastArg)) {
      const { parent } = path;
      if (isCallExpression(parent) && (getCallArguments(parent).length > 1 || getFunctionParameters(node).every(
        (param) => param.type === "Identifier" && !param.typeAnnotation
      ))) {
        expandArg = true;
      }
    }
    const parts = [
      printDeclareToken(path),
      node.async ? "async " : "",
      `function${node.generator ? "*" : ""} `,
      node.id ? print3("id") : ""
    ];
    const parametersDoc = printFunctionParameters(
      path,
      print3,
      options2,
      expandArg
    );
    const returnTypeDoc = printReturnType(path, print3);
    const shouldGroupParameters = shouldGroupFunctionParameters(
      node,
      returnTypeDoc
    );
    parts.push(
      printFunctionTypeParameters(path, options2, print3),
      group([
        shouldGroupParameters ? group(parametersDoc) : parametersDoc,
        returnTypeDoc
      ]),
      node.body ? " " : "",
      print3("body")
    );
    if (options2.semi && (node.declare || !node.body)) {
      parts.push(";");
    }
    return parts;
  }
  function printMethod(path, options2, print3) {
    const { node } = path;
    const { kind } = node;
    const value = node.value || node;
    const parts = [];
    if (!kind || kind === "init" || kind === "method" || kind === "constructor") {
      if (value.async) {
        parts.push("async ");
      }
    } else {
      assert_default.ok(kind === "get" || kind === "set");
      parts.push(kind, " ");
    }
    if (value.generator) {
      parts.push("*");
    }
    parts.push(
      printPropertyKey(path, options2, print3),
      node.optional || node.key.optional ? "?" : "",
      node === value ? printMethodValue(path, options2, print3) : print3("value")
    );
    return parts;
  }
  function printMethodValue(path, options2, print3) {
    const { node } = path;
    const parametersDoc = printFunctionParameters(path, print3, options2);
    const returnTypeDoc = printReturnType(path, print3);
    const shouldBreakParameters = shouldBreakFunctionParameters(node);
    const shouldGroupParameters = shouldGroupFunctionParameters(
      node,
      returnTypeDoc
    );
    const parts = [
      printFunctionTypeParameters(path, options2, print3),
      group([
        shouldBreakParameters ? group(parametersDoc, { shouldBreak: true }) : shouldGroupParameters ? group(parametersDoc) : parametersDoc,
        returnTypeDoc
      ])
    ];
    if (node.body) {
      parts.push(" ", print3("body"));
    } else {
      parts.push(options2.semi ? ";" : "");
    }
    return parts;
  }
  function canPrintParamsWithoutParens(node) {
    const parameters = getFunctionParameters(node);
    return parameters.length === 1 && !node.typeParameters && !hasComment(node, CommentCheckFlags.Dangling) && parameters[0].type === "Identifier" && !parameters[0].typeAnnotation && !hasComment(parameters[0]) && !parameters[0].optional && !node.predicate && !node.returnType;
  }
  function shouldPrintParamsWithoutParens(path, options2) {
    if (options2.arrowParens === "always") {
      return false;
    }
    if (options2.arrowParens === "avoid") {
      const { node } = path;
      return canPrintParamsWithoutParens(node);
    }
    return false;
  }
  function printReturnType(path, print3) {
    const { node } = path;
    const returnType = printTypeAnnotationProperty(path, print3, "returnType");
    const parts = [returnType];
    if (node.predicate) {
      parts.push(print3("predicate"));
    }
    return parts;
  }
  function printReturnOrThrowArgument(path, options2, print3) {
    const { node } = path;
    const semi = options2.semi ? ";" : "";
    const parts = [];
    if (node.argument) {
      let argumentDoc = print3("argument");
      if (returnArgumentHasLeadingComment(options2, node.argument)) {
        argumentDoc = ["(", indent([hardline, argumentDoc]), hardline, ")"];
      } else if (isBinaryish(node.argument) || node.argument.type === "SequenceExpression") {
        argumentDoc = group([
          ifBreak("("),
          indent([softline, argumentDoc]),
          softline,
          ifBreak(")")
        ]);
      }
      parts.push(" ", argumentDoc);
    }
    const hasDanglingComments = hasComment(node, CommentCheckFlags.Dangling);
    const shouldPrintSemiBeforeComments = semi && hasDanglingComments && hasComment(node, CommentCheckFlags.Last | CommentCheckFlags.Line);
    if (shouldPrintSemiBeforeComments) {
      parts.push(semi);
    }
    if (hasDanglingComments) {
      parts.push(" ", printDanglingComments(path, options2));
    }
    if (!shouldPrintSemiBeforeComments) {
      parts.push(semi);
    }
    return parts;
  }
  function printReturnStatement(path, options2, print3) {
    return ["return", printReturnOrThrowArgument(path, options2, print3)];
  }
  function printThrowStatement(path, options2, print3) {
    return ["throw", printReturnOrThrowArgument(path, options2, print3)];
  }
  function returnArgumentHasLeadingComment(options2, argument) {
    if (hasLeadingOwnLineComment(options2.originalText, argument) || hasComment(
      argument,
      CommentCheckFlags.Leading,
      (comment) => has_newline_in_range_default(
        options2.originalText,
        locStart(comment),
        locEnd(comment)
      )
    ) && !isJsxElement(argument)) {
      return true;
    }
    if (hasNakedLeftSide(argument)) {
      let leftMost = argument;
      let newLeftMost;
      while (newLeftMost = getLeftSide(leftMost)) {
        leftMost = newLeftMost;
        if (hasLeadingOwnLineComment(options2.originalText, leftMost)) {
          return true;
        }
      }
    }
    return false;
  }

  // src/language-js/print/class.js
  var isClassProperty = create_type_check_function_default([
    "ClassProperty",
    "PropertyDefinition",
    "ClassPrivateProperty",
    "ClassAccessorProperty",
    "AccessorProperty",
    "TSAbstractPropertyDefinition",
    "TSAbstractAccessorProperty"
  ]);
  function printClass(path, options2, print3) {
    const { node } = path;
    const parts = [printDeclareToken(path), printAbstractToken(path), "class"];
    const groupMode = hasComment(node.id, CommentCheckFlags.Trailing) || hasComment(node.typeParameters, CommentCheckFlags.Trailing) || hasComment(node.superClass) || is_non_empty_array_default(node.extends) || // DeclareClass
    is_non_empty_array_default(node.mixins) || is_non_empty_array_default(node.implements);
    const partsGroup = [];
    const extendsParts = [];
    if (node.id) {
      partsGroup.push(" ", print3("id"));
    }
    partsGroup.push(print3("typeParameters"));
    if (node.superClass) {
      const printed = [
        printSuperClass(path, options2, print3),
        print3("superTypeParameters")
      ];
      const printedWithComments = path.call(
        (superClass) => ["extends ", printComments(superClass, printed, options2)],
        "superClass"
      );
      if (groupMode) {
        extendsParts.push(line, group(printedWithComments));
      } else {
        extendsParts.push(" ", printedWithComments);
      }
    } else {
      extendsParts.push(printHeritageClauses(path, options2, print3, "extends"));
    }
    extendsParts.push(
      printHeritageClauses(path, options2, print3, "mixins"),
      printHeritageClauses(path, options2, print3, "implements")
    );
    if (groupMode) {
      let printedPartsGroup;
      if (shouldIndentOnlyHeritageClauses(node)) {
        printedPartsGroup = [...partsGroup, indent(extendsParts)];
      } else {
        printedPartsGroup = indent([...partsGroup, extendsParts]);
      }
      parts.push(group(printedPartsGroup, { id: getHeritageGroupId(node) }));
    } else {
      parts.push(...partsGroup, ...extendsParts);
    }
    parts.push(" ", print3("body"));
    return parts;
  }
  var getHeritageGroupId = create_group_id_mapper_default("heritageGroup");
  function printHardlineAfterHeritage(node) {
    return ifBreak(hardline, "", { groupId: getHeritageGroupId(node) });
  }
  function hasMultipleHeritage(node) {
    return ["extends", "mixins", "implements"].reduce(
      (count, key) => count + (Array.isArray(node[key]) ? node[key].length : 0),
      node.superClass ? 1 : 0
    ) > 1;
  }
  function shouldIndentOnlyHeritageClauses(node) {
    return node.typeParameters && !hasComment(
      node.typeParameters,
      CommentCheckFlags.Trailing | CommentCheckFlags.Line
    ) && !hasMultipleHeritage(node);
  }
  function printHeritageClauses(path, options2, print3, listName) {
    const { node } = path;
    if (!is_non_empty_array_default(node[listName])) {
      return "";
    }
    const printedLeadingComments = printDanglingComments(path, options2, {
      marker: listName
    });
    return [
      shouldIndentOnlyHeritageClauses(node) ? ifBreak(" ", line, {
        groupId: getTypeParametersGroupId(node.typeParameters)
      }) : line,
      printedLeadingComments,
      printedLeadingComments && hardline,
      listName,
      group(indent([line, join([",", line], path.map(print3, listName))]))
    ];
  }
  function printSuperClass(path, options2, print3) {
    const printed = print3("superClass");
    const { parent } = path;
    if (parent.type === "AssignmentExpression") {
      return group(
        ifBreak(["(", indent([softline, printed]), softline, ")"], printed)
      );
    }
    return printed;
  }
  function printClassMethod(path, options2, print3) {
    const { node } = path;
    const parts = [];
    if (is_non_empty_array_default(node.decorators)) {
      parts.push(printClassMemberDecorators(path, options2, print3));
    }
    parts.push(printTypeScriptAccessibilityToken(node));
    if (node.static) {
      parts.push("static ");
    }
    parts.push(printAbstractToken(path));
    if (node.override) {
      parts.push("override ");
    }
    parts.push(printMethod(path, options2, print3));
    return parts;
  }
  function printClassProperty(path, options2, print3) {
    const { node } = path;
    const parts = [];
    const semi = options2.semi ? ";" : "";
    if (is_non_empty_array_default(node.decorators)) {
      parts.push(printClassMemberDecorators(path, options2, print3));
    }
    parts.push(printTypeScriptAccessibilityToken(node), printDeclareToken(path));
    if (node.static) {
      parts.push("static ");
    }
    parts.push(printAbstractToken(path));
    if (node.override) {
      parts.push("override ");
    }
    if (node.readonly) {
      parts.push("readonly ");
    }
    if (node.variance) {
      parts.push(print3("variance"));
    }
    if (node.type === "ClassAccessorProperty" || node.type === "AccessorProperty" || node.type === "TSAbstractAccessorProperty") {
      parts.push("accessor ");
    }
    parts.push(
      printPropertyKey(path, options2, print3),
      printOptionalToken(path),
      printDefiniteToken(path),
      printTypeAnnotationProperty(path, print3)
    );
    const isAbstractProperty = node.type === "TSAbstractPropertyDefinition" || node.type === "TSAbstractAccessorProperty";
    return [
      printAssignment(
        path,
        options2,
        print3,
        parts,
        " =",
        isAbstractProperty ? void 0 : "value"
      ),
      semi
    ];
  }
  function printClassBody(path, options2, print3) {
    const { node } = path;
    const parts = [];
    path.each(({ node: node2, next, isLast }) => {
      parts.push(print3());
      if (!options2.semi && isClassProperty(node2) && shouldPrintSemicolonAfterClassProperty(node2, next)) {
        parts.push(";");
      }
      if (!isLast) {
        parts.push(hardline);
        if (isNextLineEmpty2(node2, options2)) {
          parts.push(hardline);
        }
      }
    }, "body");
    if (hasComment(node, CommentCheckFlags.Dangling)) {
      parts.push(printDanglingComments(path, options2));
    }
    return [
      is_non_empty_array_default(node.body) ? printHardlineAfterHeritage(path.parent) : "",
      "{",
      parts.length > 0 ? [indent([hardline, parts]), hardline] : "",
      "}"
    ];
  }
  function shouldPrintSemicolonAfterClassProperty(node, nextNode) {
    var _a;
    const { type, name } = node.key;
    if (!node.computed && type === "Identifier" && (name === "static" || name === "get" || name === "set") && !node.value && !node.typeAnnotation) {
      return true;
    }
    if (!nextNode) {
      return false;
    }
    if (nextNode.static || nextNode.accessibility) {
      return false;
    }
    if (!nextNode.computed) {
      const name2 = (_a = nextNode.key) == null ? void 0 : _a.name;
      if (name2 === "in" || name2 === "instanceof") {
        return true;
      }
    }
    if (isClassProperty(nextNode) && nextNode.variance && !nextNode.static && !nextNode.declare) {
      return true;
    }
    switch (nextNode.type) {
      case "ClassProperty":
      case "PropertyDefinition":
      case "TSAbstractPropertyDefinition":
        return nextNode.computed;
      case "MethodDefinition":
      case "TSAbstractMethodDefinition":
      case "ClassMethod":
      case "ClassPrivateMethod": {
        const isAsync = nextNode.value ? nextNode.value.async : nextNode.async;
        if (isAsync || nextNode.kind === "get" || nextNode.kind === "set") {
          return false;
        }
        const isGenerator = nextNode.value ? nextNode.value.generator : nextNode.generator;
        if (nextNode.computed || isGenerator) {
          return true;
        }
        return false;
      }
      case "TSIndexSignature":
        return true;
    }
    return false;
  }

  // src/language-js/print/object.js
  function printObject(path, options2, print3) {
    var _a;
    const semi = options2.semi ? ";" : "";
    const {
      node
    } = path;
    const isTypeAnnotation = node.type === "ObjectTypeAnnotation";
    const isEnumBody = node.type === "TSEnumDeclaration" || node.type === "EnumBooleanBody" || node.type === "EnumNumberBody" || node.type === "EnumStringBody" || node.type === "EnumSymbolBody";
    const fields = [node.type === "TSTypeLiteral" || isEnumBody ? "members" : node.type === "TSInterfaceBody" ? "body" : "properties"];
    if (isTypeAnnotation) {
      fields.push("indexers", "callProperties", "internalSlots");
    }
    const propsAndLoc = fields.flatMap((field) => path.map(({
      node: node2
    }) => ({
      node: node2,
      printed: print3(),
      loc: locStart(node2)
    }), field));
    if (fields.length > 1) {
      propsAndLoc.sort((a, b) => a.loc - b.loc);
    }
    const {
      parent,
      key
    } = path;
    const isFlowInterfaceLikeBody = isTypeAnnotation && key === "body" && (parent.type === "InterfaceDeclaration" || parent.type === "DeclareInterface" || parent.type === "DeclareClass");
    const shouldBreak = node.type === "TSInterfaceBody" || isEnumBody || isFlowInterfaceLikeBody || node.type === "ObjectPattern" && parent.type !== "FunctionDeclaration" && parent.type !== "FunctionExpression" && parent.type !== "ArrowFunctionExpression" && parent.type !== "ObjectMethod" && parent.type !== "ClassMethod" && parent.type !== "ClassPrivateMethod" && parent.type !== "AssignmentPattern" && parent.type !== "CatchClause" && node.properties.some((property) => property.value && (property.value.type === "ObjectPattern" || property.value.type === "ArrayPattern")) || node.type !== "ObjectPattern" && propsAndLoc.length > 0 && has_newline_in_range_default(options2.originalText, locStart(node), propsAndLoc[0].loc);
    const separator = isFlowInterfaceLikeBody ? ";" : node.type === "TSInterfaceBody" || node.type === "TSTypeLiteral" ? ifBreak(semi, ";") : ",";
    const leftBrace = node.type === "RecordExpression" ? "#{" : node.exact ? "{|" : "{";
    const rightBrace = node.exact ? "|}" : "}";
    let separatorParts = [];
    const props = propsAndLoc.map((prop) => {
      const result = [...separatorParts, group(prop.printed)];
      separatorParts = [separator, line];
      if ((prop.node.type === "TSPropertySignature" || prop.node.type === "TSMethodSignature" || prop.node.type === "TSConstructSignatureDeclaration" || prop.node.type === "TSCallSignatureDeclaration") && hasComment(prop.node, CommentCheckFlags.PrettierIgnore)) {
        separatorParts.shift();
      }
      if (isNextLineEmpty2(prop.node, options2)) {
        separatorParts.push(hardline);
      }
      return result;
    });
    if (node.inexact || node.hasUnknownMembers) {
      let printed;
      if (hasComment(node, CommentCheckFlags.Dangling)) {
        const hasLineComments = hasComment(node, CommentCheckFlags.Line);
        const printedDanglingComments = printDanglingComments(path, options2);
        printed = [printedDanglingComments, hasLineComments || has_newline_default(options2.originalText, locEnd(at_default(
          /* isOptionalObject*/
          false,
          getComments(node),
          -1
        ))) ? hardline : line, "..."];
      } else {
        printed = ["..."];
      }
      props.push([...separatorParts, ...printed]);
    }
    const lastElem = (_a = at_default(
      /* isOptionalObject*/
      false,
      propsAndLoc,
      -1
    )) == null ? void 0 : _a.node;
    const canHaveTrailingSeparator = !(node.inexact || node.hasUnknownMembers || lastElem && (lastElem.type === "RestElement" || (lastElem.type === "TSPropertySignature" || lastElem.type === "TSCallSignatureDeclaration" || lastElem.type === "TSMethodSignature" || lastElem.type === "TSConstructSignatureDeclaration") && hasComment(lastElem, CommentCheckFlags.PrettierIgnore)));
    let content;
    if (props.length === 0) {
      if (!hasComment(node, CommentCheckFlags.Dangling)) {
        return [leftBrace, rightBrace, printTypeAnnotationProperty(path, print3)];
      }
      content = group([leftBrace, printDanglingComments(path, options2, {
        indent: true
      }), softline, rightBrace, printOptionalToken(path), printTypeAnnotationProperty(path, print3)]);
    } else {
      content = [isFlowInterfaceLikeBody && is_non_empty_array_default(node.properties) ? printHardlineAfterHeritage(parent) : "", leftBrace, indent([options2.bracketSpacing ? line : softline, ...props]), ifBreak(canHaveTrailingSeparator && (separator !== "," || shouldPrintComma(options2)) ? separator : ""), options2.bracketSpacing ? line : softline, rightBrace, printOptionalToken(path), printTypeAnnotationProperty(path, print3)];
    }
    if (path.match((node2) => node2.type === "ObjectPattern" && !is_non_empty_array_default(node2.decorators), shouldHugTheOnlyParameter) || isObjectType(node) && (path.match(void 0, (node2, name) => name === "typeAnnotation", (node2, name) => name === "typeAnnotation", shouldHugTheOnlyParameter) || path.match(void 0, (node2, name) => node2.type === "FunctionTypeParam" && name === "typeAnnotation", shouldHugTheOnlyParameter)) || // Assignment printing logic (printAssignment) is responsible
    // for adding a group if needed
    !shouldBreak && path.match((node2) => node2.type === "ObjectPattern", (node2) => node2.type === "AssignmentExpression" || node2.type === "VariableDeclarator")) {
      return content;
    }
    return group(content, {
      shouldBreak
    });
  }
  function shouldHugTheOnlyParameter(node, name) {
    return (name === "params" || name === "parameters" || name === "this" || name === "rest") && shouldHugTheOnlyFunctionParameter(node);
  }

  // src/language-js/print/arrow-function.js
  var shouldAddParensIfNotBreakCache = /* @__PURE__ */ new WeakMap();
  function shouldAddParensIfNotBreak(node) {
    if (!shouldAddParensIfNotBreakCache.has(node)) {
      shouldAddParensIfNotBreakCache.set(
        node,
        node.type === "ConditionalExpression" && !startsWithNoLookaheadToken(
          node,
          (node2) => node2.type === "ObjectExpression"
        )
      );
    }
    return shouldAddParensIfNotBreakCache.get(node);
  }
  var shouldAlwaysAddParens = (node) => node.type === "SequenceExpression";
  function printArrowFunction(path, options2, print3, args = {}) {
    const signatureDocs = [];
    let bodyDoc;
    const bodyComments = [];
    let shouldBreakChain = false;
    const shouldPrintAsChain = !args.expandLastArg && path.node.body.type === "ArrowFunctionExpression";
    let functionBody;
    (function rec() {
      const { node } = path;
      const signatureDoc = printArrowFunctionSignature(
        path,
        options2,
        print3,
        args
      );
      if (signatureDocs.length === 0) {
        signatureDocs.push(signatureDoc);
      } else {
        const { leading, trailing } = printCommentsSeparately(path, options2);
        signatureDocs.push([leading, signatureDoc]);
        bodyComments.unshift(trailing);
      }
      if (shouldPrintAsChain) {
        shouldBreakChain || (shouldBreakChain = // Always break the chain if:
        node.returnType && getFunctionParameters(node).length > 0 || node.typeParameters || getFunctionParameters(node).some(
          (param) => param.type !== "Identifier"
        ));
      }
      if (!shouldPrintAsChain || node.body.type !== "ArrowFunctionExpression") {
        bodyDoc = print3("body", args);
        functionBody = node.body;
      } else {
        path.call(rec, "body");
      }
    })();
    const shouldPutBodyOnSameLine = !hasLeadingOwnLineComment(options2.originalText, functionBody) && (shouldAlwaysAddParens(functionBody) || mayBreakAfterShortPrefix(functionBody, bodyDoc, options2) || !shouldBreakChain && shouldAddParensIfNotBreak(functionBody));
    const isCallee = path.key === "callee" && isCallLikeExpression(path.parent);
    const chainGroupId = Symbol("arrow-chain");
    const signaturesDoc = printArrowFunctionSignatures(path, args, {
      signatureDocs,
      shouldBreak: shouldBreakChain
    });
    let shouldBreakSignatures;
    let shouldIndentSignatures = false;
    if (shouldPrintAsChain && (isCallee || // isAssignmentRhs
    args.assignmentLayout)) {
      shouldIndentSignatures = true;
      shouldBreakSignatures = args.assignmentLayout === "chain-tail-arrow-chain" || isCallee && !shouldPutBodyOnSameLine;
    }
    bodyDoc = printArrowFunctionBody(path, options2, args, {
      bodyDoc,
      bodyComments,
      functionBody,
      shouldPutBodyOnSameLine
    });
    return group([
      group(
        shouldIndentSignatures ? indent([softline, signaturesDoc]) : signaturesDoc,
        { shouldBreak: shouldBreakSignatures, id: chainGroupId }
      ),
      " =>",
      shouldPrintAsChain ? indentIfBreak(bodyDoc, { groupId: chainGroupId }) : group(bodyDoc),
      shouldPrintAsChain && isCallee ? ifBreak(softline, "", { groupId: chainGroupId }) : ""
    ]);
  }
  function printArrowFunctionSignature(path, options2, print3, args) {
    const { node } = path;
    const parts = [];
    if (node.async) {
      parts.push("async ");
    }
    if (shouldPrintParamsWithoutParens(path, options2)) {
      parts.push(print3(["params", 0]));
    } else {
      const expandArg = args.expandLastArg || args.expandFirstArg;
      let returnTypeDoc = printReturnType(path, print3);
      if (expandArg) {
        if (willBreak(returnTypeDoc)) {
          throw new ArgExpansionBailout();
        }
        returnTypeDoc = group(removeLines(returnTypeDoc));
      }
      parts.push(
        group([
          printFunctionParameters(
            path,
            print3,
            options2,
            expandArg,
            /* printTypeParams */
            true
          ),
          returnTypeDoc
        ])
      );
    }
    const dangling = printDanglingComments(path, options2, {
      filter(comment) {
        const nextCharacter = get_next_non_space_non_comment_character_index_default(
          options2.originalText,
          locEnd(comment)
        );
        return nextCharacter !== false && options2.originalText.slice(nextCharacter, nextCharacter + 2) === "=>";
      }
    });
    if (dangling) {
      parts.push(" ", dangling);
    }
    return parts;
  }
  function mayBreakAfterShortPrefix(functionBody, bodyDoc, options2) {
    var _a, _b;
    return isArrayOrTupleExpression(functionBody) || isObjectOrRecordExpression(functionBody) || functionBody.type === "ArrowFunctionExpression" || functionBody.type === "DoExpression" || functionBody.type === "BlockStatement" || isJsxElement(functionBody) || ((_a = bodyDoc.label) == null ? void 0 : _a.hug) !== false && (((_b = bodyDoc.label) == null ? void 0 : _b.embed) || isTemplateOnItsOwnLine(functionBody, options2.originalText));
  }
  function printArrowFunctionSignatures(path, args, { signatureDocs, shouldBreak }) {
    if (signatureDocs.length === 1) {
      return signatureDocs[0];
    }
    const { parent, key } = path;
    if (key !== "callee" && isCallLikeExpression(parent) || isBinaryish(parent)) {
      return group(
        [
          signatureDocs[0],
          " =>",
          indent([line, join([" =>", line], signatureDocs.slice(1))])
        ],
        { shouldBreak }
      );
    }
    if (key === "callee" && isCallLikeExpression(parent) || // isAssignmentRhs
    args.assignmentLayout) {
      return group(join([" =>", line], signatureDocs), { shouldBreak });
    }
    return group(indent(join([" =>", line], signatureDocs)), { shouldBreak });
  }
  function printArrowFunctionBody(path, options2, args, { bodyDoc, bodyComments, functionBody, shouldPutBodyOnSameLine }) {
    const { node, parent } = path;
    const trailingComma = args.expandLastArg && shouldPrintComma(options2, "all") ? ifBreak(",") : "";
    const trailingSpace = (args.expandLastArg || parent.type === "JSXExpressionContainer") && !hasComment(node) ? softline : "";
    if (shouldPutBodyOnSameLine && shouldAddParensIfNotBreak(functionBody)) {
      return [
        " ",
        group([
          ifBreak("", "("),
          indent([softline, bodyDoc]),
          ifBreak("", ")"),
          trailingComma,
          trailingSpace
        ]),
        bodyComments
      ];
    }
    if (shouldAlwaysAddParens(functionBody)) {
      bodyDoc = group(["(", indent([softline, bodyDoc]), softline, ")"]);
    }
    return shouldPutBodyOnSameLine ? [" ", bodyDoc, bodyComments] : [indent([line, bodyDoc, bodyComments]), trailingComma, trailingSpace];
  }

  // src/language-js/print/statement.js
  function printStatementSequence(path, options2, print3, property) {
    const { node } = path;
    const parts = [];
    const lastStatement = getLastStatement(node[property]);
    path.each(({ node: node2 }) => {
      if (node2.type === "EmptyStatement") {
        return;
      }
      parts.push(print3());
      if (node2 !== lastStatement) {
        parts.push(hardline);
        if (isNextLineEmpty2(node2, options2)) {
          parts.push(hardline);
        }
      }
    }, property);
    return parts;
  }
  function getLastStatement(statements) {
    for (let i = statements.length - 1; i >= 0; i--) {
      const statement = statements[i];
      if (statement.type !== "EmptyStatement") {
        return statement;
      }
    }
  }

  // src/language-js/print/block.js
  function printBlock(path, options2, print3) {
    const {
      node
    } = path;
    const parts = [];
    if (node.type === "StaticBlock") {
      parts.push("static ");
    }
    parts.push("{");
    const printed = printBlockBody(path, options2, print3);
    if (printed) {
      parts.push(indent([hardline, printed]), hardline);
    } else {
      const {
        parent
      } = path;
      const parentParent = path.grandparent;
      if (!(parent.type === "ArrowFunctionExpression" || parent.type === "FunctionExpression" || parent.type === "FunctionDeclaration" || parent.type === "ComponentDeclaration" || parent.type === "ObjectMethod" || parent.type === "ClassMethod" || parent.type === "ClassPrivateMethod" || parent.type === "ForStatement" || parent.type === "WhileStatement" || parent.type === "DoWhileStatement" || parent.type === "DoExpression" || parent.type === "CatchClause" && !parentParent.finalizer || parent.type === "TSModuleDeclaration" || parent.type === "TSDeclareFunction" || node.type === "StaticBlock")) {
        parts.push(hardline);
      }
    }
    parts.push("}");
    return parts;
  }
  function printBlockBody(path, options2, print3) {
    var _a;
    const {
      node
    } = path;
    const hasDirectives = is_non_empty_array_default(node.directives);
    const hasBody = node.body.some((node2) => node2.type !== "EmptyStatement");
    const hasDanglingComments = hasComment(node, CommentCheckFlags.Dangling);
    if (!hasDirectives && !hasBody && !hasDanglingComments) {
      return "";
    }
    const parts = [];
    if (hasDirectives) {
      parts.push(printStatementSequence(path, options2, print3, "directives"));
      if (hasBody || hasDanglingComments) {
        parts.push(hardline);
        if (isNextLineEmpty2(at_default(
          /* isOptionalObject*/
          false,
          node.directives,
          -1
        ), options2)) {
          parts.push(hardline);
        }
      }
    }
    if (hasBody) {
      parts.push(printStatementSequence(path, options2, print3, "body"));
    }
    if (hasDanglingComments) {
      parts.push(printDanglingComments(path, options2));
    }
    if (node.type === "Program" && ((_a = path.parent) == null ? void 0 : _a.type) !== "ModuleExpression") {
      parts.push(hardline);
    }
    return parts;
  }

  // src/language-js/print/semicolon.js
  function shouldPrintLeadingSemicolon(path, options2) {
    if (options2.semi || isSingleJsxExpressionStatementInMarkdown(path, options2) || isSingleVueEventBindingExpressionStatement(path, options2)) {
      return false;
    }
    const { node, key, parent } = path;
    if (node.type === "ExpressionStatement" && // `Program.directives` don't need leading semicolon
    (key === "body" && (parent.type === "Program" || parent.type === "BlockStatement" || parent.type === "StaticBlock" || parent.type === "TSModuleBlock") || key === "consequent" && parent.type === "SwitchCase") && path.call(() => expressionNeedsASIProtection(path, options2), "expression")) {
      return true;
    }
    return false;
  }
  function expressionNeedsASIProtection(path, options2) {
    const { node } = path;
    switch (node.type) {
      case "ParenthesizedExpression":
      case "TypeCastExpression":
      case "ArrayExpression":
      case "ArrayPattern":
      case "TemplateLiteral":
      case "TemplateElement":
      case "RegExpLiteral":
        return true;
      case "ArrowFunctionExpression":
        if (!shouldPrintParamsWithoutParens(path, options2)) {
          return true;
        }
        break;
      case "UnaryExpression": {
        const { prefix, operator } = node;
        if (prefix && (operator === "+" || operator === "-")) {
          return true;
        }
        break;
      }
      case "BindExpression":
        if (!node.object) {
          return true;
        }
        break;
      case "Literal":
        if (node.regex) {
          return true;
        }
        break;
      default:
        if (isJsxElement(node)) {
          return true;
        }
    }
    if (needs_parens_default(path, options2)) {
      return true;
    }
    if (!hasNakedLeftSide(node)) {
      return false;
    }
    return path.call(
      () => expressionNeedsASIProtection(path, options2),
      ...getLeftSidePathName(node)
    );
  }
  function isSingleJsxExpressionStatementInMarkdown({ node, parent }, options2) {
    return (options2.parentParser === "markdown" || options2.parentParser === "mdx") && node.type === "ExpressionStatement" && isJsxElement(node.expression) && parent.type === "Program" && parent.body.length === 1;
  }
  function isVueEventBindingExpression(node) {
    switch (node.type) {
      case "MemberExpression":
        switch (node.property.type) {
          case "Identifier":
          case "NumericLiteral":
          case "StringLiteral":
            return isVueEventBindingExpression(node.object);
        }
        return false;
      case "Identifier":
        return true;
      default:
        return false;
    }
  }
  function isSingleVueEventBindingExpressionStatement({ node, parent }, options2) {
    return (options2.parser === "__vue_event_binding" || options2.parser === "__vue_ts_event_binding") && node.type === "ExpressionStatement" && parent.type === "Program" && parent.body.length === 1;
  }

  // src/language-js/print/expression-statement.js
  function printExpressionStatement(path, options2, print3) {
    const parts = [print3("expression")];
    if (isSingleVueEventBindingExpressionStatement(path, options2)) {
      if (isVueEventBindingExpression(path.node.expression)) {
        parts.push(";");
      }
    } else if (isSingleJsxExpressionStatementInMarkdown(path, options2)) {
    } else if (options2.semi) {
      parts.push(";");
    }
    if (hasComment(
      path.node,
      CommentCheckFlags.Dangling,
      ({ marker }) => marker === markerForIfWithoutBlockAndSameLineComment
    )) {
      parts.push(
        " ",
        printDanglingComments(path, options2, {
          marker: markerForIfWithoutBlockAndSameLineComment
        })
      );
    }
    return parts;
  }

  // src/language-js/print/html-binding.js
  function printHtmlBinding(path, options2, print3) {
    if (options2.__isVueBindings || options2.__isVueForBindingLeft) {
      const parameterDocs = path.map(print3, "program", "body", 0, "params");
      if (parameterDocs.length === 1) {
        return parameterDocs[0];
      }
      const doc = join([",", line], parameterDocs);
      return options2.__isVueForBindingLeft ? ["(", indent([softline, group(doc)]), softline, ")"] : doc;
    }
    if (options2.__isEmbeddedTypescriptGenericParameters) {
      const parameterDocs = path.map(
        print3,
        "program",
        "body",
        0,
        "typeParameters",
        "params"
      );
      return join([",", line], parameterDocs);
    }
  }

  // src/language-js/print/estree.js
  function printEstree(path, options2, print3, args) {
    const { node } = path;
    if (isLiteral(node)) {
      return printLiteral(path, options2);
    }
    const semi = options2.semi ? ";" : "";
    let parts = [];
    switch (node.type) {
      case "JsExpressionRoot":
        return print3("node");
      case "JsonRoot":
        return [print3("node"), hardline];
      case "File":
        return printHtmlBinding(path, options2, print3) ?? print3("program");
      case "Program":
        return printBlockBody(path, options2, print3);
      case "EmptyStatement":
        return "";
      case "ExpressionStatement":
        return printExpressionStatement(path, options2, print3);
      case "ChainExpression":
        return print3("expression");
      case "ParenthesizedExpression": {
        const shouldHug = !hasComment(node.expression) && (isObjectOrRecordExpression(node.expression) || isArrayOrTupleExpression(node.expression));
        if (shouldHug) {
          return ["(", print3("expression"), ")"];
        }
        return group([
          "(",
          indent([softline, print3("expression")]),
          softline,
          ")"
        ]);
      }
      case "AssignmentExpression":
        return printAssignmentExpression(path, options2, print3);
      case "VariableDeclarator":
        return printVariableDeclarator(path, options2, print3);
      case "BinaryExpression":
      case "LogicalExpression":
        return printBinaryishExpression(path, options2, print3);
      case "AssignmentPattern":
        return [print3("left"), " = ", print3("right")];
      case "OptionalMemberExpression":
      case "MemberExpression":
        return printMemberExpression(path, options2, print3);
      case "MetaProperty":
        return [print3("meta"), ".", print3("property")];
      case "BindExpression":
        if (node.object) {
          parts.push(print3("object"));
        }
        parts.push(
          group(
            indent([softline, printBindExpressionCallee(path, options2, print3)])
          )
        );
        return parts;
      case "Identifier":
        return [
          node.name,
          printOptionalToken(path),
          printDefiniteToken(path),
          printTypeAnnotationProperty(path, print3)
        ];
      case "V8IntrinsicIdentifier":
        return ["%", node.name];
      case "SpreadElement":
      case "SpreadElementPattern":
      case "SpreadPropertyPattern":
      case "RestElement":
        return printRestSpread(path, print3);
      case "FunctionDeclaration":
      case "FunctionExpression":
        return printFunction(path, print3, options2, args);
      case "ArrowFunctionExpression":
        return printArrowFunction(path, options2, print3, args);
      case "YieldExpression":
        parts.push("yield");
        if (node.delegate) {
          parts.push("*");
        }
        if (node.argument) {
          parts.push(" ", print3("argument"));
        }
        return parts;
      case "AwaitExpression":
        parts.push("await");
        if (node.argument) {
          parts.push(" ", print3("argument"));
          const { parent } = path;
          if (isCallExpression(parent) && parent.callee === node || isMemberExpression(parent) && parent.object === node) {
            parts = [indent([softline, ...parts]), softline];
            const parentAwaitOrBlock = path.findAncestor(
              (node2) => node2.type === "AwaitExpression" || node2.type === "BlockStatement"
            );
            if ((parentAwaitOrBlock == null ? void 0 : parentAwaitOrBlock.type) !== "AwaitExpression" || !startsWithNoLookaheadToken(
              parentAwaitOrBlock.argument,
              (leftmostNode) => leftmostNode === node
            )) {
              return group(parts);
            }
          }
        }
        return parts;
      case "ExportDefaultDeclaration":
      case "ExportNamedDeclaration":
      case "ExportAllDeclaration":
        return printExportDeclaration(path, options2, print3);
      case "ImportDeclaration":
        return printImportDeclaration(path, options2, print3);
      case "ImportSpecifier":
      case "ExportSpecifier":
      case "ImportNamespaceSpecifier":
      case "ExportNamespaceSpecifier":
      case "ImportDefaultSpecifier":
      case "ExportDefaultSpecifier":
        return printModuleSpecifier(path, options2, print3);
      case "ImportAttribute":
        return [print3("key"), ": ", print3("value")];
      case "Import":
        return "import";
      case "BlockStatement":
      case "StaticBlock":
        return printBlock(path, options2, print3);
      case "ClassBody":
        return printClassBody(path, options2, print3);
      case "ThrowStatement":
        return printThrowStatement(path, options2, print3);
      case "ReturnStatement":
        return printReturnStatement(path, options2, print3);
      case "NewExpression":
      case "ImportExpression":
      case "OptionalCallExpression":
      case "CallExpression":
        return printCallExpression(path, options2, print3);
      case "ObjectExpression":
      case "ObjectPattern":
      case "RecordExpression":
        return printObject(path, options2, print3);
      case "ObjectProperty":
      case "Property":
        if (node.method || node.kind === "get" || node.kind === "set") {
          return printMethod(path, options2, print3);
        }
        return printProperty(path, options2, print3);
      case "ObjectMethod":
        return printMethod(path, options2, print3);
      case "Decorator":
        return ["@", print3("expression")];
      case "ArrayExpression":
      case "ArrayPattern":
      case "TupleExpression":
        return printArray(path, options2, print3);
      case "SequenceExpression": {
        const { parent } = path;
        if (parent.type === "ExpressionStatement" || parent.type === "ForStatement") {
          const parts2 = [];
          path.each(({ isFirst }) => {
            if (isFirst) {
              parts2.push(print3());
            } else {
              parts2.push(",", indent([line, print3()]));
            }
          }, "expressions");
          return group(parts2);
        }
        return group(join([",", line], path.map(print3, "expressions")));
      }
      case "ThisExpression":
        return "this";
      case "Super":
        return "super";
      case "Directive":
        return [print3("value"), semi];
      case "UnaryExpression":
        parts.push(node.operator);
        if (/[a-z]$/.test(node.operator)) {
          parts.push(" ");
        }
        if (hasComment(node.argument)) {
          parts.push(
            group(["(", indent([softline, print3("argument")]), softline, ")"])
          );
        } else {
          parts.push(print3("argument"));
        }
        return parts;
      case "UpdateExpression":
        parts.push(print3("argument"), node.operator);
        if (node.prefix) {
          parts.reverse();
        }
        return parts;
      case "ConditionalExpression":
        return printTernary(path, options2, print3);
      case "VariableDeclaration": {
        const printed = path.map(print3, "declarations");
        const parentNode = path.parent;
        const isParentForLoop = parentNode.type === "ForStatement" || parentNode.type === "ForInStatement" || parentNode.type === "ForOfStatement";
        const hasValue = node.declarations.some((decl) => decl.init);
        let firstVariable;
        if (printed.length === 1 && !hasComment(node.declarations[0])) {
          firstVariable = printed[0];
        } else if (printed.length > 0) {
          firstVariable = indent(printed[0]);
        }
        parts = [
          printDeclareToken(path),
          node.kind,
          firstVariable ? [" ", firstVariable] : "",
          indent(
            printed.slice(1).map((p) => [
              ",",
              hasValue && !isParentForLoop ? hardline : line,
              p
            ])
          )
        ];
        if (!(isParentForLoop && parentNode.body !== node)) {
          parts.push(semi);
        }
        return group(parts);
      }
      case "WithStatement":
        return group([
          "with (",
          print3("object"),
          ")",
          adjustClause(node.body, print3("body"))
        ]);
      case "IfStatement": {
        const con = adjustClause(node.consequent, print3("consequent"));
        const opening = group([
          "if (",
          group([indent([softline, print3("test")]), softline]),
          ")",
          con
        ]);
        parts.push(opening);
        if (node.alternate) {
          const commentOnOwnLine = hasComment(
            node.consequent,
            CommentCheckFlags.Trailing | CommentCheckFlags.Line
          ) || needsHardlineAfterDanglingComment(node);
          const elseOnSameLine = node.consequent.type === "BlockStatement" && !commentOnOwnLine;
          parts.push(elseOnSameLine ? " " : hardline);
          if (hasComment(node, CommentCheckFlags.Dangling)) {
            parts.push(
              printDanglingComments(path, options2),
              commentOnOwnLine ? hardline : " "
            );
          }
          parts.push(
            "else",
            group(
              adjustClause(
                node.alternate,
                print3("alternate"),
                node.alternate.type === "IfStatement"
              )
            )
          );
        }
        return parts;
      }
      case "ForStatement": {
        const body = adjustClause(node.body, print3("body"));
        const dangling = printDanglingComments(path, options2);
        const printedComments = dangling ? [dangling, softline] : "";
        if (!node.init && !node.test && !node.update) {
          return [printedComments, group(["for (;;)", body])];
        }
        return [
          printedComments,
          group([
            "for (",
            group([
              indent([
                softline,
                print3("init"),
                ";",
                line,
                print3("test"),
                ";",
                line,
                print3("update")
              ]),
              softline
            ]),
            ")",
            body
          ])
        ];
      }
      case "WhileStatement":
        return group([
          "while (",
          group([indent([softline, print3("test")]), softline]),
          ")",
          adjustClause(node.body, print3("body"))
        ]);
      case "ForInStatement":
        return group([
          "for (",
          print3("left"),
          " in ",
          print3("right"),
          ")",
          adjustClause(node.body, print3("body"))
        ]);
      case "ForOfStatement":
        return group([
          "for",
          node.await ? " await" : "",
          " (",
          print3("left"),
          " of ",
          print3("right"),
          ")",
          adjustClause(node.body, print3("body"))
        ]);
      case "DoWhileStatement": {
        const clause = adjustClause(node.body, print3("body"));
        const doBody = group(["do", clause]);
        parts = [doBody];
        if (node.body.type === "BlockStatement") {
          parts.push(" ");
        } else {
          parts.push(hardline);
        }
        parts.push(
          "while (",
          group([indent([softline, print3("test")]), softline]),
          ")",
          semi
        );
        return parts;
      }
      case "DoExpression":
        return [node.async ? "async " : "", "do ", print3("body")];
      case "BreakStatement":
      case "ContinueStatement":
        parts.push(node.type === "BreakStatement" ? "break" : "continue");
        if (node.label) {
          parts.push(" ", print3("label"));
        }
        parts.push(semi);
        return parts;
      case "LabeledStatement":
        if (node.body.type === "EmptyStatement") {
          return [print3("label"), ":;"];
        }
        return [print3("label"), ": ", print3("body")];
      case "TryStatement":
        return [
          "try ",
          print3("block"),
          node.handler ? [" ", print3("handler")] : "",
          node.finalizer ? [" finally ", print3("finalizer")] : ""
        ];
      case "CatchClause":
        if (node.param) {
          const parameterHasComments = hasComment(
            node.param,
            (comment) => !is_block_comment_default(comment) || comment.leading && has_newline_default(options2.originalText, locEnd(comment)) || comment.trailing && has_newline_default(options2.originalText, locStart(comment), {
              backwards: true
            })
          );
          const param = print3("param");
          return [
            "catch ",
            parameterHasComments ? ["(", indent([softline, param]), softline, ") "] : ["(", param, ") "],
            print3("body")
          ];
        }
        return ["catch ", print3("body")];
      case "SwitchStatement":
        return [
          group([
            "switch (",
            indent([softline, print3("discriminant")]),
            softline,
            ")"
          ]),
          " {",
          node.cases.length > 0 ? indent([
            hardline,
            join(
              hardline,
              path.map(
                ({ node: node2, isLast }) => [
                  print3(),
                  !isLast && isNextLineEmpty2(node2, options2) ? hardline : ""
                ],
                "cases"
              )
            )
          ]) : "",
          hardline,
          "}"
        ];
      case "SwitchCase": {
        if (node.test) {
          parts.push("case ", print3("test"), ":");
        } else {
          parts.push("default:");
        }
        if (hasComment(node, CommentCheckFlags.Dangling)) {
          parts.push(" ", printDanglingComments(path, options2));
        }
        const consequent = node.consequent.filter(
          (node2) => node2.type !== "EmptyStatement"
        );
        if (consequent.length > 0) {
          const cons = printStatementSequence(path, options2, print3, "consequent");
          parts.push(
            consequent.length === 1 && consequent[0].type === "BlockStatement" ? [" ", cons] : indent([hardline, cons])
          );
        }
        return parts;
      }
      case "DebuggerStatement":
        return ["debugger", semi];
      case "ClassDeclaration":
      case "ClassExpression":
        return printClass(path, options2, print3);
      case "ClassMethod":
      case "ClassPrivateMethod":
      case "MethodDefinition":
        return printClassMethod(path, options2, print3);
      case "ClassProperty":
      case "PropertyDefinition":
      case "ClassPrivateProperty":
      case "ClassAccessorProperty":
      case "AccessorProperty":
        return printClassProperty(path, options2, print3);
      case "TemplateElement":
        return replaceEndOfLine(node.value.raw);
      case "TemplateLiteral":
        return printTemplateLiteral(path, print3, options2);
      case "TaggedTemplateExpression":
        return printTaggedTemplateLiteral(print3);
      case "PrivateIdentifier":
        return ["#", node.name];
      case "PrivateName":
        return ["#", print3("id")];
      case "TopicReference":
        return "%";
      case "ArgumentPlaceholder":
        return "?";
      case "ModuleExpression": {
        parts.push("module {");
        const printed = print3("body");
        if (printed) {
          parts.push(indent([hardline, printed]), hardline);
        }
        parts.push("}");
        return parts;
      }
      case "InterpreterDirective":
      default:
        throw new unexpected_node_error_default(node, "ESTree");
    }
  }

  // src/language-js/print/angular.js
  function printAngular(path, options2, print3) {
    const { node } = path;
    if (!node.type.startsWith("NG")) {
      return;
    }
    switch (node.type) {
      case "NGRoot":
        return [
          print3("node"),
          hasComment(node.node) ? " //" + getComments(node.node)[0].value.trimEnd() : ""
        ];
      case "NGPipeExpression":
        return printBinaryishExpression(path, options2, print3);
      case "NGChainedExpression":
        return group(
          join(
            [";", line],
            path.map(
              () => hasNgSideEffect(path) ? print3() : ["(", print3(), ")"],
              "expressions"
            )
          )
        );
      case "NGEmptyExpression":
        return "";
      case "NGMicrosyntax":
        return path.map(
          () => [
            path.isFirst ? "" : isNgForOf(path) ? " " : [";", line],
            print3()
          ],
          "body"
        );
      case "NGMicrosyntaxKey":
        return /^[$_a-z][\w$]*(?:-[$_a-z][\w$])*$/i.test(node.name) ? node.name : JSON.stringify(node.name);
      case "NGMicrosyntaxExpression":
        return [
          print3("expression"),
          node.alias === null ? "" : [" as ", print3("alias")]
        ];
      case "NGMicrosyntaxKeyedExpression": {
        const { index, parent } = path;
        const shouldNotPrintColon = isNgForOf(path) || (index === 1 && (node.key.name === "then" || node.key.name === "else") || index === 2 && node.key.name === "else" && parent.body[index - 1].type === "NGMicrosyntaxKeyedExpression" && parent.body[index - 1].key.name === "then") && parent.body[0].type === "NGMicrosyntaxExpression";
        return [
          print3("key"),
          shouldNotPrintColon ? " " : ": ",
          print3("expression")
        ];
      }
      case "NGMicrosyntaxLet":
        return [
          "let ",
          print3("key"),
          node.value === null ? "" : [" = ", print3("value")]
        ];
      case "NGMicrosyntaxAs":
        return [print3("key"), " as ", print3("alias")];
      default:
        throw new unexpected_node_error_default(node, "Angular");
    }
  }
  function isNgForOf({ node, index, parent }) {
    return node.type === "NGMicrosyntaxKeyedExpression" && node.key.name === "of" && index === 1 && parent.body[0].type === "NGMicrosyntaxLet" && parent.body[0].value === null;
  }
  var hasSideEffect = create_type_check_function_default([
    "CallExpression",
    "OptionalCallExpression",
    "AssignmentExpression"
  ]);
  function hasNgSideEffect({ node }) {
    return hasNode(node, hasSideEffect);
  }

  // src/language-js/print/interface.js
  function printInterface(path, options2, print3) {
    const { node } = path;
    const parts = [printDeclareToken(path), "interface"];
    const partsGroup = [];
    const extendsParts = [];
    if (node.type !== "InterfaceTypeAnnotation") {
      partsGroup.push(" ", print3("id"), print3("typeParameters"));
    }
    const shouldIndentOnlyHeritageClauses2 = node.typeParameters && !hasComment(
      node.typeParameters,
      CommentCheckFlags.Trailing | CommentCheckFlags.Line
    );
    if (is_non_empty_array_default(node.extends)) {
      extendsParts.push(
        shouldIndentOnlyHeritageClauses2 ? ifBreak(" ", line, {
          groupId: getTypeParametersGroupId(node.typeParameters)
        }) : line,
        "extends ",
        (node.extends.length === 1 ? identity : indent)(
          join([",", line], path.map(print3, "extends"))
        )
      );
    }
    if (hasComment(node.id, CommentCheckFlags.Trailing) || is_non_empty_array_default(node.extends)) {
      if (shouldIndentOnlyHeritageClauses2) {
        parts.push(group([...partsGroup, indent(extendsParts)]));
      } else {
        parts.push(group(indent([...partsGroup, ...extendsParts])));
      }
    } else {
      parts.push(...partsGroup, ...extendsParts);
    }
    parts.push(" ", print3("body"));
    return group(parts);
  }

  // src/language-js/print/enum.js
  function printEnumMembers(path, print3, options2) {
    return printObject(path, options2, print3);
  }
  function printEnumMember(path, print3) {
    const { node } = path;
    let idDoc = print3("id");
    if (node.computed) {
      idDoc = ["[", idDoc, "]"];
    }
    let initializerDoc = "";
    if (node.initializer) {
      initializerDoc = print3("initializer");
    }
    if (node.init) {
      initializerDoc = print3("init");
    }
    if (!initializerDoc) {
      return idDoc;
    }
    return [idDoc, " = ", initializerDoc];
  }
  function printEnumBody(path, print3, options2) {
    const { node } = path;
    let type;
    if (node.type === "EnumSymbolBody" || node.explicitType) {
      switch (node.type) {
        case "EnumBooleanBody":
          type = "boolean";
          break;
        case "EnumNumberBody":
          type = "number";
          break;
        case "EnumStringBody":
          type = "string";
          break;
        case "EnumSymbolBody":
          type = "symbol";
          break;
      }
    }
    return [type ? `of ${type} ` : "", printEnumMembers(path, print3, options2)];
  }
  function printEnumDeclaration(path, print3, options2) {
    const { node } = path;
    return [
      printDeclareToken(path),
      node.const ? "const " : "",
      "enum ",
      print3("id"),
      " ",
      node.type === "TSEnumDeclaration" ? printEnumMembers(path, print3, options2) : print3("body")
    ];
  }

  // src/language-js/print/component.js
  function printComponent(path, options2, print3) {
    const {
      node
    } = path;
    const parts = [printDeclareToken(path), "component"];
    if (node.id) {
      parts.push(" ", print3("id"));
    }
    parts.push(print3("typeParameters"));
    const parametersDoc = printComponentParameters(path, print3, options2);
    if (node.rendersType) {
      const renderTypesDoc = path.call((path2) => print3("typeAnnotation"), "rendersType");
      parts.push(group([parametersDoc, " renders ", renderTypesDoc]));
    } else {
      parts.push(group([parametersDoc]));
    }
    if (node.body) {
      parts.push(" ", print3("body"));
    }
    if (options2.semi && node.type === "DeclareComponent") {
      parts.push(";");
    }
    return parts;
  }
  function printComponentParameters(path, print3, options2) {
    const {
      node: componentNode
    } = path;
    let parameters = componentNode.params;
    if (componentNode.rest) {
      parameters = [...parameters, componentNode.rest];
    }
    if (parameters.length === 0) {
      return ["(", printDanglingComments(path, options2, {
        filter: (comment) => get_next_non_space_non_comment_character_default(options2.originalText, locEnd(comment)) === ")"
      }), ")"];
    }
    const printed = [];
    iterateComponentParametersPath(path, (parameterPath, index) => {
      const isLastParameter = index === parameters.length - 1;
      if (isLastParameter && componentNode.rest) {
        printed.push("...");
      }
      printed.push(print3());
      if (isLastParameter) {
        return;
      }
      printed.push(",");
      if (isNextLineEmpty2(parameters[index], options2)) {
        printed.push(hardline, hardline);
      } else {
        printed.push(line);
      }
    });
    return ["(", indent([softline, ...printed]), ifBreak(shouldPrintComma(options2, "all") && !hasRestParameter2(componentNode, parameters) ? "," : ""), softline, ")"];
  }
  function hasRestParameter2(componentNode, parameters) {
    var _a;
    return componentNode.rest || ((_a = at_default(
      /* isOptionalObject*/
      false,
      parameters,
      -1
    )) == null ? void 0 : _a.type) === "RestElement";
  }
  function iterateComponentParametersPath(path, iteratee) {
    const {
      node
    } = path;
    let index = 0;
    const callback = (childPath) => iteratee(childPath, index++);
    path.each(callback, "params");
    if (node.rest) {
      path.call(callback, "rest");
    }
  }
  function printComponentParameter(path, options2, print3) {
    const {
      node
    } = path;
    if (node.shorthand) {
      return print3("local");
    }
    return [print3("name"), " as ", print3("local")];
  }
  function printComponentTypeParameter(path, options2, print3) {
    const {
      node
    } = path;
    const printed = [];
    if (node.name) {
      printed.push(print3("name"), node.optional ? "?: " : ": ");
    }
    printed.push(print3("typeAnnotation"));
    return printed;
  }

  // src/language-js/print/flow.js
  function printFlow(path, options2, print3) {
    const { node } = path;
    if (is_flow_keyword_type_default(node)) {
      return node.type.slice(0, -14).toLowerCase();
    }
    const semi = options2.semi ? ";" : "";
    switch (node.type) {
      case "ComponentDeclaration":
      case "DeclareComponent":
      case "ComponentTypeAnnotation":
        return printComponent(path, options2, print3);
      case "ComponentParameter":
        return printComponentParameter(path, options2, print3);
      case "ComponentTypeParameter":
        return printComponentTypeParameter(path, options2, print3);
      case "DeclareClass":
        return printClass(path, options2, print3);
      case "DeclareFunction":
        return [
          printDeclareToken(path),
          "function ",
          print3("id"),
          print3("predicate"),
          semi
        ];
      case "DeclareModule":
        return ["declare module ", print3("id"), " ", print3("body")];
      case "DeclareModuleExports":
        return [
          "declare module.exports",
          printTypeAnnotationProperty(path, print3),
          semi
        ];
      case "DeclareVariable":
        return [
          printDeclareToken(path),
          // TODO: Only use `node.kind` when babel update AST
          node.kind ?? "var",
          " ",
          print3("id"),
          semi
        ];
      case "DeclareExportDeclaration":
      case "DeclareExportAllDeclaration":
        return printExportDeclaration(path, options2, print3);
      case "DeclareOpaqueType":
      case "OpaqueType":
        return printOpaqueType(path, options2, print3);
      case "DeclareTypeAlias":
      case "TypeAlias":
        return printTypeAlias(path, options2, print3);
      case "IntersectionTypeAnnotation":
        return printIntersectionType(path, options2, print3);
      case "UnionTypeAnnotation":
        return printUnionType(path, options2, print3);
      case "ConditionalTypeAnnotation":
        return printTernary(path, options2, print3);
      case "InferTypeAnnotation":
        return printInferType(path, options2, print3);
      case "FunctionTypeAnnotation":
        return printFunctionType(path, options2, print3);
      case "TupleTypeAnnotation":
        return printArray(path, options2, print3);
      case "TupleTypeLabeledElement":
        return printNamedTupleMember(path, options2, print3);
      case "TupleTypeSpreadElement":
        return printRestType(path, options2, print3);
      case "GenericTypeAnnotation":
        return [
          print3("id"),
          printTypeParameters(path, options2, print3, "typeParameters")
        ];
      case "IndexedAccessType":
      case "OptionalIndexedAccessType":
        return printIndexedAccessType(path, options2, print3);
      case "TypeAnnotation":
        return printTypeAnnotation(path, options2, print3);
      case "TypeParameter":
        return printTypeParameter(path, options2, print3);
      case "TypeofTypeAnnotation":
        return printTypeQuery(path, print3);
      case "ExistsTypeAnnotation":
        return "*";
      case "ArrayTypeAnnotation":
        return printArrayType(print3);
      case "DeclareEnum":
      case "EnumDeclaration":
        return printEnumDeclaration(path, print3, options2);
      case "EnumBooleanBody":
      case "EnumNumberBody":
      case "EnumStringBody":
      case "EnumSymbolBody":
        return printEnumBody(path, print3, options2);
      case "EnumBooleanMember":
      case "EnumNumberMember":
      case "EnumStringMember":
      case "EnumDefaultedMember":
        return printEnumMember(path, print3);
      case "FunctionTypeParam": {
        const name = node.name ? print3("name") : path.parent.this === node ? "this" : "";
        return [
          name,
          printOptionalToken(path),
          // `flow` doesn't wrap the `typeAnnotation` with `TypeAnnotation`, so the colon
          // needs to be added separately.
          name ? ": " : "",
          print3("typeAnnotation")
        ];
      }
      case "DeclareInterface":
      case "InterfaceDeclaration":
      case "InterfaceTypeAnnotation":
        return printInterface(path, options2, print3);
      case "ClassImplements":
      case "InterfaceExtends":
        return [print3("id"), print3("typeParameters")];
      case "NullableTypeAnnotation":
        return ["?", print3("typeAnnotation")];
      case "Variance": {
        const { kind } = node;
        assert_default.ok(kind === "plus" || kind === "minus");
        return kind === "plus" ? "+" : "-";
      }
      case "KeyofTypeAnnotation":
        return ["keyof ", print3("argument")];
      case "ObjectTypeCallProperty":
        return [node.static ? "static " : "", print3("value")];
      case "ObjectTypeMappedTypeProperty":
        return printFlowMappedTypeProperty(path, options2, print3);
      case "ObjectTypeIndexer":
        return [
          node.static ? "static " : "",
          node.variance ? print3("variance") : "",
          "[",
          print3("id"),
          node.id ? ": " : "",
          print3("key"),
          "]: ",
          print3("value")
        ];
      case "ObjectTypeProperty": {
        let modifier = "";
        if (node.proto) {
          modifier = "proto ";
        } else if (node.static) {
          modifier = "static ";
        }
        return [
          modifier,
          isGetterOrSetter(node) ? node.kind + " " : "",
          node.variance ? print3("variance") : "",
          printPropertyKey(path, options2, print3),
          printOptionalToken(path),
          isFunctionNotation(node) ? "" : ": ",
          print3("value")
        ];
      }
      case "ObjectTypeAnnotation":
        return printObject(path, options2, print3);
      case "ObjectTypeInternalSlot":
        return [
          node.static ? "static " : "",
          "[[",
          print3("id"),
          "]]",
          printOptionalToken(path),
          node.method ? "" : ": ",
          print3("value")
        ];
      case "ObjectTypeSpreadProperty":
        return printRestSpread(path, print3);
      case "QualifiedTypeofIdentifier":
      case "QualifiedTypeIdentifier":
        return [print3("qualification"), ".", print3("id")];
      case "NullLiteralTypeAnnotation":
        return "null";
      case "BooleanLiteralTypeAnnotation":
        return String(node.value);
      case "StringLiteralTypeAnnotation":
        return replaceEndOfLine(print_string_default(rawText(node), options2));
      case "NumberLiteralTypeAnnotation":
        return print_number_default(node.raw ?? node.extra.raw);
      case "BigIntLiteralTypeAnnotation":
        return printBigInt(node.raw ?? node.extra.raw);
      case "TypeCastExpression":
        return [
          "(",
          print3("expression"),
          printTypeAnnotationProperty(path, print3),
          ")"
        ];
      case "TypePredicate":
        return printTypePredicate(path, print3);
      case "TypeOperator":
        return [node.operator, " ", print3("typeAnnotation")];
      case "TypeParameterDeclaration":
      case "TypeParameterInstantiation":
        return printTypeParameters(path, options2, print3, "params");
      case "InferredPredicate":
      case "DeclaredPredicate":
        return [
          // The return type will already add the colon, but otherwise we
          // need to do it ourselves
          path.key === "predicate" && path.parent.type !== "DeclareFunction" && !path.parent.returnType ? ": " : " ",
          "%checks",
          ...node.type === "DeclaredPredicate" ? ["(", print3("value"), ")"] : []
        ];
    }
  }

  // src/language-js/print/typescript.js
  function printTypescript(path, options2, print3) {
    var _a;
    const { node } = path;
    if (!node.type.startsWith("TS")) {
      return;
    }
    if (is_ts_keyword_type_default(node)) {
      return node.type.slice(2, -7).toLowerCase();
    }
    const semi = options2.semi ? ";" : "";
    const parts = [];
    switch (node.type) {
      case "TSThisType":
        return "this";
      case "TSTypeAssertion": {
        const shouldBreakAfterCast = !(isArrayOrTupleExpression(node.expression) || isObjectOrRecordExpression(node.expression));
        const castGroup = group([
          "<",
          indent([softline, print3("typeAnnotation")]),
          softline,
          ">"
        ]);
        const exprContents = [
          ifBreak("("),
          indent([softline, print3("expression")]),
          softline,
          ifBreak(")")
        ];
        if (shouldBreakAfterCast) {
          return conditionalGroup([
            [castGroup, print3("expression")],
            [castGroup, group(exprContents, { shouldBreak: true })],
            [castGroup, print3("expression")]
          ]);
        }
        return group([castGroup, print3("expression")]);
      }
      case "TSDeclareFunction":
        return printFunction(path, print3, options2);
      case "TSExportAssignment":
        return ["export = ", print3("expression"), semi];
      case "TSModuleBlock":
        return printBlock(path, options2, print3);
      case "TSInterfaceBody":
      case "TSTypeLiteral":
        return printObject(path, options2, print3);
      case "TSTypeAliasDeclaration":
        return printTypeAlias(path, options2, print3);
      case "TSQualifiedName":
        return [print3("left"), ".", print3("right")];
      case "TSAbstractMethodDefinition":
      case "TSDeclareMethod":
        return printClassMethod(path, options2, print3);
      case "TSAbstractAccessorProperty":
      case "TSAbstractPropertyDefinition":
        return printClassProperty(path, options2, print3);
      case "TSInterfaceHeritage":
      case "TSClassImplements":
      case "TSExpressionWithTypeArguments":
      case "TSInstantiationExpression":
        return [print3("expression"), print3("typeParameters")];
      case "TSTemplateLiteralType":
        return printTemplateLiteral(path, print3, options2);
      case "TSNamedTupleMember":
        return printNamedTupleMember(path, options2, print3);
      case "TSRestType":
        return printRestType(path, options2, print3);
      case "TSOptionalType":
        return [print3("typeAnnotation"), "?"];
      case "TSInterfaceDeclaration":
        return printInterface(path, options2, print3);
      case "TSTypeParameterDeclaration":
      case "TSTypeParameterInstantiation":
        return printTypeParameters(path, options2, print3, "params");
      case "TSTypeParameter":
        return printTypeParameter(path, options2, print3);
      case "TSAsExpression":
      case "TSSatisfiesExpression": {
        const operator = node.type === "TSAsExpression" ? "as" : "satisfies";
        parts.push(print3("expression"), ` ${operator} `, print3("typeAnnotation"));
        const { parent } = path;
        if (isCallExpression(parent) && parent.callee === node || isMemberExpression(parent) && parent.object === node) {
          return group([indent([softline, ...parts]), softline]);
        }
        return parts;
      }
      case "TSArrayType":
        return printArrayType(print3);
      case "TSPropertySignature":
        return [
          node.readonly ? "readonly " : "",
          printPropertyKey(path, options2, print3),
          printOptionalToken(path),
          printTypeAnnotationProperty(path, print3)
        ];
      case "TSParameterProperty":
        return [
          printTypeScriptAccessibilityToken(node),
          node.static ? "static " : "",
          node.override ? "override " : "",
          node.readonly ? "readonly " : "",
          print3("parameter")
        ];
      case "TSTypeQuery":
        return printTypeQuery(path, print3);
      case "TSIndexSignature": {
        const trailingComma = node.parameters.length > 1 ? ifBreak(shouldPrintComma(options2) ? "," : "") : "";
        const parametersGroup = group([
          indent([
            softline,
            join([", ", softline], path.map(print3, "parameters"))
          ]),
          trailingComma,
          softline
        ]);
        const isClassMember = path.parent.type === "ClassBody" && path.key === "body";
        return [
          // `static` only allowed in class member
          isClassMember && node.static ? "static " : "",
          node.readonly ? "readonly " : "",
          "[",
          node.parameters ? parametersGroup : "",
          "]",
          printTypeAnnotationProperty(path, print3),
          isClassMember ? semi : ""
        ];
      }
      case "TSTypePredicate":
        return printTypePredicate(path, print3);
      case "TSNonNullExpression":
        return [print3("expression"), "!"];
      case "TSImportType":
        return [
          !node.isTypeOf ? "" : "typeof ",
          "import(",
          print3("argument"),
          ")",
          !node.qualifier ? "" : [".", print3("qualifier")],
          printTypeParameters(
            path,
            options2,
            print3,
            node.typeArguments ? "typeArguments" : "typeParameters"
          )
        ];
      case "TSLiteralType":
        return print3("literal");
      case "TSIndexedAccessType":
        return printIndexedAccessType(path, options2, print3);
      case "TSTypeOperator":
        return [node.operator, " ", print3("typeAnnotation")];
      case "TSMappedType":
        return printTypescriptMappedType(path, options2, print3);
      case "TSMethodSignature": {
        const kind = node.kind && node.kind !== "method" ? `${node.kind} ` : "";
        parts.push(
          printTypeScriptAccessibilityToken(node),
          kind,
          node.computed ? "[" : "",
          print3("key"),
          node.computed ? "]" : "",
          printOptionalToken(path)
        );
        const parametersDoc = printFunctionParameters(
          path,
          print3,
          options2,
          /* expandArg */
          false,
          /* printTypeParams */
          true
        );
        const returnTypePropertyName = node.returnType ? "returnType" : "typeAnnotation";
        const returnTypeNode = node[returnTypePropertyName];
        const returnTypeDoc = returnTypeNode ? printTypeAnnotationProperty(path, print3, returnTypePropertyName) : "";
        const shouldGroupParameters = shouldGroupFunctionParameters(
          node,
          returnTypeDoc
        );
        parts.push(shouldGroupParameters ? group(parametersDoc) : parametersDoc);
        if (returnTypeNode) {
          parts.push(group(returnTypeDoc));
        }
        return group(parts);
      }
      case "TSNamespaceExportDeclaration":
        return ["export as namespace ", print3("id"), options2.semi ? ";" : ""];
      case "TSEnumDeclaration":
        return printEnumDeclaration(path, print3, options2);
      case "TSEnumMember":
        return printEnumMember(path, print3);
      case "TSImportEqualsDeclaration":
        return [
          node.isExport ? "export " : "",
          "import ",
          printImportKind(
            node,
            /* spaceBeforeKind */
            false
          ),
          print3("id"),
          " = ",
          print3("moduleReference"),
          options2.semi ? ";" : ""
        ];
      case "TSExternalModuleReference":
        return ["require(", print3("expression"), ")"];
      case "TSModuleDeclaration": {
        const { parent } = path;
        const parentIsDeclaration = parent.type === "TSModuleDeclaration";
        const bodyIsDeclaration = ((_a = node.body) == null ? void 0 : _a.type) === "TSModuleDeclaration";
        if (parentIsDeclaration) {
          parts.push(".");
        } else {
          parts.push(printDeclareToken(path));
          const isGlobal = node.kind === "global" || // TODO: Use `node.kind` when babel update AST
          // https://github.com/typescript-eslint/typescript-eslint/pull/6443
          node.global;
          if (!isGlobal) {
            const kind = node.kind ?? // TODO: Use `node.kind` when babel update AST
            (isStringLiteral(node.id) || /(?:^|\s)module(?:\s|$)/.test(
              options2.originalText.slice(locStart(node), locStart(node.id))
            ) ? "module" : "namespace");
            parts.push(kind, " ");
          }
        }
        parts.push(print3("id"));
        if (bodyIsDeclaration) {
          parts.push(print3("body"));
        } else if (node.body) {
          parts.push(" ", group(print3("body")));
        } else {
          parts.push(semi);
        }
        return parts;
      }
      case "TSConditionalType":
        return printTernary(path, options2, print3);
      case "TSInferType":
        return printInferType(path, options2, print3);
      case "TSIntersectionType":
        return printIntersectionType(path, options2, print3);
      case "TSUnionType":
        return printUnionType(path, options2, print3);
      case "TSFunctionType":
      case "TSCallSignatureDeclaration":
      case "TSConstructorType":
      case "TSConstructSignatureDeclaration":
        return printFunctionType(path, options2, print3);
      case "TSTupleType":
        return printArray(path, options2, print3);
      case "TSTypeReference":
        return [
          print3("typeName"),
          printTypeParameters(path, options2, print3, "typeParameters")
        ];
      case "TSTypeAnnotation":
        return printTypeAnnotation(path, options2, print3);
      case "TSEmptyBodyFunctionExpression":
        return printMethodValue(path, options2, print3);
      case "TSJSDocAllType":
        return "*";
      case "TSJSDocUnknownType":
        return "?";
      case "TSJSDocNullableType":
        return printJSDocType(
          path,
          print3,
          /* token */
          "?"
        );
      case "TSJSDocNonNullableType":
        return printJSDocType(
          path,
          print3,
          /* token */
          "!"
        );
      case "TSParenthesizedType":
      default:
        throw new unexpected_node_error_default(node, "TypeScript");
    }
  }

  // src/language-js/print/index.js
  function printWithoutParentheses(path, options2, print3, args) {
    if (is_ignored_default(path)) {
      return print_ignored_default(path, options2);
    }
    for (const printer of [
      printAngular,
      printJsx,
      printFlow,
      printTypescript,
      printEstree
    ]) {
      const doc = printer(path, options2, print3, args);
      if (doc !== void 0) {
        return doc;
      }
    }
  }
  var shouldPrintDirectly = create_type_check_function_default([
    "ClassMethod",
    "ClassPrivateMethod",
    "ClassProperty",
    "ClassAccessorProperty",
    "AccessorProperty",
    "TSAbstractAccessorProperty",
    "PropertyDefinition",
    "TSAbstractPropertyDefinition",
    "ClassPrivateProperty",
    "MethodDefinition",
    "TSAbstractMethodDefinition",
    "TSDeclareMethod"
  ]);
  function print(path, options2, print3, args) {
    var _a;
    if (path.isRoot) {
      (_a = options2.__onHtmlBindingRoot) == null ? void 0 : _a.call(options2, path.node, options2);
    }
    const doc = printWithoutParentheses(path, options2, print3, args);
    if (!doc) {
      return "";
    }
    const { node } = path;
    if (shouldPrintDirectly(node)) {
      return doc;
    }
    const hasDecorators = is_non_empty_array_default(node.decorators);
    const decoratorsDoc = printDecorators(path, options2, print3);
    const isClassExpression = node.type === "ClassExpression";
    if (hasDecorators && !isClassExpression) {
      return inheritLabel(doc, (doc2) => group([decoratorsDoc, doc2]));
    }
    const needsParens2 = needs_parens_default(path, options2);
    const needsSemi = shouldPrintLeadingSemicolon(path, options2);
    if (!decoratorsDoc && !needsParens2 && !needsSemi) {
      return doc;
    }
    return inheritLabel(doc, (doc2) => [
      needsSemi ? ";" : "",
      needsParens2 ? "(" : "",
      needsParens2 && isClassExpression && hasDecorators ? [indent([line, decoratorsDoc, doc2]), line] : [decoratorsDoc, doc2],
      needsParens2 ? ")" : ""
    ]);
  }
  var print_default = print;

  // src/language-js/embed/utils.js
  var angularComponentObjectExpressionPredicates = [
    (node, name) => node.type === "ObjectExpression" && name === "properties",
    (node, name) => node.type === "CallExpression" && node.callee.type === "Identifier" && node.callee.name === "Component" && name === "arguments",
    (node, name) => node.type === "Decorator" && name === "expression"
  ];
  function isAngularComponentStyles(path) {
    return path.match(
      (node) => node.type === "TemplateLiteral",
      (node, name) => isArrayOrTupleExpression(node) && name === "elements",
      (node, name) => isObjectProperty(node) && node.key.type === "Identifier" && node.key.name === "styles" && name === "value",
      ...angularComponentObjectExpressionPredicates
    );
  }
  function isAngularComponentTemplate(path) {
    return path.match(
      (node) => node.type === "TemplateLiteral",
      (node, name) => isObjectProperty(node) && node.key.type === "Identifier" && node.key.name === "template" && name === "value",
      ...angularComponentObjectExpressionPredicates
    );
  }
  function hasLanguageComment(node, languageName) {
    return hasComment(
      node,
      CommentCheckFlags.Block | CommentCheckFlags.Leading,
      ({ value }) => value === ` ${languageName} `
    );
  }

  // src/language-js/embed/css.js
  function printEmbedCss(textToDoc, print3, path) {
    const { node } = path;
    const rawQuasis = node.quasis.map((q) => q.value.raw);
    let placeholderID = 0;
    const text = rawQuasis.reduce(
      (prevVal, currVal, idx) => idx === 0 ? currVal : prevVal + "@prettier-placeholder-" + placeholderID++ + "-id" + currVal,
      ""
    );
    const quasisDoc = textToDoc(text, { parser: "scss" });
    const expressionDocs = printTemplateExpressions(path, print3);
    const newDoc = replacePlaceholders(quasisDoc, expressionDocs);
    if (!newDoc) {
      throw new Error("Couldn't insert all the expressions");
    }
    return ["`", indent([hardline, newDoc]), softline, "`"];
  }
  function replacePlaceholders(quasisDoc, expressionDocs) {
    if (!is_non_empty_array_default(expressionDocs)) {
      return quasisDoc;
    }
    let replaceCounter = 0;
    const newDoc = mapDoc(cleanDoc(quasisDoc), (doc) => {
      if (typeof doc !== "string" || !doc.includes("@prettier-placeholder")) {
        return doc;
      }
      return doc.split(/@prettier-placeholder-(\d+)-id/).map((component, idx) => {
        if (idx % 2 === 0) {
          return replaceEndOfLine(component);
        }
        replaceCounter++;
        return expressionDocs[component];
      });
    });
    return expressionDocs.length === replaceCounter ? newDoc : null;
  }
  function isStyledJsx({ node, parent, grandparent }) {
    return grandparent && node.quasis && parent.type === "JSXExpressionContainer" && grandparent.type === "JSXElement" && grandparent.openingElement.name.name === "style" && grandparent.openingElement.attributes.some(
      (attribute) => attribute.name.name === "jsx"
    ) || (parent == null ? void 0 : parent.type) === "TaggedTemplateExpression" && parent.tag.type === "Identifier" && parent.tag.name === "css" || (parent == null ? void 0 : parent.type) === "TaggedTemplateExpression" && parent.tag.type === "MemberExpression" && parent.tag.object.name === "css" && (parent.tag.property.name === "global" || parent.tag.property.name === "resolve");
  }
  function isStyledIdentifier(node) {
    return node.type === "Identifier" && node.name === "styled";
  }
  function isStyledExtend(node) {
    return /^[A-Z]/.test(node.object.name) && node.property.name === "extend";
  }
  function isStyledComponents({ parent }) {
    if (!parent || parent.type !== "TaggedTemplateExpression") {
      return false;
    }
    const tag = parent.tag.type === "ParenthesizedExpression" ? parent.tag.expression : parent.tag;
    switch (tag.type) {
      case "MemberExpression":
        return (
          // styled.foo``
          isStyledIdentifier(tag.object) || // Component.extend``
          isStyledExtend(tag)
        );
      case "CallExpression":
        return (
          // styled(Component)``
          isStyledIdentifier(tag.callee) || tag.callee.type === "MemberExpression" && (tag.callee.object.type === "MemberExpression" && // styled.foo.attrs({})``
          (isStyledIdentifier(tag.callee.object.object) || // Component.extend.attrs({})``
          isStyledExtend(tag.callee.object)) || // styled(Component).attrs({})``
          tag.callee.object.type === "CallExpression" && isStyledIdentifier(tag.callee.object.callee))
        );
      case "Identifier":
        return tag.name === "css";
      default:
        return false;
    }
  }
  function isCssProp({ parent, grandparent }) {
    return (grandparent == null ? void 0 : grandparent.type) === "JSXAttribute" && parent.type === "JSXExpressionContainer" && grandparent.name.type === "JSXIdentifier" && grandparent.name.name === "css";
  }
  function printCss(path) {
    if (isStyledJsx(path) || isStyledComponents(path) || isCssProp(path) || isAngularComponentStyles(path)) {
      return printEmbedCss;
    }
  }
  var css_default = printCss;

  // src/language-js/embed/graphql.js
  function printEmbedGraphQL(textToDoc, print3, path) {
    const { node } = path;
    const numQuasis = node.quasis.length;
    const expressionDocs = printTemplateExpressions(path, print3);
    const parts = [];
    for (let i = 0; i < numQuasis; i++) {
      const templateElement = node.quasis[i];
      const isFirst = i === 0;
      const isLast = i === numQuasis - 1;
      const text = templateElement.value.cooked;
      const lines = text.split("\n");
      const numLines = lines.length;
      const expressionDoc = expressionDocs[i];
      const startsWithBlankLine = numLines > 2 && lines[0].trim() === "" && lines[1].trim() === "";
      const endsWithBlankLine = numLines > 2 && lines[numLines - 1].trim() === "" && lines[numLines - 2].trim() === "";
      const commentsAndWhitespaceOnly = lines.every(
        (line2) => /^\s*(?:#[^\n\r]*)?$/.test(line2)
      );
      if (!isLast && /#[^\n\r]*$/.test(lines[numLines - 1])) {
        return null;
      }
      let doc = null;
      if (commentsAndWhitespaceOnly) {
        doc = printGraphqlComments(lines);
      } else {
        doc = textToDoc(text, { parser: "graphql" });
      }
      if (doc) {
        doc = escapeTemplateCharacters(doc, false);
        if (!isFirst && startsWithBlankLine) {
          parts.push("");
        }
        parts.push(doc);
        if (!isLast && endsWithBlankLine) {
          parts.push("");
        }
      } else if (!isFirst && !isLast && startsWithBlankLine) {
        parts.push("");
      }
      if (expressionDoc) {
        parts.push(expressionDoc);
      }
    }
    return ["`", indent([hardline, join(hardline, parts)]), hardline, "`"];
  }
  function printGraphqlComments(lines) {
    const parts = [];
    let seenComment = false;
    const array = lines.map((textLine) => textLine.trim());
    for (const [i, textLine] of array.entries()) {
      if (textLine === "") {
        continue;
      }
      if (array[i - 1] === "" && seenComment) {
        parts.push([hardline, textLine]);
      } else {
        parts.push(textLine);
      }
      seenComment = true;
    }
    return parts.length === 0 ? null : join(hardline, parts);
  }
  function isGraphQL({ node, parent }) {
    return hasLanguageComment(node, "GraphQL") || parent && (parent.type === "TaggedTemplateExpression" && (parent.tag.type === "MemberExpression" && parent.tag.object.name === "graphql" && parent.tag.property.name === "experimental" || parent.tag.type === "Identifier" && (parent.tag.name === "gql" || parent.tag.name === "graphql")) || parent.type === "CallExpression" && parent.callee.type === "Identifier" && parent.callee.name === "graphql");
  }
  function printGraphql(path) {
    if (isGraphQL(path)) {
      return printEmbedGraphQL;
    }
  }
  var graphql_default = printGraphql;

  // src/language-js/embed/html.js
  var htmlTemplateLiteralCounter = 0;
  function printEmbedHtmlLike(parser, textToDoc, print3, path, options2) {
    const {
      node
    } = path;
    const counter = htmlTemplateLiteralCounter;
    htmlTemplateLiteralCounter = htmlTemplateLiteralCounter + 1 >>> 0;
    const composePlaceholder = (index) => `PRETTIER_HTML_PLACEHOLDER_${index}_${counter}_IN_JS`;
    const text = node.quasis.map((quasi, index, quasis) => index === quasis.length - 1 ? quasi.value.cooked : quasi.value.cooked + composePlaceholder(index)).join("");
    const expressionDocs = printTemplateExpressions(path, print3);
    const placeholderRegex = new RegExp(composePlaceholder("(\\d+)"), "g");
    let topLevelCount = 0;
    const doc = textToDoc(text, {
      parser,
      __onHtmlRoot(root) {
        topLevelCount = root.children.length;
      }
    });
    const contentDoc = mapDoc(doc, (doc2) => {
      if (typeof doc2 !== "string") {
        return doc2;
      }
      const parts = [];
      const components = doc2.split(placeholderRegex);
      for (let i = 0; i < components.length; i++) {
        let component = components[i];
        if (i % 2 === 0) {
          if (component) {
            component = uncookTemplateElementValue(component);
            if (options2.__embeddedInHtml) {
              component = string_replace_all_default(
                /* isOptionalObject*/
                false,
                component,
                /<\/(?=script\b)/gi,
                "<\\/"
              );
            }
            parts.push(component);
          }
          continue;
        }
        const placeholderIndex = Number(component);
        parts.push(expressionDocs[placeholderIndex]);
      }
      return parts;
    });
    const leadingWhitespace = /^\s/.test(text) ? " " : "";
    const trailingWhitespace = /\s$/.test(text) ? " " : "";
    const linebreak = options2.htmlWhitespaceSensitivity === "ignore" ? hardline : leadingWhitespace && trailingWhitespace ? line : null;
    if (linebreak) {
      return group(["`", indent([linebreak, group(contentDoc)]), linebreak, "`"]);
    }
    return label({
      hug: false
    }, group(["`", leadingWhitespace, topLevelCount > 1 ? indent(group(contentDoc)) : group(contentDoc), trailingWhitespace, "`"]));
  }
  function isHtml(path) {
    return hasLanguageComment(path.node, "HTML") || path.match((node) => node.type === "TemplateLiteral", (node, name) => node.type === "TaggedTemplateExpression" && node.tag.type === "Identifier" && node.tag.name === "html" && name === "quasi");
  }
  var printEmbedHtml = printEmbedHtmlLike.bind(void 0, "html");
  var printEmbedAngular = printEmbedHtmlLike.bind(void 0, "angular");
  function printHtml(path) {
    if (isHtml(path)) {
      return printEmbedHtml;
    }
    if (isAngularComponentTemplate(path)) {
      return printEmbedAngular;
    }
  }
  var html_default = printHtml;

  // src/language-js/embed/markdown.js
  function printEmbedMarkdown(textToDoc, print3, path) {
    const {
      node
    } = path;
    let text = string_replace_all_default(
      /* isOptionalObject*/
      false,
      node.quasis[0].value.raw,
      /((?:\\\\)*)\\`/g,
      (_, backslashes) => "\\".repeat(backslashes.length / 2) + "`"
    );
    const indentation = getIndentation(text);
    const hasIndent = indentation !== "";
    if (hasIndent) {
      text = string_replace_all_default(
        /* isOptionalObject*/
        false,
        text,
        new RegExp(`^${indentation}`, "gm"),
        ""
      );
    }
    const doc = escapeTemplateCharacters(textToDoc(text, {
      parser: "markdown",
      __inJsTemplate: true
    }), true);
    return ["`", hasIndent ? indent([softline, doc]) : [literalline, dedentToRoot(doc)], softline, "`"];
  }
  function getIndentation(str) {
    const firstMatchedIndent = str.match(/^([^\S\n]*)\S/m);
    return firstMatchedIndent === null ? "" : firstMatchedIndent[1];
  }
  function printMarkdown(path) {
    if (isMarkdown(path)) {
      return printEmbedMarkdown;
    }
  }
  function isMarkdown({
    node,
    parent
  }) {
    return (parent == null ? void 0 : parent.type) === "TaggedTemplateExpression" && node.quasis.length === 1 && parent.tag.type === "Identifier" && (parent.tag.name === "md" || parent.tag.name === "markdown");
  }
  var markdown_default = printMarkdown;

  // src/language-js/embed/index.js
  function embed(path) {
    const { node } = path;
    if (node.type !== "TemplateLiteral" || // Bail out if any of the quasis have an invalid escape sequence
    // (which would make the `cooked` value be `null`)
    hasInvalidCookedValue(node)) {
      return;
    }
    let embedder;
    for (const getEmbedder of [
      css_default,
      graphql_default,
      html_default,
      markdown_default
    ]) {
      embedder = getEmbedder(path);
      if (!embedder) {
        continue;
      }
      if (node.quasis.length === 1 && node.quasis[0].value.raw.trim() === "") {
        return "``";
      }
      return (...args) => {
        const doc = embedder(...args);
        return doc && label({ embed: true, ...doc.label }, doc);
      };
    }
  }
  function hasInvalidCookedValue({ quasis }) {
    return quasis.some(({ value: { cooked } }) => cooked === null);
  }
  var embed_default = embed;

  // src/language-js/clean.js
  var ignoredProperties = /* @__PURE__ */ new Set(["range", "raw", "comments", "leadingComments", "trailingComments", "innerComments", "extra", "start", "end", "loc", "flags", "errors", "tokens"]);
  var removeTemplateElementsValue = (node) => {
    for (const templateElement of node.quasis) {
      delete templateElement.value;
    }
  };
  function clean(ast, newObj, parent) {
    var _a, _b;
    if (ast.type === "Program") {
      delete newObj.sourceType;
    }
    if ((ast.type === "BigIntLiteral" || ast.type === "BigIntLiteralTypeAnnotation") && newObj.value) {
      newObj.value = newObj.value.toLowerCase();
    }
    if ((ast.type === "BigIntLiteral" || ast.type === "Literal") && newObj.bigint) {
      newObj.bigint = newObj.bigint.toLowerCase();
    }
    if (ast.type === "DecimalLiteral") {
      newObj.value = Number(newObj.value);
    }
    if (ast.type === "Literal" && newObj.decimal) {
      newObj.decimal = Number(newObj.decimal);
    }
    if (ast.type === "EmptyStatement") {
      return null;
    }
    if (ast.type === "JSXText") {
      return null;
    }
    if (ast.type === "JSXExpressionContainer" && (ast.expression.type === "Literal" || ast.expression.type === "StringLiteral") && ast.expression.value === " ") {
      return null;
    }
    if ((ast.type === "Property" || ast.type === "ObjectProperty" || ast.type === "MethodDefinition" || ast.type === "ClassProperty" || ast.type === "ClassMethod" || ast.type === "PropertyDefinition" || ast.type === "TSDeclareMethod" || ast.type === "TSPropertySignature" || ast.type === "ObjectTypeProperty") && typeof ast.key === "object" && ast.key && (ast.key.type === "Literal" || ast.key.type === "NumericLiteral" || ast.key.type === "StringLiteral" || ast.key.type === "Identifier")) {
      delete newObj.key;
    }
    if (ast.type === "JSXElement" && ast.openingElement.name.name === "style" && ast.openingElement.attributes.some((attr) => attr.type === "JSXAttribute" && attr.name.name === "jsx")) {
      for (const {
        type,
        expression: expression2
      } of newObj.children) {
        if (type === "JSXExpressionContainer" && expression2.type === "TemplateLiteral") {
          removeTemplateElementsValue(expression2);
        }
      }
    }
    if (ast.type === "JSXAttribute" && ast.name.name === "css" && ast.value.type === "JSXExpressionContainer" && ast.value.expression.type === "TemplateLiteral") {
      removeTemplateElementsValue(newObj.value.expression);
    }
    if (ast.type === "JSXAttribute" && ((_a = ast.value) == null ? void 0 : _a.type) === "Literal" && /["']|&quot;|&apos;/.test(ast.value.value)) {
      newObj.value.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        newObj.value.value,
        /["']|&quot;|&apos;/g,
        '"'
      );
    }
    const expression = ast.expression || ast.callee;
    if (ast.type === "Decorator" && expression.type === "CallExpression" && expression.callee.name === "Component" && expression.arguments.length === 1) {
      const astProps = ast.expression.arguments[0].properties;
      for (const [index, prop] of newObj.expression.arguments[0].properties.entries()) {
        switch (astProps[index].key.name) {
          case "styles":
            if (isArrayOrTupleExpression(prop.value)) {
              removeTemplateElementsValue(prop.value.elements[0]);
            }
            break;
          case "template":
            if (prop.value.type === "TemplateLiteral") {
              removeTemplateElementsValue(prop.value);
            }
            break;
        }
      }
    }
    if (ast.type === "TaggedTemplateExpression" && (ast.tag.type === "MemberExpression" || ast.tag.type === "Identifier" && (ast.tag.name === "gql" || ast.tag.name === "graphql" || ast.tag.name === "css" || ast.tag.name === "md" || ast.tag.name === "markdown" || ast.tag.name === "html") || ast.tag.type === "CallExpression")) {
      removeTemplateElementsValue(newObj.quasi);
    }
    if (ast.type === "TemplateLiteral") {
      const hasLanguageComment2 = (_b = ast.leadingComments) == null ? void 0 : _b.some((comment) => is_block_comment_default(comment) && ["GraphQL", "HTML"].some((languageName) => comment.value === ` ${languageName} `));
      if (hasLanguageComment2 || parent.type === "CallExpression" && parent.callee.name === "graphql" || // TODO: check parser
      // `flow` and `typescript` don't have `leadingComments`
      !ast.leadingComments) {
        removeTemplateElementsValue(newObj);
      }
    }
    if ((ast.type === "TSIntersectionType" || ast.type === "TSUnionType") && ast.types.length === 1) {
      return newObj.types[0];
    }
    if (ast.type === "ChainExpression" && ast.expression.type === "TSNonNullExpression") {
      [newObj.type, newObj.expression.type] = [newObj.expression.type, newObj.type];
    }
  }
  clean.ignoredProperties = ignoredProperties;
  var clean_default = clean;

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

  // src/language-js/printer.js
  var experimentalFeatures = {
    // TODO: Make this default behavior
    avoidAstMutation: true
  };

  // src/language-js/languages.evaluate.js
  var languages_evaluate_default = [
    {
      "linguistLanguageId": 183,
      "name": "JavaScript",
      "type": "programming",
      "tmScope": "source.js",
      "aceMode": "javascript",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "text/javascript",
      "color": "#f1e05a",
      "aliases": [
        "js",
        "node"
      ],
      "extensions": [
        ".js",
        "._js",
        ".bones",
        ".cjs",
        ".es",
        ".es6",
        ".frag",
        ".gs",
        ".jake",
        ".javascript",
        ".jsb",
        ".jscad",
        ".jsfl",
        ".jslib",
        ".jsm",
        ".jspre",
        ".jss",
        ".mjs",
        ".njs",
        ".pac",
        ".sjs",
        ".ssjs",
        ".xsjs",
        ".xsjslib",
        ".wxs"
      ],
      "filenames": [
        "Jakefile"
      ],
      "interpreters": [
        "chakra",
        "d8",
        "gjs",
        "js",
        "node",
        "nodejs",
        "qjs",
        "rhino",
        "v8",
        "v8-shell",
        "zx"
      ],
      "parsers": [
        "babel",
        "acorn",
        "espree",
        "meriyah",
        "babel-flow",
        "babel-ts",
        "flow",
        "typescript"
      ],
      "vscodeLanguageIds": [
        "javascript",
        "mongo"
      ]
    },
    {
      "linguistLanguageId": 183,
      "name": "Flow",
      "type": "programming",
      "tmScope": "source.js",
      "aceMode": "javascript",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "text/javascript",
      "color": "#f1e05a",
      "aliases": [],
      "extensions": [
        ".js.flow"
      ],
      "filenames": [],
      "interpreters": [
        "chakra",
        "d8",
        "gjs",
        "js",
        "node",
        "nodejs",
        "qjs",
        "rhino",
        "v8",
        "v8-shell"
      ],
      "parsers": [
        "flow",
        "babel-flow"
      ],
      "vscodeLanguageIds": [
        "javascript"
      ]
    },
    {
      "linguistLanguageId": 183,
      "name": "JSX",
      "type": "programming",
      "tmScope": "source.js.jsx",
      "aceMode": "javascript",
      "codemirrorMode": "jsx",
      "codemirrorMimeType": "text/jsx",
      "color": void 0,
      "aliases": void 0,
      "extensions": [
        ".jsx"
      ],
      "filenames": void 0,
      "interpreters": void 0,
      "parsers": [
        "babel",
        "babel-flow",
        "babel-ts",
        "flow",
        "typescript",
        "espree",
        "meriyah"
      ],
      "vscodeLanguageIds": [
        "javascriptreact"
      ],
      "group": "JavaScript"
    },
    {
      "linguistLanguageId": 378,
      "name": "TypeScript",
      "type": "programming",
      "color": "#3178c6",
      "aliases": [
        "ts"
      ],
      "interpreters": [
        "deno",
        "ts-node"
      ],
      "extensions": [
        ".ts",
        ".cts",
        ".mts"
      ],
      "tmScope": "source.ts",
      "aceMode": "typescript",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "application/typescript",
      "parsers": [
        "typescript",
        "babel-ts"
      ],
      "vscodeLanguageIds": [
        "typescript"
      ]
    },
    {
      "linguistLanguageId": 94901924,
      "name": "TSX",
      "type": "programming",
      "color": "#3178c6",
      "group": "TypeScript",
      "extensions": [
        ".tsx"
      ],
      "tmScope": "source.tsx",
      "aceMode": "javascript",
      "codemirrorMode": "jsx",
      "codemirrorMimeType": "text/jsx",
      "parsers": [
        "typescript",
        "babel-ts"
      ],
      "vscodeLanguageIds": [
        "typescriptreact"
      ]
    }
  ];

  // src/language-json/printer-estree-json.js
  var printer_estree_json_exports = {};
  __export(printer_estree_json_exports, {
    getVisitorKeys: () => get_visitor_keys_default2,
    massageAstNode: () => clean2,
    print: () => genericPrint
  });

  // src/language-json/visitor-keys.js
  var visitorKeys = {
    JsonRoot: ["node"],
    ArrayExpression: ["elements"],
    ObjectExpression: ["properties"],
    ObjectProperty: ["key", "value"],
    UnaryExpression: ["argument"],
    NullLiteral: [],
    BooleanLiteral: [],
    StringLiteral: [],
    NumericLiteral: [],
    Identifier: [],
    TemplateLiteral: ["quasis"],
    TemplateElement: []
  };
  var visitor_keys_default = visitorKeys;

  // src/language-json/get-visitor-keys.js
  var getVisitorKeys2 = create_get_visitor_keys_default(visitor_keys_default);
  var get_visitor_keys_default2 = getVisitorKeys2;

  // src/language-json/printer-estree-json.js
  function genericPrint(path, options2, print3) {
    const { node } = path;
    switch (node.type) {
      case "JsonRoot":
        return [print3("node"), hardline];
      case "ArrayExpression": {
        if (node.elements.length === 0) {
          return "[]";
        }
        const printed = path.map(
          () => path.node === null ? "null" : print3(),
          "elements"
        );
        return [
          "[",
          indent([hardline, join([",", hardline], printed)]),
          hardline,
          "]"
        ];
      }
      case "ObjectExpression":
        return node.properties.length === 0 ? "{}" : [
          "{",
          indent([
            hardline,
            join([",", hardline], path.map(print3, "properties"))
          ]),
          hardline,
          "}"
        ];
      case "ObjectProperty":
        return [print3("key"), ": ", print3("value")];
      case "UnaryExpression":
        return [node.operator === "+" ? "" : node.operator, print3("argument")];
      case "NullLiteral":
        return "null";
      case "BooleanLiteral":
        return node.value ? "true" : "false";
      case "StringLiteral":
        return JSON.stringify(node.value);
      case "NumericLiteral":
        return isObjectKey(path) ? JSON.stringify(String(node.value)) : JSON.stringify(node.value);
      case "Identifier":
        return isObjectKey(path) ? JSON.stringify(node.name) : node.name;
      case "TemplateLiteral":
        return print3(["quasis", 0]);
      case "TemplateElement":
        return JSON.stringify(node.value.cooked);
      default:
        throw new unexpected_node_error_default(node, "JSON");
    }
  }
  function isObjectKey(path) {
    return path.key === "key" && path.parent.type === "ObjectProperty";
  }
  var ignoredProperties2 = /* @__PURE__ */ new Set([
    "start",
    "end",
    "extra",
    "loc",
    "comments",
    "leadingComments",
    "trailingComments",
    "innerComments",
    "errors",
    "range",
    "tokens"
  ]);
  function clean2(node, newNode) {
    const { type } = node;
    if (type === "ObjectProperty") {
      const { key } = node;
      if (key.type === "Identifier") {
        newNode.key = { type: "StringLiteral", value: key.name };
      } else if (key.type === "NumericLiteral") {
        newNode.key = { type: "StringLiteral", value: String(key.value) };
      }
      return;
    }
    if (type === "UnaryExpression" && node.operator === "+") {
      return newNode.argument;
    }
    if (type === "ArrayExpression") {
      for (const [index, element] of node.elements.entries()) {
        if (element === null) {
          newNode.elements.splice(index, 0, { type: "NullLiteral" });
        }
      }
      return;
    }
    if (type === "TemplateLiteral") {
      return { type: "StringLiteral", value: node.quasis[0].value.cooked };
    }
  }
  clean2.ignoredProperties = ignoredProperties2;

  // src/language-json/languages.evaluate.js
  var languages_evaluate_default2 = [
    {
      "linguistLanguageId": 174,
      "name": "JSON.stringify",
      "type": "data",
      "color": "#292929",
      "tmScope": "source.json",
      "aceMode": "json",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "application/json",
      "aliases": [
        "geojson",
        "jsonl",
        "topojson"
      ],
      "extensions": [
        ".importmap"
      ],
      "filenames": [
        "package.json",
        "package-lock.json",
        "composer.json"
      ],
      "parsers": [
        "json-stringify"
      ],
      "vscodeLanguageIds": [
        "json"
      ]
    },
    {
      "linguistLanguageId": 174,
      "name": "JSON",
      "type": "data",
      "color": "#292929",
      "tmScope": "source.json",
      "aceMode": "json",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "application/json",
      "aliases": [
        "geojson",
        "jsonl",
        "topojson"
      ],
      "extensions": [
        ".json",
        ".4DForm",
        ".4DProject",
        ".avsc",
        ".geojson",
        ".gltf",
        ".har",
        ".ice",
        ".JSON-tmLanguage",
        ".mcmeta",
        ".tfstate",
        ".tfstate.backup",
        ".topojson",
        ".webapp",
        ".webmanifest",
        ".yy",
        ".yyp"
      ],
      "filenames": [
        ".all-contributorsrc",
        ".arcconfig",
        ".auto-changelog",
        ".c8rc",
        ".htmlhintrc",
        ".imgbotconfig",
        ".nycrc",
        ".tern-config",
        ".tern-project",
        ".watchmanconfig",
        "Pipfile.lock",
        "composer.lock",
        "flake.lock",
        "mcmod.info"
      ],
      "parsers": [
        "json"
      ],
      "vscodeLanguageIds": [
        "json"
      ]
    },
    {
      "linguistLanguageId": 423,
      "name": "JSON with Comments",
      "type": "data",
      "color": "#292929",
      "group": "JSON",
      "tmScope": "source.js",
      "aceMode": "javascript",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "text/javascript",
      "aliases": [
        "jsonc"
      ],
      "extensions": [
        ".jsonc",
        ".code-snippets",
        ".sublime-build",
        ".sublime-commands",
        ".sublime-completions",
        ".sublime-keymap",
        ".sublime-macro",
        ".sublime-menu",
        ".sublime-mousemap",
        ".sublime-project",
        ".sublime-settings",
        ".sublime-theme",
        ".sublime-workspace",
        ".sublime_metrics",
        ".sublime_session"
      ],
      "filenames": [
        ".babelrc",
        ".devcontainer.json",
        ".eslintrc.json",
        ".jscsrc",
        ".jshintrc",
        ".jslintrc",
        ".swcrc",
        "api-extractor.json",
        "devcontainer.json",
        "jsconfig.json",
        "language-configuration.json",
        "tsconfig.json",
        "tslint.json",
        ".eslintrc"
      ],
      "parsers": [
        "json"
      ],
      "vscodeLanguageIds": [
        "jsonc"
      ]
    },
    {
      "linguistLanguageId": 175,
      "name": "JSON5",
      "type": "data",
      "color": "#267CB9",
      "extensions": [
        ".json5"
      ],
      "tmScope": "source.js",
      "aceMode": "javascript",
      "codemirrorMode": "javascript",
      "codemirrorMimeType": "application/json",
      "parsers": [
        "json5"
      ],
      "vscodeLanguageIds": [
        "json5"
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

  // src/language-js/options.js
  var CATEGORY_JAVASCRIPT = "JavaScript";
  var options = {
    arrowParens: {
      category: CATEGORY_JAVASCRIPT,
      type: "choice",
      default: "always",
      description: "Include parentheses around a sole arrow function parameter.",
      choices: [
        {
          value: "always",
          description: "Always include parens. Example: `(x) => x`"
        },
        {
          value: "avoid",
          description: "Omit parens when possible. Example: `x => x`"
        }
      ]
    },
    bracketSameLine: common_options_evaluate_default.bracketSameLine,
    bracketSpacing: common_options_evaluate_default.bracketSpacing,
    jsxBracketSameLine: {
      category: CATEGORY_JAVASCRIPT,
      type: "boolean",
      description: "Put > on the last line instead of at a new line.",
      deprecated: "2.4.0"
    },
    semi: {
      category: CATEGORY_JAVASCRIPT,
      type: "boolean",
      default: true,
      description: "Print semicolons.",
      oppositeDescription: "Do not print semicolons, except at the beginning of lines which may need them."
    },
    singleQuote: common_options_evaluate_default.singleQuote,
    jsxSingleQuote: {
      category: CATEGORY_JAVASCRIPT,
      type: "boolean",
      default: false,
      description: "Use single quotes in JSX."
    },
    quoteProps: {
      category: CATEGORY_JAVASCRIPT,
      type: "choice",
      default: "as-needed",
      description: "Change when properties in objects are quoted.",
      choices: [
        {
          value: "as-needed",
          description: "Only add quotes around object properties where required."
        },
        {
          value: "consistent",
          description: "If at least one property in an object requires quotes, quote all properties."
        },
        {
          value: "preserve",
          description: "Respect the input use of quotes in object properties."
        }
      ]
    },
    trailingComma: {
      category: CATEGORY_JAVASCRIPT,
      type: "choice",
      default: "all",
      description: "Print trailing commas wherever possible when multi-line.",
      choices: [
        {
          value: "all",
          description: "Trailing commas wherever possible (including function arguments)."
        },
        {
          value: "es5",
          description: "Trailing commas where valid in ES5 (objects, arrays, etc.)"
        },
        { value: "none", description: "No trailing commas." }
      ]
    },
    singleAttributePerLine: common_options_evaluate_default.singleAttributePerLine
  };
  var options_default = options;

  // src/plugins/estree.js
  var printers = {
    estree: printer_exports,
    "estree-json": printer_estree_json_exports
  };
  var languages = [...languages_evaluate_default, ...languages_evaluate_default2];
  return __toCommonJS(estree_exports);
});