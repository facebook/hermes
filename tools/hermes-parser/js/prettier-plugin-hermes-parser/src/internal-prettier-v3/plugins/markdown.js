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
    root.prettierPlugins.markdown = interopModuleDefault();
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

  // node_modules/collapse-white-space/index.js
  var require_collapse_white_space = __commonJS({
    "node_modules/collapse-white-space/index.js"(exports, module) {
      "use strict";
      module.exports = collapse;
      function collapse(value) {
        return String(value).replace(/\s+/g, " ");
      }
    }
  });

  // node_modules/xtend/immutable.js
  var require_immutable = __commonJS({
    "node_modules/xtend/immutable.js"(exports, module) {
      module.exports = extend;
      var hasOwnProperty = Object.prototype.hasOwnProperty;
      function extend() {
        var target = {};
        for (var i = 0; i < arguments.length; i++) {
          var source = arguments[i];
          for (var key in source) {
            if (hasOwnProperty.call(source, key)) {
              target[key] = source[key];
            }
          }
        }
        return target;
      }
    }
  });

  // node_modules/inherits/inherits_browser.js
  var require_inherits_browser = __commonJS({
    "node_modules/inherits/inherits_browser.js"(exports, module) {
      if (typeof Object.create === "function") {
        module.exports = function inherits(ctor, superCtor) {
          if (superCtor) {
            ctor.super_ = superCtor;
            ctor.prototype = Object.create(superCtor.prototype, {
              constructor: {
                value: ctor,
                enumerable: false,
                writable: true,
                configurable: true
              }
            });
          }
        };
      } else {
        module.exports = function inherits(ctor, superCtor) {
          if (superCtor) {
            ctor.super_ = superCtor;
            var TempCtor = function() {
            };
            TempCtor.prototype = superCtor.prototype;
            ctor.prototype = new TempCtor();
            ctor.prototype.constructor = ctor;
          }
        };
      }
    }
  });

  // node_modules/unherit/index.js
  var require_unherit = __commonJS({
    "node_modules/unherit/index.js"(exports, module) {
      "use strict";
      var xtend = require_immutable();
      var inherits = require_inherits_browser();
      module.exports = unherit;
      function unherit(Super) {
        var result;
        var key;
        var value;
        inherits(Of, Super);
        inherits(From, Of);
        result = Of.prototype;
        for (key in result) {
          value = result[key];
          if (value && typeof value === "object") {
            result[key] = "concat" in value ? value.concat() : xtend(value);
          }
        }
        return Of;
        function From(parameters) {
          return Super.apply(this, parameters);
        }
        function Of() {
          if (!(this instanceof Of)) {
            return new From(arguments);
          }
          return Super.apply(this, arguments);
        }
      }
    }
  });

  // node_modules/state-toggle/index.js
  var require_state_toggle = __commonJS({
    "node_modules/state-toggle/index.js"(exports, module) {
      "use strict";
      module.exports = factory;
      function factory(key, state, ctx) {
        return enter;
        function enter() {
          var context = ctx || this;
          var current = context[key];
          context[key] = !state;
          return exit;
          function exit() {
            context[key] = current;
          }
        }
      }
    }
  });

  // node_modules/vfile-location/index.js
  var require_vfile_location = __commonJS({
    "node_modules/vfile-location/index.js"(exports, module) {
      "use strict";
      module.exports = factory;
      function factory(file) {
        var value = String(file);
        var indices = [];
        var search = /\r?\n|\r/g;
        while (search.exec(value)) {
          indices.push(search.lastIndex);
        }
        indices.push(value.length + 1);
        return {
          toPoint: offsetToPoint,
          toPosition: offsetToPoint,
          toOffset: pointToOffset
        };
        function offsetToPoint(offset) {
          var index = -1;
          if (offset > -1 && offset < indices[indices.length - 1]) {
            while (++index < indices.length) {
              if (indices[index] > offset) {
                return {
                  line: index + 1,
                  column: offset - (indices[index - 1] || 0) + 1,
                  offset
                };
              }
            }
          }
          return {};
        }
        function pointToOffset(point) {
          var line2 = point && point.line;
          var column = point && point.column;
          var offset;
          if (!isNaN(line2) && !isNaN(column) && line2 - 1 in indices) {
            offset = (indices[line2 - 2] || 0) + column - 1 || 0;
          }
          return offset > -1 && offset < indices[indices.length - 1] ? offset : -1;
        }
      }
    }
  });

  // node_modules/remark-parse/lib/unescape.js
  var require_unescape = __commonJS({
    "node_modules/remark-parse/lib/unescape.js"(exports, module) {
      "use strict";
      module.exports = factory;
      var backslash = "\\";
      function factory(ctx, key) {
        return unescape;
        function unescape(value) {
          var previous = 0;
          var index = value.indexOf(backslash);
          var escape = ctx[key];
          var queue = [];
          var character;
          while (index !== -1) {
            queue.push(value.slice(previous, index));
            previous = index + 1;
            character = value.charAt(previous);
            if (!character || escape.indexOf(character) === -1) {
              queue.push(backslash);
            }
            index = value.indexOf(backslash, previous + 1);
          }
          queue.push(value.slice(previous));
          return queue.join("");
        }
      }
    }
  });

  // node_modules/character-entities-legacy/index.json
  var require_character_entities_legacy = __commonJS({
    "node_modules/character-entities-legacy/index.json"(exports, module) {
      module.exports = {
        AElig: "\xC6",
        AMP: "&",
        Aacute: "\xC1",
        Acirc: "\xC2",
        Agrave: "\xC0",
        Aring: "\xC5",
        Atilde: "\xC3",
        Auml: "\xC4",
        COPY: "\xA9",
        Ccedil: "\xC7",
        ETH: "\xD0",
        Eacute: "\xC9",
        Ecirc: "\xCA",
        Egrave: "\xC8",
        Euml: "\xCB",
        GT: ">",
        Iacute: "\xCD",
        Icirc: "\xCE",
        Igrave: "\xCC",
        Iuml: "\xCF",
        LT: "<",
        Ntilde: "\xD1",
        Oacute: "\xD3",
        Ocirc: "\xD4",
        Ograve: "\xD2",
        Oslash: "\xD8",
        Otilde: "\xD5",
        Ouml: "\xD6",
        QUOT: '"',
        REG: "\xAE",
        THORN: "\xDE",
        Uacute: "\xDA",
        Ucirc: "\xDB",
        Ugrave: "\xD9",
        Uuml: "\xDC",
        Yacute: "\xDD",
        aacute: "\xE1",
        acirc: "\xE2",
        acute: "\xB4",
        aelig: "\xE6",
        agrave: "\xE0",
        amp: "&",
        aring: "\xE5",
        atilde: "\xE3",
        auml: "\xE4",
        brvbar: "\xA6",
        ccedil: "\xE7",
        cedil: "\xB8",
        cent: "\xA2",
        copy: "\xA9",
        curren: "\xA4",
        deg: "\xB0",
        divide: "\xF7",
        eacute: "\xE9",
        ecirc: "\xEA",
        egrave: "\xE8",
        eth: "\xF0",
        euml: "\xEB",
        frac12: "\xBD",
        frac14: "\xBC",
        frac34: "\xBE",
        gt: ">",
        iacute: "\xED",
        icirc: "\xEE",
        iexcl: "\xA1",
        igrave: "\xEC",
        iquest: "\xBF",
        iuml: "\xEF",
        laquo: "\xAB",
        lt: "<",
        macr: "\xAF",
        micro: "\xB5",
        middot: "\xB7",
        nbsp: "\xA0",
        not: "\xAC",
        ntilde: "\xF1",
        oacute: "\xF3",
        ocirc: "\xF4",
        ograve: "\xF2",
        ordf: "\xAA",
        ordm: "\xBA",
        oslash: "\xF8",
        otilde: "\xF5",
        ouml: "\xF6",
        para: "\xB6",
        plusmn: "\xB1",
        pound: "\xA3",
        quot: '"',
        raquo: "\xBB",
        reg: "\xAE",
        sect: "\xA7",
        shy: "\xAD",
        sup1: "\xB9",
        sup2: "\xB2",
        sup3: "\xB3",
        szlig: "\xDF",
        thorn: "\xFE",
        times: "\xD7",
        uacute: "\xFA",
        ucirc: "\xFB",
        ugrave: "\xF9",
        uml: "\xA8",
        uuml: "\xFC",
        yacute: "\xFD",
        yen: "\xA5",
        yuml: "\xFF"
      };
    }
  });

  // node_modules/character-reference-invalid/index.json
  var require_character_reference_invalid = __commonJS({
    "node_modules/character-reference-invalid/index.json"(exports, module) {
      module.exports = {
        "0": "\uFFFD",
        "128": "\u20AC",
        "130": "\u201A",
        "131": "\u0192",
        "132": "\u201E",
        "133": "\u2026",
        "134": "\u2020",
        "135": "\u2021",
        "136": "\u02C6",
        "137": "\u2030",
        "138": "\u0160",
        "139": "\u2039",
        "140": "\u0152",
        "142": "\u017D",
        "145": "\u2018",
        "146": "\u2019",
        "147": "\u201C",
        "148": "\u201D",
        "149": "\u2022",
        "150": "\u2013",
        "151": "\u2014",
        "152": "\u02DC",
        "153": "\u2122",
        "154": "\u0161",
        "155": "\u203A",
        "156": "\u0153",
        "158": "\u017E",
        "159": "\u0178"
      };
    }
  });

  // node_modules/is-decimal/index.js
  var require_is_decimal = __commonJS({
    "node_modules/is-decimal/index.js"(exports, module) {
      "use strict";
      module.exports = decimal;
      function decimal(character) {
        var code = typeof character === "string" ? character.charCodeAt(0) : character;
        return code >= 48 && code <= 57;
      }
    }
  });

  // node_modules/is-hexadecimal/index.js
  var require_is_hexadecimal = __commonJS({
    "node_modules/is-hexadecimal/index.js"(exports, module) {
      "use strict";
      module.exports = hexadecimal;
      function hexadecimal(character) {
        var code = typeof character === "string" ? character.charCodeAt(0) : character;
        return code >= 97 && code <= 102 || code >= 65 && code <= 70 || code >= 48 && code <= 57;
      }
    }
  });

  // node_modules/is-alphabetical/index.js
  var require_is_alphabetical = __commonJS({
    "node_modules/is-alphabetical/index.js"(exports, module) {
      "use strict";
      module.exports = alphabetical;
      function alphabetical(character) {
        var code = typeof character === "string" ? character.charCodeAt(0) : character;
        return code >= 97 && code <= 122 || code >= 65 && code <= 90;
      }
    }
  });

  // node_modules/is-alphanumerical/index.js
  var require_is_alphanumerical = __commonJS({
    "node_modules/is-alphanumerical/index.js"(exports, module) {
      "use strict";
      var alphabetical = require_is_alphabetical();
      var decimal = require_is_decimal();
      module.exports = alphanumerical;
      function alphanumerical(character) {
        return alphabetical(character) || decimal(character);
      }
    }
  });

  // node_modules/character-entities/index.json
  var require_character_entities = __commonJS({
    "node_modules/character-entities/index.json"(exports, module) {
      module.exports = {
        AEli: "\xC6",
        AElig: "\xC6",
        AM: "&",
        AMP: "&",
        Aacut: "\xC1",
        Aacute: "\xC1",
        Abreve: "\u0102",
        Acir: "\xC2",
        Acirc: "\xC2",
        Acy: "\u0410",
        Afr: "\u{1D504}",
        Agrav: "\xC0",
        Agrave: "\xC0",
        Alpha: "\u0391",
        Amacr: "\u0100",
        And: "\u2A53",
        Aogon: "\u0104",
        Aopf: "\u{1D538}",
        ApplyFunction: "\u2061",
        Arin: "\xC5",
        Aring: "\xC5",
        Ascr: "\u{1D49C}",
        Assign: "\u2254",
        Atild: "\xC3",
        Atilde: "\xC3",
        Aum: "\xC4",
        Auml: "\xC4",
        Backslash: "\u2216",
        Barv: "\u2AE7",
        Barwed: "\u2306",
        Bcy: "\u0411",
        Because: "\u2235",
        Bernoullis: "\u212C",
        Beta: "\u0392",
        Bfr: "\u{1D505}",
        Bopf: "\u{1D539}",
        Breve: "\u02D8",
        Bscr: "\u212C",
        Bumpeq: "\u224E",
        CHcy: "\u0427",
        COP: "\xA9",
        COPY: "\xA9",
        Cacute: "\u0106",
        Cap: "\u22D2",
        CapitalDifferentialD: "\u2145",
        Cayleys: "\u212D",
        Ccaron: "\u010C",
        Ccedi: "\xC7",
        Ccedil: "\xC7",
        Ccirc: "\u0108",
        Cconint: "\u2230",
        Cdot: "\u010A",
        Cedilla: "\xB8",
        CenterDot: "\xB7",
        Cfr: "\u212D",
        Chi: "\u03A7",
        CircleDot: "\u2299",
        CircleMinus: "\u2296",
        CirclePlus: "\u2295",
        CircleTimes: "\u2297",
        ClockwiseContourIntegral: "\u2232",
        CloseCurlyDoubleQuote: "\u201D",
        CloseCurlyQuote: "\u2019",
        Colon: "\u2237",
        Colone: "\u2A74",
        Congruent: "\u2261",
        Conint: "\u222F",
        ContourIntegral: "\u222E",
        Copf: "\u2102",
        Coproduct: "\u2210",
        CounterClockwiseContourIntegral: "\u2233",
        Cross: "\u2A2F",
        Cscr: "\u{1D49E}",
        Cup: "\u22D3",
        CupCap: "\u224D",
        DD: "\u2145",
        DDotrahd: "\u2911",
        DJcy: "\u0402",
        DScy: "\u0405",
        DZcy: "\u040F",
        Dagger: "\u2021",
        Darr: "\u21A1",
        Dashv: "\u2AE4",
        Dcaron: "\u010E",
        Dcy: "\u0414",
        Del: "\u2207",
        Delta: "\u0394",
        Dfr: "\u{1D507}",
        DiacriticalAcute: "\xB4",
        DiacriticalDot: "\u02D9",
        DiacriticalDoubleAcute: "\u02DD",
        DiacriticalGrave: "`",
        DiacriticalTilde: "\u02DC",
        Diamond: "\u22C4",
        DifferentialD: "\u2146",
        Dopf: "\u{1D53B}",
        Dot: "\xA8",
        DotDot: "\u20DC",
        DotEqual: "\u2250",
        DoubleContourIntegral: "\u222F",
        DoubleDot: "\xA8",
        DoubleDownArrow: "\u21D3",
        DoubleLeftArrow: "\u21D0",
        DoubleLeftRightArrow: "\u21D4",
        DoubleLeftTee: "\u2AE4",
        DoubleLongLeftArrow: "\u27F8",
        DoubleLongLeftRightArrow: "\u27FA",
        DoubleLongRightArrow: "\u27F9",
        DoubleRightArrow: "\u21D2",
        DoubleRightTee: "\u22A8",
        DoubleUpArrow: "\u21D1",
        DoubleUpDownArrow: "\u21D5",
        DoubleVerticalBar: "\u2225",
        DownArrow: "\u2193",
        DownArrowBar: "\u2913",
        DownArrowUpArrow: "\u21F5",
        DownBreve: "\u0311",
        DownLeftRightVector: "\u2950",
        DownLeftTeeVector: "\u295E",
        DownLeftVector: "\u21BD",
        DownLeftVectorBar: "\u2956",
        DownRightTeeVector: "\u295F",
        DownRightVector: "\u21C1",
        DownRightVectorBar: "\u2957",
        DownTee: "\u22A4",
        DownTeeArrow: "\u21A7",
        Downarrow: "\u21D3",
        Dscr: "\u{1D49F}",
        Dstrok: "\u0110",
        ENG: "\u014A",
        ET: "\xD0",
        ETH: "\xD0",
        Eacut: "\xC9",
        Eacute: "\xC9",
        Ecaron: "\u011A",
        Ecir: "\xCA",
        Ecirc: "\xCA",
        Ecy: "\u042D",
        Edot: "\u0116",
        Efr: "\u{1D508}",
        Egrav: "\xC8",
        Egrave: "\xC8",
        Element: "\u2208",
        Emacr: "\u0112",
        EmptySmallSquare: "\u25FB",
        EmptyVerySmallSquare: "\u25AB",
        Eogon: "\u0118",
        Eopf: "\u{1D53C}",
        Epsilon: "\u0395",
        Equal: "\u2A75",
        EqualTilde: "\u2242",
        Equilibrium: "\u21CC",
        Escr: "\u2130",
        Esim: "\u2A73",
        Eta: "\u0397",
        Eum: "\xCB",
        Euml: "\xCB",
        Exists: "\u2203",
        ExponentialE: "\u2147",
        Fcy: "\u0424",
        Ffr: "\u{1D509}",
        FilledSmallSquare: "\u25FC",
        FilledVerySmallSquare: "\u25AA",
        Fopf: "\u{1D53D}",
        ForAll: "\u2200",
        Fouriertrf: "\u2131",
        Fscr: "\u2131",
        GJcy: "\u0403",
        G: ">",
        GT: ">",
        Gamma: "\u0393",
        Gammad: "\u03DC",
        Gbreve: "\u011E",
        Gcedil: "\u0122",
        Gcirc: "\u011C",
        Gcy: "\u0413",
        Gdot: "\u0120",
        Gfr: "\u{1D50A}",
        Gg: "\u22D9",
        Gopf: "\u{1D53E}",
        GreaterEqual: "\u2265",
        GreaterEqualLess: "\u22DB",
        GreaterFullEqual: "\u2267",
        GreaterGreater: "\u2AA2",
        GreaterLess: "\u2277",
        GreaterSlantEqual: "\u2A7E",
        GreaterTilde: "\u2273",
        Gscr: "\u{1D4A2}",
        Gt: "\u226B",
        HARDcy: "\u042A",
        Hacek: "\u02C7",
        Hat: "^",
        Hcirc: "\u0124",
        Hfr: "\u210C",
        HilbertSpace: "\u210B",
        Hopf: "\u210D",
        HorizontalLine: "\u2500",
        Hscr: "\u210B",
        Hstrok: "\u0126",
        HumpDownHump: "\u224E",
        HumpEqual: "\u224F",
        IEcy: "\u0415",
        IJlig: "\u0132",
        IOcy: "\u0401",
        Iacut: "\xCD",
        Iacute: "\xCD",
        Icir: "\xCE",
        Icirc: "\xCE",
        Icy: "\u0418",
        Idot: "\u0130",
        Ifr: "\u2111",
        Igrav: "\xCC",
        Igrave: "\xCC",
        Im: "\u2111",
        Imacr: "\u012A",
        ImaginaryI: "\u2148",
        Implies: "\u21D2",
        Int: "\u222C",
        Integral: "\u222B",
        Intersection: "\u22C2",
        InvisibleComma: "\u2063",
        InvisibleTimes: "\u2062",
        Iogon: "\u012E",
        Iopf: "\u{1D540}",
        Iota: "\u0399",
        Iscr: "\u2110",
        Itilde: "\u0128",
        Iukcy: "\u0406",
        Ium: "\xCF",
        Iuml: "\xCF",
        Jcirc: "\u0134",
        Jcy: "\u0419",
        Jfr: "\u{1D50D}",
        Jopf: "\u{1D541}",
        Jscr: "\u{1D4A5}",
        Jsercy: "\u0408",
        Jukcy: "\u0404",
        KHcy: "\u0425",
        KJcy: "\u040C",
        Kappa: "\u039A",
        Kcedil: "\u0136",
        Kcy: "\u041A",
        Kfr: "\u{1D50E}",
        Kopf: "\u{1D542}",
        Kscr: "\u{1D4A6}",
        LJcy: "\u0409",
        L: "<",
        LT: "<",
        Lacute: "\u0139",
        Lambda: "\u039B",
        Lang: "\u27EA",
        Laplacetrf: "\u2112",
        Larr: "\u219E",
        Lcaron: "\u013D",
        Lcedil: "\u013B",
        Lcy: "\u041B",
        LeftAngleBracket: "\u27E8",
        LeftArrow: "\u2190",
        LeftArrowBar: "\u21E4",
        LeftArrowRightArrow: "\u21C6",
        LeftCeiling: "\u2308",
        LeftDoubleBracket: "\u27E6",
        LeftDownTeeVector: "\u2961",
        LeftDownVector: "\u21C3",
        LeftDownVectorBar: "\u2959",
        LeftFloor: "\u230A",
        LeftRightArrow: "\u2194",
        LeftRightVector: "\u294E",
        LeftTee: "\u22A3",
        LeftTeeArrow: "\u21A4",
        LeftTeeVector: "\u295A",
        LeftTriangle: "\u22B2",
        LeftTriangleBar: "\u29CF",
        LeftTriangleEqual: "\u22B4",
        LeftUpDownVector: "\u2951",
        LeftUpTeeVector: "\u2960",
        LeftUpVector: "\u21BF",
        LeftUpVectorBar: "\u2958",
        LeftVector: "\u21BC",
        LeftVectorBar: "\u2952",
        Leftarrow: "\u21D0",
        Leftrightarrow: "\u21D4",
        LessEqualGreater: "\u22DA",
        LessFullEqual: "\u2266",
        LessGreater: "\u2276",
        LessLess: "\u2AA1",
        LessSlantEqual: "\u2A7D",
        LessTilde: "\u2272",
        Lfr: "\u{1D50F}",
        Ll: "\u22D8",
        Lleftarrow: "\u21DA",
        Lmidot: "\u013F",
        LongLeftArrow: "\u27F5",
        LongLeftRightArrow: "\u27F7",
        LongRightArrow: "\u27F6",
        Longleftarrow: "\u27F8",
        Longleftrightarrow: "\u27FA",
        Longrightarrow: "\u27F9",
        Lopf: "\u{1D543}",
        LowerLeftArrow: "\u2199",
        LowerRightArrow: "\u2198",
        Lscr: "\u2112",
        Lsh: "\u21B0",
        Lstrok: "\u0141",
        Lt: "\u226A",
        Map: "\u2905",
        Mcy: "\u041C",
        MediumSpace: "\u205F",
        Mellintrf: "\u2133",
        Mfr: "\u{1D510}",
        MinusPlus: "\u2213",
        Mopf: "\u{1D544}",
        Mscr: "\u2133",
        Mu: "\u039C",
        NJcy: "\u040A",
        Nacute: "\u0143",
        Ncaron: "\u0147",
        Ncedil: "\u0145",
        Ncy: "\u041D",
        NegativeMediumSpace: "\u200B",
        NegativeThickSpace: "\u200B",
        NegativeThinSpace: "\u200B",
        NegativeVeryThinSpace: "\u200B",
        NestedGreaterGreater: "\u226B",
        NestedLessLess: "\u226A",
        NewLine: "\n",
        Nfr: "\u{1D511}",
        NoBreak: "\u2060",
        NonBreakingSpace: "\xA0",
        Nopf: "\u2115",
        Not: "\u2AEC",
        NotCongruent: "\u2262",
        NotCupCap: "\u226D",
        NotDoubleVerticalBar: "\u2226",
        NotElement: "\u2209",
        NotEqual: "\u2260",
        NotEqualTilde: "\u2242\u0338",
        NotExists: "\u2204",
        NotGreater: "\u226F",
        NotGreaterEqual: "\u2271",
        NotGreaterFullEqual: "\u2267\u0338",
        NotGreaterGreater: "\u226B\u0338",
        NotGreaterLess: "\u2279",
        NotGreaterSlantEqual: "\u2A7E\u0338",
        NotGreaterTilde: "\u2275",
        NotHumpDownHump: "\u224E\u0338",
        NotHumpEqual: "\u224F\u0338",
        NotLeftTriangle: "\u22EA",
        NotLeftTriangleBar: "\u29CF\u0338",
        NotLeftTriangleEqual: "\u22EC",
        NotLess: "\u226E",
        NotLessEqual: "\u2270",
        NotLessGreater: "\u2278",
        NotLessLess: "\u226A\u0338",
        NotLessSlantEqual: "\u2A7D\u0338",
        NotLessTilde: "\u2274",
        NotNestedGreaterGreater: "\u2AA2\u0338",
        NotNestedLessLess: "\u2AA1\u0338",
        NotPrecedes: "\u2280",
        NotPrecedesEqual: "\u2AAF\u0338",
        NotPrecedesSlantEqual: "\u22E0",
        NotReverseElement: "\u220C",
        NotRightTriangle: "\u22EB",
        NotRightTriangleBar: "\u29D0\u0338",
        NotRightTriangleEqual: "\u22ED",
        NotSquareSubset: "\u228F\u0338",
        NotSquareSubsetEqual: "\u22E2",
        NotSquareSuperset: "\u2290\u0338",
        NotSquareSupersetEqual: "\u22E3",
        NotSubset: "\u2282\u20D2",
        NotSubsetEqual: "\u2288",
        NotSucceeds: "\u2281",
        NotSucceedsEqual: "\u2AB0\u0338",
        NotSucceedsSlantEqual: "\u22E1",
        NotSucceedsTilde: "\u227F\u0338",
        NotSuperset: "\u2283\u20D2",
        NotSupersetEqual: "\u2289",
        NotTilde: "\u2241",
        NotTildeEqual: "\u2244",
        NotTildeFullEqual: "\u2247",
        NotTildeTilde: "\u2249",
        NotVerticalBar: "\u2224",
        Nscr: "\u{1D4A9}",
        Ntild: "\xD1",
        Ntilde: "\xD1",
        Nu: "\u039D",
        OElig: "\u0152",
        Oacut: "\xD3",
        Oacute: "\xD3",
        Ocir: "\xD4",
        Ocirc: "\xD4",
        Ocy: "\u041E",
        Odblac: "\u0150",
        Ofr: "\u{1D512}",
        Ograv: "\xD2",
        Ograve: "\xD2",
        Omacr: "\u014C",
        Omega: "\u03A9",
        Omicron: "\u039F",
        Oopf: "\u{1D546}",
        OpenCurlyDoubleQuote: "\u201C",
        OpenCurlyQuote: "\u2018",
        Or: "\u2A54",
        Oscr: "\u{1D4AA}",
        Oslas: "\xD8",
        Oslash: "\xD8",
        Otild: "\xD5",
        Otilde: "\xD5",
        Otimes: "\u2A37",
        Oum: "\xD6",
        Ouml: "\xD6",
        OverBar: "\u203E",
        OverBrace: "\u23DE",
        OverBracket: "\u23B4",
        OverParenthesis: "\u23DC",
        PartialD: "\u2202",
        Pcy: "\u041F",
        Pfr: "\u{1D513}",
        Phi: "\u03A6",
        Pi: "\u03A0",
        PlusMinus: "\xB1",
        Poincareplane: "\u210C",
        Popf: "\u2119",
        Pr: "\u2ABB",
        Precedes: "\u227A",
        PrecedesEqual: "\u2AAF",
        PrecedesSlantEqual: "\u227C",
        PrecedesTilde: "\u227E",
        Prime: "\u2033",
        Product: "\u220F",
        Proportion: "\u2237",
        Proportional: "\u221D",
        Pscr: "\u{1D4AB}",
        Psi: "\u03A8",
        QUO: '"',
        QUOT: '"',
        Qfr: "\u{1D514}",
        Qopf: "\u211A",
        Qscr: "\u{1D4AC}",
        RBarr: "\u2910",
        RE: "\xAE",
        REG: "\xAE",
        Racute: "\u0154",
        Rang: "\u27EB",
        Rarr: "\u21A0",
        Rarrtl: "\u2916",
        Rcaron: "\u0158",
        Rcedil: "\u0156",
        Rcy: "\u0420",
        Re: "\u211C",
        ReverseElement: "\u220B",
        ReverseEquilibrium: "\u21CB",
        ReverseUpEquilibrium: "\u296F",
        Rfr: "\u211C",
        Rho: "\u03A1",
        RightAngleBracket: "\u27E9",
        RightArrow: "\u2192",
        RightArrowBar: "\u21E5",
        RightArrowLeftArrow: "\u21C4",
        RightCeiling: "\u2309",
        RightDoubleBracket: "\u27E7",
        RightDownTeeVector: "\u295D",
        RightDownVector: "\u21C2",
        RightDownVectorBar: "\u2955",
        RightFloor: "\u230B",
        RightTee: "\u22A2",
        RightTeeArrow: "\u21A6",
        RightTeeVector: "\u295B",
        RightTriangle: "\u22B3",
        RightTriangleBar: "\u29D0",
        RightTriangleEqual: "\u22B5",
        RightUpDownVector: "\u294F",
        RightUpTeeVector: "\u295C",
        RightUpVector: "\u21BE",
        RightUpVectorBar: "\u2954",
        RightVector: "\u21C0",
        RightVectorBar: "\u2953",
        Rightarrow: "\u21D2",
        Ropf: "\u211D",
        RoundImplies: "\u2970",
        Rrightarrow: "\u21DB",
        Rscr: "\u211B",
        Rsh: "\u21B1",
        RuleDelayed: "\u29F4",
        SHCHcy: "\u0429",
        SHcy: "\u0428",
        SOFTcy: "\u042C",
        Sacute: "\u015A",
        Sc: "\u2ABC",
        Scaron: "\u0160",
        Scedil: "\u015E",
        Scirc: "\u015C",
        Scy: "\u0421",
        Sfr: "\u{1D516}",
        ShortDownArrow: "\u2193",
        ShortLeftArrow: "\u2190",
        ShortRightArrow: "\u2192",
        ShortUpArrow: "\u2191",
        Sigma: "\u03A3",
        SmallCircle: "\u2218",
        Sopf: "\u{1D54A}",
        Sqrt: "\u221A",
        Square: "\u25A1",
        SquareIntersection: "\u2293",
        SquareSubset: "\u228F",
        SquareSubsetEqual: "\u2291",
        SquareSuperset: "\u2290",
        SquareSupersetEqual: "\u2292",
        SquareUnion: "\u2294",
        Sscr: "\u{1D4AE}",
        Star: "\u22C6",
        Sub: "\u22D0",
        Subset: "\u22D0",
        SubsetEqual: "\u2286",
        Succeeds: "\u227B",
        SucceedsEqual: "\u2AB0",
        SucceedsSlantEqual: "\u227D",
        SucceedsTilde: "\u227F",
        SuchThat: "\u220B",
        Sum: "\u2211",
        Sup: "\u22D1",
        Superset: "\u2283",
        SupersetEqual: "\u2287",
        Supset: "\u22D1",
        THOR: "\xDE",
        THORN: "\xDE",
        TRADE: "\u2122",
        TSHcy: "\u040B",
        TScy: "\u0426",
        Tab: "	",
        Tau: "\u03A4",
        Tcaron: "\u0164",
        Tcedil: "\u0162",
        Tcy: "\u0422",
        Tfr: "\u{1D517}",
        Therefore: "\u2234",
        Theta: "\u0398",
        ThickSpace: "\u205F\u200A",
        ThinSpace: "\u2009",
        Tilde: "\u223C",
        TildeEqual: "\u2243",
        TildeFullEqual: "\u2245",
        TildeTilde: "\u2248",
        Topf: "\u{1D54B}",
        TripleDot: "\u20DB",
        Tscr: "\u{1D4AF}",
        Tstrok: "\u0166",
        Uacut: "\xDA",
        Uacute: "\xDA",
        Uarr: "\u219F",
        Uarrocir: "\u2949",
        Ubrcy: "\u040E",
        Ubreve: "\u016C",
        Ucir: "\xDB",
        Ucirc: "\xDB",
        Ucy: "\u0423",
        Udblac: "\u0170",
        Ufr: "\u{1D518}",
        Ugrav: "\xD9",
        Ugrave: "\xD9",
        Umacr: "\u016A",
        UnderBar: "_",
        UnderBrace: "\u23DF",
        UnderBracket: "\u23B5",
        UnderParenthesis: "\u23DD",
        Union: "\u22C3",
        UnionPlus: "\u228E",
        Uogon: "\u0172",
        Uopf: "\u{1D54C}",
        UpArrow: "\u2191",
        UpArrowBar: "\u2912",
        UpArrowDownArrow: "\u21C5",
        UpDownArrow: "\u2195",
        UpEquilibrium: "\u296E",
        UpTee: "\u22A5",
        UpTeeArrow: "\u21A5",
        Uparrow: "\u21D1",
        Updownarrow: "\u21D5",
        UpperLeftArrow: "\u2196",
        UpperRightArrow: "\u2197",
        Upsi: "\u03D2",
        Upsilon: "\u03A5",
        Uring: "\u016E",
        Uscr: "\u{1D4B0}",
        Utilde: "\u0168",
        Uum: "\xDC",
        Uuml: "\xDC",
        VDash: "\u22AB",
        Vbar: "\u2AEB",
        Vcy: "\u0412",
        Vdash: "\u22A9",
        Vdashl: "\u2AE6",
        Vee: "\u22C1",
        Verbar: "\u2016",
        Vert: "\u2016",
        VerticalBar: "\u2223",
        VerticalLine: "|",
        VerticalSeparator: "\u2758",
        VerticalTilde: "\u2240",
        VeryThinSpace: "\u200A",
        Vfr: "\u{1D519}",
        Vopf: "\u{1D54D}",
        Vscr: "\u{1D4B1}",
        Vvdash: "\u22AA",
        Wcirc: "\u0174",
        Wedge: "\u22C0",
        Wfr: "\u{1D51A}",
        Wopf: "\u{1D54E}",
        Wscr: "\u{1D4B2}",
        Xfr: "\u{1D51B}",
        Xi: "\u039E",
        Xopf: "\u{1D54F}",
        Xscr: "\u{1D4B3}",
        YAcy: "\u042F",
        YIcy: "\u0407",
        YUcy: "\u042E",
        Yacut: "\xDD",
        Yacute: "\xDD",
        Ycirc: "\u0176",
        Ycy: "\u042B",
        Yfr: "\u{1D51C}",
        Yopf: "\u{1D550}",
        Yscr: "\u{1D4B4}",
        Yuml: "\u0178",
        ZHcy: "\u0416",
        Zacute: "\u0179",
        Zcaron: "\u017D",
        Zcy: "\u0417",
        Zdot: "\u017B",
        ZeroWidthSpace: "\u200B",
        Zeta: "\u0396",
        Zfr: "\u2128",
        Zopf: "\u2124",
        Zscr: "\u{1D4B5}",
        aacut: "\xE1",
        aacute: "\xE1",
        abreve: "\u0103",
        ac: "\u223E",
        acE: "\u223E\u0333",
        acd: "\u223F",
        acir: "\xE2",
        acirc: "\xE2",
        acut: "\xB4",
        acute: "\xB4",
        acy: "\u0430",
        aeli: "\xE6",
        aelig: "\xE6",
        af: "\u2061",
        afr: "\u{1D51E}",
        agrav: "\xE0",
        agrave: "\xE0",
        alefsym: "\u2135",
        aleph: "\u2135",
        alpha: "\u03B1",
        amacr: "\u0101",
        amalg: "\u2A3F",
        am: "&",
        amp: "&",
        and: "\u2227",
        andand: "\u2A55",
        andd: "\u2A5C",
        andslope: "\u2A58",
        andv: "\u2A5A",
        ang: "\u2220",
        ange: "\u29A4",
        angle: "\u2220",
        angmsd: "\u2221",
        angmsdaa: "\u29A8",
        angmsdab: "\u29A9",
        angmsdac: "\u29AA",
        angmsdad: "\u29AB",
        angmsdae: "\u29AC",
        angmsdaf: "\u29AD",
        angmsdag: "\u29AE",
        angmsdah: "\u29AF",
        angrt: "\u221F",
        angrtvb: "\u22BE",
        angrtvbd: "\u299D",
        angsph: "\u2222",
        angst: "\xC5",
        angzarr: "\u237C",
        aogon: "\u0105",
        aopf: "\u{1D552}",
        ap: "\u2248",
        apE: "\u2A70",
        apacir: "\u2A6F",
        ape: "\u224A",
        apid: "\u224B",
        apos: "'",
        approx: "\u2248",
        approxeq: "\u224A",
        arin: "\xE5",
        aring: "\xE5",
        ascr: "\u{1D4B6}",
        ast: "*",
        asymp: "\u2248",
        asympeq: "\u224D",
        atild: "\xE3",
        atilde: "\xE3",
        aum: "\xE4",
        auml: "\xE4",
        awconint: "\u2233",
        awint: "\u2A11",
        bNot: "\u2AED",
        backcong: "\u224C",
        backepsilon: "\u03F6",
        backprime: "\u2035",
        backsim: "\u223D",
        backsimeq: "\u22CD",
        barvee: "\u22BD",
        barwed: "\u2305",
        barwedge: "\u2305",
        bbrk: "\u23B5",
        bbrktbrk: "\u23B6",
        bcong: "\u224C",
        bcy: "\u0431",
        bdquo: "\u201E",
        becaus: "\u2235",
        because: "\u2235",
        bemptyv: "\u29B0",
        bepsi: "\u03F6",
        bernou: "\u212C",
        beta: "\u03B2",
        beth: "\u2136",
        between: "\u226C",
        bfr: "\u{1D51F}",
        bigcap: "\u22C2",
        bigcirc: "\u25EF",
        bigcup: "\u22C3",
        bigodot: "\u2A00",
        bigoplus: "\u2A01",
        bigotimes: "\u2A02",
        bigsqcup: "\u2A06",
        bigstar: "\u2605",
        bigtriangledown: "\u25BD",
        bigtriangleup: "\u25B3",
        biguplus: "\u2A04",
        bigvee: "\u22C1",
        bigwedge: "\u22C0",
        bkarow: "\u290D",
        blacklozenge: "\u29EB",
        blacksquare: "\u25AA",
        blacktriangle: "\u25B4",
        blacktriangledown: "\u25BE",
        blacktriangleleft: "\u25C2",
        blacktriangleright: "\u25B8",
        blank: "\u2423",
        blk12: "\u2592",
        blk14: "\u2591",
        blk34: "\u2593",
        block: "\u2588",
        bne: "=\u20E5",
        bnequiv: "\u2261\u20E5",
        bnot: "\u2310",
        bopf: "\u{1D553}",
        bot: "\u22A5",
        bottom: "\u22A5",
        bowtie: "\u22C8",
        boxDL: "\u2557",
        boxDR: "\u2554",
        boxDl: "\u2556",
        boxDr: "\u2553",
        boxH: "\u2550",
        boxHD: "\u2566",
        boxHU: "\u2569",
        boxHd: "\u2564",
        boxHu: "\u2567",
        boxUL: "\u255D",
        boxUR: "\u255A",
        boxUl: "\u255C",
        boxUr: "\u2559",
        boxV: "\u2551",
        boxVH: "\u256C",
        boxVL: "\u2563",
        boxVR: "\u2560",
        boxVh: "\u256B",
        boxVl: "\u2562",
        boxVr: "\u255F",
        boxbox: "\u29C9",
        boxdL: "\u2555",
        boxdR: "\u2552",
        boxdl: "\u2510",
        boxdr: "\u250C",
        boxh: "\u2500",
        boxhD: "\u2565",
        boxhU: "\u2568",
        boxhd: "\u252C",
        boxhu: "\u2534",
        boxminus: "\u229F",
        boxplus: "\u229E",
        boxtimes: "\u22A0",
        boxuL: "\u255B",
        boxuR: "\u2558",
        boxul: "\u2518",
        boxur: "\u2514",
        boxv: "\u2502",
        boxvH: "\u256A",
        boxvL: "\u2561",
        boxvR: "\u255E",
        boxvh: "\u253C",
        boxvl: "\u2524",
        boxvr: "\u251C",
        bprime: "\u2035",
        breve: "\u02D8",
        brvba: "\xA6",
        brvbar: "\xA6",
        bscr: "\u{1D4B7}",
        bsemi: "\u204F",
        bsim: "\u223D",
        bsime: "\u22CD",
        bsol: "\\",
        bsolb: "\u29C5",
        bsolhsub: "\u27C8",
        bull: "\u2022",
        bullet: "\u2022",
        bump: "\u224E",
        bumpE: "\u2AAE",
        bumpe: "\u224F",
        bumpeq: "\u224F",
        cacute: "\u0107",
        cap: "\u2229",
        capand: "\u2A44",
        capbrcup: "\u2A49",
        capcap: "\u2A4B",
        capcup: "\u2A47",
        capdot: "\u2A40",
        caps: "\u2229\uFE00",
        caret: "\u2041",
        caron: "\u02C7",
        ccaps: "\u2A4D",
        ccaron: "\u010D",
        ccedi: "\xE7",
        ccedil: "\xE7",
        ccirc: "\u0109",
        ccups: "\u2A4C",
        ccupssm: "\u2A50",
        cdot: "\u010B",
        cedi: "\xB8",
        cedil: "\xB8",
        cemptyv: "\u29B2",
        cen: "\xA2",
        cent: "\xA2",
        centerdot: "\xB7",
        cfr: "\u{1D520}",
        chcy: "\u0447",
        check: "\u2713",
        checkmark: "\u2713",
        chi: "\u03C7",
        cir: "\u25CB",
        cirE: "\u29C3",
        circ: "\u02C6",
        circeq: "\u2257",
        circlearrowleft: "\u21BA",
        circlearrowright: "\u21BB",
        circledR: "\xAE",
        circledS: "\u24C8",
        circledast: "\u229B",
        circledcirc: "\u229A",
        circleddash: "\u229D",
        cire: "\u2257",
        cirfnint: "\u2A10",
        cirmid: "\u2AEF",
        cirscir: "\u29C2",
        clubs: "\u2663",
        clubsuit: "\u2663",
        colon: ":",
        colone: "\u2254",
        coloneq: "\u2254",
        comma: ",",
        commat: "@",
        comp: "\u2201",
        compfn: "\u2218",
        complement: "\u2201",
        complexes: "\u2102",
        cong: "\u2245",
        congdot: "\u2A6D",
        conint: "\u222E",
        copf: "\u{1D554}",
        coprod: "\u2210",
        cop: "\xA9",
        copy: "\xA9",
        copysr: "\u2117",
        crarr: "\u21B5",
        cross: "\u2717",
        cscr: "\u{1D4B8}",
        csub: "\u2ACF",
        csube: "\u2AD1",
        csup: "\u2AD0",
        csupe: "\u2AD2",
        ctdot: "\u22EF",
        cudarrl: "\u2938",
        cudarrr: "\u2935",
        cuepr: "\u22DE",
        cuesc: "\u22DF",
        cularr: "\u21B6",
        cularrp: "\u293D",
        cup: "\u222A",
        cupbrcap: "\u2A48",
        cupcap: "\u2A46",
        cupcup: "\u2A4A",
        cupdot: "\u228D",
        cupor: "\u2A45",
        cups: "\u222A\uFE00",
        curarr: "\u21B7",
        curarrm: "\u293C",
        curlyeqprec: "\u22DE",
        curlyeqsucc: "\u22DF",
        curlyvee: "\u22CE",
        curlywedge: "\u22CF",
        curre: "\xA4",
        curren: "\xA4",
        curvearrowleft: "\u21B6",
        curvearrowright: "\u21B7",
        cuvee: "\u22CE",
        cuwed: "\u22CF",
        cwconint: "\u2232",
        cwint: "\u2231",
        cylcty: "\u232D",
        dArr: "\u21D3",
        dHar: "\u2965",
        dagger: "\u2020",
        daleth: "\u2138",
        darr: "\u2193",
        dash: "\u2010",
        dashv: "\u22A3",
        dbkarow: "\u290F",
        dblac: "\u02DD",
        dcaron: "\u010F",
        dcy: "\u0434",
        dd: "\u2146",
        ddagger: "\u2021",
        ddarr: "\u21CA",
        ddotseq: "\u2A77",
        de: "\xB0",
        deg: "\xB0",
        delta: "\u03B4",
        demptyv: "\u29B1",
        dfisht: "\u297F",
        dfr: "\u{1D521}",
        dharl: "\u21C3",
        dharr: "\u21C2",
        diam: "\u22C4",
        diamond: "\u22C4",
        diamondsuit: "\u2666",
        diams: "\u2666",
        die: "\xA8",
        digamma: "\u03DD",
        disin: "\u22F2",
        div: "\xF7",
        divid: "\xF7",
        divide: "\xF7",
        divideontimes: "\u22C7",
        divonx: "\u22C7",
        djcy: "\u0452",
        dlcorn: "\u231E",
        dlcrop: "\u230D",
        dollar: "$",
        dopf: "\u{1D555}",
        dot: "\u02D9",
        doteq: "\u2250",
        doteqdot: "\u2251",
        dotminus: "\u2238",
        dotplus: "\u2214",
        dotsquare: "\u22A1",
        doublebarwedge: "\u2306",
        downarrow: "\u2193",
        downdownarrows: "\u21CA",
        downharpoonleft: "\u21C3",
        downharpoonright: "\u21C2",
        drbkarow: "\u2910",
        drcorn: "\u231F",
        drcrop: "\u230C",
        dscr: "\u{1D4B9}",
        dscy: "\u0455",
        dsol: "\u29F6",
        dstrok: "\u0111",
        dtdot: "\u22F1",
        dtri: "\u25BF",
        dtrif: "\u25BE",
        duarr: "\u21F5",
        duhar: "\u296F",
        dwangle: "\u29A6",
        dzcy: "\u045F",
        dzigrarr: "\u27FF",
        eDDot: "\u2A77",
        eDot: "\u2251",
        eacut: "\xE9",
        eacute: "\xE9",
        easter: "\u2A6E",
        ecaron: "\u011B",
        ecir: "\xEA",
        ecirc: "\xEA",
        ecolon: "\u2255",
        ecy: "\u044D",
        edot: "\u0117",
        ee: "\u2147",
        efDot: "\u2252",
        efr: "\u{1D522}",
        eg: "\u2A9A",
        egrav: "\xE8",
        egrave: "\xE8",
        egs: "\u2A96",
        egsdot: "\u2A98",
        el: "\u2A99",
        elinters: "\u23E7",
        ell: "\u2113",
        els: "\u2A95",
        elsdot: "\u2A97",
        emacr: "\u0113",
        empty: "\u2205",
        emptyset: "\u2205",
        emptyv: "\u2205",
        emsp13: "\u2004",
        emsp14: "\u2005",
        emsp: "\u2003",
        eng: "\u014B",
        ensp: "\u2002",
        eogon: "\u0119",
        eopf: "\u{1D556}",
        epar: "\u22D5",
        eparsl: "\u29E3",
        eplus: "\u2A71",
        epsi: "\u03B5",
        epsilon: "\u03B5",
        epsiv: "\u03F5",
        eqcirc: "\u2256",
        eqcolon: "\u2255",
        eqsim: "\u2242",
        eqslantgtr: "\u2A96",
        eqslantless: "\u2A95",
        equals: "=",
        equest: "\u225F",
        equiv: "\u2261",
        equivDD: "\u2A78",
        eqvparsl: "\u29E5",
        erDot: "\u2253",
        erarr: "\u2971",
        escr: "\u212F",
        esdot: "\u2250",
        esim: "\u2242",
        eta: "\u03B7",
        et: "\xF0",
        eth: "\xF0",
        eum: "\xEB",
        euml: "\xEB",
        euro: "\u20AC",
        excl: "!",
        exist: "\u2203",
        expectation: "\u2130",
        exponentiale: "\u2147",
        fallingdotseq: "\u2252",
        fcy: "\u0444",
        female: "\u2640",
        ffilig: "\uFB03",
        fflig: "\uFB00",
        ffllig: "\uFB04",
        ffr: "\u{1D523}",
        filig: "\uFB01",
        fjlig: "fj",
        flat: "\u266D",
        fllig: "\uFB02",
        fltns: "\u25B1",
        fnof: "\u0192",
        fopf: "\u{1D557}",
        forall: "\u2200",
        fork: "\u22D4",
        forkv: "\u2AD9",
        fpartint: "\u2A0D",
        frac1: "\xBC",
        frac12: "\xBD",
        frac13: "\u2153",
        frac14: "\xBC",
        frac15: "\u2155",
        frac16: "\u2159",
        frac18: "\u215B",
        frac23: "\u2154",
        frac25: "\u2156",
        frac3: "\xBE",
        frac34: "\xBE",
        frac35: "\u2157",
        frac38: "\u215C",
        frac45: "\u2158",
        frac56: "\u215A",
        frac58: "\u215D",
        frac78: "\u215E",
        frasl: "\u2044",
        frown: "\u2322",
        fscr: "\u{1D4BB}",
        gE: "\u2267",
        gEl: "\u2A8C",
        gacute: "\u01F5",
        gamma: "\u03B3",
        gammad: "\u03DD",
        gap: "\u2A86",
        gbreve: "\u011F",
        gcirc: "\u011D",
        gcy: "\u0433",
        gdot: "\u0121",
        ge: "\u2265",
        gel: "\u22DB",
        geq: "\u2265",
        geqq: "\u2267",
        geqslant: "\u2A7E",
        ges: "\u2A7E",
        gescc: "\u2AA9",
        gesdot: "\u2A80",
        gesdoto: "\u2A82",
        gesdotol: "\u2A84",
        gesl: "\u22DB\uFE00",
        gesles: "\u2A94",
        gfr: "\u{1D524}",
        gg: "\u226B",
        ggg: "\u22D9",
        gimel: "\u2137",
        gjcy: "\u0453",
        gl: "\u2277",
        glE: "\u2A92",
        gla: "\u2AA5",
        glj: "\u2AA4",
        gnE: "\u2269",
        gnap: "\u2A8A",
        gnapprox: "\u2A8A",
        gne: "\u2A88",
        gneq: "\u2A88",
        gneqq: "\u2269",
        gnsim: "\u22E7",
        gopf: "\u{1D558}",
        grave: "`",
        gscr: "\u210A",
        gsim: "\u2273",
        gsime: "\u2A8E",
        gsiml: "\u2A90",
        g: ">",
        gt: ">",
        gtcc: "\u2AA7",
        gtcir: "\u2A7A",
        gtdot: "\u22D7",
        gtlPar: "\u2995",
        gtquest: "\u2A7C",
        gtrapprox: "\u2A86",
        gtrarr: "\u2978",
        gtrdot: "\u22D7",
        gtreqless: "\u22DB",
        gtreqqless: "\u2A8C",
        gtrless: "\u2277",
        gtrsim: "\u2273",
        gvertneqq: "\u2269\uFE00",
        gvnE: "\u2269\uFE00",
        hArr: "\u21D4",
        hairsp: "\u200A",
        half: "\xBD",
        hamilt: "\u210B",
        hardcy: "\u044A",
        harr: "\u2194",
        harrcir: "\u2948",
        harrw: "\u21AD",
        hbar: "\u210F",
        hcirc: "\u0125",
        hearts: "\u2665",
        heartsuit: "\u2665",
        hellip: "\u2026",
        hercon: "\u22B9",
        hfr: "\u{1D525}",
        hksearow: "\u2925",
        hkswarow: "\u2926",
        hoarr: "\u21FF",
        homtht: "\u223B",
        hookleftarrow: "\u21A9",
        hookrightarrow: "\u21AA",
        hopf: "\u{1D559}",
        horbar: "\u2015",
        hscr: "\u{1D4BD}",
        hslash: "\u210F",
        hstrok: "\u0127",
        hybull: "\u2043",
        hyphen: "\u2010",
        iacut: "\xED",
        iacute: "\xED",
        ic: "\u2063",
        icir: "\xEE",
        icirc: "\xEE",
        icy: "\u0438",
        iecy: "\u0435",
        iexc: "\xA1",
        iexcl: "\xA1",
        iff: "\u21D4",
        ifr: "\u{1D526}",
        igrav: "\xEC",
        igrave: "\xEC",
        ii: "\u2148",
        iiiint: "\u2A0C",
        iiint: "\u222D",
        iinfin: "\u29DC",
        iiota: "\u2129",
        ijlig: "\u0133",
        imacr: "\u012B",
        image: "\u2111",
        imagline: "\u2110",
        imagpart: "\u2111",
        imath: "\u0131",
        imof: "\u22B7",
        imped: "\u01B5",
        in: "\u2208",
        incare: "\u2105",
        infin: "\u221E",
        infintie: "\u29DD",
        inodot: "\u0131",
        int: "\u222B",
        intcal: "\u22BA",
        integers: "\u2124",
        intercal: "\u22BA",
        intlarhk: "\u2A17",
        intprod: "\u2A3C",
        iocy: "\u0451",
        iogon: "\u012F",
        iopf: "\u{1D55A}",
        iota: "\u03B9",
        iprod: "\u2A3C",
        iques: "\xBF",
        iquest: "\xBF",
        iscr: "\u{1D4BE}",
        isin: "\u2208",
        isinE: "\u22F9",
        isindot: "\u22F5",
        isins: "\u22F4",
        isinsv: "\u22F3",
        isinv: "\u2208",
        it: "\u2062",
        itilde: "\u0129",
        iukcy: "\u0456",
        ium: "\xEF",
        iuml: "\xEF",
        jcirc: "\u0135",
        jcy: "\u0439",
        jfr: "\u{1D527}",
        jmath: "\u0237",
        jopf: "\u{1D55B}",
        jscr: "\u{1D4BF}",
        jsercy: "\u0458",
        jukcy: "\u0454",
        kappa: "\u03BA",
        kappav: "\u03F0",
        kcedil: "\u0137",
        kcy: "\u043A",
        kfr: "\u{1D528}",
        kgreen: "\u0138",
        khcy: "\u0445",
        kjcy: "\u045C",
        kopf: "\u{1D55C}",
        kscr: "\u{1D4C0}",
        lAarr: "\u21DA",
        lArr: "\u21D0",
        lAtail: "\u291B",
        lBarr: "\u290E",
        lE: "\u2266",
        lEg: "\u2A8B",
        lHar: "\u2962",
        lacute: "\u013A",
        laemptyv: "\u29B4",
        lagran: "\u2112",
        lambda: "\u03BB",
        lang: "\u27E8",
        langd: "\u2991",
        langle: "\u27E8",
        lap: "\u2A85",
        laqu: "\xAB",
        laquo: "\xAB",
        larr: "\u2190",
        larrb: "\u21E4",
        larrbfs: "\u291F",
        larrfs: "\u291D",
        larrhk: "\u21A9",
        larrlp: "\u21AB",
        larrpl: "\u2939",
        larrsim: "\u2973",
        larrtl: "\u21A2",
        lat: "\u2AAB",
        latail: "\u2919",
        late: "\u2AAD",
        lates: "\u2AAD\uFE00",
        lbarr: "\u290C",
        lbbrk: "\u2772",
        lbrace: "{",
        lbrack: "[",
        lbrke: "\u298B",
        lbrksld: "\u298F",
        lbrkslu: "\u298D",
        lcaron: "\u013E",
        lcedil: "\u013C",
        lceil: "\u2308",
        lcub: "{",
        lcy: "\u043B",
        ldca: "\u2936",
        ldquo: "\u201C",
        ldquor: "\u201E",
        ldrdhar: "\u2967",
        ldrushar: "\u294B",
        ldsh: "\u21B2",
        le: "\u2264",
        leftarrow: "\u2190",
        leftarrowtail: "\u21A2",
        leftharpoondown: "\u21BD",
        leftharpoonup: "\u21BC",
        leftleftarrows: "\u21C7",
        leftrightarrow: "\u2194",
        leftrightarrows: "\u21C6",
        leftrightharpoons: "\u21CB",
        leftrightsquigarrow: "\u21AD",
        leftthreetimes: "\u22CB",
        leg: "\u22DA",
        leq: "\u2264",
        leqq: "\u2266",
        leqslant: "\u2A7D",
        les: "\u2A7D",
        lescc: "\u2AA8",
        lesdot: "\u2A7F",
        lesdoto: "\u2A81",
        lesdotor: "\u2A83",
        lesg: "\u22DA\uFE00",
        lesges: "\u2A93",
        lessapprox: "\u2A85",
        lessdot: "\u22D6",
        lesseqgtr: "\u22DA",
        lesseqqgtr: "\u2A8B",
        lessgtr: "\u2276",
        lesssim: "\u2272",
        lfisht: "\u297C",
        lfloor: "\u230A",
        lfr: "\u{1D529}",
        lg: "\u2276",
        lgE: "\u2A91",
        lhard: "\u21BD",
        lharu: "\u21BC",
        lharul: "\u296A",
        lhblk: "\u2584",
        ljcy: "\u0459",
        ll: "\u226A",
        llarr: "\u21C7",
        llcorner: "\u231E",
        llhard: "\u296B",
        lltri: "\u25FA",
        lmidot: "\u0140",
        lmoust: "\u23B0",
        lmoustache: "\u23B0",
        lnE: "\u2268",
        lnap: "\u2A89",
        lnapprox: "\u2A89",
        lne: "\u2A87",
        lneq: "\u2A87",
        lneqq: "\u2268",
        lnsim: "\u22E6",
        loang: "\u27EC",
        loarr: "\u21FD",
        lobrk: "\u27E6",
        longleftarrow: "\u27F5",
        longleftrightarrow: "\u27F7",
        longmapsto: "\u27FC",
        longrightarrow: "\u27F6",
        looparrowleft: "\u21AB",
        looparrowright: "\u21AC",
        lopar: "\u2985",
        lopf: "\u{1D55D}",
        loplus: "\u2A2D",
        lotimes: "\u2A34",
        lowast: "\u2217",
        lowbar: "_",
        loz: "\u25CA",
        lozenge: "\u25CA",
        lozf: "\u29EB",
        lpar: "(",
        lparlt: "\u2993",
        lrarr: "\u21C6",
        lrcorner: "\u231F",
        lrhar: "\u21CB",
        lrhard: "\u296D",
        lrm: "\u200E",
        lrtri: "\u22BF",
        lsaquo: "\u2039",
        lscr: "\u{1D4C1}",
        lsh: "\u21B0",
        lsim: "\u2272",
        lsime: "\u2A8D",
        lsimg: "\u2A8F",
        lsqb: "[",
        lsquo: "\u2018",
        lsquor: "\u201A",
        lstrok: "\u0142",
        l: "<",
        lt: "<",
        ltcc: "\u2AA6",
        ltcir: "\u2A79",
        ltdot: "\u22D6",
        lthree: "\u22CB",
        ltimes: "\u22C9",
        ltlarr: "\u2976",
        ltquest: "\u2A7B",
        ltrPar: "\u2996",
        ltri: "\u25C3",
        ltrie: "\u22B4",
        ltrif: "\u25C2",
        lurdshar: "\u294A",
        luruhar: "\u2966",
        lvertneqq: "\u2268\uFE00",
        lvnE: "\u2268\uFE00",
        mDDot: "\u223A",
        mac: "\xAF",
        macr: "\xAF",
        male: "\u2642",
        malt: "\u2720",
        maltese: "\u2720",
        map: "\u21A6",
        mapsto: "\u21A6",
        mapstodown: "\u21A7",
        mapstoleft: "\u21A4",
        mapstoup: "\u21A5",
        marker: "\u25AE",
        mcomma: "\u2A29",
        mcy: "\u043C",
        mdash: "\u2014",
        measuredangle: "\u2221",
        mfr: "\u{1D52A}",
        mho: "\u2127",
        micr: "\xB5",
        micro: "\xB5",
        mid: "\u2223",
        midast: "*",
        midcir: "\u2AF0",
        middo: "\xB7",
        middot: "\xB7",
        minus: "\u2212",
        minusb: "\u229F",
        minusd: "\u2238",
        minusdu: "\u2A2A",
        mlcp: "\u2ADB",
        mldr: "\u2026",
        mnplus: "\u2213",
        models: "\u22A7",
        mopf: "\u{1D55E}",
        mp: "\u2213",
        mscr: "\u{1D4C2}",
        mstpos: "\u223E",
        mu: "\u03BC",
        multimap: "\u22B8",
        mumap: "\u22B8",
        nGg: "\u22D9\u0338",
        nGt: "\u226B\u20D2",
        nGtv: "\u226B\u0338",
        nLeftarrow: "\u21CD",
        nLeftrightarrow: "\u21CE",
        nLl: "\u22D8\u0338",
        nLt: "\u226A\u20D2",
        nLtv: "\u226A\u0338",
        nRightarrow: "\u21CF",
        nVDash: "\u22AF",
        nVdash: "\u22AE",
        nabla: "\u2207",
        nacute: "\u0144",
        nang: "\u2220\u20D2",
        nap: "\u2249",
        napE: "\u2A70\u0338",
        napid: "\u224B\u0338",
        napos: "\u0149",
        napprox: "\u2249",
        natur: "\u266E",
        natural: "\u266E",
        naturals: "\u2115",
        nbs: "\xA0",
        nbsp: "\xA0",
        nbump: "\u224E\u0338",
        nbumpe: "\u224F\u0338",
        ncap: "\u2A43",
        ncaron: "\u0148",
        ncedil: "\u0146",
        ncong: "\u2247",
        ncongdot: "\u2A6D\u0338",
        ncup: "\u2A42",
        ncy: "\u043D",
        ndash: "\u2013",
        ne: "\u2260",
        neArr: "\u21D7",
        nearhk: "\u2924",
        nearr: "\u2197",
        nearrow: "\u2197",
        nedot: "\u2250\u0338",
        nequiv: "\u2262",
        nesear: "\u2928",
        nesim: "\u2242\u0338",
        nexist: "\u2204",
        nexists: "\u2204",
        nfr: "\u{1D52B}",
        ngE: "\u2267\u0338",
        nge: "\u2271",
        ngeq: "\u2271",
        ngeqq: "\u2267\u0338",
        ngeqslant: "\u2A7E\u0338",
        nges: "\u2A7E\u0338",
        ngsim: "\u2275",
        ngt: "\u226F",
        ngtr: "\u226F",
        nhArr: "\u21CE",
        nharr: "\u21AE",
        nhpar: "\u2AF2",
        ni: "\u220B",
        nis: "\u22FC",
        nisd: "\u22FA",
        niv: "\u220B",
        njcy: "\u045A",
        nlArr: "\u21CD",
        nlE: "\u2266\u0338",
        nlarr: "\u219A",
        nldr: "\u2025",
        nle: "\u2270",
        nleftarrow: "\u219A",
        nleftrightarrow: "\u21AE",
        nleq: "\u2270",
        nleqq: "\u2266\u0338",
        nleqslant: "\u2A7D\u0338",
        nles: "\u2A7D\u0338",
        nless: "\u226E",
        nlsim: "\u2274",
        nlt: "\u226E",
        nltri: "\u22EA",
        nltrie: "\u22EC",
        nmid: "\u2224",
        nopf: "\u{1D55F}",
        no: "\xAC",
        not: "\xAC",
        notin: "\u2209",
        notinE: "\u22F9\u0338",
        notindot: "\u22F5\u0338",
        notinva: "\u2209",
        notinvb: "\u22F7",
        notinvc: "\u22F6",
        notni: "\u220C",
        notniva: "\u220C",
        notnivb: "\u22FE",
        notnivc: "\u22FD",
        npar: "\u2226",
        nparallel: "\u2226",
        nparsl: "\u2AFD\u20E5",
        npart: "\u2202\u0338",
        npolint: "\u2A14",
        npr: "\u2280",
        nprcue: "\u22E0",
        npre: "\u2AAF\u0338",
        nprec: "\u2280",
        npreceq: "\u2AAF\u0338",
        nrArr: "\u21CF",
        nrarr: "\u219B",
        nrarrc: "\u2933\u0338",
        nrarrw: "\u219D\u0338",
        nrightarrow: "\u219B",
        nrtri: "\u22EB",
        nrtrie: "\u22ED",
        nsc: "\u2281",
        nsccue: "\u22E1",
        nsce: "\u2AB0\u0338",
        nscr: "\u{1D4C3}",
        nshortmid: "\u2224",
        nshortparallel: "\u2226",
        nsim: "\u2241",
        nsime: "\u2244",
        nsimeq: "\u2244",
        nsmid: "\u2224",
        nspar: "\u2226",
        nsqsube: "\u22E2",
        nsqsupe: "\u22E3",
        nsub: "\u2284",
        nsubE: "\u2AC5\u0338",
        nsube: "\u2288",
        nsubset: "\u2282\u20D2",
        nsubseteq: "\u2288",
        nsubseteqq: "\u2AC5\u0338",
        nsucc: "\u2281",
        nsucceq: "\u2AB0\u0338",
        nsup: "\u2285",
        nsupE: "\u2AC6\u0338",
        nsupe: "\u2289",
        nsupset: "\u2283\u20D2",
        nsupseteq: "\u2289",
        nsupseteqq: "\u2AC6\u0338",
        ntgl: "\u2279",
        ntild: "\xF1",
        ntilde: "\xF1",
        ntlg: "\u2278",
        ntriangleleft: "\u22EA",
        ntrianglelefteq: "\u22EC",
        ntriangleright: "\u22EB",
        ntrianglerighteq: "\u22ED",
        nu: "\u03BD",
        num: "#",
        numero: "\u2116",
        numsp: "\u2007",
        nvDash: "\u22AD",
        nvHarr: "\u2904",
        nvap: "\u224D\u20D2",
        nvdash: "\u22AC",
        nvge: "\u2265\u20D2",
        nvgt: ">\u20D2",
        nvinfin: "\u29DE",
        nvlArr: "\u2902",
        nvle: "\u2264\u20D2",
        nvlt: "<\u20D2",
        nvltrie: "\u22B4\u20D2",
        nvrArr: "\u2903",
        nvrtrie: "\u22B5\u20D2",
        nvsim: "\u223C\u20D2",
        nwArr: "\u21D6",
        nwarhk: "\u2923",
        nwarr: "\u2196",
        nwarrow: "\u2196",
        nwnear: "\u2927",
        oS: "\u24C8",
        oacut: "\xF3",
        oacute: "\xF3",
        oast: "\u229B",
        ocir: "\xF4",
        ocirc: "\xF4",
        ocy: "\u043E",
        odash: "\u229D",
        odblac: "\u0151",
        odiv: "\u2A38",
        odot: "\u2299",
        odsold: "\u29BC",
        oelig: "\u0153",
        ofcir: "\u29BF",
        ofr: "\u{1D52C}",
        ogon: "\u02DB",
        ograv: "\xF2",
        ograve: "\xF2",
        ogt: "\u29C1",
        ohbar: "\u29B5",
        ohm: "\u03A9",
        oint: "\u222E",
        olarr: "\u21BA",
        olcir: "\u29BE",
        olcross: "\u29BB",
        oline: "\u203E",
        olt: "\u29C0",
        omacr: "\u014D",
        omega: "\u03C9",
        omicron: "\u03BF",
        omid: "\u29B6",
        ominus: "\u2296",
        oopf: "\u{1D560}",
        opar: "\u29B7",
        operp: "\u29B9",
        oplus: "\u2295",
        or: "\u2228",
        orarr: "\u21BB",
        ord: "\xBA",
        order: "\u2134",
        orderof: "\u2134",
        ordf: "\xAA",
        ordm: "\xBA",
        origof: "\u22B6",
        oror: "\u2A56",
        orslope: "\u2A57",
        orv: "\u2A5B",
        oscr: "\u2134",
        oslas: "\xF8",
        oslash: "\xF8",
        osol: "\u2298",
        otild: "\xF5",
        otilde: "\xF5",
        otimes: "\u2297",
        otimesas: "\u2A36",
        oum: "\xF6",
        ouml: "\xF6",
        ovbar: "\u233D",
        par: "\xB6",
        para: "\xB6",
        parallel: "\u2225",
        parsim: "\u2AF3",
        parsl: "\u2AFD",
        part: "\u2202",
        pcy: "\u043F",
        percnt: "%",
        period: ".",
        permil: "\u2030",
        perp: "\u22A5",
        pertenk: "\u2031",
        pfr: "\u{1D52D}",
        phi: "\u03C6",
        phiv: "\u03D5",
        phmmat: "\u2133",
        phone: "\u260E",
        pi: "\u03C0",
        pitchfork: "\u22D4",
        piv: "\u03D6",
        planck: "\u210F",
        planckh: "\u210E",
        plankv: "\u210F",
        plus: "+",
        plusacir: "\u2A23",
        plusb: "\u229E",
        pluscir: "\u2A22",
        plusdo: "\u2214",
        plusdu: "\u2A25",
        pluse: "\u2A72",
        plusm: "\xB1",
        plusmn: "\xB1",
        plussim: "\u2A26",
        plustwo: "\u2A27",
        pm: "\xB1",
        pointint: "\u2A15",
        popf: "\u{1D561}",
        poun: "\xA3",
        pound: "\xA3",
        pr: "\u227A",
        prE: "\u2AB3",
        prap: "\u2AB7",
        prcue: "\u227C",
        pre: "\u2AAF",
        prec: "\u227A",
        precapprox: "\u2AB7",
        preccurlyeq: "\u227C",
        preceq: "\u2AAF",
        precnapprox: "\u2AB9",
        precneqq: "\u2AB5",
        precnsim: "\u22E8",
        precsim: "\u227E",
        prime: "\u2032",
        primes: "\u2119",
        prnE: "\u2AB5",
        prnap: "\u2AB9",
        prnsim: "\u22E8",
        prod: "\u220F",
        profalar: "\u232E",
        profline: "\u2312",
        profsurf: "\u2313",
        prop: "\u221D",
        propto: "\u221D",
        prsim: "\u227E",
        prurel: "\u22B0",
        pscr: "\u{1D4C5}",
        psi: "\u03C8",
        puncsp: "\u2008",
        qfr: "\u{1D52E}",
        qint: "\u2A0C",
        qopf: "\u{1D562}",
        qprime: "\u2057",
        qscr: "\u{1D4C6}",
        quaternions: "\u210D",
        quatint: "\u2A16",
        quest: "?",
        questeq: "\u225F",
        quo: '"',
        quot: '"',
        rAarr: "\u21DB",
        rArr: "\u21D2",
        rAtail: "\u291C",
        rBarr: "\u290F",
        rHar: "\u2964",
        race: "\u223D\u0331",
        racute: "\u0155",
        radic: "\u221A",
        raemptyv: "\u29B3",
        rang: "\u27E9",
        rangd: "\u2992",
        range: "\u29A5",
        rangle: "\u27E9",
        raqu: "\xBB",
        raquo: "\xBB",
        rarr: "\u2192",
        rarrap: "\u2975",
        rarrb: "\u21E5",
        rarrbfs: "\u2920",
        rarrc: "\u2933",
        rarrfs: "\u291E",
        rarrhk: "\u21AA",
        rarrlp: "\u21AC",
        rarrpl: "\u2945",
        rarrsim: "\u2974",
        rarrtl: "\u21A3",
        rarrw: "\u219D",
        ratail: "\u291A",
        ratio: "\u2236",
        rationals: "\u211A",
        rbarr: "\u290D",
        rbbrk: "\u2773",
        rbrace: "}",
        rbrack: "]",
        rbrke: "\u298C",
        rbrksld: "\u298E",
        rbrkslu: "\u2990",
        rcaron: "\u0159",
        rcedil: "\u0157",
        rceil: "\u2309",
        rcub: "}",
        rcy: "\u0440",
        rdca: "\u2937",
        rdldhar: "\u2969",
        rdquo: "\u201D",
        rdquor: "\u201D",
        rdsh: "\u21B3",
        real: "\u211C",
        realine: "\u211B",
        realpart: "\u211C",
        reals: "\u211D",
        rect: "\u25AD",
        re: "\xAE",
        reg: "\xAE",
        rfisht: "\u297D",
        rfloor: "\u230B",
        rfr: "\u{1D52F}",
        rhard: "\u21C1",
        rharu: "\u21C0",
        rharul: "\u296C",
        rho: "\u03C1",
        rhov: "\u03F1",
        rightarrow: "\u2192",
        rightarrowtail: "\u21A3",
        rightharpoondown: "\u21C1",
        rightharpoonup: "\u21C0",
        rightleftarrows: "\u21C4",
        rightleftharpoons: "\u21CC",
        rightrightarrows: "\u21C9",
        rightsquigarrow: "\u219D",
        rightthreetimes: "\u22CC",
        ring: "\u02DA",
        risingdotseq: "\u2253",
        rlarr: "\u21C4",
        rlhar: "\u21CC",
        rlm: "\u200F",
        rmoust: "\u23B1",
        rmoustache: "\u23B1",
        rnmid: "\u2AEE",
        roang: "\u27ED",
        roarr: "\u21FE",
        robrk: "\u27E7",
        ropar: "\u2986",
        ropf: "\u{1D563}",
        roplus: "\u2A2E",
        rotimes: "\u2A35",
        rpar: ")",
        rpargt: "\u2994",
        rppolint: "\u2A12",
        rrarr: "\u21C9",
        rsaquo: "\u203A",
        rscr: "\u{1D4C7}",
        rsh: "\u21B1",
        rsqb: "]",
        rsquo: "\u2019",
        rsquor: "\u2019",
        rthree: "\u22CC",
        rtimes: "\u22CA",
        rtri: "\u25B9",
        rtrie: "\u22B5",
        rtrif: "\u25B8",
        rtriltri: "\u29CE",
        ruluhar: "\u2968",
        rx: "\u211E",
        sacute: "\u015B",
        sbquo: "\u201A",
        sc: "\u227B",
        scE: "\u2AB4",
        scap: "\u2AB8",
        scaron: "\u0161",
        sccue: "\u227D",
        sce: "\u2AB0",
        scedil: "\u015F",
        scirc: "\u015D",
        scnE: "\u2AB6",
        scnap: "\u2ABA",
        scnsim: "\u22E9",
        scpolint: "\u2A13",
        scsim: "\u227F",
        scy: "\u0441",
        sdot: "\u22C5",
        sdotb: "\u22A1",
        sdote: "\u2A66",
        seArr: "\u21D8",
        searhk: "\u2925",
        searr: "\u2198",
        searrow: "\u2198",
        sec: "\xA7",
        sect: "\xA7",
        semi: ";",
        seswar: "\u2929",
        setminus: "\u2216",
        setmn: "\u2216",
        sext: "\u2736",
        sfr: "\u{1D530}",
        sfrown: "\u2322",
        sharp: "\u266F",
        shchcy: "\u0449",
        shcy: "\u0448",
        shortmid: "\u2223",
        shortparallel: "\u2225",
        sh: "\xAD",
        shy: "\xAD",
        sigma: "\u03C3",
        sigmaf: "\u03C2",
        sigmav: "\u03C2",
        sim: "\u223C",
        simdot: "\u2A6A",
        sime: "\u2243",
        simeq: "\u2243",
        simg: "\u2A9E",
        simgE: "\u2AA0",
        siml: "\u2A9D",
        simlE: "\u2A9F",
        simne: "\u2246",
        simplus: "\u2A24",
        simrarr: "\u2972",
        slarr: "\u2190",
        smallsetminus: "\u2216",
        smashp: "\u2A33",
        smeparsl: "\u29E4",
        smid: "\u2223",
        smile: "\u2323",
        smt: "\u2AAA",
        smte: "\u2AAC",
        smtes: "\u2AAC\uFE00",
        softcy: "\u044C",
        sol: "/",
        solb: "\u29C4",
        solbar: "\u233F",
        sopf: "\u{1D564}",
        spades: "\u2660",
        spadesuit: "\u2660",
        spar: "\u2225",
        sqcap: "\u2293",
        sqcaps: "\u2293\uFE00",
        sqcup: "\u2294",
        sqcups: "\u2294\uFE00",
        sqsub: "\u228F",
        sqsube: "\u2291",
        sqsubset: "\u228F",
        sqsubseteq: "\u2291",
        sqsup: "\u2290",
        sqsupe: "\u2292",
        sqsupset: "\u2290",
        sqsupseteq: "\u2292",
        squ: "\u25A1",
        square: "\u25A1",
        squarf: "\u25AA",
        squf: "\u25AA",
        srarr: "\u2192",
        sscr: "\u{1D4C8}",
        ssetmn: "\u2216",
        ssmile: "\u2323",
        sstarf: "\u22C6",
        star: "\u2606",
        starf: "\u2605",
        straightepsilon: "\u03F5",
        straightphi: "\u03D5",
        strns: "\xAF",
        sub: "\u2282",
        subE: "\u2AC5",
        subdot: "\u2ABD",
        sube: "\u2286",
        subedot: "\u2AC3",
        submult: "\u2AC1",
        subnE: "\u2ACB",
        subne: "\u228A",
        subplus: "\u2ABF",
        subrarr: "\u2979",
        subset: "\u2282",
        subseteq: "\u2286",
        subseteqq: "\u2AC5",
        subsetneq: "\u228A",
        subsetneqq: "\u2ACB",
        subsim: "\u2AC7",
        subsub: "\u2AD5",
        subsup: "\u2AD3",
        succ: "\u227B",
        succapprox: "\u2AB8",
        succcurlyeq: "\u227D",
        succeq: "\u2AB0",
        succnapprox: "\u2ABA",
        succneqq: "\u2AB6",
        succnsim: "\u22E9",
        succsim: "\u227F",
        sum: "\u2211",
        sung: "\u266A",
        sup: "\u2283",
        sup1: "\xB9",
        sup2: "\xB2",
        sup3: "\xB3",
        supE: "\u2AC6",
        supdot: "\u2ABE",
        supdsub: "\u2AD8",
        supe: "\u2287",
        supedot: "\u2AC4",
        suphsol: "\u27C9",
        suphsub: "\u2AD7",
        suplarr: "\u297B",
        supmult: "\u2AC2",
        supnE: "\u2ACC",
        supne: "\u228B",
        supplus: "\u2AC0",
        supset: "\u2283",
        supseteq: "\u2287",
        supseteqq: "\u2AC6",
        supsetneq: "\u228B",
        supsetneqq: "\u2ACC",
        supsim: "\u2AC8",
        supsub: "\u2AD4",
        supsup: "\u2AD6",
        swArr: "\u21D9",
        swarhk: "\u2926",
        swarr: "\u2199",
        swarrow: "\u2199",
        swnwar: "\u292A",
        szli: "\xDF",
        szlig: "\xDF",
        target: "\u2316",
        tau: "\u03C4",
        tbrk: "\u23B4",
        tcaron: "\u0165",
        tcedil: "\u0163",
        tcy: "\u0442",
        tdot: "\u20DB",
        telrec: "\u2315",
        tfr: "\u{1D531}",
        there4: "\u2234",
        therefore: "\u2234",
        theta: "\u03B8",
        thetasym: "\u03D1",
        thetav: "\u03D1",
        thickapprox: "\u2248",
        thicksim: "\u223C",
        thinsp: "\u2009",
        thkap: "\u2248",
        thksim: "\u223C",
        thor: "\xFE",
        thorn: "\xFE",
        tilde: "\u02DC",
        time: "\xD7",
        times: "\xD7",
        timesb: "\u22A0",
        timesbar: "\u2A31",
        timesd: "\u2A30",
        tint: "\u222D",
        toea: "\u2928",
        top: "\u22A4",
        topbot: "\u2336",
        topcir: "\u2AF1",
        topf: "\u{1D565}",
        topfork: "\u2ADA",
        tosa: "\u2929",
        tprime: "\u2034",
        trade: "\u2122",
        triangle: "\u25B5",
        triangledown: "\u25BF",
        triangleleft: "\u25C3",
        trianglelefteq: "\u22B4",
        triangleq: "\u225C",
        triangleright: "\u25B9",
        trianglerighteq: "\u22B5",
        tridot: "\u25EC",
        trie: "\u225C",
        triminus: "\u2A3A",
        triplus: "\u2A39",
        trisb: "\u29CD",
        tritime: "\u2A3B",
        trpezium: "\u23E2",
        tscr: "\u{1D4C9}",
        tscy: "\u0446",
        tshcy: "\u045B",
        tstrok: "\u0167",
        twixt: "\u226C",
        twoheadleftarrow: "\u219E",
        twoheadrightarrow: "\u21A0",
        uArr: "\u21D1",
        uHar: "\u2963",
        uacut: "\xFA",
        uacute: "\xFA",
        uarr: "\u2191",
        ubrcy: "\u045E",
        ubreve: "\u016D",
        ucir: "\xFB",
        ucirc: "\xFB",
        ucy: "\u0443",
        udarr: "\u21C5",
        udblac: "\u0171",
        udhar: "\u296E",
        ufisht: "\u297E",
        ufr: "\u{1D532}",
        ugrav: "\xF9",
        ugrave: "\xF9",
        uharl: "\u21BF",
        uharr: "\u21BE",
        uhblk: "\u2580",
        ulcorn: "\u231C",
        ulcorner: "\u231C",
        ulcrop: "\u230F",
        ultri: "\u25F8",
        umacr: "\u016B",
        um: "\xA8",
        uml: "\xA8",
        uogon: "\u0173",
        uopf: "\u{1D566}",
        uparrow: "\u2191",
        updownarrow: "\u2195",
        upharpoonleft: "\u21BF",
        upharpoonright: "\u21BE",
        uplus: "\u228E",
        upsi: "\u03C5",
        upsih: "\u03D2",
        upsilon: "\u03C5",
        upuparrows: "\u21C8",
        urcorn: "\u231D",
        urcorner: "\u231D",
        urcrop: "\u230E",
        uring: "\u016F",
        urtri: "\u25F9",
        uscr: "\u{1D4CA}",
        utdot: "\u22F0",
        utilde: "\u0169",
        utri: "\u25B5",
        utrif: "\u25B4",
        uuarr: "\u21C8",
        uum: "\xFC",
        uuml: "\xFC",
        uwangle: "\u29A7",
        vArr: "\u21D5",
        vBar: "\u2AE8",
        vBarv: "\u2AE9",
        vDash: "\u22A8",
        vangrt: "\u299C",
        varepsilon: "\u03F5",
        varkappa: "\u03F0",
        varnothing: "\u2205",
        varphi: "\u03D5",
        varpi: "\u03D6",
        varpropto: "\u221D",
        varr: "\u2195",
        varrho: "\u03F1",
        varsigma: "\u03C2",
        varsubsetneq: "\u228A\uFE00",
        varsubsetneqq: "\u2ACB\uFE00",
        varsupsetneq: "\u228B\uFE00",
        varsupsetneqq: "\u2ACC\uFE00",
        vartheta: "\u03D1",
        vartriangleleft: "\u22B2",
        vartriangleright: "\u22B3",
        vcy: "\u0432",
        vdash: "\u22A2",
        vee: "\u2228",
        veebar: "\u22BB",
        veeeq: "\u225A",
        vellip: "\u22EE",
        verbar: "|",
        vert: "|",
        vfr: "\u{1D533}",
        vltri: "\u22B2",
        vnsub: "\u2282\u20D2",
        vnsup: "\u2283\u20D2",
        vopf: "\u{1D567}",
        vprop: "\u221D",
        vrtri: "\u22B3",
        vscr: "\u{1D4CB}",
        vsubnE: "\u2ACB\uFE00",
        vsubne: "\u228A\uFE00",
        vsupnE: "\u2ACC\uFE00",
        vsupne: "\u228B\uFE00",
        vzigzag: "\u299A",
        wcirc: "\u0175",
        wedbar: "\u2A5F",
        wedge: "\u2227",
        wedgeq: "\u2259",
        weierp: "\u2118",
        wfr: "\u{1D534}",
        wopf: "\u{1D568}",
        wp: "\u2118",
        wr: "\u2240",
        wreath: "\u2240",
        wscr: "\u{1D4CC}",
        xcap: "\u22C2",
        xcirc: "\u25EF",
        xcup: "\u22C3",
        xdtri: "\u25BD",
        xfr: "\u{1D535}",
        xhArr: "\u27FA",
        xharr: "\u27F7",
        xi: "\u03BE",
        xlArr: "\u27F8",
        xlarr: "\u27F5",
        xmap: "\u27FC",
        xnis: "\u22FB",
        xodot: "\u2A00",
        xopf: "\u{1D569}",
        xoplus: "\u2A01",
        xotime: "\u2A02",
        xrArr: "\u27F9",
        xrarr: "\u27F6",
        xscr: "\u{1D4CD}",
        xsqcup: "\u2A06",
        xuplus: "\u2A04",
        xutri: "\u25B3",
        xvee: "\u22C1",
        xwedge: "\u22C0",
        yacut: "\xFD",
        yacute: "\xFD",
        yacy: "\u044F",
        ycirc: "\u0177",
        ycy: "\u044B",
        ye: "\xA5",
        yen: "\xA5",
        yfr: "\u{1D536}",
        yicy: "\u0457",
        yopf: "\u{1D56A}",
        yscr: "\u{1D4CE}",
        yucy: "\u044E",
        yum: "\xFF",
        yuml: "\xFF",
        zacute: "\u017A",
        zcaron: "\u017E",
        zcy: "\u0437",
        zdot: "\u017C",
        zeetrf: "\u2128",
        zeta: "\u03B6",
        zfr: "\u{1D537}",
        zhcy: "\u0436",
        zigrarr: "\u21DD",
        zopf: "\u{1D56B}",
        zscr: "\u{1D4CF}",
        zwj: "\u200D",
        zwnj: "\u200C"
      };
    }
  });

  // node_modules/parse-entities/decode-entity.js
  var require_decode_entity = __commonJS({
    "node_modules/parse-entities/decode-entity.js"(exports, module) {
      "use strict";
      var characterEntities = require_character_entities();
      module.exports = decodeEntity;
      var own = {}.hasOwnProperty;
      function decodeEntity(characters) {
        return own.call(characterEntities, characters) ? characterEntities[characters] : false;
      }
    }
  });

  // node_modules/parse-entities/index.js
  var require_parse_entities = __commonJS({
    "node_modules/parse-entities/index.js"(exports, module) {
      "use strict";
      var legacy = require_character_entities_legacy();
      var invalid = require_character_reference_invalid();
      var decimal = require_is_decimal();
      var hexadecimal = require_is_hexadecimal();
      var alphanumerical = require_is_alphanumerical();
      var decodeEntity = require_decode_entity();
      module.exports = parseEntities;
      var own = {}.hasOwnProperty;
      var fromCharCode = String.fromCharCode;
      var noop2 = Function.prototype;
      var defaults = {
        warning: null,
        reference: null,
        text: null,
        warningContext: null,
        referenceContext: null,
        textContext: null,
        position: {},
        additional: null,
        attribute: false,
        nonTerminated: true
      };
      var tab = 9;
      var lineFeed = 10;
      var formFeed = 12;
      var space = 32;
      var ampersand = 38;
      var semicolon = 59;
      var lessThan = 60;
      var equalsTo = 61;
      var numberSign = 35;
      var uppercaseX = 88;
      var lowercaseX = 120;
      var replacementCharacter = 65533;
      var name = "named";
      var hexa = "hexadecimal";
      var deci = "decimal";
      var bases = {};
      bases[hexa] = 16;
      bases[deci] = 10;
      var tests = {};
      tests[name] = alphanumerical;
      tests[deci] = decimal;
      tests[hexa] = hexadecimal;
      var namedNotTerminated = 1;
      var numericNotTerminated = 2;
      var namedEmpty = 3;
      var numericEmpty = 4;
      var namedUnknown = 5;
      var numericDisallowed = 6;
      var numericProhibited = 7;
      var messages = {};
      messages[namedNotTerminated] = "Named character references must be terminated by a semicolon";
      messages[numericNotTerminated] = "Numeric character references must be terminated by a semicolon";
      messages[namedEmpty] = "Named character references cannot be empty";
      messages[numericEmpty] = "Numeric character references cannot be empty";
      messages[namedUnknown] = "Named character references must be known";
      messages[numericDisallowed] = "Numeric character references cannot be disallowed";
      messages[numericProhibited] = "Numeric character references cannot be outside the permissible Unicode range";
      function parseEntities(value, options2) {
        var settings = {};
        var option;
        var key;
        if (!options2) {
          options2 = {};
        }
        for (key in defaults) {
          option = options2[key];
          settings[key] = option === null || option === void 0 ? defaults[key] : option;
        }
        if (settings.position.indent || settings.position.start) {
          settings.indent = settings.position.indent || [];
          settings.position = settings.position.start;
        }
        return parse2(value, settings);
      }
      function parse2(value, settings) {
        var additional = settings.additional;
        var nonTerminated = settings.nonTerminated;
        var handleText = settings.text;
        var handleReference = settings.reference;
        var handleWarning = settings.warning;
        var textContext = settings.textContext;
        var referenceContext = settings.referenceContext;
        var warningContext = settings.warningContext;
        var pos = settings.position;
        var indent2 = settings.indent || [];
        var length = value.length;
        var index = 0;
        var lines = -1;
        var column = pos.column || 1;
        var line2 = pos.line || 1;
        var queue = "";
        var result = [];
        var entityCharacters;
        var namedEntity;
        var terminated;
        var characters;
        var character;
        var reference;
        var following;
        var warning;
        var reason;
        var output;
        var entity;
        var begin;
        var start;
        var type;
        var test;
        var prev;
        var next;
        var diff;
        var end;
        if (typeof additional === "string") {
          additional = additional.charCodeAt(0);
        }
        prev = now();
        warning = handleWarning ? parseError : noop2;
        index--;
        length++;
        while (++index < length) {
          if (character === lineFeed) {
            column = indent2[lines] || 1;
          }
          character = value.charCodeAt(index);
          if (character === ampersand) {
            following = value.charCodeAt(index + 1);
            if (following === tab || following === lineFeed || following === formFeed || following === space || following === ampersand || following === lessThan || following !== following || additional && following === additional) {
              queue += fromCharCode(character);
              column++;
              continue;
            }
            start = index + 1;
            begin = start;
            end = start;
            if (following === numberSign) {
              end = ++begin;
              following = value.charCodeAt(end);
              if (following === uppercaseX || following === lowercaseX) {
                type = hexa;
                end = ++begin;
              } else {
                type = deci;
              }
            } else {
              type = name;
            }
            entityCharacters = "";
            entity = "";
            characters = "";
            test = tests[type];
            end--;
            while (++end < length) {
              following = value.charCodeAt(end);
              if (!test(following)) {
                break;
              }
              characters += fromCharCode(following);
              if (type === name && own.call(legacy, characters)) {
                entityCharacters = characters;
                entity = legacy[characters];
              }
            }
            terminated = value.charCodeAt(end) === semicolon;
            if (terminated) {
              end++;
              namedEntity = type === name ? decodeEntity(characters) : false;
              if (namedEntity) {
                entityCharacters = characters;
                entity = namedEntity;
              }
            }
            diff = 1 + end - start;
            if (!terminated && !nonTerminated) {
            } else if (!characters) {
              if (type !== name) {
                warning(numericEmpty, diff);
              }
            } else if (type === name) {
              if (terminated && !entity) {
                warning(namedUnknown, 1);
              } else {
                if (entityCharacters !== characters) {
                  end = begin + entityCharacters.length;
                  diff = 1 + end - begin;
                  terminated = false;
                }
                if (!terminated) {
                  reason = entityCharacters ? namedNotTerminated : namedEmpty;
                  if (settings.attribute) {
                    following = value.charCodeAt(end);
                    if (following === equalsTo) {
                      warning(reason, diff);
                      entity = null;
                    } else if (alphanumerical(following)) {
                      entity = null;
                    } else {
                      warning(reason, diff);
                    }
                  } else {
                    warning(reason, diff);
                  }
                }
              }
              reference = entity;
            } else {
              if (!terminated) {
                warning(numericNotTerminated, diff);
              }
              reference = parseInt(characters, bases[type]);
              if (prohibited(reference)) {
                warning(numericProhibited, diff);
                reference = fromCharCode(replacementCharacter);
              } else if (reference in invalid) {
                warning(numericDisallowed, diff);
                reference = invalid[reference];
              } else {
                output = "";
                if (disallowed(reference)) {
                  warning(numericDisallowed, diff);
                }
                if (reference > 65535) {
                  reference -= 65536;
                  output += fromCharCode(reference >>> (10 & 1023) | 55296);
                  reference = 56320 | reference & 1023;
                }
                reference = output + fromCharCode(reference);
              }
            }
            if (reference) {
              flush();
              prev = now();
              index = end - 1;
              column += end - start + 1;
              result.push(reference);
              next = now();
              next.offset++;
              if (handleReference) {
                handleReference.call(
                  referenceContext,
                  reference,
                  { start: prev, end: next },
                  value.slice(start - 1, end)
                );
              }
              prev = next;
            } else {
              characters = value.slice(start - 1, end);
              queue += characters;
              column += characters.length;
              index = end - 1;
            }
          } else {
            if (character === 10) {
              line2++;
              lines++;
              column = 0;
            }
            if (character === character) {
              queue += fromCharCode(character);
              column++;
            } else {
              flush();
            }
          }
        }
        return result.join("");
        function now() {
          return {
            line: line2,
            column,
            offset: index + (pos.offset || 0)
          };
        }
        function parseError(code, offset) {
          var position = now();
          position.column += offset;
          position.offset += offset;
          handleWarning.call(warningContext, messages[code], position, code);
        }
        function flush() {
          if (queue) {
            result.push(queue);
            if (handleText) {
              handleText.call(textContext, queue, { start: prev, end: now() });
            }
            queue = "";
          }
        }
      }
      function prohibited(code) {
        return code >= 55296 && code <= 57343 || code > 1114111;
      }
      function disallowed(code) {
        return code >= 1 && code <= 8 || code === 11 || code >= 13 && code <= 31 || code >= 127 && code <= 159 || code >= 64976 && code <= 65007 || (code & 65535) === 65535 || (code & 65535) === 65534;
      }
    }
  });

  // node_modules/remark-parse/lib/decode.js
  var require_decode = __commonJS({
    "node_modules/remark-parse/lib/decode.js"(exports, module) {
      "use strict";
      var xtend = require_immutable();
      var entities = require_parse_entities();
      module.exports = factory;
      function factory(ctx) {
        decoder.raw = decodeRaw;
        return decoder;
        function normalize(position) {
          var offsets = ctx.offset;
          var line2 = position.line;
          var result = [];
          while (++line2) {
            if (!(line2 in offsets)) {
              break;
            }
            result.push((offsets[line2] || 0) + 1);
          }
          return { start: position, indent: result };
        }
        function decoder(value, position, handler) {
          entities(value, {
            position: normalize(position),
            warning: handleWarning,
            text: handler,
            reference: handler,
            textContext: ctx,
            referenceContext: ctx
          });
        }
        function decodeRaw(value, position, options2) {
          return entities(
            value,
            xtend(options2, { position: normalize(position), warning: handleWarning })
          );
        }
        function handleWarning(reason, position, code) {
          if (code !== 3) {
            ctx.file.message(reason, position);
          }
        }
      }
    }
  });

  // node_modules/remark-parse/lib/tokenizer.js
  var require_tokenizer = __commonJS({
    "node_modules/remark-parse/lib/tokenizer.js"(exports, module) {
      "use strict";
      module.exports = factory;
      function factory(type) {
        return tokenize;
        function tokenize(value, location) {
          var self = this;
          var offset = self.offset;
          var tokens = [];
          var methods = self[type + "Methods"];
          var tokenizers = self[type + "Tokenizers"];
          var line2 = location.line;
          var column = location.column;
          var index;
          var length;
          var method;
          var name;
          var matched;
          var valueLength;
          if (!value) {
            return tokens;
          }
          eat.now = now;
          eat.file = self.file;
          updatePosition("");
          while (value) {
            index = -1;
            length = methods.length;
            matched = false;
            while (++index < length) {
              name = methods[index];
              method = tokenizers[name];
              if (method && /* istanbul ignore next */
              (!method.onlyAtStart || self.atStart) && /* istanbul ignore next */
              (!method.notInList || !self.inList) && /* istanbul ignore next */
              (!method.notInBlock || !self.inBlock) && (!method.notInLink || !self.inLink)) {
                valueLength = value.length;
                method.apply(self, [eat, value]);
                matched = valueLength !== value.length;
                if (matched) {
                  break;
                }
              }
            }
            if (!matched) {
              self.file.fail(new Error("Infinite loop"), eat.now());
            }
          }
          self.eof = now();
          return tokens;
          function updatePosition(subvalue) {
            var lastIndex = -1;
            var index2 = subvalue.indexOf("\n");
            while (index2 !== -1) {
              line2++;
              lastIndex = index2;
              index2 = subvalue.indexOf("\n", index2 + 1);
            }
            if (lastIndex === -1) {
              column += subvalue.length;
            } else {
              column = subvalue.length - lastIndex;
            }
            if (line2 in offset) {
              if (lastIndex !== -1) {
                column += offset[line2];
              } else if (column <= offset[line2]) {
                column = offset[line2] + 1;
              }
            }
          }
          function getOffset() {
            var indentation = [];
            var pos = line2 + 1;
            return function() {
              var last = line2 + 1;
              while (pos < last) {
                indentation.push((offset[pos] || 0) + 1);
                pos++;
              }
              return indentation;
            };
          }
          function now() {
            var pos = { line: line2, column };
            pos.offset = self.toOffset(pos);
            return pos;
          }
          function Position(start) {
            this.start = start;
            this.end = now();
          }
          function validateEat(subvalue) {
            if (value.slice(0, subvalue.length) !== subvalue) {
              self.file.fail(
                new Error(
                  "Incorrectly eaten value: please report this warning on https://git.io/vg5Ft"
                ),
                now()
              );
            }
          }
          function position() {
            var before = now();
            return update;
            function update(node, indent2) {
              var previous = node.position;
              var start = previous ? previous.start : before;
              var combined = [];
              var n = previous && previous.end.line;
              var l = before.line;
              node.position = new Position(start);
              if (previous && indent2 && previous.indent) {
                combined = previous.indent;
                if (n < l) {
                  while (++n < l) {
                    combined.push((offset[n] || 0) + 1);
                  }
                  combined.push(before.column);
                }
                indent2 = combined.concat(indent2);
              }
              node.position.indent = indent2 || [];
              return node;
            }
          }
          function add(node, parent) {
            var children = parent ? parent.children : tokens;
            var previous = children[children.length - 1];
            var fn;
            if (previous && node.type === previous.type && (node.type === "text" || node.type === "blockquote") && mergeable(previous) && mergeable(node)) {
              fn = node.type === "text" ? mergeText : mergeBlockquote;
              node = fn.call(self, previous, node);
            }
            if (node !== previous) {
              children.push(node);
            }
            if (self.atStart && tokens.length !== 0) {
              self.exitStart();
            }
            return node;
          }
          function eat(subvalue) {
            var indent2 = getOffset();
            var pos = position();
            var current = now();
            validateEat(subvalue);
            apply.reset = reset;
            reset.test = test;
            apply.test = test;
            value = value.slice(subvalue.length);
            updatePosition(subvalue);
            indent2 = indent2();
            return apply;
            function apply(node, parent) {
              return pos(add(pos(node), parent), indent2);
            }
            function reset() {
              var node = apply.apply(null, arguments);
              line2 = current.line;
              column = current.column;
              value = subvalue + value;
              return node;
            }
            function test() {
              var result = pos({});
              line2 = current.line;
              column = current.column;
              value = subvalue + value;
              return result.position;
            }
          }
        }
      }
      function mergeable(node) {
        var start;
        var end;
        if (node.type !== "text" || !node.position) {
          return true;
        }
        start = node.position.start;
        end = node.position.end;
        return start.line !== end.line || end.column - start.column === node.value.length;
      }
      function mergeText(previous, node) {
        previous.value += node.value;
        return previous;
      }
      function mergeBlockquote(previous, node) {
        if (this.options.commonmark || this.options.gfm) {
          return node;
        }
        previous.children = previous.children.concat(node.children);
        return previous;
      }
    }
  });

  // node_modules/markdown-escapes/index.js
  var require_markdown_escapes = __commonJS({
    "node_modules/markdown-escapes/index.js"(exports, module) {
      "use strict";
      module.exports = escapes;
      var defaults = [
        "\\",
        "`",
        "*",
        "{",
        "}",
        "[",
        "]",
        "(",
        ")",
        "#",
        "+",
        "-",
        ".",
        "!",
        "_",
        ">"
      ];
      var gfm = defaults.concat(["~", "|"]);
      var commonmark = gfm.concat([
        "\n",
        '"',
        "$",
        "%",
        "&",
        "'",
        ",",
        "/",
        ":",
        ";",
        "<",
        "=",
        "?",
        "@",
        "^"
      ]);
      escapes.default = defaults;
      escapes.gfm = gfm;
      escapes.commonmark = commonmark;
      function escapes(options2) {
        var settings = options2 || {};
        if (settings.commonmark) {
          return commonmark;
        }
        return settings.gfm ? gfm : defaults;
      }
    }
  });

  // node_modules/remark-parse/lib/block-elements.js
  var require_block_elements = __commonJS({
    "node_modules/remark-parse/lib/block-elements.js"(exports, module) {
      "use strict";
      module.exports = [
        "address",
        "article",
        "aside",
        "base",
        "basefont",
        "blockquote",
        "body",
        "caption",
        "center",
        "col",
        "colgroup",
        "dd",
        "details",
        "dialog",
        "dir",
        "div",
        "dl",
        "dt",
        "fieldset",
        "figcaption",
        "figure",
        "footer",
        "form",
        "frame",
        "frameset",
        "h1",
        "h2",
        "h3",
        "h4",
        "h5",
        "h6",
        "head",
        "header",
        "hgroup",
        "hr",
        "html",
        "iframe",
        "legend",
        "li",
        "link",
        "main",
        "menu",
        "menuitem",
        "meta",
        "nav",
        "noframes",
        "ol",
        "optgroup",
        "option",
        "p",
        "param",
        "pre",
        "section",
        "source",
        "title",
        "summary",
        "table",
        "tbody",
        "td",
        "tfoot",
        "th",
        "thead",
        "title",
        "tr",
        "track",
        "ul"
      ];
    }
  });

  // node_modules/remark-parse/lib/defaults.js
  var require_defaults = __commonJS({
    "node_modules/remark-parse/lib/defaults.js"(exports, module) {
      "use strict";
      module.exports = {
        position: true,
        gfm: true,
        commonmark: false,
        pedantic: false,
        blocks: require_block_elements()
      };
    }
  });

  // node_modules/remark-parse/lib/set-options.js
  var require_set_options = __commonJS({
    "node_modules/remark-parse/lib/set-options.js"(exports, module) {
      "use strict";
      var xtend = require_immutable();
      var escapes = require_markdown_escapes();
      var defaults = require_defaults();
      module.exports = setOptions;
      function setOptions(options2) {
        var self = this;
        var current = self.options;
        var key;
        var value;
        if (options2 == null) {
          options2 = {};
        } else if (typeof options2 === "object") {
          options2 = xtend(options2);
        } else {
          throw new Error("Invalid value `" + options2 + "` for setting `options`");
        }
        for (key in defaults) {
          value = options2[key];
          if (value == null) {
            value = current[key];
          }
          if (key !== "blocks" && typeof value !== "boolean" || key === "blocks" && typeof value !== "object") {
            throw new Error(
              "Invalid value `" + value + "` for setting `options." + key + "`"
            );
          }
          options2[key] = value;
        }
        self.options = options2;
        self.escape = escapes(options2);
        return self;
      }
    }
  });

  // node_modules/unist-util-is/convert.js
  var require_convert = __commonJS({
    "node_modules/unist-util-is/convert.js"(exports, module) {
      "use strict";
      module.exports = convert;
      function convert(test) {
        if (test == null) {
          return ok;
        }
        if (typeof test === "string") {
          return typeFactory(test);
        }
        if (typeof test === "object") {
          return "length" in test ? anyFactory(test) : allFactory(test);
        }
        if (typeof test === "function") {
          return test;
        }
        throw new Error("Expected function, string, or object as test");
      }
      function allFactory(test) {
        return all;
        function all(node) {
          var key;
          for (key in test) {
            if (node[key] !== test[key])
              return false;
          }
          return true;
        }
      }
      function anyFactory(tests) {
        var checks = [];
        var index = -1;
        while (++index < tests.length) {
          checks[index] = convert(tests[index]);
        }
        return any;
        function any() {
          var index2 = -1;
          while (++index2 < checks.length) {
            if (checks[index2].apply(this, arguments)) {
              return true;
            }
          }
          return false;
        }
      }
      function typeFactory(test) {
        return type;
        function type(node) {
          return Boolean(node && node.type === test);
        }
      }
      function ok() {
        return true;
      }
    }
  });

  // node_modules/unist-util-visit-parents/color.browser.js
  var require_color_browser = __commonJS({
    "node_modules/unist-util-visit-parents/color.browser.js"(exports, module) {
      module.exports = identity2;
      function identity2(d) {
        return d;
      }
    }
  });

  // node_modules/unist-util-visit-parents/index.js
  var require_unist_util_visit_parents = __commonJS({
    "node_modules/unist-util-visit-parents/index.js"(exports, module) {
      "use strict";
      module.exports = visitParents;
      var convert = require_convert();
      var color = require_color_browser();
      var CONTINUE = true;
      var SKIP = "skip";
      var EXIT = false;
      visitParents.CONTINUE = CONTINUE;
      visitParents.SKIP = SKIP;
      visitParents.EXIT = EXIT;
      function visitParents(tree, test, visitor, reverse) {
        var step;
        var is;
        if (typeof test === "function" && typeof visitor !== "function") {
          reverse = visitor;
          visitor = test;
          test = null;
        }
        is = convert(test);
        step = reverse ? -1 : 1;
        factory(tree, null, [])();
        function factory(node, index, parents) {
          var value = typeof node === "object" && node !== null ? node : {};
          var name;
          if (typeof value.type === "string") {
            name = typeof value.tagName === "string" ? value.tagName : typeof value.name === "string" ? value.name : void 0;
            visit.displayName = "node (" + color(value.type + (name ? "<" + name + ">" : "")) + ")";
          }
          return visit;
          function visit() {
            var grandparents = parents.concat(node);
            var result = [];
            var subresult;
            var offset;
            if (!test || is(node, index, parents[parents.length - 1] || null)) {
              result = toResult(visitor(node, parents));
              if (result[0] === EXIT) {
                return result;
              }
            }
            if (node.children && result[0] !== SKIP) {
              offset = (reverse ? node.children.length : -1) + step;
              while (offset > -1 && offset < node.children.length) {
                subresult = factory(node.children[offset], offset, grandparents)();
                if (subresult[0] === EXIT) {
                  return subresult;
                }
                offset = typeof subresult[1] === "number" ? subresult[1] : offset + step;
              }
            }
            return result;
          }
        }
      }
      function toResult(value) {
        if (value !== null && typeof value === "object" && "length" in value) {
          return value;
        }
        if (typeof value === "number") {
          return [CONTINUE, value];
        }
        return [value];
      }
    }
  });

  // node_modules/unist-util-visit/index.js
  var require_unist_util_visit = __commonJS({
    "node_modules/unist-util-visit/index.js"(exports, module) {
      "use strict";
      module.exports = visit;
      var visitParents = require_unist_util_visit_parents();
      var CONTINUE = visitParents.CONTINUE;
      var SKIP = visitParents.SKIP;
      var EXIT = visitParents.EXIT;
      visit.CONTINUE = CONTINUE;
      visit.SKIP = SKIP;
      visit.EXIT = EXIT;
      function visit(tree, test, visitor, reverse) {
        if (typeof test === "function" && typeof visitor !== "function") {
          reverse = visitor;
          visitor = test;
          test = null;
        }
        visitParents(tree, test, overload, reverse);
        function overload(node, parents) {
          var parent = parents[parents.length - 1];
          var index = parent ? parent.children.indexOf(node) : null;
          return visitor(node, index, parent);
        }
      }
    }
  });

  // node_modules/unist-util-remove-position/index.js
  var require_unist_util_remove_position = __commonJS({
    "node_modules/unist-util-remove-position/index.js"(exports, module) {
      "use strict";
      var visit = require_unist_util_visit();
      module.exports = removePosition;
      function removePosition(node, force) {
        visit(node, force ? hard : soft);
        return node;
      }
      function hard(node) {
        delete node.position;
      }
      function soft(node) {
        node.position = void 0;
      }
    }
  });

  // node_modules/remark-parse/lib/parse.js
  var require_parse = __commonJS({
    "node_modules/remark-parse/lib/parse.js"(exports, module) {
      "use strict";
      var xtend = require_immutable();
      var removePosition = require_unist_util_remove_position();
      module.exports = parse2;
      var lineFeed = "\n";
      var lineBreaksExpression = /\r\n|\r/g;
      function parse2() {
        var self = this;
        var value = String(self.file);
        var start = { line: 1, column: 1, offset: 0 };
        var content = xtend(start);
        var node;
        value = value.replace(lineBreaksExpression, lineFeed);
        if (value.charCodeAt(0) === 65279) {
          value = value.slice(1);
          content.column++;
          content.offset++;
        }
        node = {
          type: "root",
          children: self.tokenizeBlock(value, content),
          position: { start, end: self.eof || xtend(start) }
        };
        if (!self.options.position) {
          removePosition(node, true);
        }
        return node;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/blank-line.js
  var require_blank_line = __commonJS({
    "node_modules/remark-parse/lib/tokenize/blank-line.js"(exports, module) {
      "use strict";
      var reBlankLine = /^[ \t]*(\n|$)/;
      module.exports = blankLine;
      function blankLine(eat, value, silent) {
        var match;
        var subvalue = "";
        var index = 0;
        var length = value.length;
        while (index < length) {
          match = reBlankLine.exec(value.slice(index));
          if (match == null) {
            break;
          }
          index += match[0].length;
          subvalue += match[0];
        }
        if (subvalue === "") {
          return;
        }
        if (silent) {
          return true;
        }
        eat(subvalue);
      }
    }
  });

  // node_modules/repeat-string/index.js
  var require_repeat_string = __commonJS({
    "node_modules/repeat-string/index.js"(exports, module) {
      "use strict";
      var res = "";
      var cache;
      module.exports = repeat;
      function repeat(str, num) {
        if (typeof str !== "string") {
          throw new TypeError("expected a string");
        }
        if (num === 1)
          return str;
        if (num === 2)
          return str + str;
        var max = str.length * num;
        if (cache !== str || typeof cache === "undefined") {
          cache = str;
          res = "";
        } else if (res.length >= max) {
          return res.substr(0, max);
        }
        while (max > res.length && num > 1) {
          if (num & 1) {
            res += str;
          }
          num >>= 1;
          str += str;
        }
        res += str;
        res = res.substr(0, max);
        return res;
      }
    }
  });

  // node_modules/trim-trailing-lines/index.js
  var require_trim_trailing_lines = __commonJS({
    "node_modules/trim-trailing-lines/index.js"(exports, module) {
      "use strict";
      module.exports = trimTrailingLines;
      function trimTrailingLines(value) {
        return String(value).replace(/\n+$/, "");
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/code-indented.js
  var require_code_indented = __commonJS({
    "node_modules/remark-parse/lib/tokenize/code-indented.js"(exports, module) {
      "use strict";
      var repeat = require_repeat_string();
      var trim2 = require_trim_trailing_lines();
      module.exports = indentedCode;
      var lineFeed = "\n";
      var tab = "	";
      var space = " ";
      var tabSize = 4;
      var codeIndent = repeat(space, tabSize);
      function indentedCode(eat, value, silent) {
        var index = -1;
        var length = value.length;
        var subvalue = "";
        var content = "";
        var subvalueQueue = "";
        var contentQueue = "";
        var character;
        var blankQueue;
        var indent2;
        while (++index < length) {
          character = value.charAt(index);
          if (indent2) {
            indent2 = false;
            subvalue += subvalueQueue;
            content += contentQueue;
            subvalueQueue = "";
            contentQueue = "";
            if (character === lineFeed) {
              subvalueQueue = character;
              contentQueue = character;
            } else {
              subvalue += character;
              content += character;
              while (++index < length) {
                character = value.charAt(index);
                if (!character || character === lineFeed) {
                  contentQueue = character;
                  subvalueQueue = character;
                  break;
                }
                subvalue += character;
                content += character;
              }
            }
          } else if (character === space && value.charAt(index + 1) === character && value.charAt(index + 2) === character && value.charAt(index + 3) === character) {
            subvalueQueue += codeIndent;
            index += 3;
            indent2 = true;
          } else if (character === tab) {
            subvalueQueue += character;
            indent2 = true;
          } else {
            blankQueue = "";
            while (character === tab || character === space) {
              blankQueue += character;
              character = value.charAt(++index);
            }
            if (character !== lineFeed) {
              break;
            }
            subvalueQueue += blankQueue + character;
            contentQueue += character;
          }
        }
        if (content) {
          if (silent) {
            return true;
          }
          return eat(subvalue)({
            type: "code",
            lang: null,
            meta: null,
            value: trim2(content)
          });
        }
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/code-fenced.js
  var require_code_fenced = __commonJS({
    "node_modules/remark-parse/lib/tokenize/code-fenced.js"(exports, module) {
      "use strict";
      module.exports = fencedCode;
      var lineFeed = "\n";
      var tab = "	";
      var space = " ";
      var tilde = "~";
      var graveAccent = "`";
      var minFenceCount = 3;
      var tabSize = 4;
      function fencedCode(eat, value, silent) {
        var self = this;
        var gfm = self.options.gfm;
        var length = value.length + 1;
        var index = 0;
        var subvalue = "";
        var fenceCount;
        var marker;
        var character;
        var flag;
        var lang;
        var meta;
        var queue;
        var content;
        var exdentedContent;
        var closing;
        var exdentedClosing;
        var indent2;
        var now;
        if (!gfm) {
          return;
        }
        while (index < length) {
          character = value.charAt(index);
          if (character !== space && character !== tab) {
            break;
          }
          subvalue += character;
          index++;
        }
        indent2 = index;
        character = value.charAt(index);
        if (character !== tilde && character !== graveAccent) {
          return;
        }
        index++;
        marker = character;
        fenceCount = 1;
        subvalue += character;
        while (index < length) {
          character = value.charAt(index);
          if (character !== marker) {
            break;
          }
          subvalue += character;
          fenceCount++;
          index++;
        }
        if (fenceCount < minFenceCount) {
          return;
        }
        while (index < length) {
          character = value.charAt(index);
          if (character !== space && character !== tab) {
            break;
          }
          subvalue += character;
          index++;
        }
        flag = "";
        queue = "";
        while (index < length) {
          character = value.charAt(index);
          if (character === lineFeed || marker === graveAccent && character === marker) {
            break;
          }
          if (character === space || character === tab) {
            queue += character;
          } else {
            flag += queue + character;
            queue = "";
          }
          index++;
        }
        character = value.charAt(index);
        if (character && character !== lineFeed) {
          return;
        }
        if (silent) {
          return true;
        }
        now = eat.now();
        now.column += subvalue.length;
        now.offset += subvalue.length;
        subvalue += flag;
        flag = self.decode.raw(self.unescape(flag), now);
        if (queue) {
          subvalue += queue;
        }
        queue = "";
        closing = "";
        exdentedClosing = "";
        content = "";
        exdentedContent = "";
        var skip = true;
        while (index < length) {
          character = value.charAt(index);
          content += closing;
          exdentedContent += exdentedClosing;
          closing = "";
          exdentedClosing = "";
          if (character !== lineFeed) {
            content += character;
            exdentedClosing += character;
            index++;
            continue;
          }
          if (skip) {
            subvalue += character;
            skip = false;
          } else {
            closing += character;
            exdentedClosing += character;
          }
          queue = "";
          index++;
          while (index < length) {
            character = value.charAt(index);
            if (character !== space) {
              break;
            }
            queue += character;
            index++;
          }
          closing += queue;
          exdentedClosing += queue.slice(indent2);
          if (queue.length >= tabSize) {
            continue;
          }
          queue = "";
          while (index < length) {
            character = value.charAt(index);
            if (character !== marker) {
              break;
            }
            queue += character;
            index++;
          }
          closing += queue;
          exdentedClosing += queue;
          if (queue.length < fenceCount) {
            continue;
          }
          queue = "";
          while (index < length) {
            character = value.charAt(index);
            if (character !== space && character !== tab) {
              break;
            }
            closing += character;
            exdentedClosing += character;
            index++;
          }
          if (!character || character === lineFeed) {
            break;
          }
        }
        subvalue += content + closing;
        index = -1;
        length = flag.length;
        while (++index < length) {
          character = flag.charAt(index);
          if (character === space || character === tab) {
            if (!lang) {
              lang = flag.slice(0, index);
            }
          } else if (lang) {
            meta = flag.slice(index);
            break;
          }
        }
        return eat(subvalue)({
          type: "code",
          lang: lang || flag || null,
          meta: meta || null,
          value: exdentedContent
        });
      }
    }
  });

  // node_modules/trim/index.js
  var require_trim = __commonJS({
    "node_modules/trim/index.js"(exports, module) {
      exports = module.exports = trim2;
      function trim2(str) {
        if (str.trim)
          return str.trim();
        return exports.right(exports.left(str));
      }
      exports.left = function(str) {
        if (str.trimLeft)
          return str.trimLeft();
        return str.replace(/^\s\s*/, "");
      };
      exports.right = function(str) {
        if (str.trimRight)
          return str.trimRight();
        var whitespace_pattern = /\s/, i = str.length;
        while (whitespace_pattern.test(str.charAt(--i)))
          ;
        return str.slice(0, i + 1);
      };
    }
  });

  // node_modules/remark-parse/lib/util/interrupt.js
  var require_interrupt = __commonJS({
    "node_modules/remark-parse/lib/util/interrupt.js"(exports, module) {
      "use strict";
      module.exports = interrupt;
      function interrupt(interruptors, tokenizers, ctx, parameters) {
        var length = interruptors.length;
        var index = -1;
        var interruptor;
        var config;
        while (++index < length) {
          interruptor = interruptors[index];
          config = interruptor[1] || {};
          if (config.pedantic !== void 0 && config.pedantic !== ctx.options.pedantic) {
            continue;
          }
          if (config.commonmark !== void 0 && config.commonmark !== ctx.options.commonmark) {
            continue;
          }
          if (tokenizers[interruptor[0]].apply(ctx, parameters)) {
            return true;
          }
        }
        return false;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/blockquote.js
  var require_blockquote = __commonJS({
    "node_modules/remark-parse/lib/tokenize/blockquote.js"(exports, module) {
      "use strict";
      var trim2 = require_trim();
      var interrupt = require_interrupt();
      module.exports = blockquote;
      var lineFeed = "\n";
      var tab = "	";
      var space = " ";
      var greaterThan = ">";
      function blockquote(eat, value, silent) {
        var self = this;
        var offsets = self.offset;
        var tokenizers = self.blockTokenizers;
        var interruptors = self.interruptBlockquote;
        var now = eat.now();
        var currentLine = now.line;
        var length = value.length;
        var values = [];
        var contents = [];
        var indents = [];
        var add;
        var index = 0;
        var character;
        var rest;
        var nextIndex;
        var content;
        var line2;
        var startIndex;
        var prefixed;
        var exit;
        while (index < length) {
          character = value.charAt(index);
          if (character !== space && character !== tab) {
            break;
          }
          index++;
        }
        if (value.charAt(index) !== greaterThan) {
          return;
        }
        if (silent) {
          return true;
        }
        index = 0;
        while (index < length) {
          nextIndex = value.indexOf(lineFeed, index);
          startIndex = index;
          prefixed = false;
          if (nextIndex === -1) {
            nextIndex = length;
          }
          while (index < length) {
            character = value.charAt(index);
            if (character !== space && character !== tab) {
              break;
            }
            index++;
          }
          if (value.charAt(index) === greaterThan) {
            index++;
            prefixed = true;
            if (value.charAt(index) === space) {
              index++;
            }
          } else {
            index = startIndex;
          }
          content = value.slice(index, nextIndex);
          if (!prefixed && !trim2(content)) {
            index = startIndex;
            break;
          }
          if (!prefixed) {
            rest = value.slice(index);
            if (interrupt(interruptors, tokenizers, self, [eat, rest, true])) {
              break;
            }
          }
          line2 = startIndex === index ? content : value.slice(startIndex, nextIndex);
          indents.push(index - startIndex);
          values.push(line2);
          contents.push(content);
          index = nextIndex + 1;
        }
        index = -1;
        length = indents.length;
        add = eat(values.join(lineFeed));
        while (++index < length) {
          offsets[currentLine] = (offsets[currentLine] || 0) + indents[index];
          currentLine++;
        }
        exit = self.enterBlock();
        contents = self.tokenizeBlock(contents.join(lineFeed), now);
        exit();
        return add({ type: "blockquote", children: contents });
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/heading-atx.js
  var require_heading_atx = __commonJS({
    "node_modules/remark-parse/lib/tokenize/heading-atx.js"(exports, module) {
      "use strict";
      module.exports = atxHeading;
      var lineFeed = "\n";
      var tab = "	";
      var space = " ";
      var numberSign = "#";
      var maxFenceCount = 6;
      function atxHeading(eat, value, silent) {
        var self = this;
        var pedantic = self.options.pedantic;
        var length = value.length + 1;
        var index = -1;
        var now = eat.now();
        var subvalue = "";
        var content = "";
        var character;
        var queue;
        var depth;
        while (++index < length) {
          character = value.charAt(index);
          if (character !== space && character !== tab) {
            index--;
            break;
          }
          subvalue += character;
        }
        depth = 0;
        while (++index <= length) {
          character = value.charAt(index);
          if (character !== numberSign) {
            index--;
            break;
          }
          subvalue += character;
          depth++;
        }
        if (depth > maxFenceCount) {
          return;
        }
        if (!depth || !pedantic && value.charAt(index + 1) === numberSign) {
          return;
        }
        length = value.length + 1;
        queue = "";
        while (++index < length) {
          character = value.charAt(index);
          if (character !== space && character !== tab) {
            index--;
            break;
          }
          queue += character;
        }
        if (!pedantic && queue.length === 0 && character && character !== lineFeed) {
          return;
        }
        if (silent) {
          return true;
        }
        subvalue += queue;
        queue = "";
        content = "";
        while (++index < length) {
          character = value.charAt(index);
          if (!character || character === lineFeed) {
            break;
          }
          if (character !== space && character !== tab && character !== numberSign) {
            content += queue + character;
            queue = "";
            continue;
          }
          while (character === space || character === tab) {
            queue += character;
            character = value.charAt(++index);
          }
          if (!pedantic && content && !queue && character === numberSign) {
            content += character;
            continue;
          }
          while (character === numberSign) {
            queue += character;
            character = value.charAt(++index);
          }
          while (character === space || character === tab) {
            queue += character;
            character = value.charAt(++index);
          }
          index--;
        }
        now.column += subvalue.length;
        now.offset += subvalue.length;
        subvalue += content + queue;
        return eat(subvalue)({
          type: "heading",
          depth,
          children: self.tokenizeInline(content, now)
        });
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/thematic-break.js
  var require_thematic_break = __commonJS({
    "node_modules/remark-parse/lib/tokenize/thematic-break.js"(exports, module) {
      "use strict";
      module.exports = thematicBreak;
      var tab = "	";
      var lineFeed = "\n";
      var space = " ";
      var asterisk = "*";
      var dash = "-";
      var underscore = "_";
      var maxCount = 3;
      function thematicBreak(eat, value, silent) {
        var index = -1;
        var length = value.length + 1;
        var subvalue = "";
        var character;
        var marker;
        var markerCount;
        var queue;
        while (++index < length) {
          character = value.charAt(index);
          if (character !== tab && character !== space) {
            break;
          }
          subvalue += character;
        }
        if (character !== asterisk && character !== dash && character !== underscore) {
          return;
        }
        marker = character;
        subvalue += character;
        markerCount = 1;
        queue = "";
        while (++index < length) {
          character = value.charAt(index);
          if (character === marker) {
            markerCount++;
            subvalue += queue + marker;
            queue = "";
          } else if (character === space) {
            queue += character;
          } else if (markerCount >= maxCount && (!character || character === lineFeed)) {
            subvalue += queue;
            if (silent) {
              return true;
            }
            return eat(subvalue)({ type: "thematicBreak" });
          } else {
            return;
          }
        }
      }
    }
  });

  // node_modules/remark-parse/lib/util/get-indentation.js
  var require_get_indentation = __commonJS({
    "node_modules/remark-parse/lib/util/get-indentation.js"(exports, module) {
      "use strict";
      module.exports = indentation;
      var tab = "	";
      var space = " ";
      var spaceSize = 1;
      var tabSize = 4;
      function indentation(value) {
        var index = 0;
        var indent2 = 0;
        var character = value.charAt(index);
        var stops = {};
        var size;
        var lastIndent = 0;
        while (character === tab || character === space) {
          size = character === tab ? tabSize : spaceSize;
          indent2 += size;
          if (size > 1) {
            indent2 = Math.floor(indent2 / size) * size;
          }
          while (lastIndent < indent2) {
            stops[++lastIndent] = index;
          }
          character = value.charAt(++index);
        }
        return { indent: indent2, stops };
      }
    }
  });

  // node_modules/remark-parse/lib/util/remove-indentation.js
  var require_remove_indentation = __commonJS({
    "node_modules/remark-parse/lib/util/remove-indentation.js"(exports, module) {
      "use strict";
      var trim2 = require_trim();
      var repeat = require_repeat_string();
      var getIndent = require_get_indentation();
      module.exports = indentation;
      var lineFeed = "\n";
      var space = " ";
      var exclamationMark = "!";
      function indentation(value, maximum) {
        var values = value.split(lineFeed);
        var position = values.length + 1;
        var minIndent = Infinity;
        var matrix = [];
        var index;
        var indentation2;
        var stops;
        values.unshift(repeat(space, maximum) + exclamationMark);
        while (position--) {
          indentation2 = getIndent(values[position]);
          matrix[position] = indentation2.stops;
          if (trim2(values[position]).length === 0) {
            continue;
          }
          if (indentation2.indent) {
            if (indentation2.indent > 0 && indentation2.indent < minIndent) {
              minIndent = indentation2.indent;
            }
          } else {
            minIndent = Infinity;
            break;
          }
        }
        if (minIndent !== Infinity) {
          position = values.length;
          while (position--) {
            stops = matrix[position];
            index = minIndent;
            while (index && !(index in stops)) {
              index--;
            }
            values[position] = values[position].slice(stops[index] + 1);
          }
        }
        values.shift();
        return values.join(lineFeed);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/list.js
  var require_list = __commonJS({
    "node_modules/remark-parse/lib/tokenize/list.js"(exports, module) {
      "use strict";
      var trim2 = require_trim();
      var repeat = require_repeat_string();
      var decimal = require_is_decimal();
      var getIndent = require_get_indentation();
      var removeIndent = require_remove_indentation();
      var interrupt = require_interrupt();
      module.exports = list;
      var asterisk = "*";
      var underscore = "_";
      var plusSign = "+";
      var dash = "-";
      var dot = ".";
      var space = " ";
      var lineFeed = "\n";
      var tab = "	";
      var rightParenthesis = ")";
      var lowercaseX = "x";
      var tabSize = 4;
      var looseListItemExpression = /\n\n(?!\s*$)/;
      var taskItemExpression = /^\[([ X\tx])][ \t]/;
      var bulletExpression = /^([ \t]*)([*+-]|\d+[.)])( {1,4}(?! )| |\t|$|(?=\n))([^\n]*)/;
      var pedanticBulletExpression = /^([ \t]*)([*+-]|\d+[.)])([ \t]+)/;
      var initialIndentExpression = /^( {1,4}|\t)?/gm;
      function list(eat, value, silent) {
        var self = this;
        var commonmark = self.options.commonmark;
        var pedantic = self.options.pedantic;
        var tokenizers = self.blockTokenizers;
        var interuptors = self.interruptList;
        var index = 0;
        var length = value.length;
        var start = null;
        var size;
        var queue;
        var ordered;
        var character;
        var marker;
        var nextIndex;
        var startIndex;
        var prefixed;
        var currentMarker;
        var content;
        var line2;
        var previousEmpty;
        var empty;
        var items;
        var allLines;
        var emptyLines;
        var item;
        var enterTop;
        var exitBlockquote;
        var spread = false;
        var node;
        var now;
        var end;
        var indented;
        while (index < length) {
          character = value.charAt(index);
          if (character !== tab && character !== space) {
            break;
          }
          index++;
        }
        character = value.charAt(index);
        if (character === asterisk || character === plusSign || character === dash) {
          marker = character;
          ordered = false;
        } else {
          ordered = true;
          queue = "";
          while (index < length) {
            character = value.charAt(index);
            if (!decimal(character)) {
              break;
            }
            queue += character;
            index++;
          }
          character = value.charAt(index);
          if (!queue || !(character === dot || commonmark && character === rightParenthesis)) {
            return;
          }
          if (silent && queue !== "1") {
            return;
          }
          start = parseInt(queue, 10);
          marker = character;
        }
        character = value.charAt(++index);
        if (character !== space && character !== tab && (pedantic || character !== lineFeed && character !== "")) {
          return;
        }
        if (silent) {
          return true;
        }
        index = 0;
        items = [];
        allLines = [];
        emptyLines = [];
        while (index < length) {
          nextIndex = value.indexOf(lineFeed, index);
          startIndex = index;
          prefixed = false;
          indented = false;
          if (nextIndex === -1) {
            nextIndex = length;
          }
          size = 0;
          while (index < length) {
            character = value.charAt(index);
            if (character === tab) {
              size += tabSize - size % tabSize;
            } else if (character === space) {
              size++;
            } else {
              break;
            }
            index++;
          }
          if (item && size >= item.indent) {
            indented = true;
          }
          character = value.charAt(index);
          currentMarker = null;
          if (!indented) {
            if (character === asterisk || character === plusSign || character === dash) {
              currentMarker = character;
              index++;
              size++;
            } else {
              queue = "";
              while (index < length) {
                character = value.charAt(index);
                if (!decimal(character)) {
                  break;
                }
                queue += character;
                index++;
              }
              character = value.charAt(index);
              index++;
              if (queue && (character === dot || commonmark && character === rightParenthesis)) {
                currentMarker = character;
                size += queue.length + 1;
              }
            }
            if (currentMarker) {
              character = value.charAt(index);
              if (character === tab) {
                size += tabSize - size % tabSize;
                index++;
              } else if (character === space) {
                end = index + tabSize;
                while (index < end) {
                  if (value.charAt(index) !== space) {
                    break;
                  }
                  index++;
                  size++;
                }
                if (index === end && value.charAt(index) === space) {
                  index -= tabSize - 1;
                  size -= tabSize - 1;
                }
              } else if (character !== lineFeed && character !== "") {
                currentMarker = null;
              }
            }
          }
          if (currentMarker) {
            if (!pedantic && marker !== currentMarker) {
              break;
            }
            prefixed = true;
          } else {
            if (!commonmark && !indented && value.charAt(startIndex) === space) {
              indented = true;
            } else if (commonmark && item) {
              indented = size >= item.indent || size > tabSize;
            }
            prefixed = false;
            index = startIndex;
          }
          line2 = value.slice(startIndex, nextIndex);
          content = startIndex === index ? line2 : value.slice(index, nextIndex);
          if (currentMarker === asterisk || currentMarker === underscore || currentMarker === dash) {
            if (tokenizers.thematicBreak.call(self, eat, line2, true)) {
              break;
            }
          }
          previousEmpty = empty;
          empty = !prefixed && !trim2(content).length;
          if (indented && item) {
            item.value = item.value.concat(emptyLines, line2);
            allLines = allLines.concat(emptyLines, line2);
            emptyLines = [];
          } else if (prefixed) {
            if (emptyLines.length !== 0) {
              spread = true;
              item.value.push("");
              item.trail = emptyLines.concat();
            }
            item = {
              value: [line2],
              indent: size,
              trail: []
            };
            items.push(item);
            allLines = allLines.concat(emptyLines, line2);
            emptyLines = [];
          } else if (empty) {
            if (previousEmpty && !commonmark) {
              break;
            }
            emptyLines.push(line2);
          } else {
            if (previousEmpty) {
              break;
            }
            if (interrupt(interuptors, tokenizers, self, [eat, line2, true])) {
              break;
            }
            item.value = item.value.concat(emptyLines, line2);
            allLines = allLines.concat(emptyLines, line2);
            emptyLines = [];
          }
          index = nextIndex + 1;
        }
        node = eat(allLines.join(lineFeed)).reset({
          type: "list",
          ordered,
          start,
          spread,
          children: []
        });
        enterTop = self.enterList();
        exitBlockquote = self.enterBlock();
        index = -1;
        length = items.length;
        while (++index < length) {
          item = items[index].value.join(lineFeed);
          now = eat.now();
          eat(item)(listItem(self, item, now), node);
          item = items[index].trail.join(lineFeed);
          if (index !== length - 1) {
            item += lineFeed;
          }
          eat(item);
        }
        enterTop();
        exitBlockquote();
        return node;
      }
      function listItem(ctx, value, position) {
        var offsets = ctx.offset;
        var fn = ctx.options.pedantic ? pedanticListItem : normalListItem;
        var checked = null;
        var task;
        var indent2;
        value = fn.apply(null, arguments);
        if (ctx.options.gfm) {
          task = value.match(taskItemExpression);
          if (task) {
            indent2 = task[0].length;
            checked = task[1].toLowerCase() === lowercaseX;
            offsets[position.line] += indent2;
            value = value.slice(indent2);
          }
        }
        return {
          type: "listItem",
          spread: looseListItemExpression.test(value),
          checked,
          children: ctx.tokenizeBlock(value, position)
        };
      }
      function pedanticListItem(ctx, value, position) {
        var offsets = ctx.offset;
        var line2 = position.line;
        value = value.replace(pedanticBulletExpression, replacer);
        line2 = position.line;
        return value.replace(initialIndentExpression, replacer);
        function replacer($0) {
          offsets[line2] = (offsets[line2] || 0) + $0.length;
          line2++;
          return "";
        }
      }
      function normalListItem(ctx, value, position) {
        var offsets = ctx.offset;
        var line2 = position.line;
        var max;
        var bullet;
        var rest;
        var lines;
        var trimmedLines;
        var index;
        var length;
        value = value.replace(bulletExpression, replacer);
        lines = value.split(lineFeed);
        trimmedLines = removeIndent(value, getIndent(max).indent).split(lineFeed);
        trimmedLines[0] = rest;
        offsets[line2] = (offsets[line2] || 0) + bullet.length;
        line2++;
        index = 0;
        length = lines.length;
        while (++index < length) {
          offsets[line2] = (offsets[line2] || 0) + lines[index].length - trimmedLines[index].length;
          line2++;
        }
        return trimmedLines.join(lineFeed);
        function replacer($0, $1, $2, $3, $4) {
          bullet = $1 + $2 + $3;
          rest = $4;
          if (Number($2) < 10 && bullet.length % 2 === 1) {
            $2 = space + $2;
          }
          max = $1 + repeat(space, $2.length) + $3;
          return max + rest;
        }
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/heading-setext.js
  var require_heading_setext = __commonJS({
    "node_modules/remark-parse/lib/tokenize/heading-setext.js"(exports, module) {
      "use strict";
      module.exports = setextHeading;
      var lineFeed = "\n";
      var tab = "	";
      var space = " ";
      var equalsTo = "=";
      var dash = "-";
      var maxIndent = 3;
      var equalsToDepth = 1;
      var dashDepth = 2;
      function setextHeading(eat, value, silent) {
        var self = this;
        var now = eat.now();
        var length = value.length;
        var index = -1;
        var subvalue = "";
        var content;
        var queue;
        var character;
        var marker;
        var depth;
        while (++index < length) {
          character = value.charAt(index);
          if (character !== space || index >= maxIndent) {
            index--;
            break;
          }
          subvalue += character;
        }
        content = "";
        queue = "";
        while (++index < length) {
          character = value.charAt(index);
          if (character === lineFeed) {
            index--;
            break;
          }
          if (character === space || character === tab) {
            queue += character;
          } else {
            content += queue + character;
            queue = "";
          }
        }
        now.column += subvalue.length;
        now.offset += subvalue.length;
        subvalue += content + queue;
        character = value.charAt(++index);
        marker = value.charAt(++index);
        if (character !== lineFeed || marker !== equalsTo && marker !== dash) {
          return;
        }
        subvalue += character;
        queue = marker;
        depth = marker === equalsTo ? equalsToDepth : dashDepth;
        while (++index < length) {
          character = value.charAt(index);
          if (character !== marker) {
            if (character !== lineFeed) {
              return;
            }
            index--;
            break;
          }
          queue += character;
        }
        if (silent) {
          return true;
        }
        return eat(subvalue + queue)({
          type: "heading",
          depth,
          children: self.tokenizeInline(content, now)
        });
      }
    }
  });

  // node_modules/remark-parse/lib/util/html.js
  var require_html = __commonJS({
    "node_modules/remark-parse/lib/util/html.js"(exports) {
      "use strict";
      var attributeName = "[a-zA-Z_:][a-zA-Z0-9:._-]*";
      var unquoted = "[^\"'=<>`\\u0000-\\u0020]+";
      var singleQuoted = "'[^']*'";
      var doubleQuoted = '"[^"]*"';
      var attributeValue = "(?:" + unquoted + "|" + singleQuoted + "|" + doubleQuoted + ")";
      var attribute = "(?:\\s+" + attributeName + "(?:\\s*=\\s*" + attributeValue + ")?)";
      var openTag = "<[A-Za-z][A-Za-z0-9\\-]*" + attribute + "*\\s*\\/?>";
      var closeTag = "<\\/[A-Za-z][A-Za-z0-9\\-]*\\s*>";
      var comment = "<!---->|<!--(?:-?[^>-])(?:-?[^-])*-->";
      var processing = "<[?].*?[?]>";
      var declaration = "<![A-Za-z]+\\s+[^>]*>";
      var cdata = "<!\\[CDATA\\[[\\s\\S]*?\\]\\]>";
      exports.openCloseTag = new RegExp("^(?:" + openTag + "|" + closeTag + ")");
      exports.tag = new RegExp(
        "^(?:" + openTag + "|" + closeTag + "|" + comment + "|" + processing + "|" + declaration + "|" + cdata + ")"
      );
    }
  });

  // node_modules/remark-parse/lib/tokenize/html-block.js
  var require_html_block = __commonJS({
    "node_modules/remark-parse/lib/tokenize/html-block.js"(exports, module) {
      "use strict";
      var openCloseTag = require_html().openCloseTag;
      module.exports = blockHtml;
      var tab = "	";
      var space = " ";
      var lineFeed = "\n";
      var lessThan = "<";
      var rawOpenExpression = /^<(script|pre|style)(?=(\s|>|$))/i;
      var rawCloseExpression = /<\/(script|pre|style)>/i;
      var commentOpenExpression = /^<!--/;
      var commentCloseExpression = /-->/;
      var instructionOpenExpression = /^<\?/;
      var instructionCloseExpression = /\?>/;
      var directiveOpenExpression = /^<![A-Za-z]/;
      var directiveCloseExpression = />/;
      var cdataOpenExpression = /^<!\[CDATA\[/;
      var cdataCloseExpression = /]]>/;
      var elementCloseExpression = /^$/;
      var otherElementOpenExpression = new RegExp(openCloseTag.source + "\\s*$");
      function blockHtml(eat, value, silent) {
        var self = this;
        var blocks = self.options.blocks.join("|");
        var elementOpenExpression = new RegExp(
          "^</?(" + blocks + ")(?=(\\s|/?>|$))",
          "i"
        );
        var length = value.length;
        var index = 0;
        var next;
        var line2;
        var offset;
        var character;
        var count;
        var sequence;
        var subvalue;
        var sequences = [
          [rawOpenExpression, rawCloseExpression, true],
          [commentOpenExpression, commentCloseExpression, true],
          [instructionOpenExpression, instructionCloseExpression, true],
          [directiveOpenExpression, directiveCloseExpression, true],
          [cdataOpenExpression, cdataCloseExpression, true],
          [elementOpenExpression, elementCloseExpression, true],
          [otherElementOpenExpression, elementCloseExpression, false]
        ];
        while (index < length) {
          character = value.charAt(index);
          if (character !== tab && character !== space) {
            break;
          }
          index++;
        }
        if (value.charAt(index) !== lessThan) {
          return;
        }
        next = value.indexOf(lineFeed, index + 1);
        next = next === -1 ? length : next;
        line2 = value.slice(index, next);
        offset = -1;
        count = sequences.length;
        while (++offset < count) {
          if (sequences[offset][0].test(line2)) {
            sequence = sequences[offset];
            break;
          }
        }
        if (!sequence) {
          return;
        }
        if (silent) {
          return sequence[2];
        }
        index = next;
        if (!sequence[1].test(line2)) {
          while (index < length) {
            next = value.indexOf(lineFeed, index + 1);
            next = next === -1 ? length : next;
            line2 = value.slice(index + 1, next);
            if (sequence[1].test(line2)) {
              if (line2) {
                index = next;
              }
              break;
            }
            index = next;
          }
        }
        subvalue = value.slice(0, index);
        return eat(subvalue)({ type: "html", value: subvalue });
      }
    }
  });

  // node_modules/is-whitespace-character/index.js
  var require_is_whitespace_character = __commonJS({
    "node_modules/is-whitespace-character/index.js"(exports, module) {
      "use strict";
      module.exports = whitespace;
      var fromCode = String.fromCharCode;
      var re = /\s/;
      function whitespace(character) {
        return re.test(
          typeof character === "number" ? fromCode(character) : character.charAt(0)
        );
      }
    }
  });

  // node_modules/remark-parse/lib/util/normalize.js
  var require_normalize = __commonJS({
    "node_modules/remark-parse/lib/util/normalize.js"(exports, module) {
      "use strict";
      var collapseWhiteSpace3 = require_collapse_white_space();
      module.exports = normalize;
      function normalize(value) {
        return collapseWhiteSpace3(value).toLowerCase();
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/definition.js
  var require_definition = __commonJS({
    "node_modules/remark-parse/lib/tokenize/definition.js"(exports, module) {
      "use strict";
      var whitespace = require_is_whitespace_character();
      var normalize = require_normalize();
      module.exports = definition;
      var quotationMark = '"';
      var apostrophe = "'";
      var backslash = "\\";
      var lineFeed = "\n";
      var tab = "	";
      var space = " ";
      var leftSquareBracket = "[";
      var rightSquareBracket = "]";
      var leftParenthesis = "(";
      var rightParenthesis = ")";
      var colon = ":";
      var lessThan = "<";
      var greaterThan = ">";
      function definition(eat, value, silent) {
        var self = this;
        var commonmark = self.options.commonmark;
        var index = 0;
        var length = value.length;
        var subvalue = "";
        var beforeURL;
        var beforeTitle;
        var queue;
        var character;
        var test;
        var identifier;
        var url;
        var title;
        while (index < length) {
          character = value.charAt(index);
          if (character !== space && character !== tab) {
            break;
          }
          subvalue += character;
          index++;
        }
        character = value.charAt(index);
        if (character !== leftSquareBracket) {
          return;
        }
        index++;
        subvalue += character;
        queue = "";
        while (index < length) {
          character = value.charAt(index);
          if (character === rightSquareBracket) {
            break;
          } else if (character === backslash) {
            queue += character;
            index++;
            character = value.charAt(index);
          }
          queue += character;
          index++;
        }
        if (!queue || value.charAt(index) !== rightSquareBracket || value.charAt(index + 1) !== colon) {
          return;
        }
        identifier = queue;
        subvalue += queue + rightSquareBracket + colon;
        index = subvalue.length;
        queue = "";
        while (index < length) {
          character = value.charAt(index);
          if (character !== tab && character !== space && character !== lineFeed) {
            break;
          }
          subvalue += character;
          index++;
        }
        character = value.charAt(index);
        queue = "";
        beforeURL = subvalue;
        if (character === lessThan) {
          index++;
          while (index < length) {
            character = value.charAt(index);
            if (!isEnclosedURLCharacter(character)) {
              break;
            }
            queue += character;
            index++;
          }
          character = value.charAt(index);
          if (character === isEnclosedURLCharacter.delimiter) {
            subvalue += lessThan + queue + character;
            index++;
          } else {
            if (commonmark) {
              return;
            }
            index -= queue.length + 1;
            queue = "";
          }
        }
        if (!queue) {
          while (index < length) {
            character = value.charAt(index);
            if (!isUnclosedURLCharacter(character)) {
              break;
            }
            queue += character;
            index++;
          }
          subvalue += queue;
        }
        if (!queue) {
          return;
        }
        url = queue;
        queue = "";
        while (index < length) {
          character = value.charAt(index);
          if (character !== tab && character !== space && character !== lineFeed) {
            break;
          }
          queue += character;
          index++;
        }
        character = value.charAt(index);
        test = null;
        if (character === quotationMark) {
          test = quotationMark;
        } else if (character === apostrophe) {
          test = apostrophe;
        } else if (character === leftParenthesis) {
          test = rightParenthesis;
        }
        if (!test) {
          queue = "";
          index = subvalue.length;
        } else if (queue) {
          subvalue += queue + character;
          index = subvalue.length;
          queue = "";
          while (index < length) {
            character = value.charAt(index);
            if (character === test) {
              break;
            }
            if (character === lineFeed) {
              index++;
              character = value.charAt(index);
              if (character === lineFeed || character === test) {
                return;
              }
              queue += lineFeed;
            }
            queue += character;
            index++;
          }
          character = value.charAt(index);
          if (character !== test) {
            return;
          }
          beforeTitle = subvalue;
          subvalue += queue + character;
          index++;
          title = queue;
          queue = "";
        } else {
          return;
        }
        while (index < length) {
          character = value.charAt(index);
          if (character !== tab && character !== space) {
            break;
          }
          subvalue += character;
          index++;
        }
        character = value.charAt(index);
        if (!character || character === lineFeed) {
          if (silent) {
            return true;
          }
          beforeURL = eat(beforeURL).test().end;
          url = self.decode.raw(self.unescape(url), beforeURL, { nonTerminated: false });
          if (title) {
            beforeTitle = eat(beforeTitle).test().end;
            title = self.decode.raw(self.unescape(title), beforeTitle);
          }
          return eat(subvalue)({
            type: "definition",
            identifier: normalize(identifier),
            label: identifier,
            title: title || null,
            url
          });
        }
      }
      function isEnclosedURLCharacter(character) {
        return character !== greaterThan && character !== leftSquareBracket && character !== rightSquareBracket;
      }
      isEnclosedURLCharacter.delimiter = greaterThan;
      function isUnclosedURLCharacter(character) {
        return character !== leftSquareBracket && character !== rightSquareBracket && !whitespace(character);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/table.js
  var require_table = __commonJS({
    "node_modules/remark-parse/lib/tokenize/table.js"(exports, module) {
      "use strict";
      var whitespace = require_is_whitespace_character();
      module.exports = table;
      var tab = "	";
      var lineFeed = "\n";
      var space = " ";
      var dash = "-";
      var colon = ":";
      var backslash = "\\";
      var verticalBar = "|";
      var minColumns = 1;
      var minRows = 2;
      var left = "left";
      var center = "center";
      var right = "right";
      function table(eat, value, silent) {
        var self = this;
        var index;
        var alignments;
        var alignment;
        var subvalue;
        var row;
        var length;
        var lines;
        var queue;
        var character;
        var hasDash;
        var align2;
        var cell;
        var preamble;
        var now;
        var position;
        var lineCount;
        var line2;
        var rows;
        var table2;
        var lineIndex;
        var pipeIndex;
        var first;
        if (!self.options.gfm) {
          return;
        }
        index = 0;
        lineCount = 0;
        length = value.length + 1;
        lines = [];
        while (index < length) {
          lineIndex = value.indexOf(lineFeed, index);
          pipeIndex = value.indexOf(verticalBar, index + 1);
          if (lineIndex === -1) {
            lineIndex = value.length;
          }
          if (pipeIndex === -1 || pipeIndex > lineIndex) {
            if (lineCount < minRows) {
              return;
            }
            break;
          }
          lines.push(value.slice(index, lineIndex));
          lineCount++;
          index = lineIndex + 1;
        }
        subvalue = lines.join(lineFeed);
        alignments = lines.splice(1, 1)[0] || [];
        index = 0;
        length = alignments.length;
        lineCount--;
        alignment = false;
        align2 = [];
        while (index < length) {
          character = alignments.charAt(index);
          if (character === verticalBar) {
            hasDash = null;
            if (alignment === false) {
              if (first === false) {
                return;
              }
            } else {
              align2.push(alignment);
              alignment = false;
            }
            first = false;
          } else if (character === dash) {
            hasDash = true;
            alignment = alignment || null;
          } else if (character === colon) {
            if (alignment === left) {
              alignment = center;
            } else if (hasDash && alignment === null) {
              alignment = right;
            } else {
              alignment = left;
            }
          } else if (!whitespace(character)) {
            return;
          }
          index++;
        }
        if (alignment !== false) {
          align2.push(alignment);
        }
        if (align2.length < minColumns) {
          return;
        }
        if (silent) {
          return true;
        }
        position = -1;
        rows = [];
        table2 = eat(subvalue).reset({ type: "table", align: align2, children: rows });
        while (++position < lineCount) {
          line2 = lines[position];
          row = { type: "tableRow", children: [] };
          if (position) {
            eat(lineFeed);
          }
          eat(line2).reset(row, table2);
          length = line2.length + 1;
          index = 0;
          queue = "";
          cell = "";
          preamble = true;
          while (index < length) {
            character = line2.charAt(index);
            if (character === tab || character === space) {
              if (cell) {
                queue += character;
              } else {
                eat(character);
              }
              index++;
              continue;
            }
            if (character === "" || character === verticalBar) {
              if (preamble) {
                eat(character);
              } else {
                if ((cell || character) && !preamble) {
                  subvalue = cell;
                  if (queue.length > 1) {
                    if (character) {
                      subvalue += queue.slice(0, -1);
                      queue = queue.charAt(queue.length - 1);
                    } else {
                      subvalue += queue;
                      queue = "";
                    }
                  }
                  now = eat.now();
                  eat(subvalue)(
                    { type: "tableCell", children: self.tokenizeInline(cell, now) },
                    row
                  );
                }
                eat(queue + character);
                queue = "";
                cell = "";
              }
            } else {
              if (queue) {
                cell += queue;
                queue = "";
              }
              cell += character;
              if (character === backslash && index !== length - 2) {
                cell += line2.charAt(index + 1);
                index++;
              }
            }
            preamble = false;
            index++;
          }
          if (!position) {
            eat(lineFeed + alignments);
          }
        }
        return table2;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/paragraph.js
  var require_paragraph = __commonJS({
    "node_modules/remark-parse/lib/tokenize/paragraph.js"(exports, module) {
      "use strict";
      var trim2 = require_trim();
      var trimTrailingLines = require_trim_trailing_lines();
      var interrupt = require_interrupt();
      module.exports = paragraph;
      var tab = "	";
      var lineFeed = "\n";
      var space = " ";
      var tabSize = 4;
      function paragraph(eat, value, silent) {
        var self = this;
        var settings = self.options;
        var commonmark = settings.commonmark;
        var tokenizers = self.blockTokenizers;
        var interruptors = self.interruptParagraph;
        var index = value.indexOf(lineFeed);
        var length = value.length;
        var position;
        var subvalue;
        var character;
        var size;
        var now;
        while (index < length) {
          if (index === -1) {
            index = length;
            break;
          }
          if (value.charAt(index + 1) === lineFeed) {
            break;
          }
          if (commonmark) {
            size = 0;
            position = index + 1;
            while (position < length) {
              character = value.charAt(position);
              if (character === tab) {
                size = tabSize;
                break;
              } else if (character === space) {
                size++;
              } else {
                break;
              }
              position++;
            }
            if (size >= tabSize && character !== lineFeed) {
              index = value.indexOf(lineFeed, index + 1);
              continue;
            }
          }
          subvalue = value.slice(index + 1);
          if (interrupt(interruptors, tokenizers, self, [eat, subvalue, true])) {
            break;
          }
          position = index;
          index = value.indexOf(lineFeed, index + 1);
          if (index !== -1 && trim2(value.slice(position, index)) === "") {
            index = position;
            break;
          }
        }
        subvalue = value.slice(0, index);
        if (silent) {
          return true;
        }
        now = eat.now();
        subvalue = trimTrailingLines(subvalue);
        return eat(subvalue)({
          type: "paragraph",
          children: self.tokenizeInline(subvalue, now)
        });
      }
    }
  });

  // node_modules/remark-parse/lib/locate/escape.js
  var require_escape = __commonJS({
    "node_modules/remark-parse/lib/locate/escape.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        return value.indexOf("\\", fromIndex);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/escape.js
  var require_escape2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/escape.js"(exports, module) {
      "use strict";
      var locate = require_escape();
      module.exports = escape;
      escape.locator = locate;
      var lineFeed = "\n";
      var backslash = "\\";
      function escape(eat, value, silent) {
        var self = this;
        var character;
        var node;
        if (value.charAt(0) === backslash) {
          character = value.charAt(1);
          if (self.escape.indexOf(character) !== -1) {
            if (silent) {
              return true;
            }
            if (character === lineFeed) {
              node = { type: "break" };
            } else {
              node = { type: "text", value: character };
            }
            return eat(backslash + character)(node);
          }
        }
      }
    }
  });

  // node_modules/remark-parse/lib/locate/tag.js
  var require_tag = __commonJS({
    "node_modules/remark-parse/lib/locate/tag.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        return value.indexOf("<", fromIndex);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/auto-link.js
  var require_auto_link = __commonJS({
    "node_modules/remark-parse/lib/tokenize/auto-link.js"(exports, module) {
      "use strict";
      var whitespace = require_is_whitespace_character();
      var decode = require_parse_entities();
      var locate = require_tag();
      module.exports = autoLink;
      autoLink.locator = locate;
      autoLink.notInLink = true;
      var lessThan = "<";
      var greaterThan = ">";
      var atSign = "@";
      var slash = "/";
      var mailto = "mailto:";
      var mailtoLength = mailto.length;
      function autoLink(eat, value, silent) {
        var self = this;
        var subvalue = "";
        var length = value.length;
        var index = 0;
        var queue = "";
        var hasAtCharacter = false;
        var link = "";
        var character;
        var now;
        var content;
        var tokenizers;
        var exit;
        if (value.charAt(0) !== lessThan) {
          return;
        }
        index++;
        subvalue = lessThan;
        while (index < length) {
          character = value.charAt(index);
          if (whitespace(character) || character === greaterThan || character === atSign || character === ":" && value.charAt(index + 1) === slash) {
            break;
          }
          queue += character;
          index++;
        }
        if (!queue) {
          return;
        }
        link += queue;
        queue = "";
        character = value.charAt(index);
        link += character;
        index++;
        if (character === atSign) {
          hasAtCharacter = true;
        } else {
          if (character !== ":" || value.charAt(index + 1) !== slash) {
            return;
          }
          link += slash;
          index++;
        }
        while (index < length) {
          character = value.charAt(index);
          if (whitespace(character) || character === greaterThan) {
            break;
          }
          queue += character;
          index++;
        }
        character = value.charAt(index);
        if (!queue || character !== greaterThan) {
          return;
        }
        if (silent) {
          return true;
        }
        link += queue;
        content = link;
        subvalue += link + character;
        now = eat.now();
        now.column++;
        now.offset++;
        if (hasAtCharacter) {
          if (link.slice(0, mailtoLength).toLowerCase() === mailto) {
            content = content.slice(mailtoLength);
            now.column += mailtoLength;
            now.offset += mailtoLength;
          } else {
            link = mailto + link;
          }
        }
        tokenizers = self.inlineTokenizers;
        self.inlineTokenizers = { text: tokenizers.text };
        exit = self.enterLink();
        content = self.tokenizeInline(content, now);
        self.inlineTokenizers = tokenizers;
        exit();
        return eat(subvalue)({
          type: "link",
          title: null,
          url: decode(link, { nonTerminated: false }),
          children: content
        });
      }
    }
  });

  // node_modules/ccount/index.js
  var require_ccount = __commonJS({
    "node_modules/ccount/index.js"(exports, module) {
      "use strict";
      module.exports = ccount;
      function ccount(source, character) {
        var value = String(source);
        var count = 0;
        var index;
        if (typeof character !== "string") {
          throw new Error("Expected character");
        }
        index = value.indexOf(character);
        while (index !== -1) {
          count++;
          index = value.indexOf(character, index + character.length);
        }
        return count;
      }
    }
  });

  // node_modules/remark-parse/lib/locate/url.js
  var require_url = __commonJS({
    "node_modules/remark-parse/lib/locate/url.js"(exports, module) {
      "use strict";
      module.exports = locate;
      var values = ["www.", "http://", "https://"];
      function locate(value, fromIndex) {
        var min = -1;
        var index;
        var length;
        var position;
        if (!this.options.gfm) {
          return min;
        }
        length = values.length;
        index = -1;
        while (++index < length) {
          position = value.indexOf(values[index], fromIndex);
          if (position !== -1 && (min === -1 || position < min)) {
            min = position;
          }
        }
        return min;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/url.js
  var require_url2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/url.js"(exports, module) {
      "use strict";
      var ccount = require_ccount();
      var decode = require_parse_entities();
      var decimal = require_is_decimal();
      var alphabetical = require_is_alphabetical();
      var whitespace = require_is_whitespace_character();
      var locate = require_url();
      module.exports = url;
      url.locator = locate;
      url.notInLink = true;
      var exclamationMark = 33;
      var ampersand = 38;
      var rightParenthesis = 41;
      var asterisk = 42;
      var comma = 44;
      var dash = 45;
      var dot = 46;
      var colon = 58;
      var semicolon = 59;
      var questionMark = 63;
      var lessThan = 60;
      var underscore = 95;
      var tilde = 126;
      var leftParenthesisCharacter = "(";
      var rightParenthesisCharacter = ")";
      function url(eat, value, silent) {
        var self = this;
        var gfm = self.options.gfm;
        var tokenizers = self.inlineTokenizers;
        var length = value.length;
        var previousDot = -1;
        var protocolless = false;
        var dots;
        var lastTwoPartsStart;
        var start;
        var index;
        var pathStart;
        var path;
        var code;
        var end;
        var leftCount;
        var rightCount;
        var content;
        var children;
        var url2;
        var exit;
        if (!gfm) {
          return;
        }
        if (value.slice(0, 4) === "www.") {
          protocolless = true;
          index = 4;
        } else if (value.slice(0, 7).toLowerCase() === "http://") {
          index = 7;
        } else if (value.slice(0, 8).toLowerCase() === "https://") {
          index = 8;
        } else {
          return;
        }
        previousDot = index - 1;
        start = index;
        dots = [];
        while (index < length) {
          code = value.charCodeAt(index);
          if (code === dot) {
            if (previousDot === index - 1) {
              break;
            }
            dots.push(index);
            previousDot = index;
            index++;
            continue;
          }
          if (decimal(code) || alphabetical(code) || code === dash || code === underscore) {
            index++;
            continue;
          }
          break;
        }
        if (code === dot) {
          dots.pop();
          index--;
        }
        if (dots[0] === void 0) {
          return;
        }
        lastTwoPartsStart = dots.length < 2 ? start : dots[dots.length - 2] + 1;
        if (value.slice(lastTwoPartsStart, index).indexOf("_") !== -1) {
          return;
        }
        if (silent) {
          return true;
        }
        end = index;
        pathStart = index;
        while (index < length) {
          code = value.charCodeAt(index);
          if (whitespace(code) || code === lessThan) {
            break;
          }
          index++;
          if (code === exclamationMark || code === asterisk || code === comma || code === dot || code === colon || code === questionMark || code === underscore || code === tilde) {
          } else {
            end = index;
          }
        }
        index = end;
        if (value.charCodeAt(index - 1) === rightParenthesis) {
          path = value.slice(pathStart, index);
          leftCount = ccount(path, leftParenthesisCharacter);
          rightCount = ccount(path, rightParenthesisCharacter);
          while (rightCount > leftCount) {
            index = pathStart + path.lastIndexOf(rightParenthesisCharacter);
            path = value.slice(pathStart, index);
            rightCount--;
          }
        }
        if (value.charCodeAt(index - 1) === semicolon) {
          index--;
          if (alphabetical(value.charCodeAt(index - 1))) {
            end = index - 2;
            while (alphabetical(value.charCodeAt(end))) {
              end--;
            }
            if (value.charCodeAt(end) === ampersand) {
              index = end;
            }
          }
        }
        content = value.slice(0, index);
        url2 = decode(content, { nonTerminated: false });
        if (protocolless) {
          url2 = "http://" + url2;
        }
        exit = self.enterLink();
        self.inlineTokenizers = { text: tokenizers.text };
        children = self.tokenizeInline(content, eat.now());
        self.inlineTokenizers = tokenizers;
        exit();
        return eat(content)({ type: "link", title: null, url: url2, children });
      }
    }
  });

  // node_modules/remark-parse/lib/locate/email.js
  var require_email = __commonJS({
    "node_modules/remark-parse/lib/locate/email.js"(exports, module) {
      "use strict";
      var decimal = require_is_decimal();
      var alphabetical = require_is_alphabetical();
      var plusSign = 43;
      var dash = 45;
      var dot = 46;
      var underscore = 95;
      module.exports = locate;
      function locate(value, fromIndex) {
        var self = this;
        var at2;
        var position;
        if (!this.options.gfm) {
          return -1;
        }
        at2 = value.indexOf("@", fromIndex);
        if (at2 === -1) {
          return -1;
        }
        position = at2;
        if (position === fromIndex || !isGfmAtext(value.charCodeAt(position - 1))) {
          return locate.call(self, value, at2 + 1);
        }
        while (position > fromIndex && isGfmAtext(value.charCodeAt(position - 1))) {
          position--;
        }
        return position;
      }
      function isGfmAtext(code) {
        return decimal(code) || alphabetical(code) || code === plusSign || code === dash || code === dot || code === underscore;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/email.js
  var require_email2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/email.js"(exports, module) {
      "use strict";
      var decode = require_parse_entities();
      var decimal = require_is_decimal();
      var alphabetical = require_is_alphabetical();
      var locate = require_email();
      module.exports = email;
      email.locator = locate;
      email.notInLink = true;
      var plusSign = 43;
      var dash = 45;
      var dot = 46;
      var atSign = 64;
      var underscore = 95;
      function email(eat, value, silent) {
        var self = this;
        var gfm = self.options.gfm;
        var tokenizers = self.inlineTokenizers;
        var index = 0;
        var length = value.length;
        var firstDot = -1;
        var code;
        var content;
        var children;
        var exit;
        if (!gfm) {
          return;
        }
        code = value.charCodeAt(index);
        while (decimal(code) || alphabetical(code) || code === plusSign || code === dash || code === dot || code === underscore) {
          code = value.charCodeAt(++index);
        }
        if (index === 0) {
          return;
        }
        if (code !== atSign) {
          return;
        }
        index++;
        while (index < length) {
          code = value.charCodeAt(index);
          if (decimal(code) || alphabetical(code) || code === dash || code === dot || code === underscore) {
            index++;
            if (firstDot === -1 && code === dot) {
              firstDot = index;
            }
            continue;
          }
          break;
        }
        if (firstDot === -1 || firstDot === index || code === dash || code === underscore) {
          return;
        }
        if (code === dot) {
          index--;
        }
        content = value.slice(0, index);
        if (silent) {
          return true;
        }
        exit = self.enterLink();
        self.inlineTokenizers = { text: tokenizers.text };
        children = self.tokenizeInline(content, eat.now());
        self.inlineTokenizers = tokenizers;
        exit();
        return eat(content)({
          type: "link",
          title: null,
          url: "mailto:" + decode(content, { nonTerminated: false }),
          children
        });
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/html-inline.js
  var require_html_inline = __commonJS({
    "node_modules/remark-parse/lib/tokenize/html-inline.js"(exports, module) {
      "use strict";
      var alphabetical = require_is_alphabetical();
      var locate = require_tag();
      var tag = require_html().tag;
      module.exports = inlineHTML;
      inlineHTML.locator = locate;
      var lessThan = "<";
      var questionMark = "?";
      var exclamationMark = "!";
      var slash = "/";
      var htmlLinkOpenExpression = /^<a /i;
      var htmlLinkCloseExpression = /^<\/a>/i;
      function inlineHTML(eat, value, silent) {
        var self = this;
        var length = value.length;
        var character;
        var subvalue;
        if (value.charAt(0) !== lessThan || length < 3) {
          return;
        }
        character = value.charAt(1);
        if (!alphabetical(character) && character !== questionMark && character !== exclamationMark && character !== slash) {
          return;
        }
        subvalue = value.match(tag);
        if (!subvalue) {
          return;
        }
        if (silent) {
          return true;
        }
        subvalue = subvalue[0];
        if (!self.inLink && htmlLinkOpenExpression.test(subvalue)) {
          self.inLink = true;
        } else if (self.inLink && htmlLinkCloseExpression.test(subvalue)) {
          self.inLink = false;
        }
        return eat(subvalue)({ type: "html", value: subvalue });
      }
    }
  });

  // node_modules/remark-parse/lib/locate/link.js
  var require_link = __commonJS({
    "node_modules/remark-parse/lib/locate/link.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        var link = value.indexOf("[", fromIndex);
        var image = value.indexOf("![", fromIndex);
        if (image === -1) {
          return link;
        }
        return link < image ? link : image;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/link.js
  var require_link2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/link.js"(exports, module) {
      "use strict";
      var whitespace = require_is_whitespace_character();
      var locate = require_link();
      module.exports = link;
      link.locator = locate;
      var lineFeed = "\n";
      var exclamationMark = "!";
      var quotationMark = '"';
      var apostrophe = "'";
      var leftParenthesis = "(";
      var rightParenthesis = ")";
      var lessThan = "<";
      var greaterThan = ">";
      var leftSquareBracket = "[";
      var backslash = "\\";
      var rightSquareBracket = "]";
      var graveAccent = "`";
      function link(eat, value, silent) {
        var self = this;
        var subvalue = "";
        var index = 0;
        var character = value.charAt(0);
        var pedantic = self.options.pedantic;
        var commonmark = self.options.commonmark;
        var gfm = self.options.gfm;
        var closed;
        var count;
        var opening;
        var beforeURL;
        var beforeTitle;
        var subqueue;
        var hasMarker;
        var isImage;
        var content;
        var marker;
        var length;
        var title;
        var depth;
        var queue;
        var url;
        var now;
        var exit;
        var node;
        if (character === exclamationMark) {
          isImage = true;
          subvalue = character;
          character = value.charAt(++index);
        }
        if (character !== leftSquareBracket) {
          return;
        }
        if (!isImage && self.inLink) {
          return;
        }
        subvalue += character;
        queue = "";
        index++;
        length = value.length;
        now = eat.now();
        depth = 0;
        now.column += index;
        now.offset += index;
        while (index < length) {
          character = value.charAt(index);
          subqueue = character;
          if (character === graveAccent) {
            count = 1;
            while (value.charAt(index + 1) === graveAccent) {
              subqueue += character;
              index++;
              count++;
            }
            if (!opening) {
              opening = count;
            } else if (count >= opening) {
              opening = 0;
            }
          } else if (character === backslash) {
            index++;
            subqueue += value.charAt(index);
          } else if ((!opening || gfm) && character === leftSquareBracket) {
            depth++;
          } else if ((!opening || gfm) && character === rightSquareBracket) {
            if (depth) {
              depth--;
            } else {
              if (value.charAt(index + 1) !== leftParenthesis) {
                return;
              }
              subqueue += leftParenthesis;
              closed = true;
              index++;
              break;
            }
          }
          queue += subqueue;
          subqueue = "";
          index++;
        }
        if (!closed) {
          return;
        }
        content = queue;
        subvalue += queue + subqueue;
        index++;
        while (index < length) {
          character = value.charAt(index);
          if (!whitespace(character)) {
            break;
          }
          subvalue += character;
          index++;
        }
        character = value.charAt(index);
        queue = "";
        beforeURL = subvalue;
        if (character === lessThan) {
          index++;
          beforeURL += lessThan;
          while (index < length) {
            character = value.charAt(index);
            if (character === greaterThan) {
              break;
            }
            if (commonmark && character === lineFeed) {
              return;
            }
            queue += character;
            index++;
          }
          if (value.charAt(index) !== greaterThan) {
            return;
          }
          subvalue += lessThan + queue + greaterThan;
          url = queue;
          index++;
        } else {
          character = null;
          subqueue = "";
          while (index < length) {
            character = value.charAt(index);
            if (subqueue && (character === quotationMark || character === apostrophe || commonmark && character === leftParenthesis)) {
              break;
            }
            if (whitespace(character)) {
              if (!pedantic) {
                break;
              }
              subqueue += character;
            } else {
              if (character === leftParenthesis) {
                depth++;
              } else if (character === rightParenthesis) {
                if (depth === 0) {
                  break;
                }
                depth--;
              }
              queue += subqueue;
              subqueue = "";
              if (character === backslash) {
                queue += backslash;
                character = value.charAt(++index);
              }
              queue += character;
            }
            index++;
          }
          subvalue += queue;
          url = queue;
          index = subvalue.length;
        }
        queue = "";
        while (index < length) {
          character = value.charAt(index);
          if (!whitespace(character)) {
            break;
          }
          queue += character;
          index++;
        }
        character = value.charAt(index);
        subvalue += queue;
        if (queue && (character === quotationMark || character === apostrophe || commonmark && character === leftParenthesis)) {
          index++;
          subvalue += character;
          queue = "";
          marker = character === leftParenthesis ? rightParenthesis : character;
          beforeTitle = subvalue;
          if (commonmark) {
            while (index < length) {
              character = value.charAt(index);
              if (character === marker) {
                break;
              }
              if (character === backslash) {
                queue += backslash;
                character = value.charAt(++index);
              }
              index++;
              queue += character;
            }
            character = value.charAt(index);
            if (character !== marker) {
              return;
            }
            title = queue;
            subvalue += queue + character;
            index++;
            while (index < length) {
              character = value.charAt(index);
              if (!whitespace(character)) {
                break;
              }
              subvalue += character;
              index++;
            }
          } else {
            subqueue = "";
            while (index < length) {
              character = value.charAt(index);
              if (character === marker) {
                if (hasMarker) {
                  queue += marker + subqueue;
                  subqueue = "";
                }
                hasMarker = true;
              } else if (!hasMarker) {
                queue += character;
              } else if (character === rightParenthesis) {
                subvalue += queue + marker + subqueue;
                title = queue;
                break;
              } else if (whitespace(character)) {
                subqueue += character;
              } else {
                queue += marker + subqueue + character;
                subqueue = "";
                hasMarker = false;
              }
              index++;
            }
          }
        }
        if (value.charAt(index) !== rightParenthesis) {
          return;
        }
        if (silent) {
          return true;
        }
        subvalue += rightParenthesis;
        url = self.decode.raw(self.unescape(url), eat(beforeURL).test().end, {
          nonTerminated: false
        });
        if (title) {
          beforeTitle = eat(beforeTitle).test().end;
          title = self.decode.raw(self.unescape(title), beforeTitle);
        }
        node = {
          type: isImage ? "image" : "link",
          title: title || null,
          url
        };
        if (isImage) {
          node.alt = self.decode.raw(self.unescape(content), now) || null;
        } else {
          exit = self.enterLink();
          node.children = self.tokenizeInline(content, now);
          exit();
        }
        return eat(subvalue)(node);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/reference.js
  var require_reference = __commonJS({
    "node_modules/remark-parse/lib/tokenize/reference.js"(exports, module) {
      "use strict";
      var whitespace = require_is_whitespace_character();
      var locate = require_link();
      var normalize = require_normalize();
      module.exports = reference;
      reference.locator = locate;
      var link = "link";
      var image = "image";
      var shortcut = "shortcut";
      var collapsed = "collapsed";
      var full = "full";
      var exclamationMark = "!";
      var leftSquareBracket = "[";
      var backslash = "\\";
      var rightSquareBracket = "]";
      function reference(eat, value, silent) {
        var self = this;
        var commonmark = self.options.commonmark;
        var character = value.charAt(0);
        var index = 0;
        var length = value.length;
        var subvalue = "";
        var intro = "";
        var type = link;
        var referenceType = shortcut;
        var content;
        var identifier;
        var now;
        var node;
        var exit;
        var queue;
        var bracketed;
        var depth;
        if (character === exclamationMark) {
          type = image;
          intro = character;
          character = value.charAt(++index);
        }
        if (character !== leftSquareBracket) {
          return;
        }
        index++;
        intro += character;
        queue = "";
        depth = 0;
        while (index < length) {
          character = value.charAt(index);
          if (character === leftSquareBracket) {
            bracketed = true;
            depth++;
          } else if (character === rightSquareBracket) {
            if (!depth) {
              break;
            }
            depth--;
          }
          if (character === backslash) {
            queue += backslash;
            character = value.charAt(++index);
          }
          queue += character;
          index++;
        }
        subvalue = queue;
        content = queue;
        character = value.charAt(index);
        if (character !== rightSquareBracket) {
          return;
        }
        index++;
        subvalue += character;
        queue = "";
        if (!commonmark) {
          while (index < length) {
            character = value.charAt(index);
            if (!whitespace(character)) {
              break;
            }
            queue += character;
            index++;
          }
        }
        character = value.charAt(index);
        if (character === leftSquareBracket) {
          identifier = "";
          queue += character;
          index++;
          while (index < length) {
            character = value.charAt(index);
            if (character === leftSquareBracket || character === rightSquareBracket) {
              break;
            }
            if (character === backslash) {
              identifier += backslash;
              character = value.charAt(++index);
            }
            identifier += character;
            index++;
          }
          character = value.charAt(index);
          if (character === rightSquareBracket) {
            referenceType = identifier ? full : collapsed;
            queue += identifier + character;
            index++;
          } else {
            identifier = "";
          }
          subvalue += queue;
          queue = "";
        } else {
          if (!content) {
            return;
          }
          identifier = content;
        }
        if (referenceType !== full && bracketed) {
          return;
        }
        subvalue = intro + subvalue;
        if (type === link && self.inLink) {
          return null;
        }
        if (silent) {
          return true;
        }
        now = eat.now();
        now.column += intro.length;
        now.offset += intro.length;
        identifier = referenceType === full ? identifier : content;
        node = {
          type: type + "Reference",
          identifier: normalize(identifier),
          label: identifier,
          referenceType
        };
        if (type === link) {
          exit = self.enterLink();
          node.children = self.tokenizeInline(content, now);
          exit();
        } else {
          node.alt = self.decode.raw(self.unescape(content), now) || null;
        }
        return eat(subvalue)(node);
      }
    }
  });

  // node_modules/remark-parse/lib/locate/strong.js
  var require_strong = __commonJS({
    "node_modules/remark-parse/lib/locate/strong.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        var asterisk = value.indexOf("**", fromIndex);
        var underscore = value.indexOf("__", fromIndex);
        if (underscore === -1) {
          return asterisk;
        }
        if (asterisk === -1) {
          return underscore;
        }
        return underscore < asterisk ? underscore : asterisk;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/strong.js
  var require_strong2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/strong.js"(exports, module) {
      "use strict";
      var trim2 = require_trim();
      var whitespace = require_is_whitespace_character();
      var locate = require_strong();
      module.exports = strong;
      strong.locator = locate;
      var backslash = "\\";
      var asterisk = "*";
      var underscore = "_";
      function strong(eat, value, silent) {
        var self = this;
        var index = 0;
        var character = value.charAt(index);
        var now;
        var pedantic;
        var marker;
        var queue;
        var subvalue;
        var length;
        var previous;
        if (character !== asterisk && character !== underscore || value.charAt(++index) !== character) {
          return;
        }
        pedantic = self.options.pedantic;
        marker = character;
        subvalue = marker + marker;
        length = value.length;
        index++;
        queue = "";
        character = "";
        if (pedantic && whitespace(value.charAt(index))) {
          return;
        }
        while (index < length) {
          previous = character;
          character = value.charAt(index);
          if (character === marker && value.charAt(index + 1) === marker && (!pedantic || !whitespace(previous))) {
            character = value.charAt(index + 2);
            if (character !== marker) {
              if (!trim2(queue)) {
                return;
              }
              if (silent) {
                return true;
              }
              now = eat.now();
              now.column += 2;
              now.offset += 2;
              return eat(subvalue + queue + subvalue)({
                type: "strong",
                children: self.tokenizeInline(queue, now)
              });
            }
          }
          if (!pedantic && character === backslash) {
            queue += character;
            character = value.charAt(++index);
          }
          queue += character;
          index++;
        }
      }
    }
  });

  // node_modules/is-word-character/index.js
  var require_is_word_character = __commonJS({
    "node_modules/is-word-character/index.js"(exports, module) {
      "use strict";
      module.exports = wordCharacter;
      var fromCode = String.fromCharCode;
      var re = /\w/;
      function wordCharacter(character) {
        return re.test(
          typeof character === "number" ? fromCode(character) : character.charAt(0)
        );
      }
    }
  });

  // node_modules/remark-parse/lib/locate/emphasis.js
  var require_emphasis = __commonJS({
    "node_modules/remark-parse/lib/locate/emphasis.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        var asterisk = value.indexOf("*", fromIndex);
        var underscore = value.indexOf("_", fromIndex);
        if (underscore === -1) {
          return asterisk;
        }
        if (asterisk === -1) {
          return underscore;
        }
        return underscore < asterisk ? underscore : asterisk;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/emphasis.js
  var require_emphasis2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/emphasis.js"(exports, module) {
      "use strict";
      var trim2 = require_trim();
      var word = require_is_word_character();
      var whitespace = require_is_whitespace_character();
      var locate = require_emphasis();
      module.exports = emphasis;
      emphasis.locator = locate;
      var asterisk = "*";
      var underscore = "_";
      var backslash = "\\";
      function emphasis(eat, value, silent) {
        var self = this;
        var index = 0;
        var character = value.charAt(index);
        var now;
        var pedantic;
        var marker;
        var queue;
        var subvalue;
        var length;
        var previous;
        if (character !== asterisk && character !== underscore) {
          return;
        }
        pedantic = self.options.pedantic;
        subvalue = character;
        marker = character;
        length = value.length;
        index++;
        queue = "";
        character = "";
        if (pedantic && whitespace(value.charAt(index))) {
          return;
        }
        while (index < length) {
          previous = character;
          character = value.charAt(index);
          if (character === marker && (!pedantic || !whitespace(previous))) {
            character = value.charAt(++index);
            if (character !== marker) {
              if (!trim2(queue) || previous === marker) {
                return;
              }
              if (!pedantic && marker === underscore && word(character)) {
                queue += marker;
                continue;
              }
              if (silent) {
                return true;
              }
              now = eat.now();
              now.column++;
              now.offset++;
              return eat(subvalue + queue + marker)({
                type: "emphasis",
                children: self.tokenizeInline(queue, now)
              });
            }
            queue += marker;
          }
          if (!pedantic && character === backslash) {
            queue += character;
            character = value.charAt(++index);
          }
          queue += character;
          index++;
        }
      }
    }
  });

  // node_modules/remark-parse/lib/locate/delete.js
  var require_delete = __commonJS({
    "node_modules/remark-parse/lib/locate/delete.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        return value.indexOf("~~", fromIndex);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/delete.js
  var require_delete2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/delete.js"(exports, module) {
      "use strict";
      var whitespace = require_is_whitespace_character();
      var locate = require_delete();
      module.exports = strikethrough;
      strikethrough.locator = locate;
      var tilde = "~";
      var fence = "~~";
      function strikethrough(eat, value, silent) {
        var self = this;
        var character = "";
        var previous = "";
        var preceding = "";
        var subvalue = "";
        var index;
        var length;
        var now;
        if (!self.options.gfm || value.charAt(0) !== tilde || value.charAt(1) !== tilde || whitespace(value.charAt(2))) {
          return;
        }
        index = 1;
        length = value.length;
        now = eat.now();
        now.column += 2;
        now.offset += 2;
        while (++index < length) {
          character = value.charAt(index);
          if (character === tilde && previous === tilde && (!preceding || !whitespace(preceding))) {
            if (silent) {
              return true;
            }
            return eat(fence + subvalue + fence)({
              type: "delete",
              children: self.tokenizeInline(subvalue, now)
            });
          }
          subvalue += previous;
          preceding = previous;
          previous = character;
        }
      }
    }
  });

  // node_modules/remark-parse/lib/locate/code-inline.js
  var require_code_inline = __commonJS({
    "node_modules/remark-parse/lib/locate/code-inline.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        return value.indexOf("`", fromIndex);
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/code-inline.js
  var require_code_inline2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/code-inline.js"(exports, module) {
      "use strict";
      var locate = require_code_inline();
      module.exports = inlineCode;
      inlineCode.locator = locate;
      var lineFeed = 10;
      var space = 32;
      var graveAccent = 96;
      function inlineCode(eat, value, silent) {
        var length = value.length;
        var index = 0;
        var openingFenceEnd;
        var closingFenceStart;
        var closingFenceEnd;
        var code;
        var next;
        var found;
        while (index < length) {
          if (value.charCodeAt(index) !== graveAccent) {
            break;
          }
          index++;
        }
        if (index === 0 || index === length) {
          return;
        }
        openingFenceEnd = index;
        next = value.charCodeAt(index);
        while (index < length) {
          code = next;
          next = value.charCodeAt(index + 1);
          if (code === graveAccent) {
            if (closingFenceStart === void 0) {
              closingFenceStart = index;
            }
            closingFenceEnd = index + 1;
            if (next !== graveAccent && closingFenceEnd - closingFenceStart === openingFenceEnd) {
              found = true;
              break;
            }
          } else if (closingFenceStart !== void 0) {
            closingFenceStart = void 0;
            closingFenceEnd = void 0;
          }
          index++;
        }
        if (!found) {
          return;
        }
        if (silent) {
          return true;
        }
        index = openingFenceEnd;
        length = closingFenceStart;
        code = value.charCodeAt(index);
        next = value.charCodeAt(length - 1);
        found = false;
        if (length - index > 2 && (code === space || code === lineFeed) && (next === space || next === lineFeed)) {
          index++;
          length--;
          while (index < length) {
            code = value.charCodeAt(index);
            if (code !== space && code !== lineFeed) {
              found = true;
              break;
            }
            index++;
          }
          if (found === true) {
            openingFenceEnd++;
            closingFenceStart--;
          }
        }
        return eat(value.slice(0, closingFenceEnd))({
          type: "inlineCode",
          value: value.slice(openingFenceEnd, closingFenceStart)
        });
      }
    }
  });

  // node_modules/remark-parse/lib/locate/break.js
  var require_break = __commonJS({
    "node_modules/remark-parse/lib/locate/break.js"(exports, module) {
      "use strict";
      module.exports = locate;
      function locate(value, fromIndex) {
        var index = value.indexOf("\n", fromIndex);
        while (index > fromIndex) {
          if (value.charAt(index - 1) !== " ") {
            break;
          }
          index--;
        }
        return index;
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/break.js
  var require_break2 = __commonJS({
    "node_modules/remark-parse/lib/tokenize/break.js"(exports, module) {
      "use strict";
      var locate = require_break();
      module.exports = hardBreak;
      hardBreak.locator = locate;
      var space = " ";
      var lineFeed = "\n";
      var minBreakLength = 2;
      function hardBreak(eat, value, silent) {
        var length = value.length;
        var index = -1;
        var queue = "";
        var character;
        while (++index < length) {
          character = value.charAt(index);
          if (character === lineFeed) {
            if (index < minBreakLength) {
              return;
            }
            if (silent) {
              return true;
            }
            queue += character;
            return eat(queue)({ type: "break" });
          }
          if (character !== space) {
            return;
          }
          queue += character;
        }
      }
    }
  });

  // node_modules/remark-parse/lib/tokenize/text.js
  var require_text = __commonJS({
    "node_modules/remark-parse/lib/tokenize/text.js"(exports, module) {
      "use strict";
      module.exports = text;
      function text(eat, value, silent) {
        var self = this;
        var methods;
        var tokenizers;
        var index;
        var length;
        var subvalue;
        var position;
        var tokenizer;
        var name;
        var min;
        var now;
        if (silent) {
          return true;
        }
        methods = self.inlineMethods;
        length = methods.length;
        tokenizers = self.inlineTokenizers;
        index = -1;
        min = value.length;
        while (++index < length) {
          name = methods[index];
          if (name === "text" || !tokenizers[name]) {
            continue;
          }
          tokenizer = tokenizers[name].locator;
          if (!tokenizer) {
            eat.file.fail("Missing locator: `" + name + "`");
          }
          position = tokenizer.call(self, value, 1);
          if (position !== -1 && position < min) {
            min = position;
          }
        }
        subvalue = value.slice(0, min);
        now = eat.now();
        self.decode(subvalue, now, handler);
        function handler(content, position2, source) {
          eat(source || content)({ type: "text", value: content });
        }
      }
    }
  });

  // node_modules/remark-parse/lib/parser.js
  var require_parser = __commonJS({
    "node_modules/remark-parse/lib/parser.js"(exports, module) {
      "use strict";
      var xtend = require_immutable();
      var toggle = require_state_toggle();
      var vfileLocation = require_vfile_location();
      var unescape = require_unescape();
      var decode = require_decode();
      var tokenizer = require_tokenizer();
      module.exports = Parser;
      function Parser(doc, file) {
        this.file = file;
        this.offset = {};
        this.options = xtend(this.options);
        this.setOptions({});
        this.inList = false;
        this.inBlock = false;
        this.inLink = false;
        this.atStart = true;
        this.toOffset = vfileLocation(file).toOffset;
        this.unescape = unescape(this, "escape");
        this.decode = decode(this);
      }
      var proto = Parser.prototype;
      proto.setOptions = require_set_options();
      proto.parse = require_parse();
      proto.options = require_defaults();
      proto.exitStart = toggle("atStart", true);
      proto.enterList = toggle("inList", false);
      proto.enterLink = toggle("inLink", false);
      proto.enterBlock = toggle("inBlock", false);
      proto.interruptParagraph = [
        ["thematicBreak"],
        ["list"],
        ["atxHeading"],
        ["fencedCode"],
        ["blockquote"],
        ["html"],
        ["setextHeading", { commonmark: false }],
        ["definition", { commonmark: false }]
      ];
      proto.interruptList = [
        ["atxHeading", { pedantic: false }],
        ["fencedCode", { pedantic: false }],
        ["thematicBreak", { pedantic: false }],
        ["definition", { commonmark: false }]
      ];
      proto.interruptBlockquote = [
        ["indentedCode", { commonmark: true }],
        ["fencedCode", { commonmark: true }],
        ["atxHeading", { commonmark: true }],
        ["setextHeading", { commonmark: true }],
        ["thematicBreak", { commonmark: true }],
        ["html", { commonmark: true }],
        ["list", { commonmark: true }],
        ["definition", { commonmark: false }]
      ];
      proto.blockTokenizers = {
        blankLine: require_blank_line(),
        indentedCode: require_code_indented(),
        fencedCode: require_code_fenced(),
        blockquote: require_blockquote(),
        atxHeading: require_heading_atx(),
        thematicBreak: require_thematic_break(),
        list: require_list(),
        setextHeading: require_heading_setext(),
        html: require_html_block(),
        definition: require_definition(),
        table: require_table(),
        paragraph: require_paragraph()
      };
      proto.inlineTokenizers = {
        escape: require_escape2(),
        autoLink: require_auto_link(),
        url: require_url2(),
        email: require_email2(),
        html: require_html_inline(),
        link: require_link2(),
        reference: require_reference(),
        strong: require_strong2(),
        emphasis: require_emphasis2(),
        deletion: require_delete2(),
        code: require_code_inline2(),
        break: require_break2(),
        text: require_text()
      };
      proto.blockMethods = keys(proto.blockTokenizers);
      proto.inlineMethods = keys(proto.inlineTokenizers);
      proto.tokenizeBlock = tokenizer("block");
      proto.tokenizeInline = tokenizer("inline");
      proto.tokenizeFactory = tokenizer;
      function keys(value) {
        var result = [];
        var key;
        for (key in value) {
          result.push(key);
        }
        return result;
      }
    }
  });

  // node_modules/remark-parse/index.js
  var require_remark_parse = __commonJS({
    "node_modules/remark-parse/index.js"(exports, module) {
      "use strict";
      var unherit = require_unherit();
      var xtend = require_immutable();
      var Parser = require_parser();
      module.exports = parse2;
      parse2.Parser = Parser;
      function parse2(options2) {
        var settings = this.data("settings");
        var Local = unherit(Parser);
        Local.prototype.options = xtend(Local.prototype.options, settings, options2);
        this.Parser = Local;
      }
    }
  });

  // node_modules/bail/index.js
  var require_bail = __commonJS({
    "node_modules/bail/index.js"(exports, module) {
      "use strict";
      module.exports = bail;
      function bail(err) {
        if (err) {
          throw err;
        }
      }
    }
  });

  // node_modules/is-buffer/index.js
  var require_is_buffer = __commonJS({
    "node_modules/is-buffer/index.js"(exports, module) {
      module.exports = function isBuffer(obj) {
        return obj != null && obj.constructor != null && typeof obj.constructor.isBuffer === "function" && obj.constructor.isBuffer(obj);
      };
    }
  });

  // node_modules/extend/index.js
  var require_extend = __commonJS({
    "node_modules/extend/index.js"(exports, module) {
      "use strict";
      var hasOwn = Object.prototype.hasOwnProperty;
      var toStr = Object.prototype.toString;
      var defineProperty = Object.defineProperty;
      var gOPD = Object.getOwnPropertyDescriptor;
      var isArray = function isArray2(arr) {
        if (typeof Array.isArray === "function") {
          return Array.isArray(arr);
        }
        return toStr.call(arr) === "[object Array]";
      };
      var isPlainObject = function isPlainObject2(obj) {
        if (!obj || toStr.call(obj) !== "[object Object]") {
          return false;
        }
        var hasOwnConstructor = hasOwn.call(obj, "constructor");
        var hasIsPrototypeOf = obj.constructor && obj.constructor.prototype && hasOwn.call(obj.constructor.prototype, "isPrototypeOf");
        if (obj.constructor && !hasOwnConstructor && !hasIsPrototypeOf) {
          return false;
        }
        var key;
        for (key in obj) {
        }
        return typeof key === "undefined" || hasOwn.call(obj, key);
      };
      var setProperty = function setProperty2(target, options2) {
        if (defineProperty && options2.name === "__proto__") {
          defineProperty(target, options2.name, {
            enumerable: true,
            configurable: true,
            value: options2.newValue,
            writable: true
          });
        } else {
          target[options2.name] = options2.newValue;
        }
      };
      var getProperty = function getProperty2(obj, name) {
        if (name === "__proto__") {
          if (!hasOwn.call(obj, name)) {
            return void 0;
          } else if (gOPD) {
            return gOPD(obj, name).value;
          }
        }
        return obj[name];
      };
      module.exports = function extend() {
        var options2, name, src, copy, copyIsArray, clone;
        var target = arguments[0];
        var i = 1;
        var length = arguments.length;
        var deep = false;
        if (typeof target === "boolean") {
          deep = target;
          target = arguments[1] || {};
          i = 2;
        }
        if (target == null || typeof target !== "object" && typeof target !== "function") {
          target = {};
        }
        for (; i < length; ++i) {
          options2 = arguments[i];
          if (options2 != null) {
            for (name in options2) {
              src = getProperty(target, name);
              copy = getProperty(options2, name);
              if (target !== copy) {
                if (deep && copy && (isPlainObject(copy) || (copyIsArray = isArray(copy)))) {
                  if (copyIsArray) {
                    copyIsArray = false;
                    clone = src && isArray(src) ? src : [];
                  } else {
                    clone = src && isPlainObject(src) ? src : {};
                  }
                  setProperty(target, { name, newValue: extend(deep, clone, copy) });
                } else if (typeof copy !== "undefined") {
                  setProperty(target, { name, newValue: copy });
                }
              }
            }
          }
        }
        return target;
      };
    }
  });

  // node_modules/is-plain-obj/index.js
  var require_is_plain_obj = __commonJS({
    "node_modules/is-plain-obj/index.js"(exports, module) {
      "use strict";
      module.exports = (value) => {
        if (Object.prototype.toString.call(value) !== "[object Object]") {
          return false;
        }
        const prototype = Object.getPrototypeOf(value);
        return prototype === null || prototype === Object.prototype;
      };
    }
  });

  // node_modules/trough/wrap.js
  var require_wrap = __commonJS({
    "node_modules/trough/wrap.js"(exports, module) {
      "use strict";
      var slice = [].slice;
      module.exports = wrap;
      function wrap(fn, callback) {
        var invoked;
        return wrapped;
        function wrapped() {
          var params = slice.call(arguments, 0);
          var callback2 = fn.length > params.length;
          var result;
          if (callback2) {
            params.push(done);
          }
          try {
            result = fn.apply(null, params);
          } catch (error) {
            if (callback2 && invoked) {
              throw error;
            }
            return done(error);
          }
          if (!callback2) {
            if (result && typeof result.then === "function") {
              result.then(then, done);
            } else if (result instanceof Error) {
              done(result);
            } else {
              then(result);
            }
          }
        }
        function done() {
          if (!invoked) {
            invoked = true;
            callback.apply(null, arguments);
          }
        }
        function then(value) {
          done(null, value);
        }
      }
    }
  });

  // node_modules/trough/index.js
  var require_trough = __commonJS({
    "node_modules/trough/index.js"(exports, module) {
      "use strict";
      var wrap = require_wrap();
      module.exports = trough;
      trough.wrap = wrap;
      var slice = [].slice;
      function trough() {
        var fns = [];
        var middleware = {};
        middleware.run = run;
        middleware.use = use;
        return middleware;
        function run() {
          var index = -1;
          var input = slice.call(arguments, 0, -1);
          var done = arguments[arguments.length - 1];
          if (typeof done !== "function") {
            throw new Error("Expected function as last argument, not " + done);
          }
          next.apply(null, [null].concat(input));
          function next(err) {
            var fn = fns[++index];
            var params = slice.call(arguments, 0);
            var values = params.slice(1);
            var length = input.length;
            var pos = -1;
            if (err) {
              done(err);
              return;
            }
            while (++pos < length) {
              if (values[pos] === null || values[pos] === void 0) {
                values[pos] = input[pos];
              }
            }
            input = values;
            if (fn) {
              wrap(fn, next).apply(null, input);
            } else {
              done.apply(null, [null].concat(input));
            }
          }
        }
        function use(fn) {
          if (typeof fn !== "function") {
            throw new Error("Expected `fn` to be a function, not " + fn);
          }
          fns.push(fn);
          return middleware;
        }
      }
    }
  });

  // node_modules/unist-util-stringify-position/index.js
  var require_unist_util_stringify_position = __commonJS({
    "node_modules/unist-util-stringify-position/index.js"(exports, module) {
      "use strict";
      var own = {}.hasOwnProperty;
      module.exports = stringify;
      function stringify(value) {
        if (!value || typeof value !== "object") {
          return "";
        }
        if (own.call(value, "position") || own.call(value, "type")) {
          return position(value.position);
        }
        if (own.call(value, "start") || own.call(value, "end")) {
          return position(value);
        }
        if (own.call(value, "line") || own.call(value, "column")) {
          return point(value);
        }
        return "";
      }
      function point(point2) {
        if (!point2 || typeof point2 !== "object") {
          point2 = {};
        }
        return index(point2.line) + ":" + index(point2.column);
      }
      function position(pos) {
        if (!pos || typeof pos !== "object") {
          pos = {};
        }
        return point(pos.start) + "-" + point(pos.end);
      }
      function index(value) {
        return value && typeof value === "number" ? value : 1;
      }
    }
  });

  // node_modules/vfile-message/index.js
  var require_vfile_message = __commonJS({
    "node_modules/vfile-message/index.js"(exports, module) {
      "use strict";
      var stringify = require_unist_util_stringify_position();
      module.exports = VMessage;
      function VMessagePrototype() {
      }
      VMessagePrototype.prototype = Error.prototype;
      VMessage.prototype = new VMessagePrototype();
      var proto = VMessage.prototype;
      proto.file = "";
      proto.name = "";
      proto.reason = "";
      proto.message = "";
      proto.stack = "";
      proto.fatal = null;
      proto.column = null;
      proto.line = null;
      function VMessage(reason, position, origin) {
        var parts;
        var range;
        var location;
        if (typeof position === "string") {
          origin = position;
          position = null;
        }
        parts = parseOrigin(origin);
        range = stringify(position) || "1:1";
        location = {
          start: { line: null, column: null },
          end: { line: null, column: null }
        };
        if (position && position.position) {
          position = position.position;
        }
        if (position) {
          if (position.start) {
            location = position;
            position = position.start;
          } else {
            location.start = position;
          }
        }
        if (reason.stack) {
          this.stack = reason.stack;
          reason = reason.message;
        }
        this.message = reason;
        this.name = range;
        this.reason = reason;
        this.line = position ? position.line : null;
        this.column = position ? position.column : null;
        this.location = location;
        this.source = parts[0];
        this.ruleId = parts[1];
      }
      function parseOrigin(origin) {
        var result = [null, null];
        var index;
        if (typeof origin === "string") {
          index = origin.indexOf(":");
          if (index === -1) {
            result[1] = origin;
          } else {
            result[0] = origin.slice(0, index);
            result[1] = origin.slice(index + 1);
          }
        }
        return result;
      }
    }
  });

  // node_modules/vfile/lib/minpath.browser.js
  var require_minpath_browser = __commonJS({
    "node_modules/vfile/lib/minpath.browser.js"(exports) {
      "use strict";
      exports.basename = basename;
      exports.dirname = dirname;
      exports.extname = extname;
      exports.join = join2;
      exports.sep = "/";
      function basename(path, ext) {
        var start = 0;
        var end = -1;
        var index;
        var firstNonSlashEnd;
        var seenNonSlash;
        var extIndex;
        if (ext !== void 0 && typeof ext !== "string") {
          throw new TypeError('"ext" argument must be a string');
        }
        assertPath(path);
        index = path.length;
        if (ext === void 0 || !ext.length || ext.length > path.length) {
          while (index--) {
            if (path.charCodeAt(index) === 47) {
              if (seenNonSlash) {
                start = index + 1;
                break;
              }
            } else if (end < 0) {
              seenNonSlash = true;
              end = index + 1;
            }
          }
          return end < 0 ? "" : path.slice(start, end);
        }
        if (ext === path) {
          return "";
        }
        firstNonSlashEnd = -1;
        extIndex = ext.length - 1;
        while (index--) {
          if (path.charCodeAt(index) === 47) {
            if (seenNonSlash) {
              start = index + 1;
              break;
            }
          } else {
            if (firstNonSlashEnd < 0) {
              seenNonSlash = true;
              firstNonSlashEnd = index + 1;
            }
            if (extIndex > -1) {
              if (path.charCodeAt(index) === ext.charCodeAt(extIndex--)) {
                if (extIndex < 0) {
                  end = index;
                }
              } else {
                extIndex = -1;
                end = firstNonSlashEnd;
              }
            }
          }
        }
        if (start === end) {
          end = firstNonSlashEnd;
        } else if (end < 0) {
          end = path.length;
        }
        return path.slice(start, end);
      }
      function dirname(path) {
        var end;
        var unmatchedSlash;
        var index;
        assertPath(path);
        if (!path.length) {
          return ".";
        }
        end = -1;
        index = path.length;
        while (--index) {
          if (path.charCodeAt(index) === 47) {
            if (unmatchedSlash) {
              end = index;
              break;
            }
          } else if (!unmatchedSlash) {
            unmatchedSlash = true;
          }
        }
        return end < 0 ? path.charCodeAt(0) === 47 ? "/" : "." : end === 1 && path.charCodeAt(0) === 47 ? "//" : path.slice(0, end);
      }
      function extname(path) {
        var startDot = -1;
        var startPart = 0;
        var end = -1;
        var preDotState = 0;
        var unmatchedSlash;
        var code;
        var index;
        assertPath(path);
        index = path.length;
        while (index--) {
          code = path.charCodeAt(index);
          if (code === 47) {
            if (unmatchedSlash) {
              startPart = index + 1;
              break;
            }
            continue;
          }
          if (end < 0) {
            unmatchedSlash = true;
            end = index + 1;
          }
          if (code === 46) {
            if (startDot < 0) {
              startDot = index;
            } else if (preDotState !== 1) {
              preDotState = 1;
            }
          } else if (startDot > -1) {
            preDotState = -1;
          }
        }
        if (startDot < 0 || end < 0 || // We saw a non-dot character immediately before the dot.
        preDotState === 0 || // The (right-most) trimmed path component is exactly `..`.
        preDotState === 1 && startDot === end - 1 && startDot === startPart + 1) {
          return "";
        }
        return path.slice(startDot, end);
      }
      function join2() {
        var index = -1;
        var joined;
        while (++index < arguments.length) {
          assertPath(arguments[index]);
          if (arguments[index]) {
            joined = joined === void 0 ? arguments[index] : joined + "/" + arguments[index];
          }
        }
        return joined === void 0 ? "." : normalize(joined);
      }
      function normalize(path) {
        var absolute;
        var value;
        assertPath(path);
        absolute = path.charCodeAt(0) === 47;
        value = normalizeString(path, !absolute);
        if (!value.length && !absolute) {
          value = ".";
        }
        if (value.length && path.charCodeAt(path.length - 1) === 47) {
          value += "/";
        }
        return absolute ? "/" + value : value;
      }
      function normalizeString(path, allowAboveRoot) {
        var result = "";
        var lastSegmentLength = 0;
        var lastSlash = -1;
        var dots = 0;
        var index = -1;
        var code;
        var lastSlashIndex;
        while (++index <= path.length) {
          if (index < path.length) {
            code = path.charCodeAt(index);
          } else if (code === 47) {
            break;
          } else {
            code = 47;
          }
          if (code === 47) {
            if (lastSlash === index - 1 || dots === 1) {
            } else if (lastSlash !== index - 1 && dots === 2) {
              if (result.length < 2 || lastSegmentLength !== 2 || result.charCodeAt(result.length - 1) !== 46 || result.charCodeAt(result.length - 2) !== 46) {
                if (result.length > 2) {
                  lastSlashIndex = result.lastIndexOf("/");
                  if (lastSlashIndex !== result.length - 1) {
                    if (lastSlashIndex < 0) {
                      result = "";
                      lastSegmentLength = 0;
                    } else {
                      result = result.slice(0, lastSlashIndex);
                      lastSegmentLength = result.length - 1 - result.lastIndexOf("/");
                    }
                    lastSlash = index;
                    dots = 0;
                    continue;
                  }
                } else if (result.length) {
                  result = "";
                  lastSegmentLength = 0;
                  lastSlash = index;
                  dots = 0;
                  continue;
                }
              }
              if (allowAboveRoot) {
                result = result.length ? result + "/.." : "..";
                lastSegmentLength = 2;
              }
            } else {
              if (result.length) {
                result += "/" + path.slice(lastSlash + 1, index);
              } else {
                result = path.slice(lastSlash + 1, index);
              }
              lastSegmentLength = index - lastSlash - 1;
            }
            lastSlash = index;
            dots = 0;
          } else if (code === 46 && dots > -1) {
            dots++;
          } else {
            dots = -1;
          }
        }
        return result;
      }
      function assertPath(path) {
        if (typeof path !== "string") {
          throw new TypeError(
            "Path must be a string. Received " + JSON.stringify(path)
          );
        }
      }
    }
  });

  // node_modules/vfile/lib/minproc.browser.js
  var require_minproc_browser = __commonJS({
    "node_modules/vfile/lib/minproc.browser.js"(exports) {
      "use strict";
      exports.cwd = cwd;
      function cwd() {
        return "/";
      }
    }
  });

  // node_modules/vfile/lib/core.js
  var require_core = __commonJS({
    "node_modules/vfile/lib/core.js"(exports, module) {
      "use strict";
      var p = require_minpath_browser();
      var proc = require_minproc_browser();
      var buffer = require_is_buffer();
      module.exports = VFile;
      var own = {}.hasOwnProperty;
      var order = ["history", "path", "basename", "stem", "extname", "dirname"];
      VFile.prototype.toString = toString;
      Object.defineProperty(VFile.prototype, "path", { get: getPath, set: setPath });
      Object.defineProperty(VFile.prototype, "dirname", {
        get: getDirname,
        set: setDirname
      });
      Object.defineProperty(VFile.prototype, "basename", {
        get: getBasename,
        set: setBasename
      });
      Object.defineProperty(VFile.prototype, "extname", {
        get: getExtname,
        set: setExtname
      });
      Object.defineProperty(VFile.prototype, "stem", { get: getStem, set: setStem });
      function VFile(options2) {
        var prop;
        var index;
        if (!options2) {
          options2 = {};
        } else if (typeof options2 === "string" || buffer(options2)) {
          options2 = { contents: options2 };
        } else if ("message" in options2 && "messages" in options2) {
          return options2;
        }
        if (!(this instanceof VFile)) {
          return new VFile(options2);
        }
        this.data = {};
        this.messages = [];
        this.history = [];
        this.cwd = proc.cwd();
        index = -1;
        while (++index < order.length) {
          prop = order[index];
          if (own.call(options2, prop)) {
            this[prop] = options2[prop];
          }
        }
        for (prop in options2) {
          if (order.indexOf(prop) < 0) {
            this[prop] = options2[prop];
          }
        }
      }
      function getPath() {
        return this.history[this.history.length - 1];
      }
      function setPath(path) {
        assertNonEmpty(path, "path");
        if (this.path !== path) {
          this.history.push(path);
        }
      }
      function getDirname() {
        return typeof this.path === "string" ? p.dirname(this.path) : void 0;
      }
      function setDirname(dirname) {
        assertPath(this.path, "dirname");
        this.path = p.join(dirname || "", this.basename);
      }
      function getBasename() {
        return typeof this.path === "string" ? p.basename(this.path) : void 0;
      }
      function setBasename(basename) {
        assertNonEmpty(basename, "basename");
        assertPart(basename, "basename");
        this.path = p.join(this.dirname || "", basename);
      }
      function getExtname() {
        return typeof this.path === "string" ? p.extname(this.path) : void 0;
      }
      function setExtname(extname) {
        assertPart(extname, "extname");
        assertPath(this.path, "extname");
        if (extname) {
          if (extname.charCodeAt(0) !== 46) {
            throw new Error("`extname` must start with `.`");
          }
          if (extname.indexOf(".", 1) > -1) {
            throw new Error("`extname` cannot contain multiple dots");
          }
        }
        this.path = p.join(this.dirname, this.stem + (extname || ""));
      }
      function getStem() {
        return typeof this.path === "string" ? p.basename(this.path, this.extname) : void 0;
      }
      function setStem(stem) {
        assertNonEmpty(stem, "stem");
        assertPart(stem, "stem");
        this.path = p.join(this.dirname || "", stem + (this.extname || ""));
      }
      function toString(encoding) {
        return (this.contents || "").toString(encoding);
      }
      function assertPart(part, name) {
        if (part && part.indexOf(p.sep) > -1) {
          throw new Error(
            "`" + name + "` cannot be a path: did not expect `" + p.sep + "`"
          );
        }
      }
      function assertNonEmpty(part, name) {
        if (!part) {
          throw new Error("`" + name + "` cannot be empty");
        }
      }
      function assertPath(path, name) {
        if (!path) {
          throw new Error("Setting `" + name + "` requires `path` to be set too");
        }
      }
    }
  });

  // node_modules/vfile/lib/index.js
  var require_lib = __commonJS({
    "node_modules/vfile/lib/index.js"(exports, module) {
      "use strict";
      var VMessage = require_vfile_message();
      var VFile = require_core();
      module.exports = VFile;
      VFile.prototype.message = message;
      VFile.prototype.info = info;
      VFile.prototype.fail = fail;
      function message(reason, position, origin) {
        var message2 = new VMessage(reason, position, origin);
        if (this.path) {
          message2.name = this.path + ":" + message2.name;
          message2.file = this.path;
        }
        message2.fatal = false;
        this.messages.push(message2);
        return message2;
      }
      function fail() {
        var message2 = this.message.apply(this, arguments);
        message2.fatal = true;
        throw message2;
      }
      function info() {
        var message2 = this.message.apply(this, arguments);
        message2.fatal = null;
        return message2;
      }
    }
  });

  // node_modules/vfile/index.js
  var require_vfile = __commonJS({
    "node_modules/vfile/index.js"(exports, module) {
      "use strict";
      module.exports = require_lib();
    }
  });

  // node_modules/unified/index.js
  var require_unified = __commonJS({
    "node_modules/unified/index.js"(exports, module) {
      "use strict";
      var bail = require_bail();
      var buffer = require_is_buffer();
      var extend = require_extend();
      var plain = require_is_plain_obj();
      var trough = require_trough();
      var vfile = require_vfile();
      module.exports = unified2().freeze();
      var slice = [].slice;
      var own = {}.hasOwnProperty;
      var pipeline = trough().use(pipelineParse).use(pipelineRun).use(pipelineStringify);
      function pipelineParse(p, ctx) {
        ctx.tree = p.parse(ctx.file);
      }
      function pipelineRun(p, ctx, next) {
        p.run(ctx.tree, ctx.file, done);
        function done(error, tree, file) {
          if (error) {
            next(error);
          } else {
            ctx.tree = tree;
            ctx.file = file;
            next();
          }
        }
      }
      function pipelineStringify(p, ctx) {
        var result = p.stringify(ctx.tree, ctx.file);
        if (result === void 0 || result === null) {
        } else if (typeof result === "string" || buffer(result)) {
          if ("value" in ctx.file) {
            ctx.file.value = result;
          }
          ctx.file.contents = result;
        } else {
          ctx.file.result = result;
        }
      }
      function unified2() {
        var attachers = [];
        var transformers = trough();
        var namespace = {};
        var freezeIndex = -1;
        var frozen;
        processor.data = data;
        processor.freeze = freeze;
        processor.attachers = attachers;
        processor.use = use;
        processor.parse = parse2;
        processor.stringify = stringify;
        processor.run = run;
        processor.runSync = runSync;
        processor.process = process2;
        processor.processSync = processSync;
        return processor;
        function processor() {
          var destination = unified2();
          var index = -1;
          while (++index < attachers.length) {
            destination.use.apply(null, attachers[index]);
          }
          destination.data(extend(true, {}, namespace));
          return destination;
        }
        function freeze() {
          var values;
          var transformer;
          if (frozen) {
            return processor;
          }
          while (++freezeIndex < attachers.length) {
            values = attachers[freezeIndex];
            if (values[1] === false) {
              continue;
            }
            if (values[1] === true) {
              values[1] = void 0;
            }
            transformer = values[0].apply(processor, values.slice(1));
            if (typeof transformer === "function") {
              transformers.use(transformer);
            }
          }
          frozen = true;
          freezeIndex = Infinity;
          return processor;
        }
        function data(key, value) {
          if (typeof key === "string") {
            if (arguments.length === 2) {
              assertUnfrozen("data", frozen);
              namespace[key] = value;
              return processor;
            }
            return own.call(namespace, key) && namespace[key] || null;
          }
          if (key) {
            assertUnfrozen("data", frozen);
            namespace = key;
            return processor;
          }
          return namespace;
        }
        function use(value) {
          var settings;
          assertUnfrozen("use", frozen);
          if (value === null || value === void 0) {
          } else if (typeof value === "function") {
            addPlugin.apply(null, arguments);
          } else if (typeof value === "object") {
            if ("length" in value) {
              addList(value);
            } else {
              addPreset(value);
            }
          } else {
            throw new Error("Expected usable value, not `" + value + "`");
          }
          if (settings) {
            namespace.settings = extend(namespace.settings || {}, settings);
          }
          return processor;
          function addPreset(result) {
            addList(result.plugins);
            if (result.settings) {
              settings = extend(settings || {}, result.settings);
            }
          }
          function add(value2) {
            if (typeof value2 === "function") {
              addPlugin(value2);
            } else if (typeof value2 === "object") {
              if ("length" in value2) {
                addPlugin.apply(null, value2);
              } else {
                addPreset(value2);
              }
            } else {
              throw new Error("Expected usable value, not `" + value2 + "`");
            }
          }
          function addList(plugins) {
            var index = -1;
            if (plugins === null || plugins === void 0) {
            } else if (typeof plugins === "object" && "length" in plugins) {
              while (++index < plugins.length) {
                add(plugins[index]);
              }
            } else {
              throw new Error("Expected a list of plugins, not `" + plugins + "`");
            }
          }
          function addPlugin(plugin, value2) {
            var entry = find(plugin);
            if (entry) {
              if (plain(entry[1]) && plain(value2)) {
                value2 = extend(true, entry[1], value2);
              }
              entry[1] = value2;
            } else {
              attachers.push(slice.call(arguments));
            }
          }
        }
        function find(plugin) {
          var index = -1;
          while (++index < attachers.length) {
            if (attachers[index][0] === plugin) {
              return attachers[index];
            }
          }
        }
        function parse2(doc) {
          var file = vfile(doc);
          var Parser;
          freeze();
          Parser = processor.Parser;
          assertParser("parse", Parser);
          if (newable(Parser, "parse")) {
            return new Parser(String(file), file).parse();
          }
          return Parser(String(file), file);
        }
        function run(node, file, cb) {
          assertNode(node);
          freeze();
          if (!cb && typeof file === "function") {
            cb = file;
            file = null;
          }
          if (!cb) {
            return new Promise(executor);
          }
          executor(null, cb);
          function executor(resolve, reject) {
            transformers.run(node, vfile(file), done);
            function done(error, tree, file2) {
              tree = tree || node;
              if (error) {
                reject(error);
              } else if (resolve) {
                resolve(tree);
              } else {
                cb(null, tree, file2);
              }
            }
          }
        }
        function runSync(node, file) {
          var result;
          var complete;
          run(node, file, done);
          assertDone("runSync", "run", complete);
          return result;
          function done(error, tree) {
            complete = true;
            result = tree;
            bail(error);
          }
        }
        function stringify(node, doc) {
          var file = vfile(doc);
          var Compiler;
          freeze();
          Compiler = processor.Compiler;
          assertCompiler("stringify", Compiler);
          assertNode(node);
          if (newable(Compiler, "compile")) {
            return new Compiler(node, file).compile();
          }
          return Compiler(node, file);
        }
        function process2(doc, cb) {
          freeze();
          assertParser("process", processor.Parser);
          assertCompiler("process", processor.Compiler);
          if (!cb) {
            return new Promise(executor);
          }
          executor(null, cb);
          function executor(resolve, reject) {
            var file = vfile(doc);
            pipeline.run(processor, { file }, done);
            function done(error) {
              if (error) {
                reject(error);
              } else if (resolve) {
                resolve(file);
              } else {
                cb(null, file);
              }
            }
          }
        }
        function processSync(doc) {
          var file;
          var complete;
          freeze();
          assertParser("processSync", processor.Parser);
          assertCompiler("processSync", processor.Compiler);
          file = vfile(doc);
          process2(file, done);
          assertDone("processSync", "process", complete);
          return file;
          function done(error) {
            complete = true;
            bail(error);
          }
        }
      }
      function newable(value, name) {
        return typeof value === "function" && value.prototype && // A function with keys in its prototype is probably a constructor.
        // Classes’ prototype methods are not enumerable, so we check if some value
        // exists in the prototype.
        (keys(value.prototype) || name in value.prototype);
      }
      function keys(value) {
        var key;
        for (key in value) {
          return true;
        }
        return false;
      }
      function assertParser(name, Parser) {
        if (typeof Parser !== "function") {
          throw new Error("Cannot `" + name + "` without `Parser`");
        }
      }
      function assertCompiler(name, Compiler) {
        if (typeof Compiler !== "function") {
          throw new Error("Cannot `" + name + "` without `Compiler`");
        }
      }
      function assertUnfrozen(name, frozen) {
        if (frozen) {
          throw new Error(
            "Cannot invoke `" + name + "` on a frozen processor.\nCreate a new processor first, by invoking it: use `processor()` instead of `processor`."
          );
        }
      }
      function assertNode(node) {
        if (!node || typeof node.type !== "string") {
          throw new Error("Expected node, got `" + node + "`");
        }
      }
      function assertDone(name, asyncName, complete) {
        if (!complete) {
          throw new Error(
            "`" + name + "` finished async. Use `" + asyncName + "` instead"
          );
        }
      }
    }
  });

  // node_modules/remark-math/util.js
  var require_util = __commonJS({
    "node_modules/remark-math/util.js"(exports) {
      exports.isRemarkParser = isRemarkParser;
      exports.isRemarkCompiler = isRemarkCompiler;
      function isRemarkParser(parser) {
        return Boolean(parser && parser.prototype && parser.prototype.blockTokenizers);
      }
      function isRemarkCompiler(compiler) {
        return Boolean(compiler && compiler.prototype && compiler.prototype.visitors);
      }
    }
  });

  // node_modules/remark-math/inline.js
  var require_inline = __commonJS({
    "node_modules/remark-math/inline.js"(exports, module) {
      var util = require_util();
      module.exports = mathInline;
      var tab = 9;
      var space = 32;
      var dollarSign = 36;
      var digit0 = 48;
      var digit9 = 57;
      var backslash = 92;
      var classList = ["math", "math-inline"];
      var mathDisplay = "math-display";
      function mathInline(options2) {
        const parser = this.Parser;
        const compiler = this.Compiler;
        if (util.isRemarkParser(parser)) {
          attachParser(parser, options2);
        }
        if (util.isRemarkCompiler(compiler)) {
          attachCompiler(compiler, options2);
        }
      }
      function attachParser(parser, options2) {
        const proto = parser.prototype;
        const inlineMethods = proto.inlineMethods;
        mathInlineTokenizer.locator = locator;
        proto.inlineTokenizers.math = mathInlineTokenizer;
        inlineMethods.splice(inlineMethods.indexOf("text"), 0, "math");
        function locator(value, fromIndex) {
          return value.indexOf("$", fromIndex);
        }
        function mathInlineTokenizer(eat, value, silent) {
          const length = value.length;
          let double = false;
          let escaped = false;
          let index = 0;
          let previous;
          let code;
          let next;
          let contentStart;
          let contentEnd;
          let valueEnd;
          let content;
          if (value.charCodeAt(index) === backslash) {
            escaped = true;
            index++;
          }
          if (value.charCodeAt(index) !== dollarSign) {
            return;
          }
          index++;
          if (escaped) {
            if (silent) {
              return true;
            }
            return eat(value.slice(0, index))({ type: "text", value: "$" });
          }
          if (value.charCodeAt(index) === dollarSign) {
            double = true;
            index++;
          }
          next = value.charCodeAt(index);
          if (next === space || next === tab) {
            return;
          }
          contentStart = index;
          while (index < length) {
            code = next;
            next = value.charCodeAt(index + 1);
            if (code === dollarSign) {
              previous = value.charCodeAt(index - 1);
              if (previous !== space && previous !== tab && // eslint-disable-next-line no-self-compare
              (next !== next || next < digit0 || next > digit9) && (!double || next === dollarSign)) {
                contentEnd = index - 1;
                index++;
                if (double) {
                  index++;
                }
                valueEnd = index;
                break;
              }
            } else if (code === backslash) {
              index++;
              next = value.charCodeAt(index + 1);
            }
            index++;
          }
          if (valueEnd === void 0) {
            return;
          }
          if (silent) {
            return true;
          }
          content = value.slice(contentStart, contentEnd + 1);
          return eat(value.slice(0, valueEnd))({
            type: "inlineMath",
            value: content,
            data: {
              hName: "span",
              hProperties: {
                className: classList.concat(
                  double && options2.inlineMathDouble ? [mathDisplay] : []
                )
              },
              hChildren: [{ type: "text", value: content }]
            }
          });
        }
      }
      function attachCompiler(compiler) {
        const proto = compiler.prototype;
        proto.visitors.inlineMath = compileInlineMath;
        function compileInlineMath(node) {
          let fence = "$";
          const classes = node.data && node.data.hProperties && node.data.hProperties.className || [];
          if (classes.includes(mathDisplay)) {
            fence = "$$";
          }
          return fence + node.value + fence;
        }
      }
    }
  });

  // node_modules/remark-math/block.js
  var require_block = __commonJS({
    "node_modules/remark-math/block.js"(exports, module) {
      var util = require_util();
      module.exports = mathBlock;
      var lineFeed = 10;
      var space = 32;
      var dollarSign = 36;
      var lineFeedChar = "\n";
      var dollarSignChar = "$";
      var minFenceCount = 2;
      var classList = ["math", "math-display"];
      function mathBlock() {
        const parser = this.Parser;
        const compiler = this.Compiler;
        if (util.isRemarkParser(parser)) {
          attachParser(parser);
        }
        if (util.isRemarkCompiler(compiler)) {
          attachCompiler(compiler);
        }
      }
      function attachParser(parser) {
        const proto = parser.prototype;
        const blockMethods = proto.blockMethods;
        const interruptParagraph = proto.interruptParagraph;
        const interruptList = proto.interruptList;
        const interruptBlockquote = proto.interruptBlockquote;
        proto.blockTokenizers.math = mathBlockTokenizer;
        blockMethods.splice(blockMethods.indexOf("fencedCode") + 1, 0, "math");
        interruptParagraph.splice(interruptParagraph.indexOf("fencedCode") + 1, 0, [
          "math"
        ]);
        interruptList.splice(interruptList.indexOf("fencedCode") + 1, 0, ["math"]);
        interruptBlockquote.splice(interruptBlockquote.indexOf("fencedCode") + 1, 0, [
          "math"
        ]);
        function mathBlockTokenizer(eat, value, silent) {
          var length = value.length;
          var index = 0;
          let code;
          let content;
          let lineEnd;
          let lineIndex;
          let openingFenceIndentSize;
          let openingFenceSize;
          let openingFenceContentStart;
          let closingFence;
          let closingFenceSize;
          let lineContentStart;
          let lineContentEnd;
          while (index < length && value.charCodeAt(index) === space) {
            index++;
          }
          openingFenceIndentSize = index;
          while (index < length && value.charCodeAt(index) === dollarSign) {
            index++;
          }
          openingFenceSize = index - openingFenceIndentSize;
          if (openingFenceSize < minFenceCount) {
            return;
          }
          while (index < length && value.charCodeAt(index) === space) {
            index++;
          }
          openingFenceContentStart = index;
          while (index < length) {
            code = value.charCodeAt(index);
            if (code === dollarSign) {
              return;
            }
            if (code === lineFeed) {
              break;
            }
            index++;
          }
          if (value.charCodeAt(index) !== lineFeed) {
            return;
          }
          if (silent) {
            return true;
          }
          content = [];
          if (openingFenceContentStart !== index) {
            content.push(value.slice(openingFenceContentStart, index));
          }
          index++;
          lineEnd = value.indexOf(lineFeedChar, index + 1);
          lineEnd = lineEnd === -1 ? length : lineEnd;
          while (index < length) {
            closingFence = false;
            lineContentStart = index;
            lineContentEnd = lineEnd;
            lineIndex = lineEnd;
            closingFenceSize = 0;
            while (lineIndex > lineContentStart && value.charCodeAt(lineIndex - 1) === space) {
              lineIndex--;
            }
            while (lineIndex > lineContentStart && value.charCodeAt(lineIndex - 1) === dollarSign) {
              closingFenceSize++;
              lineIndex--;
            }
            if (openingFenceSize <= closingFenceSize && value.indexOf(dollarSignChar, lineContentStart) === lineIndex) {
              closingFence = true;
              lineContentEnd = lineIndex;
            }
            while (lineContentStart <= lineContentEnd && lineContentStart - index < openingFenceIndentSize && value.charCodeAt(lineContentStart) === space) {
              lineContentStart++;
            }
            if (closingFence) {
              while (lineContentEnd > lineContentStart && value.charCodeAt(lineContentEnd - 1) === space) {
                lineContentEnd--;
              }
            }
            if (!closingFence || lineContentStart !== lineContentEnd) {
              content.push(value.slice(lineContentStart, lineContentEnd));
            }
            if (closingFence) {
              break;
            }
            index = lineEnd + 1;
            lineEnd = value.indexOf(lineFeedChar, index + 1);
            lineEnd = lineEnd === -1 ? length : lineEnd;
          }
          content = content.join("\n");
          return eat(value.slice(0, lineEnd))({
            type: "math",
            value: content,
            data: {
              hName: "div",
              hProperties: { className: classList.concat() },
              hChildren: [{ type: "text", value: content }]
            }
          });
        }
      }
      function attachCompiler(compiler) {
        const proto = compiler.prototype;
        proto.visitors.math = compileBlockMath;
        function compileBlockMath(node) {
          return "$$\n" + node.value + "\n$$";
        }
      }
    }
  });

  // node_modules/remark-math/index.js
  var require_remark_math = __commonJS({
    "node_modules/remark-math/index.js"(exports, module) {
      var inlinePlugin = require_inline();
      var blockPlugin = require_block();
      module.exports = math;
      function math(options2) {
        var settings = options2 || {};
        blockPlugin.call(this, settings);
        inlinePlugin.call(this, settings);
      }
    }
  });

  // node_modules/remark-footnotes/index.js
  var require_remark_footnotes = __commonJS({
    "node_modules/remark-footnotes/index.js"(exports, module) {
      "use strict";
      module.exports = footnotes2;
      var tab = 9;
      var lineFeed = 10;
      var space = 32;
      var exclamationMark = 33;
      var colon = 58;
      var leftSquareBracket = 91;
      var backslash = 92;
      var rightSquareBracket = 93;
      var caret = 94;
      var graveAccent = 96;
      var tabSize = 4;
      var maxSlice = 1024;
      function footnotes2(options2) {
        var parser = this.Parser;
        var compiler = this.Compiler;
        if (isRemarkParser(parser)) {
          attachParser(parser, options2);
        }
        if (isRemarkCompiler(compiler)) {
          attachCompiler(compiler);
        }
      }
      function isRemarkParser(parser) {
        return Boolean(parser && parser.prototype && parser.prototype.blockTokenizers);
      }
      function isRemarkCompiler(compiler) {
        return Boolean(compiler && compiler.prototype && compiler.prototype.visitors);
      }
      function attachParser(parser, options2) {
        var settings = options2 || {};
        var proto = parser.prototype;
        var blocks = proto.blockTokenizers;
        var spans = proto.inlineTokenizers;
        var blockMethods = proto.blockMethods;
        var inlineMethods = proto.inlineMethods;
        var originalDefinition = blocks.definition;
        var originalReference = spans.reference;
        var interruptors = [];
        var index = -1;
        var length = blockMethods.length;
        var method;
        while (++index < length) {
          method = blockMethods[index];
          if (method === "newline" || method === "indentedCode" || method === "paragraph" || method === "footnoteDefinition") {
            continue;
          }
          interruptors.push([method]);
        }
        interruptors.push(["footnoteDefinition"]);
        if (settings.inlineNotes) {
          before(inlineMethods, "reference", "inlineNote");
          spans.inlineNote = footnote;
        }
        before(blockMethods, "definition", "footnoteDefinition");
        before(inlineMethods, "reference", "footnoteCall");
        blocks.definition = definition;
        blocks.footnoteDefinition = footnoteDefinition;
        spans.footnoteCall = footnoteCall;
        spans.reference = reference;
        proto.interruptFootnoteDefinition = interruptors;
        reference.locator = originalReference.locator;
        footnoteCall.locator = locateFootnoteCall;
        footnote.locator = locateFootnote;
        function footnoteDefinition(eat, value, silent) {
          var self = this;
          var interruptors2 = self.interruptFootnoteDefinition;
          var offsets = self.offset;
          var length2 = value.length + 1;
          var index2 = 0;
          var content = [];
          var label;
          var labelStart;
          var labelEnd;
          var code;
          var now;
          var add;
          var exit;
          var children;
          var start;
          var indent2;
          var contentStart;
          var lines;
          var line2;
          while (index2 < length2) {
            code = value.charCodeAt(index2);
            if (code !== tab && code !== space)
              break;
            index2++;
          }
          if (value.charCodeAt(index2++) !== leftSquareBracket)
            return;
          if (value.charCodeAt(index2++) !== caret)
            return;
          labelStart = index2;
          while (index2 < length2) {
            code = value.charCodeAt(index2);
            if (code !== code || code === lineFeed || code === tab || code === space) {
              return;
            }
            if (code === rightSquareBracket) {
              labelEnd = index2;
              index2++;
              break;
            }
            index2++;
          }
          if (labelEnd === void 0 || labelStart === labelEnd || value.charCodeAt(index2++) !== colon) {
            return;
          }
          if (silent) {
            return true;
          }
          label = value.slice(labelStart, labelEnd);
          now = eat.now();
          start = 0;
          indent2 = 0;
          contentStart = index2;
          lines = [];
          while (index2 < length2) {
            code = value.charCodeAt(index2);
            if (code !== code || code === lineFeed) {
              line2 = {
                start,
                contentStart: contentStart || index2,
                contentEnd: index2,
                end: index2
              };
              lines.push(line2);
              if (code === lineFeed) {
                start = index2 + 1;
                indent2 = 0;
                contentStart = void 0;
                line2.end = start;
              }
            } else if (indent2 !== void 0) {
              if (code === space || code === tab) {
                indent2 += code === space ? 1 : tabSize - indent2 % tabSize;
                if (indent2 > tabSize) {
                  indent2 = void 0;
                  contentStart = index2;
                }
              } else {
                if (indent2 < tabSize && line2 && (line2.contentStart === line2.contentEnd || interrupt(interruptors2, blocks, self, [
                  eat,
                  value.slice(index2, maxSlice),
                  true
                ]))) {
                  break;
                }
                indent2 = void 0;
                contentStart = index2;
              }
            }
            index2++;
          }
          index2 = -1;
          length2 = lines.length;
          while (length2 > 0) {
            line2 = lines[length2 - 1];
            if (line2.contentStart !== line2.contentEnd) {
              break;
            }
            length2--;
          }
          add = eat(value.slice(0, line2.contentEnd));
          while (++index2 < length2) {
            line2 = lines[index2];
            offsets[now.line + index2] = (offsets[now.line + index2] || 0) + (line2.contentStart - line2.start);
            content.push(value.slice(line2.contentStart, line2.end));
          }
          exit = self.enterBlock();
          children = self.tokenizeBlock(content.join(""), now);
          exit();
          return add({
            type: "footnoteDefinition",
            identifier: label.toLowerCase(),
            label,
            children
          });
        }
        function footnoteCall(eat, value, silent) {
          var length2 = value.length + 1;
          var index2 = 0;
          var label;
          var labelStart;
          var labelEnd;
          var code;
          if (value.charCodeAt(index2++) !== leftSquareBracket)
            return;
          if (value.charCodeAt(index2++) !== caret)
            return;
          labelStart = index2;
          while (index2 < length2) {
            code = value.charCodeAt(index2);
            if (code !== code || code === lineFeed || code === tab || code === space) {
              return;
            }
            if (code === rightSquareBracket) {
              labelEnd = index2;
              index2++;
              break;
            }
            index2++;
          }
          if (labelEnd === void 0 || labelStart === labelEnd) {
            return;
          }
          if (silent) {
            return true;
          }
          label = value.slice(labelStart, labelEnd);
          return eat(value.slice(0, index2))({
            type: "footnoteReference",
            identifier: label.toLowerCase(),
            label
          });
        }
        function footnote(eat, value, silent) {
          var self = this;
          var length2 = value.length + 1;
          var index2 = 0;
          var balance = 0;
          var now;
          var code;
          var contentStart;
          var contentEnd;
          var fenceStart;
          var fenceOpenSize;
          var fenceCloseSize;
          if (value.charCodeAt(index2++) !== caret)
            return;
          if (value.charCodeAt(index2++) !== leftSquareBracket)
            return;
          contentStart = index2;
          while (index2 < length2) {
            code = value.charCodeAt(index2);
            if (code !== code) {
              return;
            }
            if (fenceOpenSize === void 0) {
              if (code === backslash) {
                index2 += 2;
              } else if (code === leftSquareBracket) {
                balance++;
                index2++;
              } else if (code === rightSquareBracket) {
                if (balance === 0) {
                  contentEnd = index2;
                  index2++;
                  break;
                } else {
                  balance--;
                  index2++;
                }
              } else if (code === graveAccent) {
                fenceStart = index2;
                fenceOpenSize = 1;
                while (value.charCodeAt(fenceStart + fenceOpenSize) === graveAccent) {
                  fenceOpenSize++;
                }
                index2 += fenceOpenSize;
              } else {
                index2++;
              }
            } else {
              if (code === graveAccent) {
                fenceStart = index2;
                fenceCloseSize = 1;
                while (value.charCodeAt(fenceStart + fenceCloseSize) === graveAccent) {
                  fenceCloseSize++;
                }
                index2 += fenceCloseSize;
                if (fenceOpenSize === fenceCloseSize) {
                  fenceOpenSize = void 0;
                }
                fenceCloseSize = void 0;
              } else {
                index2++;
              }
            }
          }
          if (contentEnd === void 0) {
            return;
          }
          if (silent) {
            return true;
          }
          now = eat.now();
          now.column += 2;
          now.offset += 2;
          return eat(value.slice(0, index2))({
            type: "footnote",
            children: self.tokenizeInline(value.slice(contentStart, contentEnd), now)
          });
        }
        function reference(eat, value, silent) {
          var index2 = 0;
          if (value.charCodeAt(index2) === exclamationMark)
            index2++;
          if (value.charCodeAt(index2) !== leftSquareBracket)
            return;
          if (value.charCodeAt(index2 + 1) === caret)
            return;
          return originalReference.call(this, eat, value, silent);
        }
        function definition(eat, value, silent) {
          var index2 = 0;
          var code = value.charCodeAt(index2);
          while (code === space || code === tab)
            code = value.charCodeAt(++index2);
          if (code !== leftSquareBracket)
            return;
          if (value.charCodeAt(index2 + 1) === caret)
            return;
          return originalDefinition.call(this, eat, value, silent);
        }
        function locateFootnoteCall(value, from) {
          return value.indexOf("[", from);
        }
        function locateFootnote(value, from) {
          return value.indexOf("^[", from);
        }
      }
      function attachCompiler(compiler) {
        var serializers = compiler.prototype.visitors;
        var indent2 = "    ";
        serializers.footnote = footnote;
        serializers.footnoteReference = footnoteReference;
        serializers.footnoteDefinition = footnoteDefinition;
        function footnote(node) {
          return "^[" + this.all(node).join("") + "]";
        }
        function footnoteReference(node) {
          return "[^" + (node.label || node.identifier) + "]";
        }
        function footnoteDefinition(node) {
          var lines = this.all(node).join("\n\n").split("\n");
          var index = 0;
          var length = lines.length;
          var line2;
          while (++index < length) {
            line2 = lines[index];
            if (line2 === "")
              continue;
            lines[index] = indent2 + line2;
          }
          return "[^" + (node.label || node.identifier) + "]: " + lines.join("\n");
        }
      }
      function before(list, before2, value) {
        list.splice(list.indexOf(before2), 0, value);
      }
      function interrupt(list, tokenizers, ctx, parameters) {
        var length = list.length;
        var index = -1;
        while (++index < length) {
          if (tokenizers[list[index][0]].apply(ctx, parameters)) {
            return true;
          }
        }
        return false;
      }
    }
  });

  // src/plugins/markdown.js
  var markdown_exports = {};
  __export(markdown_exports, {
    languages: () => languages_evaluate_default,
    options: () => options_default,
    parsers: () => parser_markdown_exports,
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

  // src/language-markdown/printer-markdown.js
  var import_collapse_white_space2 = __toESM(require_collapse_white_space(), 1);

  // node_modules/escape-string-regexp/index.js
  function escapeStringRegexp(string) {
    if (typeof string !== "string") {
      throw new TypeError("Expected a string");
    }
    return string.replace(/[|\\{}()[\]^$+*?.]/g, "\\$&").replace(/-/g, "\\x2d");
  }

  // src/utils/get-min-not-present-continuous-count.js
  function getMinNotPresentContinuousCount(text, searchString) {
    const matches = text.match(
      new RegExp(`(${escapeStringRegexp(searchString)})+`, "g")
    );
    if (matches === null) {
      return 0;
    }
    const countPresent = /* @__PURE__ */ new Map();
    let max = 0;
    for (const match of matches) {
      const count = match.length / searchString.length;
      countPresent.set(count, true);
      if (count > max) {
        max = count;
      }
    }
    for (let i = 1; i < max; i++) {
      if (!countPresent.get(i)) {
        return i;
      }
    }
    return max + 1;
  }
  var get_min_not_present_continuous_count_default = getMinNotPresentContinuousCount;

  // src/utils/get-max-continuous-count.js
  function getMaxContinuousCount(text, searchString) {
    const results = text.match(
      new RegExp(`(${escapeStringRegexp(searchString)})+`, "g")
    );
    if (results === null) {
      return 0;
    }
    return results.reduce(
      (maxCount, result) => Math.max(maxCount, result.length / searchString.length),
      0
    );
  }
  var get_max_continuous_count_default = getMaxContinuousCount;

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
  function markAsRoot(contents) {
    return align({ type: "root" }, contents);
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
  var literallineWithoutBreakParent = {
    type: DOC_TYPE_LINE,
    hard: true,
    literal: true
  };
  var line = { type: DOC_TYPE_LINE };
  var softline = { type: DOC_TYPE_LINE, soft: true };
  var hardline = [hardlineWithoutBreakParent, breakParent];
  var literalline = [literallineWithoutBreakParent, breakParent];
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
  function normalizeParts(parts) {
    const newParts = [];
    const restParts = parts.filter(Boolean);
    while (restParts.length > 0) {
      const part = restParts.shift();
      if (!part) {
        continue;
      }
      if (Array.isArray(part)) {
        restParts.unshift(...part);
        continue;
      }
      if (newParts.length > 0 && typeof at_default(
        /* isOptionalObject*/
        false,
        newParts,
        -1
      ) === "string" && typeof part === "string") {
        newParts[newParts.length - 1] += part;
        continue;
      }
      newParts.push(part);
    }
    return newParts;
  }
  function normalizeDoc(doc) {
    return mapDoc(doc, (currentDoc) => {
      if (Array.isArray(currentDoc)) {
        return normalizeParts(currentDoc);
      }
      if (!currentDoc.parts) {
        return currentDoc;
      }
      return {
        ...currentDoc,
        parts: normalizeParts(currentDoc.parts)
      };
    });
  }
  function replaceEndOfLine(doc, replacement = literalline) {
    return mapDoc(doc, (currentDoc) => typeof currentDoc === "string" ? join(replacement, currentDoc.split("\n")) : currentDoc);
  }

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
    const lineSuffix = [];
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
              const hasLineSuffix = lineSuffix.length > 0;
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
          const contentFits = fits(contentFlatCmd, [], rem, lineSuffix.length > 0, groupModeMap, true);
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
          const firstAndSecondContentFits = fits(firstAndSecondContentFlatCmd, [], rem, lineSuffix.length > 0, groupModeMap, true);
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
          lineSuffix.push({
            ind,
            mode,
            doc: doc2.contents
          });
          break;
        case DOC_TYPE_LINE_SUFFIX_BOUNDARY:
          if (lineSuffix.length > 0) {
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
              if (lineSuffix.length > 0) {
                cmds.push({
                  ind,
                  mode,
                  doc: doc2
                }, ...lineSuffix.reverse());
                lineSuffix.length = 0;
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
      if (cmds.length === 0 && lineSuffix.length > 0) {
        cmds.push(...lineSuffix.reverse());
        lineSuffix.length = 0;
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
  function inferParser(options2, fileInfo) {
    const languages = options2.plugins.flatMap(
      (plugin) => (
        // @ts-expect-error -- Safe
        plugin.languages ?? []
      )
    );
    const language = getLanguageByName(languages, fileInfo.language) ?? getLanguageByFilename(languages, fileInfo.physicalFile) ?? getLanguageByFilename(languages, fileInfo.file) ?? getLanguageByInterpreter(languages, fileInfo.physicalFile);
    return language == null ? void 0 : language.parsers[0];
  }
  var infer_parser_default = inferParser;

  // scripts/build/shims/assert.js
  var assert = () => {
  };
  assert.ok = assert;
  assert.strictEqual = assert;

  // src/language-markdown/loc.js
  function locStart(node) {
    return node.position.start.offset;
  }
  function locEnd(node) {
    return node.position.end.offset;
  }

  // src/language-markdown/constants.evaluate.js
  var cjkPattern = "(?:[\\u02ea-\\u02eb\\u1100-\\u11ff\\u2e80-\\u2e99\\u2e9b-\\u2ef3\\u2f00-\\u2fd5\\u2ff0-\\u303f\\u3041-\\u3096\\u3099-\\u309f\\u30a1-\\u30fa\\u30fc-\\u30ff\\u3105-\\u312f\\u3131-\\u318e\\u3190-\\u3191\\u3196-\\u31ba\\u31c0-\\u31e3\\u31f0-\\u321e\\u322a-\\u3247\\u3260-\\u327e\\u328a-\\u32b0\\u32c0-\\u32cb\\u32d0-\\u3370\\u337b-\\u337f\\u33e0-\\u33fe\\u3400-\\u4db5\\u4e00-\\u9fef\\ua960-\\ua97c\\uac00-\\ud7a3\\ud7b0-\\ud7c6\\ud7cb-\\ud7fb\\uf900-\\ufa6d\\ufa70-\\ufad9\\ufe10-\\ufe1f\\ufe30-\\ufe6f\\uff00-\\uffef]|[\\ud840-\\ud868\\ud86a-\\ud86c\\ud86f-\\ud872\\ud874-\\ud879][\\udc00-\\udfff]|\\ud82c[\\udc00-\\udd1e\\udd50-\\udd52\\udd64-\\udd67]|\\ud83c[\\ude00\\ude50-\\ude51]|\\ud869[\\udc00-\\uded6\\udf00-\\udfff]|\\ud86d[\\udc00-\\udf34\\udf40-\\udfff]|\\ud86e[\\udc00-\\udc1d\\udc20-\\udfff]|\\ud873[\\udc00-\\udea1\\udeb0-\\udfff]|\\ud87a[\\udc00-\\udfe0]|\\ud87e[\\udc00-\\ude1d])(?:[\\ufe00-\\ufe0f]|\\udb40[\\udd00-\\uddef])?";
  var kRegex = new RegExp("[\\u1100-\\u11ff\\u3001-\\u3003\\u3008-\\u3011\\u3013-\\u301f\\u302e-\\u3030\\u3037\\u30fb\\u3131-\\u318e\\u3200-\\u321e\\u3260-\\u327e\\ua960-\\ua97c\\uac00-\\ud7a3\\ud7b0-\\ud7c6\\ud7cb-\\ud7fb\\ufe45-\\ufe46\\uff61-\\uff65\\uffa0-\\uffbe\\uffc2-\\uffc7\\uffca-\\uffcf\\uffd2-\\uffd7\\uffda-\\uffdc]", "");
  var punctuationPattern = "[\\u0021-\\u002f\\u003a-\\u0040\\u005b-\\u0060\\u007b-\\u007e\\u00a1\\u00a7\\u00ab\\u00b6-\\u00b7\\u00bb\\u00bf\\u037e\\u0387\\u055a-\\u055f\\u0589-\\u058a\\u05be\\u05c0\\u05c3\\u05c6\\u05f3-\\u05f4\\u0609-\\u060a\\u060c-\\u060d\\u061b\\u061e-\\u061f\\u066a-\\u066d\\u06d4\\u0700-\\u070d\\u07f7-\\u07f9\\u0830-\\u083e\\u085e\\u0964-\\u0965\\u0970\\u09fd\\u0a76\\u0af0\\u0c77\\u0c84\\u0df4\\u0e4f\\u0e5a-\\u0e5b\\u0f04-\\u0f12\\u0f14\\u0f3a-\\u0f3d\\u0f85\\u0fd0-\\u0fd4\\u0fd9-\\u0fda\\u104a-\\u104f\\u10fb\\u1360-\\u1368\\u1400\\u166e\\u169b-\\u169c\\u16eb-\\u16ed\\u1735-\\u1736\\u17d4-\\u17d6\\u17d8-\\u17da\\u1800-\\u180a\\u1944-\\u1945\\u1a1e-\\u1a1f\\u1aa0-\\u1aa6\\u1aa8-\\u1aad\\u1b5a-\\u1b60\\u1bfc-\\u1bff\\u1c3b-\\u1c3f\\u1c7e-\\u1c7f\\u1cc0-\\u1cc7\\u1cd3\\u2010-\\u2027\\u2030-\\u2043\\u2045-\\u2051\\u2053-\\u205e\\u207d-\\u207e\\u208d-\\u208e\\u2308-\\u230b\\u2329-\\u232a\\u2768-\\u2775\\u27c5-\\u27c6\\u27e6-\\u27ef\\u2983-\\u2998\\u29d8-\\u29db\\u29fc-\\u29fd\\u2cf9-\\u2cfc\\u2cfe-\\u2cff\\u2d70\\u2e00-\\u2e2e\\u2e30-\\u2e4f\\u3001-\\u3003\\u3008-\\u3011\\u3014-\\u301f\\u3030\\u303d\\u30a0\\u30fb\\ua4fe-\\ua4ff\\ua60d-\\ua60f\\ua673\\ua67e\\ua6f2-\\ua6f7\\ua874-\\ua877\\ua8ce-\\ua8cf\\ua8f8-\\ua8fa\\ua8fc\\ua92e-\\ua92f\\ua95f\\ua9c1-\\ua9cd\\ua9de-\\ua9df\\uaa5c-\\uaa5f\\uaade-\\uaadf\\uaaf0-\\uaaf1\\uabeb\\ufd3e-\\ufd3f\\ufe10-\\ufe19\\ufe30-\\ufe52\\ufe54-\\ufe61\\ufe63\\ufe68\\ufe6a-\\ufe6b\\uff01-\\uff03\\uff05-\\uff0a\\uff0c-\\uff0f\\uff1a-\\uff1b\\uff1f-\\uff20\\uff3b-\\uff3d\\uff3f\\uff5b\\uff5d\\uff5f-\\uff65]|\\ud800[\\udd00-\\udd02\\udf9f\\udfd0]|\\ud801[\\udd6f]|\\ud802[\\udc57\\udd1f\\udd3f\\ude50-\\ude58\\ude7f\\udef0-\\udef6\\udf39-\\udf3f\\udf99-\\udf9c]|\\ud803[\\udf55-\\udf59]|\\ud804[\\udc47-\\udc4d\\udcbb-\\udcbc\\udcbe-\\udcc1\\udd40-\\udd43\\udd74-\\udd75\\uddc5-\\uddc8\\uddcd\\udddb\\udddd-\\udddf\\ude38-\\ude3d\\udea9]|\\ud805[\\udc4b-\\udc4f\\udc5b\\udc5d\\udcc6\\uddc1-\\uddd7\\ude41-\\ude43\\ude60-\\ude6c\\udf3c-\\udf3e]|\\ud806[\\udc3b\\udde2\\ude3f-\\ude46\\ude9a-\\ude9c\\ude9e-\\udea2]|\\ud807[\\udc41-\\udc45\\udc70-\\udc71\\udef7-\\udef8\\udfff]|\\ud809[\\udc70-\\udc74]|\\ud81a[\\ude6e-\\ude6f\\udef5\\udf37-\\udf3b\\udf44]|\\ud81b[\\ude97-\\ude9a\\udfe2]|\\ud82f[\\udc9f]|\\ud836[\\ude87-\\ude8b]|\\ud83a[\\udd5e-\\udd5f]";

  // src/language-markdown/utils.js
  var INLINE_NODE_TYPES = /* @__PURE__ */ new Set(["liquidNode", "inlineCode", "emphasis", "esComment", "strong", "delete", "wikiLink", "link", "linkReference", "image", "imageReference", "footnote", "footnoteReference", "sentence", "whitespace", "word", "break", "inlineMath"]);
  var INLINE_NODE_WRAPPER_TYPES = /* @__PURE__ */ new Set([...INLINE_NODE_TYPES, "tableCell", "paragraph", "heading"]);
  var punctuationRegex = new RegExp(punctuationPattern);
  var KIND_NON_CJK = "non-cjk";
  var KIND_CJ_LETTER = "cj-letter";
  var KIND_K_LETTER = "k-letter";
  var KIND_CJK_PUNCTUATION = "cjk-punctuation";
  function splitText(text) {
    const nodes = [];
    const tokens = text.split(/([\t\n ]+)/);
    for (const [index, token] of tokens.entries()) {
      if (index % 2 === 1) {
        nodes.push({
          type: "whitespace",
          value: /\n/.test(token) ? "\n" : " "
        });
        continue;
      }
      if ((index === 0 || index === tokens.length - 1) && token === "") {
        continue;
      }
      const innerTokens = token.split(new RegExp(`(${cjkPattern})`));
      for (const [innerIndex, innerToken] of innerTokens.entries()) {
        if ((innerIndex === 0 || innerIndex === innerTokens.length - 1) && innerToken === "") {
          continue;
        }
        if (innerIndex % 2 === 0) {
          if (innerToken !== "") {
            appendNode({
              type: "word",
              value: innerToken,
              kind: KIND_NON_CJK,
              hasLeadingPunctuation: punctuationRegex.test(innerToken[0]),
              hasTrailingPunctuation: punctuationRegex.test(at_default(
                /* isOptionalObject*/
                false,
                innerToken,
                -1
              ))
            });
          }
          continue;
        }
        appendNode(punctuationRegex.test(innerToken) ? {
          type: "word",
          value: innerToken,
          kind: KIND_CJK_PUNCTUATION,
          hasLeadingPunctuation: true,
          hasTrailingPunctuation: true
        } : {
          type: "word",
          value: innerToken,
          // Korean uses space to divide words, but Chinese & Japanese do not
          kind: kRegex.test(innerToken) ? KIND_K_LETTER : KIND_CJ_LETTER,
          hasLeadingPunctuation: false,
          hasTrailingPunctuation: false
        });
      }
    }
    if (false) {
      for (let i = 1; i < nodes.length; i++) {
        assert_default(!(nodes[i - 1].type === "whitespace" && nodes[i].type === "whitespace"), "splitText should not create consecutive whitespace nodes");
      }
    }
    return nodes;
    function appendNode(node) {
      const lastNode = at_default(
        /* isOptionalObject*/
        false,
        nodes,
        -1
      );
      if ((lastNode == null ? void 0 : lastNode.type) === "word" && !isBetween(KIND_NON_CJK, KIND_CJK_PUNCTUATION) && // disallow leading/trailing full-width whitespace
      ![lastNode.value, node.value].some((value) => /\u3000/.test(value))) {
        nodes.push({
          type: "whitespace",
          value: ""
        });
      }
      nodes.push(node);
      function isBetween(kind1, kind2) {
        return lastNode.kind === kind1 && node.kind === kind2 || lastNode.kind === kind2 && node.kind === kind1;
      }
    }
  }
  function getOrderedListItemInfo(orderListItem, originalText) {
    const [, numberText, marker, leadingSpaces] = originalText.slice(orderListItem.position.start.offset, orderListItem.position.end.offset).match(/^\s*(\d+)(\.|\))(\s*)/);
    return {
      numberText,
      marker,
      leadingSpaces
    };
  }
  function hasGitDiffFriendlyOrderedList(node, options2) {
    if (!node.ordered) {
      return false;
    }
    if (node.children.length < 2) {
      return false;
    }
    const firstNumber = Number(getOrderedListItemInfo(node.children[0], options2.originalText).numberText);
    const secondNumber = Number(getOrderedListItemInfo(node.children[1], options2.originalText).numberText);
    if (firstNumber === 0 && node.children.length > 2) {
      const thirdNumber = Number(getOrderedListItemInfo(node.children[2], options2.originalText).numberText);
      return secondNumber === 1 && thirdNumber === 1;
    }
    return secondNumber === 1;
  }
  function getFencedCodeBlockValue(node, originalText) {
    const {
      value
    } = node;
    if (node.position.end.offset === originalText.length && value.endsWith("\n") && // Code block has no end mark
    originalText.endsWith("\n")) {
      return value.slice(0, -1);
    }
    return value;
  }
  function mapAst(ast, handler) {
    return function preorder(node, index, parentStack) {
      const newNode = {
        ...handler(node, index, parentStack)
      };
      if (newNode.children) {
        newNode.children = newNode.children.map((child, index2) => preorder(child, index2, [newNode, ...parentStack]));
      }
      return newNode;
    }(ast, null, []);
  }
  function isAutolink(node) {
    if ((node == null ? void 0 : node.type) !== "link" || node.children.length !== 1) {
      return false;
    }
    const [child] = node.children;
    return locStart(node) === locStart(child) && locEnd(node) === locEnd(child);
  }

  // src/language-markdown/embed.js
  function embed(path, options2) {
    const { node } = path;
    if (node.type === "code" && node.lang !== null) {
      const parser = infer_parser_default(options2, { language: node.lang });
      if (parser) {
        return (textToDoc) => {
          const styleUnit = options2.__inJsTemplate ? "~" : "`";
          const style = styleUnit.repeat(
            Math.max(3, get_max_continuous_count_default(node.value, styleUnit) + 1)
          );
          const newOptions = { parser };
          if (node.lang === "tsx") {
            newOptions.filepath = "dummy.tsx";
          }
          const doc = textToDoc(
            getFencedCodeBlockValue(node, options2.originalText),
            newOptions
          );
          return markAsRoot([
            style,
            node.lang,
            node.meta ? " " + node.meta : "",
            hardline,
            replaceEndOfLine(doc),
            hardline,
            style
          ]);
        };
      }
    }
    switch (node.type) {
      case "front-matter":
        return (textToDoc) => print_default(node, textToDoc);
      case "import":
      case "export":
        return (textToDoc) => textToDoc(node.value, { parser: "babel" });
      case "jsx":
        return (textToDoc) => textToDoc(`<$>${node.value}</$>`, {
          parser: "__js_expression",
          rootMarker: "mdx"
        });
    }
    return null;
  }
  var embed_default = embed;

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
    const frontMatter2 = {
      type: "front-matter",
      lang,
      value,
      startDelimiter,
      endDelimiter,
      raw: raw.replace(/\n$/, "")
    };
    return {
      frontMatter: frontMatter2,
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

  // src/language-markdown/pragma.js
  var pragmas = ["format", "prettier"];
  function startWithPragma(text) {
    const pragma = `@(${pragmas.join("|")})`;
    const regex = new RegExp(
      // eslint-disable-next-line regexp/match-any -- possible bug
      [
        `<!--\\s*${pragma}\\s*-->`,
        `{\\s*\\/\\*\\s*${pragma}\\s*\\*\\/\\s*}`,
        `<!--.*\r?
[\\s\\S]*(^|
)[^\\S
]*${pragma}[^\\S
]*($|
)[\\s\\S]*
.*-->`
      ].join("|"),
      "m"
    );
    const matched = text.match(regex);
    return (matched == null ? void 0 : matched.index) === 0;
  }
  var hasPragma = (text) => startWithPragma(parse_default(text).content.trimStart());
  var insertPragma = (text) => {
    const extracted = parse_default(text);
    const pragma = `<!-- @${pragmas[0]} -->`;
    return extracted.frontMatter ? `${extracted.frontMatter.raw}

${pragma}

${extracted.content}` : `${pragma}

${extracted.content}`;
  };

  // src/language-markdown/print-preprocess.js
  var isSingleCharRegex = /^.$/su;
  function preprocess(ast, options2) {
    ast = restoreUnescapedCharacter(ast, options2);
    ast = mergeContinuousTexts(ast);
    ast = transformIndentedCodeblockAndMarkItsParentList(ast, options2);
    ast = markAlignedList(ast, options2);
    ast = splitTextIntoSentences(ast);
    return ast;
  }
  function restoreUnescapedCharacter(ast, options2) {
    return mapAst(ast, (node) => node.type !== "text" || node.value === "*" || node.value === "_" || // handle these cases in printer
    !isSingleCharRegex.test(node.value) || node.position.end.offset - node.position.start.offset === node.value.length ? node : {
      ...node,
      value: options2.originalText.slice(node.position.start.offset, node.position.end.offset)
    });
  }
  function mergeChildren(ast, shouldMerge, mergeNode) {
    return mapAst(ast, (node) => {
      if (!node.children) {
        return node;
      }
      const children = node.children.reduce((current, child) => {
        const lastChild = at_default(
          /* isOptionalObject*/
          false,
          current,
          -1
        );
        if (lastChild && shouldMerge(lastChild, child)) {
          current.splice(-1, 1, mergeNode(lastChild, child));
        } else {
          current.push(child);
        }
        return current;
      }, []);
      return {
        ...node,
        children
      };
    });
  }
  function mergeContinuousTexts(ast) {
    return mergeChildren(ast, (prevNode, node) => prevNode.type === "text" && node.type === "text", (prevNode, node) => ({
      type: "text",
      value: prevNode.value + node.value,
      position: {
        start: prevNode.position.start,
        end: node.position.end
      }
    }));
  }
  function splitTextIntoSentences(ast) {
    return mapAst(ast, (node, index, [parentNode]) => {
      if (node.type !== "text") {
        return node;
      }
      let {
        value
      } = node;
      if (parentNode.type === "paragraph") {
        if (index === 0) {
          value = value.trimStart();
        }
        if (index === parentNode.children.length - 1) {
          value = value.trimEnd();
        }
      }
      return {
        type: "sentence",
        position: node.position,
        children: splitText(value)
      };
    });
  }
  function transformIndentedCodeblockAndMarkItsParentList(ast, options2) {
    return mapAst(ast, (node, index, parentStack) => {
      if (node.type === "code") {
        const isIndented = /^\n?(?: {4,}|\t)/.test(options2.originalText.slice(node.position.start.offset, node.position.end.offset));
        node.isIndented = isIndented;
        if (isIndented) {
          for (let i = 0; i < parentStack.length; i++) {
            const parent = parentStack[i];
            if (parent.hasIndentedCodeblock) {
              break;
            }
            if (parent.type === "list") {
              parent.hasIndentedCodeblock = true;
            }
          }
        }
      }
      return node;
    });
  }
  function markAlignedList(ast, options2) {
    return mapAst(ast, (node, index, parentStack) => {
      if (node.type === "list" && node.children.length > 0) {
        for (let i = 0; i < parentStack.length; i++) {
          const parent = parentStack[i];
          if (parent.type === "list" && !parent.isAligned) {
            node.isAligned = false;
            return node;
          }
        }
        node.isAligned = isAligned(node);
      }
      return node;
    });
    function getListItemStart(listItem) {
      return listItem.children.length === 0 ? -1 : listItem.children[0].position.start.column - 1;
    }
    function isAligned(list) {
      if (!list.ordered) {
        return true;
      }
      const [firstItem, secondItem] = list.children;
      const firstInfo = getOrderedListItemInfo(firstItem, options2.originalText);
      if (firstInfo.leadingSpaces.length > 1) {
        return true;
      }
      const firstStart = getListItemStart(firstItem);
      if (firstStart === -1) {
        return false;
      }
      if (list.children.length === 1) {
        return firstStart % options2.tabWidth === 0;
      }
      const secondStart = getListItemStart(secondItem);
      if (firstStart !== secondStart) {
        return false;
      }
      if (firstStart % options2.tabWidth === 0) {
        return true;
      }
      const secondInfo = getOrderedListItemInfo(secondItem, options2.originalText);
      return secondInfo.leadingSpaces.length > 1;
    }
  }
  var print_preprocess_default = preprocess;

  // src/language-markdown/clean.js
  var import_collapse_white_space = __toESM(require_collapse_white_space(), 1);

  // src/utils/front-matter/is-front-matter.js
  function isFrontMatter(node) {
    return (node == null ? void 0 : node.type) === "front-matter";
  }
  var is_front_matter_default = isFrontMatter;

  // src/language-markdown/clean.js
  var ignoredProperties = /* @__PURE__ */ new Set([
    "position",
    "raw"
    // front-matter
  ]);
  function clean(ast, newObj, parent) {
    if (ast.type === "front-matter" || ast.type === "code" || ast.type === "yaml" || ast.type === "import" || ast.type === "export" || ast.type === "jsx") {
      delete newObj.value;
    }
    if (ast.type === "list") {
      delete newObj.isAligned;
    }
    if (ast.type === "list" || ast.type === "listItem") {
      delete newObj.spread;
    }
    if (ast.type === "text") {
      return null;
    }
    if (ast.type === "inlineCode") {
      newObj.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        ast.value,
        "\n",
        " "
      );
    }
    if (ast.type === "wikiLink") {
      newObj.value = string_replace_all_default(
        /* isOptionalObject*/
        false,
        ast.value.trim(),
        /[\t\n]+/g,
        " "
      );
    }
    if (ast.type === "definition" || ast.type === "linkReference" || ast.type === "imageReference") {
      newObj.label = (0, import_collapse_white_space.default)(ast.label);
    }
    if ((ast.type === "definition" || ast.type === "link" || ast.type === "image") && ast.title) {
      newObj.title = string_replace_all_default(
        /* isOptionalObject*/
        false,
        ast.title,
        /\\(?=["')])/g,
        ""
      );
    }
    if ((parent == null ? void 0 : parent.type) === "root" && parent.children.length > 0 && (parent.children[0] === ast || is_front_matter_default(parent.children[0]) && parent.children[1] === ast) && ast.type === "html" && startWithPragma(ast.value)) {
      return null;
    }
  }
  clean.ignoredProperties = ignoredProperties;
  var clean_default = clean;

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

  // src/language-markdown/visitor-keys.js
  var visitorKeys = {
    "front-matter": [],
    root: ["children"],
    paragraph: ["children"],
    sentence: ["children"],
    word: [],
    whitespace: [],
    emphasis: ["children"],
    strong: ["children"],
    delete: ["children"],
    inlineCode: [],
    wikiLink: [],
    link: ["children"],
    image: [],
    blockquote: ["children"],
    heading: ["children"],
    code: [],
    html: [],
    list: ["children"],
    thematicBreak: [],
    linkReference: ["children"],
    imageReference: [],
    definition: [],
    footnote: ["children"],
    footnoteReference: [],
    footnoteDefinition: ["children"],
    table: ["children"],
    tableCell: ["children"],
    break: [],
    liquidNode: [],
    import: [],
    export: [],
    esComment: [],
    jsx: [],
    math: [],
    inlineMath: [],
    tableRow: ["children"],
    listItem: ["children"],
    text: []
  };
  var visitor_keys_default = visitorKeys;

  // src/language-markdown/get-visitor-keys.js
  var getVisitorKeys = create_get_visitor_keys_default(visitor_keys_default);
  var get_visitor_keys_default = getVisitorKeys;

  // src/language-markdown/print-whitespace.js
  var SINGLE_LINE_NODE_TYPES = /* @__PURE__ */ new Set(["heading", "tableCell", "link", "wikiLink"]);
  var noBreakAfter = new Set(`$(\xA3\xA5\xB7'"\u3008\u300A\u300C\u300E\u3010\u3014\u3016\u301D\uFE59\uFE5B\uFF04\uFF08\uFF3B\uFF5B\uFFE1\uFFE5[{\u2035\uFE34\uFE35\uFE37\uFE39\uFE3B\uFE3D\uFE3F\uFE41\uFE43\uFE4F\u3018\uFF5F\xAB`);
  var noBreakBefore = new Set(`!%),.:;?]}\xA2\xB0\xB7'"\u2020\u2021\u203A\u2103\u2236\u3001\u3002\u3003\u3006\u3015\u3017\u301E\uFE5A\uFE5C\uFF01\uFF02\uFF05\uFF07\uFF09\uFF0C\uFF0E\uFF1A\uFF1B\uFF1F\uFF3D\uFF5D\uFF5E\u2013\u2014\u2022\u3009\u300B\u300D\uFE30\uFE31\uFE32\uFE33\uFE50\uFE51\uFE52\uFE53\uFE54\uFE55\uFE56\uFE58\uFE36\uFE38\uFE3A\uFE3C\uFE3E\uFE40\uFE42\uFE57\uFF5C\uFF64\u300F\u3011\u3019\u301F\uFF60\xBB\u30FD\u30FE\u30FC\u30A1\u30A3\u30A5\u30A7\u30A9\u30C3\u30E3\u30E5\u30E7\u30EE\u30F5\u30F6\u3041\u3043\u3045\u3047\u3049\u3063\u3083\u3085\u3087\u308E\u3095\u3096\u31F0\u31F1\u31F2\u31F3\u31F4\u31F5\u31F6\u31F7\u31F8\u31F9\u31FA\u31FB\u31FC\u31FD\u31FE\u31FF\u3005\u303B\u2010\u30A0\u301C\uFF5E\u203C\u2047\u2048\u2049\u30FB`);
  var lineBreakBetweenTheseAndCJConvertsToSpace = new Set("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
  function isInSentenceWithCJSpaces({
    parent: sentenceNode
  }) {
    if (sentenceNode.usesCJSpaces === void 0) {
      const stats = {
        " ": 0,
        "": 0
      };
      const {
        children
      } = sentenceNode;
      for (let i = 1; i < children.length - 1; ++i) {
        const node = children[i];
        if (node.type === "whitespace" && (node.value === " " || node.value === "")) {
          const previousKind = children[i - 1].kind;
          const nextKind = children[i + 1].kind;
          if (previousKind === KIND_CJ_LETTER && nextKind === KIND_NON_CJK || previousKind === KIND_NON_CJK && nextKind === KIND_CJ_LETTER) {
            ++stats[node.value];
          }
        }
      }
      sentenceNode.usesCJSpaces = stats[" "] > stats[""];
    }
    return sentenceNode.usesCJSpaces;
  }
  function lineBreakCanBeConvertedToSpace(path, isLink) {
    if (isLink) {
      return true;
    }
    const {
      previous,
      next
    } = path;
    if (!previous || !next) {
      return true;
    }
    const previousKind = previous.kind;
    const nextKind = next.kind;
    if (
      // "\n" between non-CJK or Korean characters always can be converted to a
      // space. Korean Hangul simulates Latin words. See
      // https://github.com/prettier/prettier/issues/6516
      isNonCJKOrKoreanLetter(previousKind) && isNonCJKOrKoreanLetter(nextKind) || // Han & Hangul: same way preferred
      previousKind === KIND_K_LETTER && nextKind === KIND_CJ_LETTER || nextKind === KIND_K_LETTER && previousKind === KIND_CJ_LETTER
    ) {
      return true;
    }
    if (
      // around CJK punctuation
      previousKind === KIND_CJK_PUNCTUATION || nextKind === KIND_CJK_PUNCTUATION || // between CJ
      previousKind === KIND_CJ_LETTER && nextKind === KIND_CJ_LETTER
    ) {
      return false;
    }
    if (lineBreakBetweenTheseAndCJConvertsToSpace.has(next.value[0]) || lineBreakBetweenTheseAndCJConvertsToSpace.has(at_default(
      /* isOptionalObject*/
      false,
      previous.value,
      -1
    ))) {
      return true;
    }
    if (previous.hasTrailingPunctuation || next.hasLeadingPunctuation) {
      return false;
    }
    return isInSentenceWithCJSpaces(path);
  }
  function isLetter(kind) {
    return kind === KIND_NON_CJK || kind === KIND_CJ_LETTER || kind === KIND_K_LETTER;
  }
  function isNonCJKOrKoreanLetter(kind) {
    return kind === KIND_NON_CJK || kind === KIND_K_LETTER;
  }
  function isBreakable(path, value, proseWrap, isLink, canBeSpace) {
    if (proseWrap !== "always" || path.hasAncestor((node) => SINGLE_LINE_NODE_TYPES.has(node.type))) {
      return false;
    }
    if (isLink) {
      return value !== "";
    }
    if (value === " ") {
      return true;
    }
    const {
      previous,
      next
    } = path;
    if (value === "" && ((previous == null ? void 0 : previous.kind) === KIND_K_LETTER && isLetter(next == null ? void 0 : next.kind) || (next == null ? void 0 : next.kind) === KIND_K_LETTER && isLetter(previous == null ? void 0 : previous.kind))) {
      return false;
    }
    const violatesCJKLineBreakingRules = !canBeSpace && (next && noBreakBefore.has(next.value[0]) || previous && noBreakAfter.has(at_default(
      /* isOptionalObject*/
      false,
      previous.value,
      -1
    )));
    if (violatesCJKLineBreakingRules) {
      return false;
    }
    return true;
  }
  function printWhitespace(path, value, proseWrap, isLink) {
    if (proseWrap === "preserve" && value === "\n") {
      return hardline;
    }
    const canBeSpace = value === " " || value === "\n" && lineBreakCanBeConvertedToSpace(path, isLink);
    if (isBreakable(path, value, proseWrap, isLink, canBeSpace)) {
      return canBeSpace ? line : softline;
    }
    return canBeSpace ? " " : "";
  }

  // src/language-markdown/printer-markdown.js
  var SIBLING_NODE_TYPES = /* @__PURE__ */ new Set(["listItem", "definition", "footnoteDefinition"]);
  function genericPrint(path, options2, print2) {
    var _a, _b, _c;
    const {
      node
    } = path;
    if (shouldRemainTheSameContent(path)) {
      return splitText(options2.originalText.slice(node.position.start.offset, node.position.end.offset)).map((node2) => node2.type === "word" ? node2.value : printWhitespace(path, node2.value, options2.proseWrap, true));
    }
    switch (node.type) {
      case "front-matter":
        return options2.originalText.slice(node.position.start.offset, node.position.end.offset);
      case "root":
        if (node.children.length === 0) {
          return "";
        }
        return [normalizeDoc(printRoot(path, options2, print2)), hardline];
      case "paragraph":
        return printChildren(path, options2, print2, {
          postprocessor: fill
        });
      case "sentence":
        return printChildren(path, options2, print2);
      case "word": {
        let escapedValue = string_replace_all_default(
          /* isOptionalObject*/
          false,
          string_replace_all_default(
            /* isOptionalObject*/
            false,
            node.value,
            "*",
            "\\*"
          ),
          new RegExp([`(^|${punctuationPattern})(_+)`, `(_+)(${punctuationPattern}|$)`].join("|"), "g"),
          (_, text1, underscore1, underscore2, text2) => string_replace_all_default(
            /* isOptionalObject*/
            false,
            underscore1 ? `${text1}${underscore1}` : `${underscore2}${text2}`,
            "_",
            "\\_"
          )
        );
        const isFirstSentence = (node2, name, index) => node2.type === "sentence" && index === 0;
        const isLastChildAutolink = (node2, name, index) => isAutolink(node2.children[index - 1]);
        if (escapedValue !== node.value && (path.match(void 0, isFirstSentence, isLastChildAutolink) || path.match(void 0, isFirstSentence, (node2, name, index) => node2.type === "emphasis" && index === 0, isLastChildAutolink))) {
          escapedValue = escapedValue.replace(/^(\\?[*_])+/, (prefix) => string_replace_all_default(
            /* isOptionalObject*/
            false,
            prefix,
            "\\",
            ""
          ));
        }
        return escapedValue;
      }
      case "whitespace": {
        const {
          next
        } = path;
        const proseWrap = (
          // leading char that may cause different syntax
          next && /^>|^(?:[*+-]|#{1,6}|\d+[).])$/.test(next.value) ? "never" : options2.proseWrap
        );
        return printWhitespace(path, node.value, proseWrap);
      }
      case "emphasis": {
        let style;
        if (isAutolink(node.children[0])) {
          style = options2.originalText[node.position.start.offset];
        } else {
          const {
            previous,
            next
          } = path;
          const hasPrevOrNextWord = (
            // `1*2*3` is considered emphasis but `1_2_3` is not
            (previous == null ? void 0 : previous.type) === "sentence" && ((_a = at_default(
              /* isOptionalObject*/
              false,
              previous.children,
              -1
            )) == null ? void 0 : _a.type) === "word" && !at_default(
              /* isOptionalObject*/
              false,
              previous.children,
              -1
            ).hasTrailingPunctuation || (next == null ? void 0 : next.type) === "sentence" && ((_b = next.children[0]) == null ? void 0 : _b.type) === "word" && !next.children[0].hasLeadingPunctuation
          );
          style = hasPrevOrNextWord || path.hasAncestor((node2) => node2.type === "emphasis") ? "*" : "_";
        }
        return [style, printChildren(path, options2, print2), style];
      }
      case "strong":
        return ["**", printChildren(path, options2, print2), "**"];
      case "delete":
        return ["~~", printChildren(path, options2, print2), "~~"];
      case "inlineCode": {
        const code = options2.proseWrap === "preserve" ? node.value : string_replace_all_default(
          /* isOptionalObject*/
          false,
          node.value,
          "\n",
          " "
        );
        const backtickCount = get_min_not_present_continuous_count_default(code, "`");
        const backtickString = "`".repeat(backtickCount || 1);
        const padding = code.startsWith("`") || code.endsWith("`") || /^[\n ]/.test(code) && /[\n ]$/.test(code) && /[^\n ]/.test(code) ? " " : "";
        return [backtickString, padding, code, padding, backtickString];
      }
      case "wikiLink": {
        let contents = "";
        if (options2.proseWrap === "preserve") {
          contents = node.value;
        } else {
          contents = string_replace_all_default(
            /* isOptionalObject*/
            false,
            node.value,
            /[\t\n]+/g,
            " "
          );
        }
        return ["[[", contents, "]]"];
      }
      case "link":
        switch (options2.originalText[node.position.start.offset]) {
          case "<": {
            const mailto = "mailto:";
            const url = (
              // <hello@example.com> is parsed as { url: "mailto:hello@example.com" }
              node.url.startsWith(mailto) && options2.originalText.slice(node.position.start.offset + 1, node.position.start.offset + 1 + mailto.length) !== mailto ? node.url.slice(mailto.length) : node.url
            );
            return ["<", url, ">"];
          }
          case "[":
            return ["[", printChildren(path, options2, print2), "](", printUrl(node.url, ")"), printTitle(node.title, options2), ")"];
          default:
            return options2.originalText.slice(node.position.start.offset, node.position.end.offset);
        }
      case "image":
        return ["![", node.alt || "", "](", printUrl(node.url, ")"), printTitle(node.title, options2), ")"];
      case "blockquote":
        return ["> ", align("> ", printChildren(path, options2, print2))];
      case "heading":
        return ["#".repeat(node.depth) + " ", printChildren(path, options2, print2)];
      case "code": {
        if (node.isIndented) {
          const alignment = " ".repeat(4);
          return align(alignment, [alignment, replaceEndOfLine(node.value, hardline)]);
        }
        const styleUnit = options2.__inJsTemplate ? "~" : "`";
        const style = styleUnit.repeat(Math.max(3, get_max_continuous_count_default(node.value, styleUnit) + 1));
        return [style, node.lang || "", node.meta ? " " + node.meta : "", hardline, replaceEndOfLine(getFencedCodeBlockValue(node, options2.originalText), hardline), hardline, style];
      }
      case "html": {
        const {
          parent,
          isLast
        } = path;
        const value = parent.type === "root" && isLast ? node.value.trimEnd() : node.value;
        const isHtmlComment = /^<!--.*-->$/s.test(value);
        return replaceEndOfLine(
          value,
          // @ts-expect-error
          isHtmlComment ? hardline : markAsRoot(literalline)
        );
      }
      case "list": {
        const nthSiblingIndex = getNthListSiblingIndex(node, path.parent);
        const isGitDiffFriendlyOrderedList = hasGitDiffFriendlyOrderedList(node, options2);
        return printChildren(path, options2, print2, {
          processor(childPath) {
            const prefix = getPrefix();
            const childNode = childPath.node;
            if (childNode.children.length === 2 && childNode.children[1].type === "html" && childNode.children[0].position.start.column !== childNode.children[1].position.start.column) {
              return [prefix, printListItem(childPath, options2, print2, prefix)];
            }
            return [prefix, align(" ".repeat(prefix.length), printListItem(childPath, options2, print2, prefix))];
            function getPrefix() {
              const rawPrefix = node.ordered ? (childPath.isFirst ? node.start : isGitDiffFriendlyOrderedList ? 1 : node.start + childPath.index) + (nthSiblingIndex % 2 === 0 ? ". " : ") ") : nthSiblingIndex % 2 === 0 ? "- " : "* ";
              return node.isAligned || /* workaround for https://github.com/remarkjs/remark/issues/315 */
              node.hasIndentedCodeblock ? alignListPrefix(rawPrefix, options2) : rawPrefix;
            }
          }
        });
      }
      case "thematicBreak": {
        const {
          ancestors
        } = path;
        const counter = ancestors.findIndex((node2) => node2.type === "list");
        if (counter === -1) {
          return "---";
        }
        const nthSiblingIndex = getNthListSiblingIndex(ancestors[counter], ancestors[counter + 1]);
        return nthSiblingIndex % 2 === 0 ? "***" : "---";
      }
      case "linkReference":
        return ["[", printChildren(path, options2, print2), "]", node.referenceType === "full" ? printLinkReference(node) : node.referenceType === "collapsed" ? "[]" : ""];
      case "imageReference":
        switch (node.referenceType) {
          case "full":
            return ["![", node.alt || "", "]", printLinkReference(node)];
          default:
            return ["![", node.alt, "]", node.referenceType === "collapsed" ? "[]" : ""];
        }
      case "definition": {
        const lineOrSpace = options2.proseWrap === "always" ? line : " ";
        return group([printLinkReference(node), ":", indent([lineOrSpace, printUrl(node.url), node.title === null ? "" : [lineOrSpace, printTitle(node.title, options2, false)]])]);
      }
      case "footnote":
        return ["[^", printChildren(path, options2, print2), "]"];
      case "footnoteReference":
        return printFootnoteReference(node);
      case "footnoteDefinition": {
        const shouldInlineFootnote = node.children.length === 1 && node.children[0].type === "paragraph" && (options2.proseWrap === "never" || options2.proseWrap === "preserve" && node.children[0].position.start.line === node.children[0].position.end.line);
        return [printFootnoteReference(node), ": ", shouldInlineFootnote ? printChildren(path, options2, print2) : group([align(" ".repeat(4), printChildren(path, options2, print2, {
          processor: ({
            isFirst
          }) => isFirst ? group([softline, print2()]) : print2()
        })), ((_c = path.next) == null ? void 0 : _c.type) === "footnoteDefinition" ? softline : ""])];
      }
      case "table":
        return printTable(path, options2, print2);
      case "tableCell":
        return printChildren(path, options2, print2);
      case "break":
        return /\s/.test(options2.originalText[node.position.start.offset]) ? ["  ", markAsRoot(literalline)] : ["\\", hardline];
      case "liquidNode":
        return replaceEndOfLine(node.value, hardline);
      case "import":
      case "export":
      case "jsx":
        return node.value;
      case "esComment":
        return ["{/* ", node.value, " */}"];
      case "math":
        return ["$$", hardline, node.value ? [replaceEndOfLine(node.value, hardline), hardline] : "", "$$"];
      case "inlineMath":
        return options2.originalText.slice(locStart(node), locEnd(node));
      case "tableRow":
      case "listItem":
      case "text":
      default:
        throw new unexpected_node_error_default(node, "Markdown");
    }
  }
  function printListItem(path, options2, print2, listPrefix) {
    const {
      node
    } = path;
    const prefix = node.checked === null ? "" : node.checked ? "[x] " : "[ ] ";
    return [prefix, printChildren(path, options2, print2, {
      processor({
        node: node2,
        isFirst
      }) {
        if (isFirst && node2.type !== "list") {
          return align(" ".repeat(prefix.length), print2());
        }
        const alignment = " ".repeat(
          clamp(options2.tabWidth - listPrefix.length, 0, 3)
          // 4+ will cause indented code block
        );
        return [alignment, align(alignment, print2())];
      }
    })];
  }
  function alignListPrefix(prefix, options2) {
    const additionalSpaces = getAdditionalSpaces();
    return prefix + " ".repeat(
      additionalSpaces >= 4 ? 0 : additionalSpaces
      // 4+ will cause indented code block
    );
    function getAdditionalSpaces() {
      const restSpaces = prefix.length % options2.tabWidth;
      return restSpaces === 0 ? 0 : options2.tabWidth - restSpaces;
    }
  }
  function getNthListSiblingIndex(node, parentNode) {
    return getNthSiblingIndex(node, parentNode, (siblingNode) => siblingNode.ordered === node.ordered);
  }
  function getNthSiblingIndex(node, parentNode, condition) {
    let index = -1;
    for (const childNode of parentNode.children) {
      if (childNode.type === node.type && condition(childNode)) {
        index++;
      } else {
        index = -1;
      }
      if (childNode === node) {
        return index;
      }
    }
  }
  function printTable(path, options2, print2) {
    const {
      node
    } = path;
    const columnMaxWidths = [];
    const contents = path.map(() => path.map(({
      index: columnIndex
    }) => {
      const text = printDocToString(print2(), options2).formatted;
      const width = get_string_width_default(text);
      columnMaxWidths[columnIndex] = Math.max(
        columnMaxWidths[columnIndex] || 3,
        // minimum width = 3 (---, :--, :-:, --:)
        width
      );
      return {
        text,
        width
      };
    }, "children"), "children");
    const alignedTable = printTableContents(
      /* isCompact */
      false
    );
    if (options2.proseWrap !== "never") {
      return [breakParent, alignedTable];
    }
    const compactTable = printTableContents(
      /* isCompact */
      true
    );
    return [breakParent, group(ifBreak(compactTable, alignedTable))];
    function printTableContents(isCompact) {
      const parts = [printRow(contents[0], isCompact), printAlign(isCompact)];
      if (contents.length > 1) {
        parts.push(join(hardlineWithoutBreakParent, contents.slice(1).map((rowContents) => printRow(rowContents, isCompact))));
      }
      return join(hardlineWithoutBreakParent, parts);
    }
    function printAlign(isCompact) {
      const align2 = columnMaxWidths.map((width, index) => {
        const align3 = node.align[index];
        const first = align3 === "center" || align3 === "left" ? ":" : "-";
        const last = align3 === "center" || align3 === "right" ? ":" : "-";
        const middle = isCompact ? "-" : "-".repeat(width - 2);
        return `${first}${middle}${last}`;
      });
      return `| ${align2.join(" | ")} |`;
    }
    function printRow(rowContents, isCompact) {
      const columns = rowContents.map(({
        text,
        width
      }, columnIndex) => {
        if (isCompact) {
          return text;
        }
        const spaces = columnMaxWidths[columnIndex] - width;
        const align2 = node.align[columnIndex];
        let before = 0;
        if (align2 === "right") {
          before = spaces;
        } else if (align2 === "center") {
          before = Math.floor(spaces / 2);
        }
        const after = spaces - before;
        return `${" ".repeat(before)}${text}${" ".repeat(after)}`;
      });
      return `| ${columns.join(" | ")} |`;
    }
  }
  function printRoot(path, options2, print2) {
    const ignoreRanges = [];
    let ignoreStart = null;
    const {
      children
    } = path.node;
    for (const [index, childNode] of children.entries()) {
      switch (isPrettierIgnore(childNode)) {
        case "start":
          if (ignoreStart === null) {
            ignoreStart = {
              index,
              offset: childNode.position.end.offset
            };
          }
          break;
        case "end":
          if (ignoreStart !== null) {
            ignoreRanges.push({
              start: ignoreStart,
              end: {
                index,
                offset: childNode.position.start.offset
              }
            });
            ignoreStart = null;
          }
          break;
        default:
          break;
      }
    }
    return printChildren(path, options2, print2, {
      processor({
        index
      }) {
        if (ignoreRanges.length > 0) {
          const ignoreRange = ignoreRanges[0];
          if (index === ignoreRange.start.index) {
            return [printIgnoreComment(children[ignoreRange.start.index]), options2.originalText.slice(ignoreRange.start.offset, ignoreRange.end.offset), printIgnoreComment(children[ignoreRange.end.index])];
          }
          if (ignoreRange.start.index < index && index < ignoreRange.end.index) {
            return false;
          }
          if (index === ignoreRange.end.index) {
            ignoreRanges.shift();
            return false;
          }
        }
        return print2();
      }
    });
  }
  function printChildren(path, options2, print2, events = {}) {
    const {
      postprocessor = (parts2) => parts2,
      processor = () => print2()
    } = events;
    const parts = [];
    path.each(() => {
      const result = processor(path);
      if (result !== false) {
        if (parts.length > 0 && shouldPrePrintHardline(path)) {
          parts.push(hardline);
          if (shouldPrePrintDoubleHardline(path, options2) || shouldPrePrintTripleHardline(path)) {
            parts.push(hardline);
          }
          if (shouldPrePrintTripleHardline(path)) {
            parts.push(hardline);
          }
        }
        parts.push(result);
      }
    }, "children");
    return postprocessor(parts);
  }
  function printIgnoreComment(node) {
    if (node.type === "html") {
      return node.value;
    }
    if (node.type === "paragraph" && Array.isArray(node.children) && node.children.length === 1 && node.children[0].type === "esComment") {
      return ["{/* ", node.children[0].value, " */}"];
    }
  }
  function isPrettierIgnore(node) {
    let match;
    if (node.type === "html") {
      match = node.value.match(/^<!--\s*prettier-ignore(?:-(start|end))?\s*-->$/);
    } else {
      let comment;
      if (node.type === "esComment") {
        comment = node;
      } else if (node.type === "paragraph" && node.children.length === 1 && node.children[0].type === "esComment") {
        comment = node.children[0];
      }
      if (comment) {
        match = comment.value.match(/^prettier-ignore(?:-(start|end))?$/);
      }
    }
    return match ? match[1] || "next" : false;
  }
  function shouldPrePrintHardline({
    node,
    parent
  }) {
    const isInlineNode = INLINE_NODE_TYPES.has(node.type);
    const isInlineHTML = node.type === "html" && INLINE_NODE_WRAPPER_TYPES.has(parent.type);
    return !isInlineNode && !isInlineHTML;
  }
  function isLooseListItem(node, options2) {
    return node.type === "listItem" && (node.spread || // Check if `listItem` ends with `\n`
    // since it can't be empty, so we only need check the last character
    options2.originalText.charAt(node.position.end.offset - 1) === "\n");
  }
  function shouldPrePrintDoubleHardline({
    node,
    previous,
    parent
  }, options2) {
    const isPrevNodeLooseListItem = isLooseListItem(previous, options2);
    if (isPrevNodeLooseListItem) {
      return true;
    }
    const isSequence = previous.type === node.type;
    const isSiblingNode = isSequence && SIBLING_NODE_TYPES.has(node.type);
    const isInTightListItem = parent.type === "listItem" && !isLooseListItem(parent, options2);
    const isPrevNodePrettierIgnore = isPrettierIgnore(previous) === "next";
    const isBlockHtmlWithoutBlankLineBetweenPrevHtml = node.type === "html" && previous.type === "html" && previous.position.end.line + 1 === node.position.start.line;
    const isHtmlDirectAfterListItem = node.type === "html" && parent.type === "listItem" && previous.type === "paragraph" && previous.position.end.line + 1 === node.position.start.line;
    return !(isSiblingNode || isInTightListItem || isPrevNodePrettierIgnore || isBlockHtmlWithoutBlankLineBetweenPrevHtml || isHtmlDirectAfterListItem);
  }
  function shouldPrePrintTripleHardline({
    node,
    previous
  }) {
    const isPrevNodeList = previous.type === "list";
    const isIndentedCode = node.type === "code" && node.isIndented;
    return isPrevNodeList && isIndentedCode;
  }
  function shouldRemainTheSameContent(path) {
    const node = path.findAncestor((node2) => node2.type === "linkReference" || node2.type === "imageReference");
    return node && (node.type !== "linkReference" || node.referenceType !== "full");
  }
  function printUrl(url, dangerousCharOrChars = []) {
    const dangerousChars = [" ", ...Array.isArray(dangerousCharOrChars) ? dangerousCharOrChars : [dangerousCharOrChars]];
    return new RegExp(dangerousChars.map((x) => `\\${x}`).join("|")).test(url) ? `<${url}>` : url;
  }
  function printTitle(title, options2, printSpace = true) {
    if (!title) {
      return "";
    }
    if (printSpace) {
      return " " + printTitle(title, options2, false);
    }
    title = string_replace_all_default(
      /* isOptionalObject*/
      false,
      title,
      /\\(?=["')])/g,
      ""
    );
    if (title.includes('"') && title.includes("'") && !title.includes(")")) {
      return `(${title})`;
    }
    const quote = get_preferred_quote_default(title, options2.singleQuote);
    title = string_replace_all_default(
      /* isOptionalObject*/
      false,
      title,
      "\\",
      "\\\\"
    );
    title = string_replace_all_default(
      /* isOptionalObject*/
      false,
      title,
      quote,
      `\\${quote}`
    );
    return `${quote}${title}${quote}`;
  }
  function clamp(value, min, max) {
    return value < min ? min : value > max ? max : value;
  }
  function hasPrettierIgnore(path) {
    return path.index > 0 && isPrettierIgnore(path.previous) === "next";
  }
  function printLinkReference(node) {
    return `[${(0, import_collapse_white_space2.default)(node.label)}]`;
  }
  function printFootnoteReference(node) {
    return `[^${node.label}]`;
  }
  var printer = {
    preprocess: print_preprocess_default,
    print: genericPrint,
    embed: embed_default,
    massageAstNode: clean_default,
    hasPrettierIgnore,
    insertPragma,
    getVisitorKeys: get_visitor_keys_default
  };
  var printer_markdown_default = printer;

  // src/language-markdown/parser-markdown.js
  var parser_markdown_exports = {};
  __export(parser_markdown_exports, {
    markdown: () => markdown,
    mdx: () => mdx,
    remark: () => markdown
  });
  var import_remark_parse = __toESM(require_remark_parse(), 1);
  var import_unified = __toESM(require_unified(), 1);
  var import_remark_math = __toESM(require_remark_math(), 1);
  var import_remark_footnotes = __toESM(require_remark_footnotes(), 1);

  // src/language-markdown/mdx.js
  var IMPORT_REGEX = /^import\s/;
  var EXPORT_REGEX = /^export\s/;
  var BLOCKS_REGEX = "[a-z][a-z0-9]*(\\.[a-z][a-z0-9]*)*|";
  var COMMENT_REGEX = /<!---->|<!---?[^>-](?:-?[^-])*-->/;
  var ES_COMMENT_REGEX = /^{\s*\/\*(.*)\*\/\s*}/;
  var EMPTY_NEWLINE = "\n\n";
  var isImport = (text) => IMPORT_REGEX.test(text);
  var isExport = (text) => EXPORT_REGEX.test(text);
  var tokenizeEsSyntax = (eat, value) => {
    const index = value.indexOf(EMPTY_NEWLINE);
    const subvalue = value.slice(0, index);
    if (isExport(subvalue) || isImport(subvalue)) {
      return eat(subvalue)({
        type: isExport(subvalue) ? "export" : "import",
        value: subvalue
      });
    }
  };
  var tokenizeEsComment = (eat, value) => {
    const match = ES_COMMENT_REGEX.exec(value);
    if (match) {
      return eat(match[0])({
        type: "esComment",
        value: match[1].trim()
      });
    }
  };
  tokenizeEsSyntax.locator = (value) => isExport(value) || isImport(value) ? -1 : 1;
  tokenizeEsComment.locator = (value, fromIndex) => value.indexOf("{", fromIndex);
  function esSyntax() {
    const { Parser } = this;
    const { blockTokenizers, blockMethods, inlineTokenizers, inlineMethods } = Parser.prototype;
    blockTokenizers.esSyntax = tokenizeEsSyntax;
    inlineTokenizers.esComment = tokenizeEsComment;
    blockMethods.splice(blockMethods.indexOf("paragraph"), 0, "esSyntax");
    inlineMethods.splice(inlineMethods.indexOf("text"), 0, "esComment");
  }

  // src/language-markdown/unified-plugins/html-to-jsx.js
  function htmlToJsx() {
    return (ast) => mapAst(ast, (node, _index, [parent]) => {
      if (node.type !== "html" || // Keep HTML-style comments (legacy MDX)
      COMMENT_REGEX.test(node.value) || INLINE_NODE_WRAPPER_TYPES.has(parent.type)) {
        return node;
      }
      return { ...node, type: "jsx" };
    });
  }
  var html_to_jsx_default = htmlToJsx;

  // src/language-markdown/unified-plugins/front-matter.js
  function frontMatter() {
    const proto = this.Parser.prototype;
    proto.blockMethods = ["frontMatter", ...proto.blockMethods];
    proto.blockTokenizers.frontMatter = tokenizer;
    function tokenizer(eat, value) {
      const parsed = parse_default(value);
      if (parsed.frontMatter) {
        return eat(parsed.frontMatter.raw)(parsed.frontMatter);
      }
    }
    tokenizer.onlyAtStart = true;
  }
  var front_matter_default = frontMatter;

  // src/language-markdown/unified-plugins/liquid.js
  function liquid() {
    const proto = this.Parser.prototype;
    const methods = proto.inlineMethods;
    methods.splice(methods.indexOf("text"), 0, "liquid");
    proto.inlineTokenizers.liquid = tokenizer;
    function tokenizer(eat, value) {
      const match = value.match(/^({%.*?%}|{{.*?}})/s);
      if (match) {
        return eat(match[0])({
          type: "liquidNode",
          value: match[0]
        });
      }
    }
    tokenizer.locator = function(value, fromIndex) {
      return value.indexOf("{", fromIndex);
    };
  }
  var liquid_default = liquid;

  // src/language-markdown/unified-plugins/wiki-link.js
  function wikiLink() {
    const entityType = "wikiLink";
    const wikiLinkRegex = /^\[\[(?<linkContents>.+?)]]/s;
    const proto = this.Parser.prototype;
    const methods = proto.inlineMethods;
    methods.splice(methods.indexOf("link"), 0, entityType);
    proto.inlineTokenizers.wikiLink = tokenizer;
    function tokenizer(eat, value) {
      const match = wikiLinkRegex.exec(value);
      if (match) {
        const linkContents = match.groups.linkContents.trim();
        return eat(match[0])({
          type: entityType,
          value: linkContents
        });
      }
    }
    tokenizer.locator = function(value, fromIndex) {
      return value.indexOf("[", fromIndex);
    };
  }
  var wiki_link_default = wikiLink;

  // src/language-markdown/parser-markdown.js
  function createParse({ isMDX }) {
    return (text) => {
      const processor = (0, import_unified.default)().use(import_remark_parse.default, {
        commonmark: true,
        ...isMDX && { blocks: [BLOCKS_REGEX] }
      }).use(import_remark_footnotes.default).use(front_matter_default).use(import_remark_math.default).use(isMDX ? esSyntax : identity).use(liquid_default).use(isMDX ? html_to_jsx_default : identity).use(wiki_link_default);
      return processor.run(processor.parse(text));
    };
  }
  function identity(x) {
    return x;
  }
  var baseParser = {
    astFormat: "mdast",
    hasPragma,
    locStart,
    locEnd
  };
  var markdown = { ...baseParser, parse: createParse({ isMDX: false }) };
  var mdx = { ...baseParser, parse: createParse({ isMDX: true }) };

  // src/language-markdown/languages.evaluate.js
  var languages_evaluate_default = [
    {
      "linguistLanguageId": 222,
      "name": "Markdown",
      "type": "prose",
      "color": "#083fa1",
      "aliases": [
        "pandoc"
      ],
      "aceMode": "markdown",
      "codemirrorMode": "gfm",
      "codemirrorMimeType": "text/x-gfm",
      "wrap": true,
      "extensions": [
        ".md",
        ".livemd",
        ".markdown",
        ".mdown",
        ".mdwn",
        ".mkd",
        ".mkdn",
        ".mkdown",
        ".ronn",
        ".scd",
        ".workbook"
      ],
      "filenames": [
        "contents.lr",
        "README"
      ],
      "tmScope": "source.gfm",
      "parsers": [
        "markdown"
      ],
      "vscodeLanguageIds": [
        "markdown"
      ]
    },
    {
      "linguistLanguageId": 222,
      "name": "MDX",
      "type": "prose",
      "color": "#083fa1",
      "aliases": [
        "pandoc"
      ],
      "aceMode": "markdown",
      "codemirrorMode": "gfm",
      "codemirrorMimeType": "text/x-gfm",
      "wrap": true,
      "extensions": [
        ".mdx"
      ],
      "filenames": [],
      "tmScope": "source.gfm",
      "parsers": [
        "mdx"
      ],
      "vscodeLanguageIds": [
        "mdx"
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

  // src/language-markdown/options.js
  var options = {
    proseWrap: common_options_evaluate_default.proseWrap,
    singleQuote: common_options_evaluate_default.singleQuote
  };
  var options_default = options;

  // src/language-markdown/index.js
  var printers = {
    mdast: printer_markdown_default
  };
  return __toCommonJS(markdown_exports);
});