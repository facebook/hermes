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
})(function () {
  "use strict";
  var __defProp = Object.defineProperty;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
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
  var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

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

  // src/language-js/loc.js
  function locStart(node) {
    var _a, _b, _c;
    const start = ((_a = node.range) == null ? void 0 : _a[0]) ?? node.start;
    if (false) {
      throw new TypeError("Can't not locate node.");
    }
    const firstDecorator = (_c = ((_b = node.declaration) == null ? void 0 : _b.decorators) ?? node.decorators) == null ? void 0 : _c[0];
    if (firstDecorator) {
      return Math.min(locStart(firstDecorator), start);
    }
    return start;
  }
  function locEnd(node) {
    var _a;
    const end = ((_a = node.range) == null ? void 0 : _a[1]) ?? node.end;
    if (false) {
      throw new TypeError("Can't not locate node.");
    }
    return end;
  }

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

  // node_modules/jest-docblock/build/index.js
  var commentEndRe = /\*\/$/;
  var commentStartRe = /^\/\*\*?/;
  var docblockRe = /^\s*(\/\*\*?(.|\r?\n)*?\*\/)/;
  var lineCommentRe = /(^|\s+)\/\/([^\n\r]*)/g;
  var ltrimNewlineRe = /^(\r?\n)+/;
  var multilineRe = /(?:^|\r?\n) *(@[^\n\r]*?) *\r?\n *(?![^\n\r@]*\/\/[^]*)([^\s@][^\n\r@]+?) *\r?\n/g;
  var propertyRe = /(?:^|\r?\n) *@(\S+) *([^\n\r]*)/g;
  var stringStartRe = /(\r?\n|^) *\* ?/g;
  var STRING_ARRAY = [];
  function extract(contents) {
    const match = contents.match(docblockRe);
    return match ? match[0].trimStart() : "";
  }
  function parseWithComments(docblock) {
    const line = "\n";
    docblock = string_replace_all_default(
      /* isOptionalObject */
      false,
      docblock.replace(commentStartRe, "").replace(commentEndRe, ""),
      stringStartRe,
      "$1"
    );
    let prev = "";
    while (prev !== docblock) {
      prev = docblock;
      docblock = string_replace_all_default(
        /* isOptionalObject */
        false,
        docblock,
        multilineRe,
        `${line}$1 $2${line}`
      );
    }
    docblock = docblock.replace(ltrimNewlineRe, "").trimEnd();
    const result = /* @__PURE__ */ Object.create(null);
    const comments = string_replace_all_default(
      /* isOptionalObject */
      false,
      docblock,
      propertyRe,
      ""
    ).replace(ltrimNewlineRe, "").trimEnd();
    let match;
    while (match = propertyRe.exec(docblock)) {
      const nextPragma = string_replace_all_default(
        /* isOptionalObject */
        false,
        match[2],
        lineCommentRe,
        ""
      );
      if (typeof result[match[1]] === "string" || Array.isArray(result[match[1]])) {
        const resultElement = result[match[1]];
        result[match[1]] = [...STRING_ARRAY, ...Array.isArray(resultElement) ? resultElement : [resultElement], nextPragma];
      } else {
        result[match[1]] = nextPragma;
      }
    }
    return {
      comments,
      pragmas: result
    };
  }

  // src/utils/pragma/pragma.evaluate.js
  var FORMAT_IGNORE_PRAGMAS = [
    "noformat",
    "noprettier"
  ];
  var FORMAT_PRAGMAS = [
    "format",
    "prettier"
  ];

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
    const docBlock = extract(text);
    const { pragmas, comments } = parseWithComments(docBlock);
    return { shebang, text, pragmas, comments };
  }
  function hasPragma(text) {
    const { pragmas } = parseDocBlock(text);
    return FORMAT_PRAGMAS.some((pragma) => Object.prototype.hasOwnProperty.call(pragmas, pragma));
  }
  function hasIgnorePragma(text) {
    const { pragmas } = parseDocBlock(text);
    return FORMAT_IGNORE_PRAGMAS.some((pragma) => Object.prototype.hasOwnProperty.call(pragmas, pragma));
  }

  // src/language-js/parse/utils/create-parser.js
  function createParser(options) {
    options = typeof options === "function" ? { parse: options } : options;
    return {
      astFormat: "estree",
      hasPragma,
      hasIgnorePragma,
      locStart,
      locEnd,
      ...options
    };
  }
  var create_parser_default = createParser;

  // src/language-js/parse/flow.js
  function parse(text) {
    void text;
  }
  var flow = create_parser_default(parse);
  return __toCommonJS(flow_exports2);
});