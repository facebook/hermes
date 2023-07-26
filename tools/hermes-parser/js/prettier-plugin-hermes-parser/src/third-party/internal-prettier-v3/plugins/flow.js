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
    root.prettierPlugins.flow = interopModuleDefault();
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
      exports.parse = parse2;
      exports.parseWithComments = parseWithComments2;
      exports.print = print2;
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
      function parse2(docblock) {
        return parseWithComments2(docblock).pragmas;
      }
      function parseWithComments2(docblock) {
        const line = "\n";
        docblock = docblock.replace(commentStartRe, "").replace(commentEndRe, "").replace(stringStartRe, "$1");
        let prev = "";
        while (prev !== docblock) {
          prev = docblock;
          docblock = docblock.replace(multilineRe, `${line}$1 $2${line}`);
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
      function print2({ comments = "", pragmas = {} }) {
        const line = "\n";
        const head = "/**";
        const start = " *";
        const tail = " */";
        const keys = Object.keys(pragmas);
        const printedObject = keys.map((key) => printKeyValues(key, pragmas[key])).reduce((arr, next) => arr.concat(next), []).map((keyValue) => `${start} ${keyValue}${line}`).join("");
        if (!comments) {
          if (keys.length === 0) {
            return "";
          }
          if (keys.length === 1 && !Array.isArray(pragmas[keys[0]])) {
            const value = pragmas[keys[0]];
            return `${head} ${printKeyValues(keys[0], value)[0]}${tail}`;
          }
        }
        const printedComments = comments.split(line).map((textLine) => `${start} ${textLine}`).join(line) + line;
        return head + line + (comments ? printedComments : "") + (comments && keys.length ? start + line : "") + printedObject + tail;
      }
      function printKeyValues(key, valueOrArray) {
        return STRING_ARRAY.concat(valueOrArray).map(
          (value) => `@${key} ${value}`.trim()
        );
      }
    }
  });

  // src/plugins/flow.js
  var flow_exports2 = {};
  __export(flow_exports2, {
    parsers: () => flow_exports
  });

  // src/language-js/parse/flow.js
  var flow_exports = {};
  __export(flow_exports, {
    flow: () => flow
  });

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

  // src/utils/is-non-empty-array.js
  function isNonEmptyArray(object) {
    return Array.isArray(object) && object.length > 0;
  }
  var is_non_empty_array_default = isNonEmptyArray;

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

  // src/language-js/parse/utils/create-parser.js
  function createParser(options) {
    options = typeof options === "function" ? { parse: options } : options;
    return {
      astFormat: "estree",
      hasPragma,
      locStart,
      locEnd,
      ...options
    };
  }
  var create_parser_default = createParser;

  // src/language-js/parse/flow.js
  function parse(text) {
  }
  var flow = create_parser_default(parse);
  return __toCommonJS(flow_exports2);
});