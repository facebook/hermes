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

  // node_modules/vnopts/node_modules/tslib/tslib.es6.js
  var tslib_es6_exports = {};
  __export(tslib_es6_exports, {
    __assign: () => __assign,
    __asyncDelegator: () => __asyncDelegator,
    __asyncGenerator: () => __asyncGenerator,
    __asyncValues: () => __asyncValues,
    __await: () => __await,
    __awaiter: () => __awaiter,
    __classPrivateFieldGet: () => __classPrivateFieldGet,
    __classPrivateFieldSet: () => __classPrivateFieldSet,
    __createBinding: () => __createBinding,
    __decorate: () => __decorate,
    __exportStar: () => __exportStar,
    __extends: () => __extends,
    __generator: () => __generator,
    __importDefault: () => __importDefault,
    __importStar: () => __importStar,
    __makeTemplateObject: () => __makeTemplateObject,
    __metadata: () => __metadata,
    __param: () => __param,
    __read: () => __read,
    __rest: () => __rest,
    __spread: () => __spread,
    __spreadArrays: () => __spreadArrays,
    __values: () => __values
  });
  function __extends(d, b) {
    extendStatics(d, b);
    function __() {
      this.constructor = d;
    }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
  }
  function __rest(s, e) {
    var t = {};
    for (var p in s)
      if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
        t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
      for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
        if (e.indexOf(p[i]) < 0 && Object.prototype.propertyIsEnumerable.call(s, p[i]))
          t[p[i]] = s[p[i]];
      }
    return t;
  }
  function __decorate(decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function")
      r = Reflect.decorate(decorators, target, key, desc);
    else
      for (var i = decorators.length - 1; i >= 0; i--)
        if (d = decorators[i])
          r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
  }
  function __param(paramIndex, decorator) {
    return function(target, key) {
      decorator(target, key, paramIndex);
    };
  }
  function __metadata(metadataKey, metadataValue) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function")
      return Reflect.metadata(metadataKey, metadataValue);
  }
  function __awaiter(thisArg, _arguments, P, generator) {
    function adopt(value) {
      return value instanceof P ? value : new P(function(resolve) {
        resolve(value);
      });
    }
    return new (P || (P = Promise))(function(resolve, reject) {
      function fulfilled(value) {
        try {
          step(generator.next(value));
        } catch (e) {
          reject(e);
        }
      }
      function rejected(value) {
        try {
          step(generator["throw"](value));
        } catch (e) {
          reject(e);
        }
      }
      function step(result) {
        result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected);
      }
      step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
  }
  function __generator(thisArg, body) {
    var _ = { label: 0, sent: function() {
      if (t[0] & 1)
        throw t[1];
      return t[1];
    }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() {
      return this;
    }), g;
    function verb(n) {
      return function(v) {
        return step([n, v]);
      };
    }
    function step(op) {
      if (f)
        throw new TypeError("Generator is already executing.");
      while (_)
        try {
          if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done)
            return t;
          if (y = 0, t)
            op = [op[0] & 2, t.value];
          switch (op[0]) {
            case 0:
            case 1:
              t = op;
              break;
            case 4:
              _.label++;
              return { value: op[1], done: false };
            case 5:
              _.label++;
              y = op[1];
              op = [0];
              continue;
            case 7:
              op = _.ops.pop();
              _.trys.pop();
              continue;
            default:
              if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) {
                _ = 0;
                continue;
              }
              if (op[0] === 3 && (!t || op[1] > t[0] && op[1] < t[3])) {
                _.label = op[1];
                break;
              }
              if (op[0] === 6 && _.label < t[1]) {
                _.label = t[1];
                t = op;
                break;
              }
              if (t && _.label < t[2]) {
                _.label = t[2];
                _.ops.push(op);
                break;
              }
              if (t[2])
                _.ops.pop();
              _.trys.pop();
              continue;
          }
          op = body.call(thisArg, _);
        } catch (e) {
          op = [6, e];
          y = 0;
        } finally {
          f = t = 0;
        }
      if (op[0] & 5)
        throw op[1];
      return { value: op[0] ? op[1] : void 0, done: true };
    }
  }
  function __createBinding(o, m, k, k2) {
    if (k2 === void 0)
      k2 = k;
    o[k2] = m[k];
  }
  function __exportStar(m, exports) {
    for (var p in m)
      if (p !== "default" && !exports.hasOwnProperty(p))
        exports[p] = m[p];
  }
  function __values(o) {
    var s = typeof Symbol === "function" && Symbol.iterator, m = s && o[s], i = 0;
    if (m)
      return m.call(o);
    if (o && typeof o.length === "number")
      return {
        next: function() {
          if (o && i >= o.length)
            o = void 0;
          return { value: o && o[i++], done: !o };
        }
      };
    throw new TypeError(s ? "Object is not iterable." : "Symbol.iterator is not defined.");
  }
  function __read(o, n) {
    var m = typeof Symbol === "function" && o[Symbol.iterator];
    if (!m)
      return o;
    var i = m.call(o), r, ar = [], e;
    try {
      while ((n === void 0 || n-- > 0) && !(r = i.next()).done)
        ar.push(r.value);
    } catch (error) {
      e = { error };
    } finally {
      try {
        if (r && !r.done && (m = i["return"]))
          m.call(i);
      } finally {
        if (e)
          throw e.error;
      }
    }
    return ar;
  }
  function __spread() {
    for (var ar = [], i = 0; i < arguments.length; i++)
      ar = ar.concat(__read(arguments[i]));
    return ar;
  }
  function __spreadArrays() {
    for (var s = 0, i = 0, il = arguments.length; i < il; i++)
      s += arguments[i].length;
    for (var r = Array(s), k = 0, i = 0; i < il; i++)
      for (var a = arguments[i], j = 0, jl = a.length; j < jl; j++, k++)
        r[k] = a[j];
    return r;
  }
  function __await(v) {
    return this instanceof __await ? (this.v = v, this) : new __await(v);
  }
  function __asyncGenerator(thisArg, _arguments, generator) {
    if (!Symbol.asyncIterator)
      throw new TypeError("Symbol.asyncIterator is not defined.");
    var g = generator.apply(thisArg, _arguments || []), i, q = [];
    return i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function() {
      return this;
    }, i;
    function verb(n) {
      if (g[n])
        i[n] = function(v) {
          return new Promise(function(a, b) {
            q.push([n, v, a, b]) > 1 || resume(n, v);
          });
        };
    }
    function resume(n, v) {
      try {
        step(g[n](v));
      } catch (e) {
        settle(q[0][3], e);
      }
    }
    function step(r) {
      r.value instanceof __await ? Promise.resolve(r.value.v).then(fulfill, reject) : settle(q[0][2], r);
    }
    function fulfill(value) {
      resume("next", value);
    }
    function reject(value) {
      resume("throw", value);
    }
    function settle(f, v) {
      if (f(v), q.shift(), q.length)
        resume(q[0][0], q[0][1]);
    }
  }
  function __asyncDelegator(o) {
    var i, p;
    return i = {}, verb("next"), verb("throw", function(e) {
      throw e;
    }), verb("return"), i[Symbol.iterator] = function() {
      return this;
    }, i;
    function verb(n, f) {
      i[n] = o[n] ? function(v) {
        return (p = !p) ? { value: __await(o[n](v)), done: n === "return" } : f ? f(v) : v;
      } : f;
    }
  }
  function __asyncValues(o) {
    if (!Symbol.asyncIterator)
      throw new TypeError("Symbol.asyncIterator is not defined.");
    var m = o[Symbol.asyncIterator], i;
    return m ? m.call(o) : (o = typeof __values === "function" ? __values(o) : o[Symbol.iterator](), i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function() {
      return this;
    }, i);
    function verb(n) {
      i[n] = o[n] && function(v) {
        return new Promise(function(resolve, reject) {
          v = o[n](v), settle(resolve, reject, v.done, v.value);
        });
      };
    }
    function settle(resolve, reject, d, v) {
      Promise.resolve(v).then(function(v2) {
        resolve({ value: v2, done: d });
      }, reject);
    }
  }
  function __makeTemplateObject(cooked, raw) {
    if (Object.defineProperty) {
      Object.defineProperty(cooked, "raw", { value: raw });
    } else {
      cooked.raw = raw;
    }
    return cooked;
  }
  function __importStar(mod) {
    if (mod && mod.__esModule)
      return mod;
    var result = {};
    if (mod != null) {
      for (var k in mod)
        if (Object.hasOwnProperty.call(mod, k))
          result[k] = mod[k];
    }
    result.default = mod;
    return result;
  }
  function __importDefault(mod) {
    return mod && mod.__esModule ? mod : { default: mod };
  }
  function __classPrivateFieldGet(receiver, privateMap) {
    if (!privateMap.has(receiver)) {
      throw new TypeError("attempted to get private field on non-instance");
    }
    return privateMap.get(receiver);
  }
  function __classPrivateFieldSet(receiver, privateMap, value) {
    if (!privateMap.has(receiver)) {
      throw new TypeError("attempted to set private field on non-instance");
    }
    privateMap.set(receiver, value);
    return value;
  }
  var extendStatics, __assign;
  var init_tslib_es6 = __esm({
    "node_modules/vnopts/node_modules/tslib/tslib.es6.js"() {
      extendStatics = function(d, b) {
        extendStatics = Object.setPrototypeOf || { __proto__: [] } instanceof Array && function(d2, b2) {
          d2.__proto__ = b2;
        } || function(d2, b2) {
          for (var p in b2)
            if (b2.hasOwnProperty(p))
              d2[p] = b2[p];
        };
        return extendStatics(d, b);
      };
      __assign = function() {
        __assign = Object.assign || function __assign2(t) {
          for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s)
              if (Object.prototype.hasOwnProperty.call(s, p))
                t[p] = s[p];
          }
          return t;
        };
        return __assign.apply(this, arguments);
      };
    }
  });

  // node_modules/vnopts/lib/descriptors/api.js
  var require_api = __commonJS({
    "node_modules/vnopts/lib/descriptors/api.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      exports.apiDescriptor = {
        key: (key) => /^[$_a-zA-Z][$_a-zA-Z0-9]*$/.test(key) ? key : JSON.stringify(key),
        value(value) {
          if (value === null || typeof value !== "object") {
            return JSON.stringify(value);
          }
          if (Array.isArray(value)) {
            return `[${value.map((subValue) => exports.apiDescriptor.value(subValue)).join(", ")}]`;
          }
          const keys = Object.keys(value);
          return keys.length === 0 ? "{}" : `{ ${keys.map((key) => `${exports.apiDescriptor.key(key)}: ${exports.apiDescriptor.value(value[key])}`).join(", ")} }`;
        },
        pair: ({ key, value }) => exports.apiDescriptor.value({ [key]: value })
      };
    }
  });

  // node_modules/vnopts/lib/descriptors/index.js
  var require_descriptors = __commonJS({
    "node_modules/vnopts/lib/descriptors/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_api(), exports);
    }
  });

  // node_modules/vnopts/node_modules/escape-string-regexp/index.js
  var require_escape_string_regexp = __commonJS({
    "node_modules/vnopts/node_modules/escape-string-regexp/index.js"(exports, module) {
      "use strict";
      var matchOperatorsRe = /[|\\{}()[\]^$+*?.]/g;
      module.exports = function(str) {
        if (typeof str !== "string") {
          throw new TypeError("Expected a string");
        }
        return str.replace(matchOperatorsRe, "\\$&");
      };
    }
  });

  // node_modules/color-name/index.js
  var require_color_name = __commonJS({
    "node_modules/color-name/index.js"(exports, module) {
      "use strict";
      module.exports = {
        "aliceblue": [240, 248, 255],
        "antiquewhite": [250, 235, 215],
        "aqua": [0, 255, 255],
        "aquamarine": [127, 255, 212],
        "azure": [240, 255, 255],
        "beige": [245, 245, 220],
        "bisque": [255, 228, 196],
        "black": [0, 0, 0],
        "blanchedalmond": [255, 235, 205],
        "blue": [0, 0, 255],
        "blueviolet": [138, 43, 226],
        "brown": [165, 42, 42],
        "burlywood": [222, 184, 135],
        "cadetblue": [95, 158, 160],
        "chartreuse": [127, 255, 0],
        "chocolate": [210, 105, 30],
        "coral": [255, 127, 80],
        "cornflowerblue": [100, 149, 237],
        "cornsilk": [255, 248, 220],
        "crimson": [220, 20, 60],
        "cyan": [0, 255, 255],
        "darkblue": [0, 0, 139],
        "darkcyan": [0, 139, 139],
        "darkgoldenrod": [184, 134, 11],
        "darkgray": [169, 169, 169],
        "darkgreen": [0, 100, 0],
        "darkgrey": [169, 169, 169],
        "darkkhaki": [189, 183, 107],
        "darkmagenta": [139, 0, 139],
        "darkolivegreen": [85, 107, 47],
        "darkorange": [255, 140, 0],
        "darkorchid": [153, 50, 204],
        "darkred": [139, 0, 0],
        "darksalmon": [233, 150, 122],
        "darkseagreen": [143, 188, 143],
        "darkslateblue": [72, 61, 139],
        "darkslategray": [47, 79, 79],
        "darkslategrey": [47, 79, 79],
        "darkturquoise": [0, 206, 209],
        "darkviolet": [148, 0, 211],
        "deeppink": [255, 20, 147],
        "deepskyblue": [0, 191, 255],
        "dimgray": [105, 105, 105],
        "dimgrey": [105, 105, 105],
        "dodgerblue": [30, 144, 255],
        "firebrick": [178, 34, 34],
        "floralwhite": [255, 250, 240],
        "forestgreen": [34, 139, 34],
        "fuchsia": [255, 0, 255],
        "gainsboro": [220, 220, 220],
        "ghostwhite": [248, 248, 255],
        "gold": [255, 215, 0],
        "goldenrod": [218, 165, 32],
        "gray": [128, 128, 128],
        "green": [0, 128, 0],
        "greenyellow": [173, 255, 47],
        "grey": [128, 128, 128],
        "honeydew": [240, 255, 240],
        "hotpink": [255, 105, 180],
        "indianred": [205, 92, 92],
        "indigo": [75, 0, 130],
        "ivory": [255, 255, 240],
        "khaki": [240, 230, 140],
        "lavender": [230, 230, 250],
        "lavenderblush": [255, 240, 245],
        "lawngreen": [124, 252, 0],
        "lemonchiffon": [255, 250, 205],
        "lightblue": [173, 216, 230],
        "lightcoral": [240, 128, 128],
        "lightcyan": [224, 255, 255],
        "lightgoldenrodyellow": [250, 250, 210],
        "lightgray": [211, 211, 211],
        "lightgreen": [144, 238, 144],
        "lightgrey": [211, 211, 211],
        "lightpink": [255, 182, 193],
        "lightsalmon": [255, 160, 122],
        "lightseagreen": [32, 178, 170],
        "lightskyblue": [135, 206, 250],
        "lightslategray": [119, 136, 153],
        "lightslategrey": [119, 136, 153],
        "lightsteelblue": [176, 196, 222],
        "lightyellow": [255, 255, 224],
        "lime": [0, 255, 0],
        "limegreen": [50, 205, 50],
        "linen": [250, 240, 230],
        "magenta": [255, 0, 255],
        "maroon": [128, 0, 0],
        "mediumaquamarine": [102, 205, 170],
        "mediumblue": [0, 0, 205],
        "mediumorchid": [186, 85, 211],
        "mediumpurple": [147, 112, 219],
        "mediumseagreen": [60, 179, 113],
        "mediumslateblue": [123, 104, 238],
        "mediumspringgreen": [0, 250, 154],
        "mediumturquoise": [72, 209, 204],
        "mediumvioletred": [199, 21, 133],
        "midnightblue": [25, 25, 112],
        "mintcream": [245, 255, 250],
        "mistyrose": [255, 228, 225],
        "moccasin": [255, 228, 181],
        "navajowhite": [255, 222, 173],
        "navy": [0, 0, 128],
        "oldlace": [253, 245, 230],
        "olive": [128, 128, 0],
        "olivedrab": [107, 142, 35],
        "orange": [255, 165, 0],
        "orangered": [255, 69, 0],
        "orchid": [218, 112, 214],
        "palegoldenrod": [238, 232, 170],
        "palegreen": [152, 251, 152],
        "paleturquoise": [175, 238, 238],
        "palevioletred": [219, 112, 147],
        "papayawhip": [255, 239, 213],
        "peachpuff": [255, 218, 185],
        "peru": [205, 133, 63],
        "pink": [255, 192, 203],
        "plum": [221, 160, 221],
        "powderblue": [176, 224, 230],
        "purple": [128, 0, 128],
        "rebeccapurple": [102, 51, 153],
        "red": [255, 0, 0],
        "rosybrown": [188, 143, 143],
        "royalblue": [65, 105, 225],
        "saddlebrown": [139, 69, 19],
        "salmon": [250, 128, 114],
        "sandybrown": [244, 164, 96],
        "seagreen": [46, 139, 87],
        "seashell": [255, 245, 238],
        "sienna": [160, 82, 45],
        "silver": [192, 192, 192],
        "skyblue": [135, 206, 235],
        "slateblue": [106, 90, 205],
        "slategray": [112, 128, 144],
        "slategrey": [112, 128, 144],
        "snow": [255, 250, 250],
        "springgreen": [0, 255, 127],
        "steelblue": [70, 130, 180],
        "tan": [210, 180, 140],
        "teal": [0, 128, 128],
        "thistle": [216, 191, 216],
        "tomato": [255, 99, 71],
        "turquoise": [64, 224, 208],
        "violet": [238, 130, 238],
        "wheat": [245, 222, 179],
        "white": [255, 255, 255],
        "whitesmoke": [245, 245, 245],
        "yellow": [255, 255, 0],
        "yellowgreen": [154, 205, 50]
      };
    }
  });

  // node_modules/color-convert/conversions.js
  var require_conversions = __commonJS({
    "node_modules/color-convert/conversions.js"(exports, module) {
      var cssKeywords = require_color_name();
      var reverseKeywords = {};
      for (key in cssKeywords) {
        if (cssKeywords.hasOwnProperty(key)) {
          reverseKeywords[cssKeywords[key]] = key;
        }
      }
      var key;
      var convert = module.exports = {
        rgb: { channels: 3, labels: "rgb" },
        hsl: { channels: 3, labels: "hsl" },
        hsv: { channels: 3, labels: "hsv" },
        hwb: { channels: 3, labels: "hwb" },
        cmyk: { channels: 4, labels: "cmyk" },
        xyz: { channels: 3, labels: "xyz" },
        lab: { channels: 3, labels: "lab" },
        lch: { channels: 3, labels: "lch" },
        hex: { channels: 1, labels: ["hex"] },
        keyword: { channels: 1, labels: ["keyword"] },
        ansi16: { channels: 1, labels: ["ansi16"] },
        ansi256: { channels: 1, labels: ["ansi256"] },
        hcg: { channels: 3, labels: ["h", "c", "g"] },
        apple: { channels: 3, labels: ["r16", "g16", "b16"] },
        gray: { channels: 1, labels: ["gray"] }
      };
      for (model in convert) {
        if (convert.hasOwnProperty(model)) {
          if (!("channels" in convert[model])) {
            throw new Error("missing channels property: " + model);
          }
          if (!("labels" in convert[model])) {
            throw new Error("missing channel labels property: " + model);
          }
          if (convert[model].labels.length !== convert[model].channels) {
            throw new Error("channel and label counts mismatch: " + model);
          }
          channels = convert[model].channels;
          labels = convert[model].labels;
          delete convert[model].channels;
          delete convert[model].labels;
          Object.defineProperty(convert[model], "channels", { value: channels });
          Object.defineProperty(convert[model], "labels", { value: labels });
        }
      }
      var channels;
      var labels;
      var model;
      convert.rgb.hsl = function(rgb) {
        var r = rgb[0] / 255;
        var g = rgb[1] / 255;
        var b = rgb[2] / 255;
        var min = Math.min(r, g, b);
        var max = Math.max(r, g, b);
        var delta = max - min;
        var h;
        var s;
        var l;
        if (max === min) {
          h = 0;
        } else if (r === max) {
          h = (g - b) / delta;
        } else if (g === max) {
          h = 2 + (b - r) / delta;
        } else if (b === max) {
          h = 4 + (r - g) / delta;
        }
        h = Math.min(h * 60, 360);
        if (h < 0) {
          h += 360;
        }
        l = (min + max) / 2;
        if (max === min) {
          s = 0;
        } else if (l <= 0.5) {
          s = delta / (max + min);
        } else {
          s = delta / (2 - max - min);
        }
        return [h, s * 100, l * 100];
      };
      convert.rgb.hsv = function(rgb) {
        var rdif;
        var gdif;
        var bdif;
        var h;
        var s;
        var r = rgb[0] / 255;
        var g = rgb[1] / 255;
        var b = rgb[2] / 255;
        var v = Math.max(r, g, b);
        var diff = v - Math.min(r, g, b);
        var diffc = function(c) {
          return (v - c) / 6 / diff + 1 / 2;
        };
        if (diff === 0) {
          h = s = 0;
        } else {
          s = diff / v;
          rdif = diffc(r);
          gdif = diffc(g);
          bdif = diffc(b);
          if (r === v) {
            h = bdif - gdif;
          } else if (g === v) {
            h = 1 / 3 + rdif - bdif;
          } else if (b === v) {
            h = 2 / 3 + gdif - rdif;
          }
          if (h < 0) {
            h += 1;
          } else if (h > 1) {
            h -= 1;
          }
        }
        return [
          h * 360,
          s * 100,
          v * 100
        ];
      };
      convert.rgb.hwb = function(rgb) {
        var r = rgb[0];
        var g = rgb[1];
        var b = rgb[2];
        var h = convert.rgb.hsl(rgb)[0];
        var w = 1 / 255 * Math.min(r, Math.min(g, b));
        b = 1 - 1 / 255 * Math.max(r, Math.max(g, b));
        return [h, w * 100, b * 100];
      };
      convert.rgb.cmyk = function(rgb) {
        var r = rgb[0] / 255;
        var g = rgb[1] / 255;
        var b = rgb[2] / 255;
        var c;
        var m;
        var y;
        var k;
        k = Math.min(1 - r, 1 - g, 1 - b);
        c = (1 - r - k) / (1 - k) || 0;
        m = (1 - g - k) / (1 - k) || 0;
        y = (1 - b - k) / (1 - k) || 0;
        return [c * 100, m * 100, y * 100, k * 100];
      };
      function comparativeDistance(x, y) {
        return Math.pow(x[0] - y[0], 2) + Math.pow(x[1] - y[1], 2) + Math.pow(x[2] - y[2], 2);
      }
      convert.rgb.keyword = function(rgb) {
        var reversed = reverseKeywords[rgb];
        if (reversed) {
          return reversed;
        }
        var currentClosestDistance = Infinity;
        var currentClosestKeyword;
        for (var keyword in cssKeywords) {
          if (cssKeywords.hasOwnProperty(keyword)) {
            var value = cssKeywords[keyword];
            var distance = comparativeDistance(rgb, value);
            if (distance < currentClosestDistance) {
              currentClosestDistance = distance;
              currentClosestKeyword = keyword;
            }
          }
        }
        return currentClosestKeyword;
      };
      convert.keyword.rgb = function(keyword) {
        return cssKeywords[keyword];
      };
      convert.rgb.xyz = function(rgb) {
        var r = rgb[0] / 255;
        var g = rgb[1] / 255;
        var b = rgb[2] / 255;
        r = r > 0.04045 ? Math.pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
        g = g > 0.04045 ? Math.pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
        b = b > 0.04045 ? Math.pow((b + 0.055) / 1.055, 2.4) : b / 12.92;
        var x = r * 0.4124 + g * 0.3576 + b * 0.1805;
        var y = r * 0.2126 + g * 0.7152 + b * 0.0722;
        var z = r * 0.0193 + g * 0.1192 + b * 0.9505;
        return [x * 100, y * 100, z * 100];
      };
      convert.rgb.lab = function(rgb) {
        var xyz = convert.rgb.xyz(rgb);
        var x = xyz[0];
        var y = xyz[1];
        var z = xyz[2];
        var l;
        var a;
        var b;
        x /= 95.047;
        y /= 100;
        z /= 108.883;
        x = x > 8856e-6 ? Math.pow(x, 1 / 3) : 7.787 * x + 16 / 116;
        y = y > 8856e-6 ? Math.pow(y, 1 / 3) : 7.787 * y + 16 / 116;
        z = z > 8856e-6 ? Math.pow(z, 1 / 3) : 7.787 * z + 16 / 116;
        l = 116 * y - 16;
        a = 500 * (x - y);
        b = 200 * (y - z);
        return [l, a, b];
      };
      convert.hsl.rgb = function(hsl) {
        var h = hsl[0] / 360;
        var s = hsl[1] / 100;
        var l = hsl[2] / 100;
        var t1;
        var t2;
        var t3;
        var rgb;
        var val;
        if (s === 0) {
          val = l * 255;
          return [val, val, val];
        }
        if (l < 0.5) {
          t2 = l * (1 + s);
        } else {
          t2 = l + s - l * s;
        }
        t1 = 2 * l - t2;
        rgb = [0, 0, 0];
        for (var i = 0; i < 3; i++) {
          t3 = h + 1 / 3 * -(i - 1);
          if (t3 < 0) {
            t3++;
          }
          if (t3 > 1) {
            t3--;
          }
          if (6 * t3 < 1) {
            val = t1 + (t2 - t1) * 6 * t3;
          } else if (2 * t3 < 1) {
            val = t2;
          } else if (3 * t3 < 2) {
            val = t1 + (t2 - t1) * (2 / 3 - t3) * 6;
          } else {
            val = t1;
          }
          rgb[i] = val * 255;
        }
        return rgb;
      };
      convert.hsl.hsv = function(hsl) {
        var h = hsl[0];
        var s = hsl[1] / 100;
        var l = hsl[2] / 100;
        var smin = s;
        var lmin = Math.max(l, 0.01);
        var sv;
        var v;
        l *= 2;
        s *= l <= 1 ? l : 2 - l;
        smin *= lmin <= 1 ? lmin : 2 - lmin;
        v = (l + s) / 2;
        sv = l === 0 ? 2 * smin / (lmin + smin) : 2 * s / (l + s);
        return [h, sv * 100, v * 100];
      };
      convert.hsv.rgb = function(hsv) {
        var h = hsv[0] / 60;
        var s = hsv[1] / 100;
        var v = hsv[2] / 100;
        var hi = Math.floor(h) % 6;
        var f = h - Math.floor(h);
        var p = 255 * v * (1 - s);
        var q = 255 * v * (1 - s * f);
        var t = 255 * v * (1 - s * (1 - f));
        v *= 255;
        switch (hi) {
          case 0:
            return [v, t, p];
          case 1:
            return [q, v, p];
          case 2:
            return [p, v, t];
          case 3:
            return [p, q, v];
          case 4:
            return [t, p, v];
          case 5:
            return [v, p, q];
        }
      };
      convert.hsv.hsl = function(hsv) {
        var h = hsv[0];
        var s = hsv[1] / 100;
        var v = hsv[2] / 100;
        var vmin = Math.max(v, 0.01);
        var lmin;
        var sl;
        var l;
        l = (2 - s) * v;
        lmin = (2 - s) * vmin;
        sl = s * vmin;
        sl /= lmin <= 1 ? lmin : 2 - lmin;
        sl = sl || 0;
        l /= 2;
        return [h, sl * 100, l * 100];
      };
      convert.hwb.rgb = function(hwb) {
        var h = hwb[0] / 360;
        var wh = hwb[1] / 100;
        var bl = hwb[2] / 100;
        var ratio = wh + bl;
        var i;
        var v;
        var f;
        var n;
        if (ratio > 1) {
          wh /= ratio;
          bl /= ratio;
        }
        i = Math.floor(6 * h);
        v = 1 - bl;
        f = 6 * h - i;
        if ((i & 1) !== 0) {
          f = 1 - f;
        }
        n = wh + f * (v - wh);
        var r;
        var g;
        var b;
        switch (i) {
          default:
          case 6:
          case 0:
            r = v;
            g = n;
            b = wh;
            break;
          case 1:
            r = n;
            g = v;
            b = wh;
            break;
          case 2:
            r = wh;
            g = v;
            b = n;
            break;
          case 3:
            r = wh;
            g = n;
            b = v;
            break;
          case 4:
            r = n;
            g = wh;
            b = v;
            break;
          case 5:
            r = v;
            g = wh;
            b = n;
            break;
        }
        return [r * 255, g * 255, b * 255];
      };
      convert.cmyk.rgb = function(cmyk) {
        var c = cmyk[0] / 100;
        var m = cmyk[1] / 100;
        var y = cmyk[2] / 100;
        var k = cmyk[3] / 100;
        var r;
        var g;
        var b;
        r = 1 - Math.min(1, c * (1 - k) + k);
        g = 1 - Math.min(1, m * (1 - k) + k);
        b = 1 - Math.min(1, y * (1 - k) + k);
        return [r * 255, g * 255, b * 255];
      };
      convert.xyz.rgb = function(xyz) {
        var x = xyz[0] / 100;
        var y = xyz[1] / 100;
        var z = xyz[2] / 100;
        var r;
        var g;
        var b;
        r = x * 3.2406 + y * -1.5372 + z * -0.4986;
        g = x * -0.9689 + y * 1.8758 + z * 0.0415;
        b = x * 0.0557 + y * -0.204 + z * 1.057;
        r = r > 31308e-7 ? 1.055 * Math.pow(r, 1 / 2.4) - 0.055 : r * 12.92;
        g = g > 31308e-7 ? 1.055 * Math.pow(g, 1 / 2.4) - 0.055 : g * 12.92;
        b = b > 31308e-7 ? 1.055 * Math.pow(b, 1 / 2.4) - 0.055 : b * 12.92;
        r = Math.min(Math.max(0, r), 1);
        g = Math.min(Math.max(0, g), 1);
        b = Math.min(Math.max(0, b), 1);
        return [r * 255, g * 255, b * 255];
      };
      convert.xyz.lab = function(xyz) {
        var x = xyz[0];
        var y = xyz[1];
        var z = xyz[2];
        var l;
        var a;
        var b;
        x /= 95.047;
        y /= 100;
        z /= 108.883;
        x = x > 8856e-6 ? Math.pow(x, 1 / 3) : 7.787 * x + 16 / 116;
        y = y > 8856e-6 ? Math.pow(y, 1 / 3) : 7.787 * y + 16 / 116;
        z = z > 8856e-6 ? Math.pow(z, 1 / 3) : 7.787 * z + 16 / 116;
        l = 116 * y - 16;
        a = 500 * (x - y);
        b = 200 * (y - z);
        return [l, a, b];
      };
      convert.lab.xyz = function(lab) {
        var l = lab[0];
        var a = lab[1];
        var b = lab[2];
        var x;
        var y;
        var z;
        y = (l + 16) / 116;
        x = a / 500 + y;
        z = y - b / 200;
        var y2 = Math.pow(y, 3);
        var x2 = Math.pow(x, 3);
        var z2 = Math.pow(z, 3);
        y = y2 > 8856e-6 ? y2 : (y - 16 / 116) / 7.787;
        x = x2 > 8856e-6 ? x2 : (x - 16 / 116) / 7.787;
        z = z2 > 8856e-6 ? z2 : (z - 16 / 116) / 7.787;
        x *= 95.047;
        y *= 100;
        z *= 108.883;
        return [x, y, z];
      };
      convert.lab.lch = function(lab) {
        var l = lab[0];
        var a = lab[1];
        var b = lab[2];
        var hr;
        var h;
        var c;
        hr = Math.atan2(b, a);
        h = hr * 360 / 2 / Math.PI;
        if (h < 0) {
          h += 360;
        }
        c = Math.sqrt(a * a + b * b);
        return [l, c, h];
      };
      convert.lch.lab = function(lch) {
        var l = lch[0];
        var c = lch[1];
        var h = lch[2];
        var a;
        var b;
        var hr;
        hr = h / 360 * 2 * Math.PI;
        a = c * Math.cos(hr);
        b = c * Math.sin(hr);
        return [l, a, b];
      };
      convert.rgb.ansi16 = function(args) {
        var r = args[0];
        var g = args[1];
        var b = args[2];
        var value = 1 in arguments ? arguments[1] : convert.rgb.hsv(args)[2];
        value = Math.round(value / 50);
        if (value === 0) {
          return 30;
        }
        var ansi = 30 + (Math.round(b / 255) << 2 | Math.round(g / 255) << 1 | Math.round(r / 255));
        if (value === 2) {
          ansi += 60;
        }
        return ansi;
      };
      convert.hsv.ansi16 = function(args) {
        return convert.rgb.ansi16(convert.hsv.rgb(args), args[2]);
      };
      convert.rgb.ansi256 = function(args) {
        var r = args[0];
        var g = args[1];
        var b = args[2];
        if (r === g && g === b) {
          if (r < 8) {
            return 16;
          }
          if (r > 248) {
            return 231;
          }
          return Math.round((r - 8) / 247 * 24) + 232;
        }
        var ansi = 16 + 36 * Math.round(r / 255 * 5) + 6 * Math.round(g / 255 * 5) + Math.round(b / 255 * 5);
        return ansi;
      };
      convert.ansi16.rgb = function(args) {
        var color = args % 10;
        if (color === 0 || color === 7) {
          if (args > 50) {
            color += 3.5;
          }
          color = color / 10.5 * 255;
          return [color, color, color];
        }
        var mult = (~~(args > 50) + 1) * 0.5;
        var r = (color & 1) * mult * 255;
        var g = (color >> 1 & 1) * mult * 255;
        var b = (color >> 2 & 1) * mult * 255;
        return [r, g, b];
      };
      convert.ansi256.rgb = function(args) {
        if (args >= 232) {
          var c = (args - 232) * 10 + 8;
          return [c, c, c];
        }
        args -= 16;
        var rem;
        var r = Math.floor(args / 36) / 5 * 255;
        var g = Math.floor((rem = args % 36) / 6) / 5 * 255;
        var b = rem % 6 / 5 * 255;
        return [r, g, b];
      };
      convert.rgb.hex = function(args) {
        var integer = ((Math.round(args[0]) & 255) << 16) + ((Math.round(args[1]) & 255) << 8) + (Math.round(args[2]) & 255);
        var string = integer.toString(16).toUpperCase();
        return "000000".substring(string.length) + string;
      };
      convert.hex.rgb = function(args) {
        var match = args.toString(16).match(/[a-f0-9]{6}|[a-f0-9]{3}/i);
        if (!match) {
          return [0, 0, 0];
        }
        var colorString = match[0];
        if (match[0].length === 3) {
          colorString = colorString.split("").map(function(char) {
            return char + char;
          }).join("");
        }
        var integer = parseInt(colorString, 16);
        var r = integer >> 16 & 255;
        var g = integer >> 8 & 255;
        var b = integer & 255;
        return [r, g, b];
      };
      convert.rgb.hcg = function(rgb) {
        var r = rgb[0] / 255;
        var g = rgb[1] / 255;
        var b = rgb[2] / 255;
        var max = Math.max(Math.max(r, g), b);
        var min = Math.min(Math.min(r, g), b);
        var chroma = max - min;
        var grayscale;
        var hue;
        if (chroma < 1) {
          grayscale = min / (1 - chroma);
        } else {
          grayscale = 0;
        }
        if (chroma <= 0) {
          hue = 0;
        } else if (max === r) {
          hue = (g - b) / chroma % 6;
        } else if (max === g) {
          hue = 2 + (b - r) / chroma;
        } else {
          hue = 4 + (r - g) / chroma + 4;
        }
        hue /= 6;
        hue %= 1;
        return [hue * 360, chroma * 100, grayscale * 100];
      };
      convert.hsl.hcg = function(hsl) {
        var s = hsl[1] / 100;
        var l = hsl[2] / 100;
        var c = 1;
        var f = 0;
        if (l < 0.5) {
          c = 2 * s * l;
        } else {
          c = 2 * s * (1 - l);
        }
        if (c < 1) {
          f = (l - 0.5 * c) / (1 - c);
        }
        return [hsl[0], c * 100, f * 100];
      };
      convert.hsv.hcg = function(hsv) {
        var s = hsv[1] / 100;
        var v = hsv[2] / 100;
        var c = s * v;
        var f = 0;
        if (c < 1) {
          f = (v - c) / (1 - c);
        }
        return [hsv[0], c * 100, f * 100];
      };
      convert.hcg.rgb = function(hcg) {
        var h = hcg[0] / 360;
        var c = hcg[1] / 100;
        var g = hcg[2] / 100;
        if (c === 0) {
          return [g * 255, g * 255, g * 255];
        }
        var pure = [0, 0, 0];
        var hi = h % 1 * 6;
        var v = hi % 1;
        var w = 1 - v;
        var mg = 0;
        switch (Math.floor(hi)) {
          case 0:
            pure[0] = 1;
            pure[1] = v;
            pure[2] = 0;
            break;
          case 1:
            pure[0] = w;
            pure[1] = 1;
            pure[2] = 0;
            break;
          case 2:
            pure[0] = 0;
            pure[1] = 1;
            pure[2] = v;
            break;
          case 3:
            pure[0] = 0;
            pure[1] = w;
            pure[2] = 1;
            break;
          case 4:
            pure[0] = v;
            pure[1] = 0;
            pure[2] = 1;
            break;
          default:
            pure[0] = 1;
            pure[1] = 0;
            pure[2] = w;
        }
        mg = (1 - c) * g;
        return [
          (c * pure[0] + mg) * 255,
          (c * pure[1] + mg) * 255,
          (c * pure[2] + mg) * 255
        ];
      };
      convert.hcg.hsv = function(hcg) {
        var c = hcg[1] / 100;
        var g = hcg[2] / 100;
        var v = c + g * (1 - c);
        var f = 0;
        if (v > 0) {
          f = c / v;
        }
        return [hcg[0], f * 100, v * 100];
      };
      convert.hcg.hsl = function(hcg) {
        var c = hcg[1] / 100;
        var g = hcg[2] / 100;
        var l = g * (1 - c) + 0.5 * c;
        var s = 0;
        if (l > 0 && l < 0.5) {
          s = c / (2 * l);
        } else if (l >= 0.5 && l < 1) {
          s = c / (2 * (1 - l));
        }
        return [hcg[0], s * 100, l * 100];
      };
      convert.hcg.hwb = function(hcg) {
        var c = hcg[1] / 100;
        var g = hcg[2] / 100;
        var v = c + g * (1 - c);
        return [hcg[0], (v - c) * 100, (1 - v) * 100];
      };
      convert.hwb.hcg = function(hwb) {
        var w = hwb[1] / 100;
        var b = hwb[2] / 100;
        var v = 1 - b;
        var c = v - w;
        var g = 0;
        if (c < 1) {
          g = (v - c) / (1 - c);
        }
        return [hwb[0], c * 100, g * 100];
      };
      convert.apple.rgb = function(apple) {
        return [apple[0] / 65535 * 255, apple[1] / 65535 * 255, apple[2] / 65535 * 255];
      };
      convert.rgb.apple = function(rgb) {
        return [rgb[0] / 255 * 65535, rgb[1] / 255 * 65535, rgb[2] / 255 * 65535];
      };
      convert.gray.rgb = function(args) {
        return [args[0] / 100 * 255, args[0] / 100 * 255, args[0] / 100 * 255];
      };
      convert.gray.hsl = convert.gray.hsv = function(args) {
        return [0, 0, args[0]];
      };
      convert.gray.hwb = function(gray) {
        return [0, 100, gray[0]];
      };
      convert.gray.cmyk = function(gray) {
        return [0, 0, 0, gray[0]];
      };
      convert.gray.lab = function(gray) {
        return [gray[0], 0, 0];
      };
      convert.gray.hex = function(gray) {
        var val = Math.round(gray[0] / 100 * 255) & 255;
        var integer = (val << 16) + (val << 8) + val;
        var string = integer.toString(16).toUpperCase();
        return "000000".substring(string.length) + string;
      };
      convert.rgb.gray = function(rgb) {
        var val = (rgb[0] + rgb[1] + rgb[2]) / 3;
        return [val / 255 * 100];
      };
    }
  });

  // node_modules/color-convert/route.js
  var require_route = __commonJS({
    "node_modules/color-convert/route.js"(exports, module) {
      var conversions = require_conversions();
      function buildGraph() {
        var graph = {};
        var models = Object.keys(conversions);
        for (var len = models.length, i = 0; i < len; i++) {
          graph[models[i]] = {
            // http://jsperf.com/1-vs-infinity
            // micro-opt, but this is simple.
            distance: -1,
            parent: null
          };
        }
        return graph;
      }
      function deriveBFS(fromModel) {
        var graph = buildGraph();
        var queue = [fromModel];
        graph[fromModel].distance = 0;
        while (queue.length) {
          var current = queue.pop();
          var adjacents = Object.keys(conversions[current]);
          for (var len = adjacents.length, i = 0; i < len; i++) {
            var adjacent = adjacents[i];
            var node = graph[adjacent];
            if (node.distance === -1) {
              node.distance = graph[current].distance + 1;
              node.parent = current;
              queue.unshift(adjacent);
            }
          }
        }
        return graph;
      }
      function link(from, to) {
        return function(args) {
          return to(from(args));
        };
      }
      function wrapConversion(toModel, graph) {
        var path = [graph[toModel].parent, toModel];
        var fn = conversions[graph[toModel].parent][toModel];
        var cur = graph[toModel].parent;
        while (graph[cur].parent) {
          path.unshift(graph[cur].parent);
          fn = link(conversions[graph[cur].parent][cur], fn);
          cur = graph[cur].parent;
        }
        fn.conversion = path;
        return fn;
      }
      module.exports = function(fromModel) {
        var graph = deriveBFS(fromModel);
        var conversion = {};
        var models = Object.keys(graph);
        for (var len = models.length, i = 0; i < len; i++) {
          var toModel = models[i];
          var node = graph[toModel];
          if (node.parent === null) {
            continue;
          }
          conversion[toModel] = wrapConversion(toModel, graph);
        }
        return conversion;
      };
    }
  });

  // node_modules/color-convert/index.js
  var require_color_convert = __commonJS({
    "node_modules/color-convert/index.js"(exports, module) {
      var conversions = require_conversions();
      var route = require_route();
      var convert = {};
      var models = Object.keys(conversions);
      function wrapRaw(fn) {
        var wrappedFn = function(args) {
          if (args === void 0 || args === null) {
            return args;
          }
          if (arguments.length > 1) {
            args = Array.prototype.slice.call(arguments);
          }
          return fn(args);
        };
        if ("conversion" in fn) {
          wrappedFn.conversion = fn.conversion;
        }
        return wrappedFn;
      }
      function wrapRounded(fn) {
        var wrappedFn = function(args) {
          if (args === void 0 || args === null) {
            return args;
          }
          if (arguments.length > 1) {
            args = Array.prototype.slice.call(arguments);
          }
          var result = fn(args);
          if (typeof result === "object") {
            for (var len = result.length, i = 0; i < len; i++) {
              result[i] = Math.round(result[i]);
            }
          }
          return result;
        };
        if ("conversion" in fn) {
          wrappedFn.conversion = fn.conversion;
        }
        return wrappedFn;
      }
      models.forEach(function(fromModel) {
        convert[fromModel] = {};
        Object.defineProperty(convert[fromModel], "channels", { value: conversions[fromModel].channels });
        Object.defineProperty(convert[fromModel], "labels", { value: conversions[fromModel].labels });
        var routes = route(fromModel);
        var routeModels = Object.keys(routes);
        routeModels.forEach(function(toModel) {
          var fn = routes[toModel];
          convert[fromModel][toModel] = wrapRounded(fn);
          convert[fromModel][toModel].raw = wrapRaw(fn);
        });
      });
      module.exports = convert;
    }
  });

  // node_modules/ansi-styles/index.js
  var require_ansi_styles = __commonJS({
    "node_modules/ansi-styles/index.js"(exports, module) {
      "use strict";
      var colorConvert = require_color_convert();
      var wrapAnsi16 = (fn, offset) => function() {
        const code = fn.apply(colorConvert, arguments);
        return `\x1B[${code + offset}m`;
      };
      var wrapAnsi256 = (fn, offset) => function() {
        const code = fn.apply(colorConvert, arguments);
        return `\x1B[${38 + offset};5;${code}m`;
      };
      var wrapAnsi16m = (fn, offset) => function() {
        const rgb = fn.apply(colorConvert, arguments);
        return `\x1B[${38 + offset};2;${rgb[0]};${rgb[1]};${rgb[2]}m`;
      };
      function assembleStyles() {
        const codes = /* @__PURE__ */ new Map();
        const styles = {
          modifier: {
            reset: [0, 0],
            // 21 isn't widely supported and 22 does the same thing
            bold: [1, 22],
            dim: [2, 22],
            italic: [3, 23],
            underline: [4, 24],
            inverse: [7, 27],
            hidden: [8, 28],
            strikethrough: [9, 29]
          },
          color: {
            black: [30, 39],
            red: [31, 39],
            green: [32, 39],
            yellow: [33, 39],
            blue: [34, 39],
            magenta: [35, 39],
            cyan: [36, 39],
            white: [37, 39],
            gray: [90, 39],
            // Bright color
            redBright: [91, 39],
            greenBright: [92, 39],
            yellowBright: [93, 39],
            blueBright: [94, 39],
            magentaBright: [95, 39],
            cyanBright: [96, 39],
            whiteBright: [97, 39]
          },
          bgColor: {
            bgBlack: [40, 49],
            bgRed: [41, 49],
            bgGreen: [42, 49],
            bgYellow: [43, 49],
            bgBlue: [44, 49],
            bgMagenta: [45, 49],
            bgCyan: [46, 49],
            bgWhite: [47, 49],
            // Bright color
            bgBlackBright: [100, 49],
            bgRedBright: [101, 49],
            bgGreenBright: [102, 49],
            bgYellowBright: [103, 49],
            bgBlueBright: [104, 49],
            bgMagentaBright: [105, 49],
            bgCyanBright: [106, 49],
            bgWhiteBright: [107, 49]
          }
        };
        styles.color.grey = styles.color.gray;
        for (const groupName of Object.keys(styles)) {
          const group = styles[groupName];
          for (const styleName of Object.keys(group)) {
            const style = group[styleName];
            styles[styleName] = {
              open: `\x1B[${style[0]}m`,
              close: `\x1B[${style[1]}m`
            };
            group[styleName] = styles[styleName];
            codes.set(style[0], style[1]);
          }
          Object.defineProperty(styles, groupName, {
            value: group,
            enumerable: false
          });
          Object.defineProperty(styles, "codes", {
            value: codes,
            enumerable: false
          });
        }
        const ansi2ansi = (n) => n;
        const rgb2rgb = (r, g, b) => [r, g, b];
        styles.color.close = "\x1B[39m";
        styles.bgColor.close = "\x1B[49m";
        styles.color.ansi = {
          ansi: wrapAnsi16(ansi2ansi, 0)
        };
        styles.color.ansi256 = {
          ansi256: wrapAnsi256(ansi2ansi, 0)
        };
        styles.color.ansi16m = {
          rgb: wrapAnsi16m(rgb2rgb, 0)
        };
        styles.bgColor.ansi = {
          ansi: wrapAnsi16(ansi2ansi, 10)
        };
        styles.bgColor.ansi256 = {
          ansi256: wrapAnsi256(ansi2ansi, 10)
        };
        styles.bgColor.ansi16m = {
          rgb: wrapAnsi16m(rgb2rgb, 10)
        };
        for (let key of Object.keys(colorConvert)) {
          if (typeof colorConvert[key] !== "object") {
            continue;
          }
          const suite = colorConvert[key];
          if (key === "ansi16") {
            key = "ansi";
          }
          if ("ansi16" in suite) {
            styles.color.ansi[key] = wrapAnsi16(suite.ansi16, 0);
            styles.bgColor.ansi[key] = wrapAnsi16(suite.ansi16, 10);
          }
          if ("ansi256" in suite) {
            styles.color.ansi256[key] = wrapAnsi256(suite.ansi256, 0);
            styles.bgColor.ansi256[key] = wrapAnsi256(suite.ansi256, 10);
          }
          if ("rgb" in suite) {
            styles.color.ansi16m[key] = wrapAnsi16m(suite.rgb, 0);
            styles.bgColor.ansi16m[key] = wrapAnsi16m(suite.rgb, 10);
          }
        }
        return styles;
      }
      Object.defineProperty(module, "exports", {
        enumerable: true,
        get: assembleStyles
      });
    }
  });

  // node_modules/vnopts/node_modules/supports-color/browser.js
  var require_browser = __commonJS({
    "node_modules/vnopts/node_modules/supports-color/browser.js"(exports, module) {
      "use strict";
      module.exports = {
        stdout: false,
        stderr: false
      };
    }
  });

  // node_modules/vnopts/node_modules/chalk/templates.js
  var require_templates = __commonJS({
    "node_modules/vnopts/node_modules/chalk/templates.js"(exports, module) {
      "use strict";
      var TEMPLATE_REGEX = /(?:\\(u[a-f\d]{4}|x[a-f\d]{2}|.))|(?:\{(~)?(\w+(?:\([^)]*\))?(?:\.\w+(?:\([^)]*\))?)*)(?:[ \t]|(?=\r?\n)))|(\})|((?:.|[\r\n\f])+?)/gi;
      var STYLE_REGEX = /(?:^|\.)(\w+)(?:\(([^)]*)\))?/g;
      var STRING_REGEX = /^(['"])((?:\\.|(?!\1)[^\\])*)\1$/;
      var ESCAPE_REGEX = /\\(u[a-f\d]{4}|x[a-f\d]{2}|.)|([^\\])/gi;
      var ESCAPES = /* @__PURE__ */ new Map([
        ["n", "\n"],
        ["r", "\r"],
        ["t", "	"],
        ["b", "\b"],
        ["f", "\f"],
        ["v", "\v"],
        ["0", "\0"],
        ["\\", "\\"],
        ["e", "\x1B"],
        ["a", "\x07"]
      ]);
      function unescape(c) {
        if (c[0] === "u" && c.length === 5 || c[0] === "x" && c.length === 3) {
          return String.fromCharCode(parseInt(c.slice(1), 16));
        }
        return ESCAPES.get(c) || c;
      }
      function parseArguments(name, args) {
        const results = [];
        const chunks = args.trim().split(/\s*,\s*/g);
        let matches;
        for (const chunk of chunks) {
          if (!isNaN(chunk)) {
            results.push(Number(chunk));
          } else if (matches = chunk.match(STRING_REGEX)) {
            results.push(matches[2].replace(ESCAPE_REGEX, (m, escape, chr) => escape ? unescape(escape) : chr));
          } else {
            throw new Error(`Invalid Chalk template style argument: ${chunk} (in style '${name}')`);
          }
        }
        return results;
      }
      function parseStyle(style) {
        STYLE_REGEX.lastIndex = 0;
        const results = [];
        let matches;
        while ((matches = STYLE_REGEX.exec(style)) !== null) {
          const name = matches[1];
          if (matches[2]) {
            const args = parseArguments(name, matches[2]);
            results.push([name].concat(args));
          } else {
            results.push([name]);
          }
        }
        return results;
      }
      function buildStyle(chalk, styles) {
        const enabled = {};
        for (const layer of styles) {
          for (const style of layer.styles) {
            enabled[style[0]] = layer.inverse ? null : style.slice(1);
          }
        }
        let current = chalk;
        for (const styleName of Object.keys(enabled)) {
          if (Array.isArray(enabled[styleName])) {
            if (!(styleName in current)) {
              throw new Error(`Unknown Chalk style: ${styleName}`);
            }
            if (enabled[styleName].length > 0) {
              current = current[styleName].apply(current, enabled[styleName]);
            } else {
              current = current[styleName];
            }
          }
        }
        return current;
      }
      module.exports = (chalk, tmp) => {
        const styles = [];
        const chunks = [];
        let chunk = [];
        tmp.replace(TEMPLATE_REGEX, (m, escapeChar, inverse, style, close, chr) => {
          if (escapeChar) {
            chunk.push(unescape(escapeChar));
          } else if (style) {
            const str = chunk.join("");
            chunk = [];
            chunks.push(styles.length === 0 ? str : buildStyle(chalk, styles)(str));
            styles.push({ inverse, styles: parseStyle(style) });
          } else if (close) {
            if (styles.length === 0) {
              throw new Error("Found extraneous } in Chalk template literal");
            }
            chunks.push(buildStyle(chalk, styles)(chunk.join("")));
            chunk = [];
            styles.pop();
          } else {
            chunk.push(chr);
          }
        });
        chunks.push(chunk.join(""));
        if (styles.length > 0) {
          const errMsg = `Chalk template literal is missing ${styles.length} closing bracket${styles.length === 1 ? "" : "s"} (\`}\`)`;
          throw new Error(errMsg);
        }
        return chunks.join("");
      };
    }
  });

  // node_modules/vnopts/node_modules/chalk/index.js
  var require_chalk = __commonJS({
    "node_modules/vnopts/node_modules/chalk/index.js"(exports, module) {
      "use strict";
      var escapeStringRegexp = require_escape_string_regexp();
      var ansiStyles = require_ansi_styles();
      var stdoutColor = require_browser().stdout;
      var template = require_templates();
      var isSimpleWindowsTerm = process.platform === "win32" && !(process.env.TERM || "").toLowerCase().startsWith("xterm");
      var levelMapping = ["ansi", "ansi", "ansi256", "ansi16m"];
      var skipModels = /* @__PURE__ */ new Set(["gray"]);
      var styles = /* @__PURE__ */ Object.create(null);
      function applyOptions(obj, options) {
        options = options || {};
        const scLevel = stdoutColor ? stdoutColor.level : 0;
        obj.level = options.level === void 0 ? scLevel : options.level;
        obj.enabled = "enabled" in options ? options.enabled : obj.level > 0;
      }
      function Chalk(options) {
        if (!this || !(this instanceof Chalk) || this.template) {
          const chalk = {};
          applyOptions(chalk, options);
          chalk.template = function() {
            const args = [].slice.call(arguments);
            return chalkTag.apply(null, [chalk.template].concat(args));
          };
          Object.setPrototypeOf(chalk, Chalk.prototype);
          Object.setPrototypeOf(chalk.template, chalk);
          chalk.template.constructor = Chalk;
          return chalk.template;
        }
        applyOptions(this, options);
      }
      if (isSimpleWindowsTerm) {
        ansiStyles.blue.open = "\x1B[94m";
      }
      for (const key of Object.keys(ansiStyles)) {
        ansiStyles[key].closeRe = new RegExp(escapeStringRegexp(ansiStyles[key].close), "g");
        styles[key] = {
          get() {
            const codes = ansiStyles[key];
            return build.call(this, this._styles ? this._styles.concat(codes) : [codes], this._empty, key);
          }
        };
      }
      styles.visible = {
        get() {
          return build.call(this, this._styles || [], true, "visible");
        }
      };
      ansiStyles.color.closeRe = new RegExp(escapeStringRegexp(ansiStyles.color.close), "g");
      for (const model of Object.keys(ansiStyles.color.ansi)) {
        if (skipModels.has(model)) {
          continue;
        }
        styles[model] = {
          get() {
            const level = this.level;
            return function() {
              const open = ansiStyles.color[levelMapping[level]][model].apply(null, arguments);
              const codes = {
                open,
                close: ansiStyles.color.close,
                closeRe: ansiStyles.color.closeRe
              };
              return build.call(this, this._styles ? this._styles.concat(codes) : [codes], this._empty, model);
            };
          }
        };
      }
      ansiStyles.bgColor.closeRe = new RegExp(escapeStringRegexp(ansiStyles.bgColor.close), "g");
      for (const model of Object.keys(ansiStyles.bgColor.ansi)) {
        if (skipModels.has(model)) {
          continue;
        }
        const bgModel = "bg" + model[0].toUpperCase() + model.slice(1);
        styles[bgModel] = {
          get() {
            const level = this.level;
            return function() {
              const open = ansiStyles.bgColor[levelMapping[level]][model].apply(null, arguments);
              const codes = {
                open,
                close: ansiStyles.bgColor.close,
                closeRe: ansiStyles.bgColor.closeRe
              };
              return build.call(this, this._styles ? this._styles.concat(codes) : [codes], this._empty, model);
            };
          }
        };
      }
      var proto = Object.defineProperties(() => {
      }, styles);
      function build(_styles, _empty, key) {
        const builder = function() {
          return applyStyle.apply(builder, arguments);
        };
        builder._styles = _styles;
        builder._empty = _empty;
        const self = this;
        Object.defineProperty(builder, "level", {
          enumerable: true,
          get() {
            return self.level;
          },
          set(level) {
            self.level = level;
          }
        });
        Object.defineProperty(builder, "enabled", {
          enumerable: true,
          get() {
            return self.enabled;
          },
          set(enabled) {
            self.enabled = enabled;
          }
        });
        builder.hasGrey = this.hasGrey || key === "gray" || key === "grey";
        builder.__proto__ = proto;
        return builder;
      }
      function applyStyle() {
        const args = arguments;
        const argsLen = args.length;
        let str = String(arguments[0]);
        if (argsLen === 0) {
          return "";
        }
        if (argsLen > 1) {
          for (let a = 1; a < argsLen; a++) {
            str += " " + args[a];
          }
        }
        if (!this.enabled || this.level <= 0 || !str) {
          return this._empty ? "" : str;
        }
        const originalDim = ansiStyles.dim.open;
        if (isSimpleWindowsTerm && this.hasGrey) {
          ansiStyles.dim.open = "";
        }
        for (const code of this._styles.slice().reverse()) {
          str = code.open + str.replace(code.closeRe, code.open) + code.close;
          str = str.replace(/\r?\n/g, `${code.close}$&${code.open}`);
        }
        ansiStyles.dim.open = originalDim;
        return str;
      }
      function chalkTag(chalk, strings) {
        if (!Array.isArray(strings)) {
          return [].slice.call(arguments, 1).join(" ");
        }
        const args = [].slice.call(arguments, 2);
        const parts = [strings.raw[0]];
        for (let i = 1; i < strings.length; i++) {
          parts.push(String(args[i - 1]).replace(/[{}\\]/g, "\\$&"));
          parts.push(String(strings.raw[i]));
        }
        return template(chalk, parts.join(""));
      }
      Object.defineProperties(Chalk.prototype, styles);
      module.exports = Chalk();
      module.exports.supportsColor = stdoutColor;
      module.exports.default = module.exports;
    }
  });

  // node_modules/vnopts/lib/handlers/deprecated/common.js
  var require_common = __commonJS({
    "node_modules/vnopts/lib/handlers/deprecated/common.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var chalk_1 = require_chalk();
      exports.commonDeprecatedHandler = (keyOrPair, redirectTo, { descriptor }) => {
        const messages = [
          `${chalk_1.default.yellow(typeof keyOrPair === "string" ? descriptor.key(keyOrPair) : descriptor.pair(keyOrPair))} is deprecated`
        ];
        if (redirectTo) {
          messages.push(`we now treat it as ${chalk_1.default.blue(typeof redirectTo === "string" ? descriptor.key(redirectTo) : descriptor.pair(redirectTo))}`);
        }
        return messages.join("; ") + ".";
      };
    }
  });

  // node_modules/vnopts/lib/handlers/deprecated/index.js
  var require_deprecated = __commonJS({
    "node_modules/vnopts/lib/handlers/deprecated/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_common(), exports);
    }
  });

  // node_modules/vnopts/lib/handlers/invalid/common.js
  var require_common2 = __commonJS({
    "node_modules/vnopts/lib/handlers/invalid/common.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var chalk_1 = require_chalk();
      exports.commonInvalidHandler = (key, value, utils) => [
        `Invalid ${chalk_1.default.red(utils.descriptor.key(key))} value.`,
        `Expected ${chalk_1.default.blue(utils.schemas[key].expected(utils))},`,
        `but received ${chalk_1.default.red(utils.descriptor.value(value))}.`
      ].join(" ");
    }
  });

  // node_modules/vnopts/lib/handlers/invalid/index.js
  var require_invalid = __commonJS({
    "node_modules/vnopts/lib/handlers/invalid/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_common2(), exports);
    }
  });

  // node_modules/vnopts/node_modules/leven/index.js
  var require_leven = __commonJS({
    "node_modules/vnopts/node_modules/leven/index.js"(exports, module) {
      "use strict";
      var arr = [];
      var charCodeCache = [];
      module.exports = function(a, b) {
        if (a === b) {
          return 0;
        }
        var swap = a;
        if (a.length > b.length) {
          a = b;
          b = swap;
        }
        var aLen = a.length;
        var bLen = b.length;
        if (aLen === 0) {
          return bLen;
        }
        if (bLen === 0) {
          return aLen;
        }
        while (aLen > 0 && a.charCodeAt(~-aLen) === b.charCodeAt(~-bLen)) {
          aLen--;
          bLen--;
        }
        if (aLen === 0) {
          return bLen;
        }
        var start = 0;
        while (start < aLen && a.charCodeAt(start) === b.charCodeAt(start)) {
          start++;
        }
        aLen -= start;
        bLen -= start;
        if (aLen === 0) {
          return bLen;
        }
        var bCharCode;
        var ret;
        var tmp;
        var tmp2;
        var i = 0;
        var j = 0;
        while (i < aLen) {
          charCodeCache[start + i] = a.charCodeAt(start + i);
          arr[i] = ++i;
        }
        while (j < bLen) {
          bCharCode = b.charCodeAt(start + j);
          tmp = j++;
          ret = j;
          for (i = 0; i < aLen; i++) {
            tmp2 = bCharCode === charCodeCache[start + i] ? tmp : tmp + 1;
            tmp = arr[i];
            ret = arr[i] = tmp > ret ? tmp2 > ret ? ret + 1 : tmp2 : tmp2 > tmp ? tmp + 1 : tmp2;
          }
        }
        return ret;
      };
    }
  });

  // node_modules/vnopts/lib/handlers/unknown/leven.js
  var require_leven2 = __commonJS({
    "node_modules/vnopts/lib/handlers/unknown/leven.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var chalk_1 = require_chalk();
      var leven = require_leven();
      exports.levenUnknownHandler = (key, value, { descriptor, logger, schemas }) => {
        const messages = [
          `Ignored unknown option ${chalk_1.default.yellow(descriptor.pair({ key, value }))}.`
        ];
        const suggestion = Object.keys(schemas).sort().find((knownKey) => leven(key, knownKey) < 3);
        if (suggestion) {
          messages.push(`Did you mean ${chalk_1.default.blue(descriptor.key(suggestion))}?`);
        }
        logger.warn(messages.join(" "));
      };
    }
  });

  // node_modules/vnopts/lib/handlers/unknown/index.js
  var require_unknown = __commonJS({
    "node_modules/vnopts/lib/handlers/unknown/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_leven2(), exports);
    }
  });

  // node_modules/vnopts/lib/handlers/index.js
  var require_handlers = __commonJS({
    "node_modules/vnopts/lib/handlers/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_deprecated(), exports);
      tslib_1.__exportStar(require_invalid(), exports);
      tslib_1.__exportStar(require_unknown(), exports);
    }
  });

  // node_modules/vnopts/lib/schema.js
  var require_schema = __commonJS({
    "node_modules/vnopts/lib/schema.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
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
      exports.createSchema = createSchema;
      var Schema = class {
        constructor(parameters) {
          this.name = parameters.name;
        }
        static create(parameters) {
          return createSchema(this, parameters);
        }
        default(_utils) {
          return void 0;
        }
        // istanbul ignore next: this is actually an abstract method but we need a placeholder to get `function.length`
        expected(_utils) {
          return "nothing";
        }
        // istanbul ignore next: this is actually an abstract method but we need a placeholder to get `function.length`
        validate(_value, _utils) {
          return false;
        }
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
        postprocess(value, _utils) {
          return value;
        }
      };
      exports.Schema = Schema;
      function normalizeHandler(handler, superSchema, handlerArgumentsLength) {
        return typeof handler === "function" ? (...args) => handler(...args.slice(0, handlerArgumentsLength - 1), superSchema, ...args.slice(handlerArgumentsLength - 1)) : () => handler;
      }
    }
  });

  // node_modules/vnopts/lib/schemas/alias.js
  var require_alias = __commonJS({
    "node_modules/vnopts/lib/schemas/alias.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var schema_1 = require_schema();
      var AliasSchema = class extends schema_1.Schema {
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
      exports.AliasSchema = AliasSchema;
    }
  });

  // node_modules/vnopts/lib/schemas/any.js
  var require_any = __commonJS({
    "node_modules/vnopts/lib/schemas/any.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var schema_1 = require_schema();
      var AnySchema = class extends schema_1.Schema {
        expected() {
          return "anything";
        }
        validate() {
          return true;
        }
      };
      exports.AnySchema = AnySchema;
    }
  });

  // node_modules/vnopts/lib/schemas/array.js
  var require_array = __commonJS({
    "node_modules/vnopts/lib/schemas/array.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      var schema_1 = require_schema();
      var ArraySchema = class extends schema_1.Schema {
        constructor(_a) {
          var { valueSchema, name = valueSchema.name } = _a, handlers = tslib_1.__rest(_a, ["valueSchema", "name"]);
          super(Object.assign({}, handlers, { name }));
          this._valueSchema = valueSchema;
        }
        expected(utils) {
          return `an array of ${this._valueSchema.expected(utils)}`;
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
      exports.ArraySchema = ArraySchema;
      function wrapTransferResult({ from, to }) {
        return { from: [from], to };
      }
    }
  });

  // node_modules/vnopts/lib/schemas/boolean.js
  var require_boolean = __commonJS({
    "node_modules/vnopts/lib/schemas/boolean.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var schema_1 = require_schema();
      var BooleanSchema = class extends schema_1.Schema {
        expected() {
          return "true or false";
        }
        validate(value) {
          return typeof value === "boolean";
        }
      };
      exports.BooleanSchema = BooleanSchema;
    }
  });

  // node_modules/vnopts/lib/utils.js
  var require_utils = __commonJS({
    "node_modules/vnopts/lib/utils.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      function recordFromArray(array, mainKey) {
        const record = /* @__PURE__ */ Object.create(null);
        for (const value of array) {
          const key = value[mainKey];
          if (record[key]) {
            throw new Error(`Duplicate ${mainKey} ${JSON.stringify(key)}`);
          }
          record[key] = value;
        }
        return record;
      }
      exports.recordFromArray = recordFromArray;
      function mapFromArray(array, mainKey) {
        const map = /* @__PURE__ */ new Map();
        for (const value of array) {
          const key = value[mainKey];
          if (map.has(key)) {
            throw new Error(`Duplicate ${mainKey} ${JSON.stringify(key)}`);
          }
          map.set(key, value);
        }
        return map;
      }
      exports.mapFromArray = mapFromArray;
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
      exports.createAutoChecklist = createAutoChecklist;
      function partition(array, predicate) {
        const trueArray = [];
        const falseArray = [];
        for (const value of array) {
          if (predicate(value)) {
            trueArray.push(value);
          } else {
            falseArray.push(value);
          }
        }
        return [trueArray, falseArray];
      }
      exports.partition = partition;
      function isInt(value) {
        return value === Math.floor(value);
      }
      exports.isInt = isInt;
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
      exports.comparePrimitive = comparePrimitive;
      function normalizeDefaultResult(result) {
        return result === void 0 ? {} : result;
      }
      exports.normalizeDefaultResult = normalizeDefaultResult;
      function normalizeValidateResult(result, value) {
        return result === true ? true : result === false ? { value } : result;
      }
      exports.normalizeValidateResult = normalizeValidateResult;
      function normalizeDeprecatedResult(result, value, doNotNormalizeTrue = false) {
        return result === false ? false : result === true ? doNotNormalizeTrue ? true : [{ value }] : "value" in result ? [result] : result.length === 0 ? false : result;
      }
      exports.normalizeDeprecatedResult = normalizeDeprecatedResult;
      function normalizeTransferResult(result, value) {
        return typeof result === "string" || "key" in result ? { from: value, to: result } : "from" in result ? { from: result.from, to: result.to } : { from: value, to: result.to };
      }
      exports.normalizeTransferResult = normalizeTransferResult;
      function normalizeForwardResult(result, value) {
        return result === void 0 ? [] : Array.isArray(result) ? result.map((transferResult) => normalizeTransferResult(transferResult, value)) : [normalizeTransferResult(result, value)];
      }
      exports.normalizeForwardResult = normalizeForwardResult;
      function normalizeRedirectResult(result, value) {
        const redirect = normalizeForwardResult(typeof result === "object" && "redirect" in result ? result.redirect : result, value);
        return redirect.length === 0 ? { remain: value, redirect } : typeof result === "object" && "remain" in result ? { remain: result.remain, redirect } : { redirect };
      }
      exports.normalizeRedirectResult = normalizeRedirectResult;
    }
  });

  // node_modules/vnopts/lib/schemas/choice.js
  var require_choice = __commonJS({
    "node_modules/vnopts/lib/schemas/choice.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var schema_1 = require_schema();
      var utils_1 = require_utils();
      var ChoiceSchema = class extends schema_1.Schema {
        constructor(parameters) {
          super(parameters);
          this._choices = utils_1.mapFromArray(parameters.choices.map((choice) => choice && typeof choice === "object" ? choice : { value: choice }), "value");
        }
        expected({ descriptor }) {
          const choiceValues = Array.from(this._choices.keys()).map((value) => this._choices.get(value)).filter((choiceInfo) => !choiceInfo.deprecated).map((choiceInfo) => choiceInfo.value).sort(utils_1.comparePrimitive).map(descriptor.value);
          const head = choiceValues.slice(0, -2);
          const tail = choiceValues.slice(-2);
          return head.concat(tail.join(" or ")).join(", ");
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
      exports.ChoiceSchema = ChoiceSchema;
    }
  });

  // node_modules/vnopts/lib/schemas/number.js
  var require_number = __commonJS({
    "node_modules/vnopts/lib/schemas/number.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var schema_1 = require_schema();
      var NumberSchema = class extends schema_1.Schema {
        expected() {
          return "a number";
        }
        validate(value, _utils) {
          return typeof value === "number";
        }
      };
      exports.NumberSchema = NumberSchema;
    }
  });

  // node_modules/vnopts/lib/schemas/integer.js
  var require_integer = __commonJS({
    "node_modules/vnopts/lib/schemas/integer.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var utils_1 = require_utils();
      var number_1 = require_number();
      var IntegerSchema = class extends number_1.NumberSchema {
        expected() {
          return "an integer";
        }
        validate(value, utils) {
          return utils.normalizeValidateResult(super.validate(value, utils), value) === true && utils_1.isInt(value);
        }
      };
      exports.IntegerSchema = IntegerSchema;
    }
  });

  // node_modules/vnopts/lib/schemas/string.js
  var require_string = __commonJS({
    "node_modules/vnopts/lib/schemas/string.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var schema_1 = require_schema();
      var StringSchema = class extends schema_1.Schema {
        expected() {
          return "a string";
        }
        validate(value) {
          return typeof value === "string";
        }
      };
      exports.StringSchema = StringSchema;
    }
  });

  // node_modules/vnopts/lib/schemas/index.js
  var require_schemas = __commonJS({
    "node_modules/vnopts/lib/schemas/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_alias(), exports);
      tslib_1.__exportStar(require_any(), exports);
      tslib_1.__exportStar(require_array(), exports);
      tslib_1.__exportStar(require_boolean(), exports);
      tslib_1.__exportStar(require_choice(), exports);
      tslib_1.__exportStar(require_integer(), exports);
      tslib_1.__exportStar(require_number(), exports);
      tslib_1.__exportStar(require_string(), exports);
    }
  });

  // node_modules/vnopts/lib/defaults.js
  var require_defaults = __commonJS({
    "node_modules/vnopts/lib/defaults.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var api_1 = require_api();
      var common_1 = require_common();
      var invalid_1 = require_invalid();
      var leven_1 = require_leven2();
      exports.defaultDescriptor = api_1.apiDescriptor;
      exports.defaultUnknownHandler = leven_1.levenUnknownHandler;
      exports.defaultInvalidHandler = invalid_1.commonInvalidHandler;
      exports.defaultDeprecatedHandler = common_1.commonDeprecatedHandler;
    }
  });

  // node_modules/vnopts/lib/normalize.js
  var require_normalize = __commonJS({
    "node_modules/vnopts/lib/normalize.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var defaults_1 = require_defaults();
      var utils_1 = require_utils();
      exports.normalize = (options, schemas, opts) => new Normalizer(schemas, opts).normalize(options);
      var Normalizer = class {
        constructor(schemas, opts) {
          const { logger = console, descriptor = defaults_1.defaultDescriptor, unknown = defaults_1.defaultUnknownHandler, invalid = defaults_1.defaultInvalidHandler, deprecated = defaults_1.defaultDeprecatedHandler } = opts || {};
          this._utils = {
            descriptor,
            logger: (
              /* istanbul ignore next */
              logger || { warn: () => {
              } }
            ),
            schemas: utils_1.recordFromArray(schemas, "name"),
            normalizeDefaultResult: utils_1.normalizeDefaultResult,
            normalizeDeprecatedResult: utils_1.normalizeDeprecatedResult,
            normalizeForwardResult: utils_1.normalizeForwardResult,
            normalizeRedirectResult: utils_1.normalizeRedirectResult,
            normalizeValidateResult: utils_1.normalizeValidateResult
          };
          this._unknownHandler = unknown;
          this._invalidHandler = invalid;
          this._deprecatedHandler = deprecated;
          this.cleanHistory();
        }
        cleanHistory() {
          this._hasDeprecationWarned = utils_1.createAutoChecklist();
        }
        normalize(options) {
          const normalized = {};
          const restOptionsArray = [options];
          const applyNormalization = () => {
            while (restOptionsArray.length !== 0) {
              const currentOptions = restOptionsArray.shift();
              const transferredOptionsArray = this._applyNormalization(currentOptions, normalized);
              restOptionsArray.push(...transferredOptionsArray);
            }
          };
          applyNormalization();
          for (const key of Object.keys(this._utils.schemas)) {
            const schema = this._utils.schemas[key];
            if (!(key in normalized)) {
              const defaultResult = utils_1.normalizeDefaultResult(schema.default(this._utils));
              if ("value" in defaultResult) {
                restOptionsArray.push({ [key]: defaultResult.value });
              }
            }
          }
          applyNormalization();
          for (const key of Object.keys(this._utils.schemas)) {
            const schema = this._utils.schemas[key];
            if (key in normalized) {
              normalized[key] = schema.postprocess(normalized[key], this._utils);
            }
          }
          return normalized;
        }
        _applyNormalization(options, normalized) {
          const transferredOptionsArray = [];
          const [knownOptionNames, unknownOptionNames] = utils_1.partition(Object.keys(options), (key) => key in this._utils.schemas);
          for (const key of knownOptionNames) {
            const schema = this._utils.schemas[key];
            const value = schema.preprocess(options[key], this._utils);
            const validateResult = utils_1.normalizeValidateResult(schema.validate(value, this._utils), value);
            if (validateResult !== true) {
              const { value: invalidValue } = validateResult;
              const errorMessageOrError = this._invalidHandler(key, invalidValue, this._utils);
              throw typeof errorMessageOrError === "string" ? new Error(errorMessageOrError) : (
                /* istanbul ignore next*/
                errorMessageOrError
              );
            }
            const appendTransferredOptions = ({ from, to }) => {
              transferredOptionsArray.push(typeof to === "string" ? { [to]: from } : { [to.key]: to.value });
            };
            const warnDeprecated = ({ value: currentValue, redirectTo }) => {
              const deprecatedResult = utils_1.normalizeDeprecatedResult(
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
            const forwardResult = utils_1.normalizeForwardResult(schema.forward(value, this._utils), value);
            forwardResult.forEach(appendTransferredOptions);
            const redirectResult = utils_1.normalizeRedirectResult(schema.redirect(value, this._utils), value);
            redirectResult.redirect.forEach(appendTransferredOptions);
            if ("remain" in redirectResult) {
              const remainingValue = redirectResult.remain;
              normalized[key] = key in normalized ? schema.overlap(normalized[key], remainingValue, this._utils) : remainingValue;
              warnDeprecated({ value: remainingValue });
            }
            for (const { from, to } of redirectResult.redirect) {
              warnDeprecated({ value: from, redirectTo: to });
            }
          }
          for (const key of unknownOptionNames) {
            const value = options[key];
            const unknownResult = this._unknownHandler(key, value, this._utils);
            if (unknownResult) {
              for (const unknownKey of Object.keys(unknownResult)) {
                const unknownOption = { [unknownKey]: unknownResult[unknownKey] };
                if (unknownKey in this._utils.schemas) {
                  transferredOptionsArray.push(unknownOption);
                } else {
                  Object.assign(normalized, unknownOption);
                }
              }
            }
          }
          return transferredOptionsArray;
        }
      };
      exports.Normalizer = Normalizer;
    }
  });

  // node_modules/vnopts/lib/index.js
  var require_lib = __commonJS({
    "node_modules/vnopts/lib/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var tslib_1 = (init_tslib_es6(), __toCommonJS(tslib_es6_exports));
      tslib_1.__exportStar(require_descriptors(), exports);
      tslib_1.__exportStar(require_handlers(), exports);
      tslib_1.__exportStar(require_schemas(), exports);
      tslib_1.__exportStar(require_normalize(), exports);
      tslib_1.__exportStar(require_schema(), exports);
    }
  });

  // node_modules/js-tokens/index.js
  var require_js_tokens = __commonJS({
    "node_modules/js-tokens/index.js"(exports) {
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.default = /((['"])(?:(?!\2|\\).|\\(?:\r\n|[\s\S]))*(\2)?|`(?:[^`\\$]|\\[\s\S]|\$(?!\{)|\$\{(?:[^{}]|\{[^}]*\}?)*\}?)*(`)?)|(\/\/.*)|(\/\*(?:[^*]|\*(?!\/))*(\*\/)?)|(\/(?!\*)(?:\[(?:(?![\]\\]).|\\.)*\]|(?![\/\]\\]).|\\.)+\/(?:(?!\s*(?:\b|[\u0080-\uFFFF$\\'"~({]|[+\-!](?!=)|\.?\d))|[gmiyus]{1,6}\b(?![\u0080-\uFFFF$\\]|\s*(?:[+\-*%&|^<>!=?({]|\/(?![\/*])))))|(0[xX][\da-fA-F]+|0[oO][0-7]+|0[bB][01]+|(?:\d*\.\d+|\d+\.?)(?:[eE][+-]?\d+)?)|((?!\d)(?:(?!\s)[$\w\u0080-\uFFFF]|\\u[\da-fA-F]{4}|\\u\{[\da-fA-F]+\})+)|(--|\+\+|&&|\|\||=>|\.{3}|(?:[+\-\/%&|^]|\*{1,2}|<{1,2}|>{1,3}|!=?|={1,2})=?|[?~.,:;[\](){}])|(\s+)|(^$|[\s\S])/g;
      exports.matchToToken = function(match) {
        var token = { type: "invalid", value: match[0], closed: void 0 };
        if (match[1])
          token.type = "string", token.closed = !!(match[3] || match[4]);
        else if (match[5])
          token.type = "comment";
        else if (match[6])
          token.type = "comment", token.closed = !!match[7];
        else if (match[8])
          token.type = "regex";
        else if (match[9])
          token.type = "number";
        else if (match[10])
          token.type = "name";
        else if (match[11])
          token.type = "punctuator";
        else if (match[12])
          token.type = "whitespace";
        return token;
      };
    }
  });

  // node_modules/@babel/helper-validator-identifier/lib/identifier.js
  var require_identifier = __commonJS({
    "node_modules/@babel/helper-validator-identifier/lib/identifier.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.isIdentifierChar = isIdentifierChar;
      exports.isIdentifierName = isIdentifierName;
      exports.isIdentifierStart = isIdentifierStart;
      var nonASCIIidentifierStartChars = "\xAA\xB5\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0370-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386\u0388-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u048A-\u052F\u0531-\u0556\u0559\u0560-\u0588\u05D0-\u05EA\u05EF-\u05F2\u0620-\u064A\u066E\u066F\u0671-\u06D3\u06D5\u06E5\u06E6\u06EE\u06EF\u06FA-\u06FC\u06FF\u0710\u0712-\u072F\u074D-\u07A5\u07B1\u07CA-\u07EA\u07F4\u07F5\u07FA\u0800-\u0815\u081A\u0824\u0828\u0840-\u0858\u0860-\u086A\u0870-\u0887\u0889-\u088E\u08A0-\u08C9\u0904-\u0939\u093D\u0950\u0958-\u0961\u0971-\u0980\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BD\u09CE\u09DC\u09DD\u09DF-\u09E1\u09F0\u09F1\u09FC\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A59-\u0A5C\u0A5E\u0A72-\u0A74\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABD\u0AD0\u0AE0\u0AE1\u0AF9\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3D\u0B5C\u0B5D\u0B5F-\u0B61\u0B71\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BD0\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D\u0C58-\u0C5A\u0C5D\u0C60\u0C61\u0C80\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBD\u0CDD\u0CDE\u0CE0\u0CE1\u0CF1\u0CF2\u0D04-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D\u0D4E\u0D54-\u0D56\u0D5F-\u0D61\u0D7A-\u0D7F\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0E01-\u0E30\u0E32\u0E33\u0E40-\u0E46\u0E81\u0E82\u0E84\u0E86-\u0E8A\u0E8C-\u0EA3\u0EA5\u0EA7-\u0EB0\u0EB2\u0EB3\u0EBD\u0EC0-\u0EC4\u0EC6\u0EDC-\u0EDF\u0F00\u0F40-\u0F47\u0F49-\u0F6C\u0F88-\u0F8C\u1000-\u102A\u103F\u1050-\u1055\u105A-\u105D\u1061\u1065\u1066\u106E-\u1070\u1075-\u1081\u108E\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u1711\u171F-\u1731\u1740-\u1751\u1760-\u176C\u176E-\u1770\u1780-\u17B3\u17D7\u17DC\u1820-\u1878\u1880-\u18A8\u18AA\u18B0-\u18F5\u1900-\u191E\u1950-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u1A00-\u1A16\u1A20-\u1A54\u1AA7\u1B05-\u1B33\u1B45-\u1B4C\u1B83-\u1BA0\u1BAE\u1BAF\u1BBA-\u1BE5\u1C00-\u1C23\u1C4D-\u1C4F\u1C5A-\u1C7D\u1C80-\u1C88\u1C90-\u1CBA\u1CBD-\u1CBF\u1CE9-\u1CEC\u1CEE-\u1CF3\u1CF5\u1CF6\u1CFA\u1D00-\u1DBF\u1E00-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u2071\u207F\u2090-\u209C\u2102\u2107\u210A-\u2113\u2115\u2118-\u211D\u2124\u2126\u2128\u212A-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2CE4\u2CEB-\u2CEE\u2CF2\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D80-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u3005-\u3007\u3021-\u3029\u3031-\u3035\u3038-\u303C\u3041-\u3096\u309B-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312F\u3131-\u318E\u31A0-\u31BF\u31F0-\u31FF\u3400-\u4DBF\u4E00-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA61F\uA62A\uA62B\uA640-\uA66E\uA67F-\uA69D\uA6A0-\uA6EF\uA717-\uA71F\uA722-\uA788\uA78B-\uA7CA\uA7D0\uA7D1\uA7D3\uA7D5-\uA7D9\uA7F2-\uA801\uA803-\uA805\uA807-\uA80A\uA80C-\uA822\uA840-\uA873\uA882-\uA8B3\uA8F2-\uA8F7\uA8FB\uA8FD\uA8FE\uA90A-\uA925\uA930-\uA946\uA960-\uA97C\uA984-\uA9B2\uA9CF\uA9E0-\uA9E4\uA9E6-\uA9EF\uA9FA-\uA9FE\uAA00-\uAA28\uAA40-\uAA42\uAA44-\uAA4B\uAA60-\uAA76\uAA7A\uAA7E-\uAAAF\uAAB1\uAAB5\uAAB6\uAAB9-\uAABD\uAAC0\uAAC2\uAADB-\uAADD\uAAE0-\uAAEA\uAAF2-\uAAF4\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB69\uAB70-\uABE2\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D\uFB1F-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE70-\uFE74\uFE76-\uFEFC\uFF21-\uFF3A\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC";
      var nonASCIIidentifierChars = "\u200C\u200D\xB7\u0300-\u036F\u0387\u0483-\u0487\u0591-\u05BD\u05BF\u05C1\u05C2\u05C4\u05C5\u05C7\u0610-\u061A\u064B-\u0669\u0670\u06D6-\u06DC\u06DF-\u06E4\u06E7\u06E8\u06EA-\u06ED\u06F0-\u06F9\u0711\u0730-\u074A\u07A6-\u07B0\u07C0-\u07C9\u07EB-\u07F3\u07FD\u0816-\u0819\u081B-\u0823\u0825-\u0827\u0829-\u082D\u0859-\u085B\u0898-\u089F\u08CA-\u08E1\u08E3-\u0903\u093A-\u093C\u093E-\u094F\u0951-\u0957\u0962\u0963\u0966-\u096F\u0981-\u0983\u09BC\u09BE-\u09C4\u09C7\u09C8\u09CB-\u09CD\u09D7\u09E2\u09E3\u09E6-\u09EF\u09FE\u0A01-\u0A03\u0A3C\u0A3E-\u0A42\u0A47\u0A48\u0A4B-\u0A4D\u0A51\u0A66-\u0A71\u0A75\u0A81-\u0A83\u0ABC\u0ABE-\u0AC5\u0AC7-\u0AC9\u0ACB-\u0ACD\u0AE2\u0AE3\u0AE6-\u0AEF\u0AFA-\u0AFF\u0B01-\u0B03\u0B3C\u0B3E-\u0B44\u0B47\u0B48\u0B4B-\u0B4D\u0B55-\u0B57\u0B62\u0B63\u0B66-\u0B6F\u0B82\u0BBE-\u0BC2\u0BC6-\u0BC8\u0BCA-\u0BCD\u0BD7\u0BE6-\u0BEF\u0C00-\u0C04\u0C3C\u0C3E-\u0C44\u0C46-\u0C48\u0C4A-\u0C4D\u0C55\u0C56\u0C62\u0C63\u0C66-\u0C6F\u0C81-\u0C83\u0CBC\u0CBE-\u0CC4\u0CC6-\u0CC8\u0CCA-\u0CCD\u0CD5\u0CD6\u0CE2\u0CE3\u0CE6-\u0CEF\u0CF3\u0D00-\u0D03\u0D3B\u0D3C\u0D3E-\u0D44\u0D46-\u0D48\u0D4A-\u0D4D\u0D57\u0D62\u0D63\u0D66-\u0D6F\u0D81-\u0D83\u0DCA\u0DCF-\u0DD4\u0DD6\u0DD8-\u0DDF\u0DE6-\u0DEF\u0DF2\u0DF3\u0E31\u0E34-\u0E3A\u0E47-\u0E4E\u0E50-\u0E59\u0EB1\u0EB4-\u0EBC\u0EC8-\u0ECE\u0ED0-\u0ED9\u0F18\u0F19\u0F20-\u0F29\u0F35\u0F37\u0F39\u0F3E\u0F3F\u0F71-\u0F84\u0F86\u0F87\u0F8D-\u0F97\u0F99-\u0FBC\u0FC6\u102B-\u103E\u1040-\u1049\u1056-\u1059\u105E-\u1060\u1062-\u1064\u1067-\u106D\u1071-\u1074\u1082-\u108D\u108F-\u109D\u135D-\u135F\u1369-\u1371\u1712-\u1715\u1732-\u1734\u1752\u1753\u1772\u1773\u17B4-\u17D3\u17DD\u17E0-\u17E9\u180B-\u180D\u180F-\u1819\u18A9\u1920-\u192B\u1930-\u193B\u1946-\u194F\u19D0-\u19DA\u1A17-\u1A1B\u1A55-\u1A5E\u1A60-\u1A7C\u1A7F-\u1A89\u1A90-\u1A99\u1AB0-\u1ABD\u1ABF-\u1ACE\u1B00-\u1B04\u1B34-\u1B44\u1B50-\u1B59\u1B6B-\u1B73\u1B80-\u1B82\u1BA1-\u1BAD\u1BB0-\u1BB9\u1BE6-\u1BF3\u1C24-\u1C37\u1C40-\u1C49\u1C50-\u1C59\u1CD0-\u1CD2\u1CD4-\u1CE8\u1CED\u1CF4\u1CF7-\u1CF9\u1DC0-\u1DFF\u203F\u2040\u2054\u20D0-\u20DC\u20E1\u20E5-\u20F0\u2CEF-\u2CF1\u2D7F\u2DE0-\u2DFF\u302A-\u302F\u3099\u309A\uA620-\uA629\uA66F\uA674-\uA67D\uA69E\uA69F\uA6F0\uA6F1\uA802\uA806\uA80B\uA823-\uA827\uA82C\uA880\uA881\uA8B4-\uA8C5\uA8D0-\uA8D9\uA8E0-\uA8F1\uA8FF-\uA909\uA926-\uA92D\uA947-\uA953\uA980-\uA983\uA9B3-\uA9C0\uA9D0-\uA9D9\uA9E5\uA9F0-\uA9F9\uAA29-\uAA36\uAA43\uAA4C\uAA4D\uAA50-\uAA59\uAA7B-\uAA7D\uAAB0\uAAB2-\uAAB4\uAAB7\uAAB8\uAABE\uAABF\uAAC1\uAAEB-\uAAEF\uAAF5\uAAF6\uABE3-\uABEA\uABEC\uABED\uABF0-\uABF9\uFB1E\uFE00-\uFE0F\uFE20-\uFE2F\uFE33\uFE34\uFE4D-\uFE4F\uFF10-\uFF19\uFF3F";
      var nonASCIIidentifierStart = new RegExp("[" + nonASCIIidentifierStartChars + "]");
      var nonASCIIidentifier = new RegExp("[" + nonASCIIidentifierStartChars + nonASCIIidentifierChars + "]");
      nonASCIIidentifierStartChars = nonASCIIidentifierChars = null;
      var astralIdentifierStartCodes = [0, 11, 2, 25, 2, 18, 2, 1, 2, 14, 3, 13, 35, 122, 70, 52, 268, 28, 4, 48, 48, 31, 14, 29, 6, 37, 11, 29, 3, 35, 5, 7, 2, 4, 43, 157, 19, 35, 5, 35, 5, 39, 9, 51, 13, 10, 2, 14, 2, 6, 2, 1, 2, 10, 2, 14, 2, 6, 2, 1, 68, 310, 10, 21, 11, 7, 25, 5, 2, 41, 2, 8, 70, 5, 3, 0, 2, 43, 2, 1, 4, 0, 3, 22, 11, 22, 10, 30, 66, 18, 2, 1, 11, 21, 11, 25, 71, 55, 7, 1, 65, 0, 16, 3, 2, 2, 2, 28, 43, 28, 4, 28, 36, 7, 2, 27, 28, 53, 11, 21, 11, 18, 14, 17, 111, 72, 56, 50, 14, 50, 14, 35, 349, 41, 7, 1, 79, 28, 11, 0, 9, 21, 43, 17, 47, 20, 28, 22, 13, 52, 58, 1, 3, 0, 14, 44, 33, 24, 27, 35, 30, 0, 3, 0, 9, 34, 4, 0, 13, 47, 15, 3, 22, 0, 2, 0, 36, 17, 2, 24, 20, 1, 64, 6, 2, 0, 2, 3, 2, 14, 2, 9, 8, 46, 39, 7, 3, 1, 3, 21, 2, 6, 2, 1, 2, 4, 4, 0, 19, 0, 13, 4, 159, 52, 19, 3, 21, 2, 31, 47, 21, 1, 2, 0, 185, 46, 42, 3, 37, 47, 21, 0, 60, 42, 14, 0, 72, 26, 38, 6, 186, 43, 117, 63, 32, 7, 3, 0, 3, 7, 2, 1, 2, 23, 16, 0, 2, 0, 95, 7, 3, 38, 17, 0, 2, 0, 29, 0, 11, 39, 8, 0, 22, 0, 12, 45, 20, 0, 19, 72, 264, 8, 2, 36, 18, 0, 50, 29, 113, 6, 2, 1, 2, 37, 22, 0, 26, 5, 2, 1, 2, 31, 15, 0, 328, 18, 16, 0, 2, 12, 2, 33, 125, 0, 80, 921, 103, 110, 18, 195, 2637, 96, 16, 1071, 18, 5, 4026, 582, 8634, 568, 8, 30, 18, 78, 18, 29, 19, 47, 17, 3, 32, 20, 6, 18, 689, 63, 129, 74, 6, 0, 67, 12, 65, 1, 2, 0, 29, 6135, 9, 1237, 43, 8, 8936, 3, 2, 6, 2, 1, 2, 290, 16, 0, 30, 2, 3, 0, 15, 3, 9, 395, 2309, 106, 6, 12, 4, 8, 8, 9, 5991, 84, 2, 70, 2, 1, 3, 0, 3, 1, 3, 3, 2, 11, 2, 0, 2, 6, 2, 64, 2, 3, 3, 7, 2, 6, 2, 27, 2, 3, 2, 4, 2, 0, 4, 6, 2, 339, 3, 24, 2, 24, 2, 30, 2, 24, 2, 30, 2, 24, 2, 30, 2, 24, 2, 30, 2, 24, 2, 7, 1845, 30, 7, 5, 262, 61, 147, 44, 11, 6, 17, 0, 322, 29, 19, 43, 485, 27, 757, 6, 2, 3, 2, 1, 2, 14, 2, 196, 60, 67, 8, 0, 1205, 3, 2, 26, 2, 1, 2, 0, 3, 0, 2, 9, 2, 3, 2, 0, 2, 0, 7, 0, 5, 0, 2, 0, 2, 0, 2, 2, 2, 1, 2, 0, 3, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 1, 2, 0, 3, 3, 2, 6, 2, 3, 2, 3, 2, 0, 2, 9, 2, 16, 6, 2, 2, 4, 2, 16, 4421, 42719, 33, 4153, 7, 221, 3, 5761, 15, 7472, 3104, 541, 1507, 4938, 6, 4191];
      var astralIdentifierCodes = [509, 0, 227, 0, 150, 4, 294, 9, 1368, 2, 2, 1, 6, 3, 41, 2, 5, 0, 166, 1, 574, 3, 9, 9, 370, 1, 81, 2, 71, 10, 50, 3, 123, 2, 54, 14, 32, 10, 3, 1, 11, 3, 46, 10, 8, 0, 46, 9, 7, 2, 37, 13, 2, 9, 6, 1, 45, 0, 13, 2, 49, 13, 9, 3, 2, 11, 83, 11, 7, 0, 3, 0, 158, 11, 6, 9, 7, 3, 56, 1, 2, 6, 3, 1, 3, 2, 10, 0, 11, 1, 3, 6, 4, 4, 193, 17, 10, 9, 5, 0, 82, 19, 13, 9, 214, 6, 3, 8, 28, 1, 83, 16, 16, 9, 82, 12, 9, 9, 84, 14, 5, 9, 243, 14, 166, 9, 71, 5, 2, 1, 3, 3, 2, 0, 2, 1, 13, 9, 120, 6, 3, 6, 4, 0, 29, 9, 41, 6, 2, 3, 9, 0, 10, 10, 47, 15, 406, 7, 2, 7, 17, 9, 57, 21, 2, 13, 123, 5, 4, 0, 2, 1, 2, 6, 2, 0, 9, 9, 49, 4, 2, 1, 2, 4, 9, 9, 330, 3, 10, 1, 2, 0, 49, 6, 4, 4, 14, 9, 5351, 0, 7, 14, 13835, 9, 87, 9, 39, 4, 60, 6, 26, 9, 1014, 0, 2, 54, 8, 3, 82, 0, 12, 1, 19628, 1, 4706, 45, 3, 22, 543, 4, 4, 5, 9, 7, 3, 6, 31, 3, 149, 2, 1418, 49, 513, 54, 5, 49, 9, 0, 15, 0, 23, 4, 2, 14, 1361, 6, 2, 16, 3, 6, 2, 1, 2, 4, 101, 0, 161, 6, 10, 9, 357, 0, 62, 13, 499, 13, 983, 6, 110, 6, 6, 9, 4759, 9, 787719, 239];
      function isInAstralSet(code, set) {
        let pos = 65536;
        for (let i = 0, length = set.length; i < length; i += 2) {
          pos += set[i];
          if (pos > code)
            return false;
          pos += set[i + 1];
          if (pos >= code)
            return true;
        }
        return false;
      }
      function isIdentifierStart(code) {
        if (code < 65)
          return code === 36;
        if (code <= 90)
          return true;
        if (code < 97)
          return code === 95;
        if (code <= 122)
          return true;
        if (code <= 65535) {
          return code >= 170 && nonASCIIidentifierStart.test(String.fromCharCode(code));
        }
        return isInAstralSet(code, astralIdentifierStartCodes);
      }
      function isIdentifierChar(code) {
        if (code < 48)
          return code === 36;
        if (code < 58)
          return true;
        if (code < 65)
          return false;
        if (code <= 90)
          return true;
        if (code < 97)
          return code === 95;
        if (code <= 122)
          return true;
        if (code <= 65535) {
          return code >= 170 && nonASCIIidentifier.test(String.fromCharCode(code));
        }
        return isInAstralSet(code, astralIdentifierStartCodes) || isInAstralSet(code, astralIdentifierCodes);
      }
      function isIdentifierName(name) {
        let isFirst = true;
        for (let i = 0; i < name.length; i++) {
          let cp = name.charCodeAt(i);
          if ((cp & 64512) === 55296 && i + 1 < name.length) {
            const trail = name.charCodeAt(++i);
            if ((trail & 64512) === 56320) {
              cp = 65536 + ((cp & 1023) << 10) + (trail & 1023);
            }
          }
          if (isFirst) {
            isFirst = false;
            if (!isIdentifierStart(cp)) {
              return false;
            }
          } else if (!isIdentifierChar(cp)) {
            return false;
          }
        }
        return !isFirst;
      }
    }
  });

  // node_modules/@babel/helper-validator-identifier/lib/keyword.js
  var require_keyword = __commonJS({
    "node_modules/@babel/helper-validator-identifier/lib/keyword.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.isKeyword = isKeyword;
      exports.isReservedWord = isReservedWord;
      exports.isStrictBindOnlyReservedWord = isStrictBindOnlyReservedWord;
      exports.isStrictBindReservedWord = isStrictBindReservedWord;
      exports.isStrictReservedWord = isStrictReservedWord;
      var reservedWords = {
        keyword: ["break", "case", "catch", "continue", "debugger", "default", "do", "else", "finally", "for", "function", "if", "return", "switch", "throw", "try", "var", "const", "while", "with", "new", "this", "super", "class", "extends", "export", "import", "null", "true", "false", "in", "instanceof", "typeof", "void", "delete"],
        strict: ["implements", "interface", "let", "package", "private", "protected", "public", "static", "yield"],
        strictBind: ["eval", "arguments"]
      };
      var keywords = new Set(reservedWords.keyword);
      var reservedWordsStrictSet = new Set(reservedWords.strict);
      var reservedWordsStrictBindSet = new Set(reservedWords.strictBind);
      function isReservedWord(word, inModule) {
        return inModule && word === "await" || word === "enum";
      }
      function isStrictReservedWord(word, inModule) {
        return isReservedWord(word, inModule) || reservedWordsStrictSet.has(word);
      }
      function isStrictBindOnlyReservedWord(word) {
        return reservedWordsStrictBindSet.has(word);
      }
      function isStrictBindReservedWord(word, inModule) {
        return isStrictReservedWord(word, inModule) || isStrictBindOnlyReservedWord(word);
      }
      function isKeyword(word) {
        return keywords.has(word);
      }
    }
  });

  // node_modules/@babel/helper-validator-identifier/lib/index.js
  var require_lib2 = __commonJS({
    "node_modules/@babel/helper-validator-identifier/lib/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      Object.defineProperty(exports, "isIdentifierChar", {
        enumerable: true,
        get: function() {
          return _identifier.isIdentifierChar;
        }
      });
      Object.defineProperty(exports, "isIdentifierName", {
        enumerable: true,
        get: function() {
          return _identifier.isIdentifierName;
        }
      });
      Object.defineProperty(exports, "isIdentifierStart", {
        enumerable: true,
        get: function() {
          return _identifier.isIdentifierStart;
        }
      });
      Object.defineProperty(exports, "isKeyword", {
        enumerable: true,
        get: function() {
          return _keyword.isKeyword;
        }
      });
      Object.defineProperty(exports, "isReservedWord", {
        enumerable: true,
        get: function() {
          return _keyword.isReservedWord;
        }
      });
      Object.defineProperty(exports, "isStrictBindOnlyReservedWord", {
        enumerable: true,
        get: function() {
          return _keyword.isStrictBindOnlyReservedWord;
        }
      });
      Object.defineProperty(exports, "isStrictBindReservedWord", {
        enumerable: true,
        get: function() {
          return _keyword.isStrictBindReservedWord;
        }
      });
      Object.defineProperty(exports, "isStrictReservedWord", {
        enumerable: true,
        get: function() {
          return _keyword.isStrictReservedWord;
        }
      });
      var _identifier = require_identifier();
      var _keyword = require_keyword();
    }
  });

  // node_modules/@babel/highlight/node_modules/escape-string-regexp/index.js
  var require_escape_string_regexp2 = __commonJS({
    "node_modules/@babel/highlight/node_modules/escape-string-regexp/index.js"(exports, module) {
      "use strict";
      var matchOperatorsRe = /[|\\{}()[\]^$+*?.]/g;
      module.exports = function(str) {
        if (typeof str !== "string") {
          throw new TypeError("Expected a string");
        }
        return str.replace(matchOperatorsRe, "\\$&");
      };
    }
  });

  // node_modules/@babel/highlight/node_modules/supports-color/browser.js
  var require_browser2 = __commonJS({
    "node_modules/@babel/highlight/node_modules/supports-color/browser.js"(exports, module) {
      "use strict";
      module.exports = {
        stdout: false,
        stderr: false
      };
    }
  });

  // node_modules/@babel/highlight/node_modules/chalk/templates.js
  var require_templates2 = __commonJS({
    "node_modules/@babel/highlight/node_modules/chalk/templates.js"(exports, module) {
      "use strict";
      var TEMPLATE_REGEX = /(?:\\(u[a-f\d]{4}|x[a-f\d]{2}|.))|(?:\{(~)?(\w+(?:\([^)]*\))?(?:\.\w+(?:\([^)]*\))?)*)(?:[ \t]|(?=\r?\n)))|(\})|((?:.|[\r\n\f])+?)/gi;
      var STYLE_REGEX = /(?:^|\.)(\w+)(?:\(([^)]*)\))?/g;
      var STRING_REGEX = /^(['"])((?:\\.|(?!\1)[^\\])*)\1$/;
      var ESCAPE_REGEX = /\\(u[a-f\d]{4}|x[a-f\d]{2}|.)|([^\\])/gi;
      var ESCAPES = /* @__PURE__ */ new Map([
        ["n", "\n"],
        ["r", "\r"],
        ["t", "	"],
        ["b", "\b"],
        ["f", "\f"],
        ["v", "\v"],
        ["0", "\0"],
        ["\\", "\\"],
        ["e", "\x1B"],
        ["a", "\x07"]
      ]);
      function unescape(c) {
        if (c[0] === "u" && c.length === 5 || c[0] === "x" && c.length === 3) {
          return String.fromCharCode(parseInt(c.slice(1), 16));
        }
        return ESCAPES.get(c) || c;
      }
      function parseArguments(name, args) {
        const results = [];
        const chunks = args.trim().split(/\s*,\s*/g);
        let matches;
        for (const chunk of chunks) {
          if (!isNaN(chunk)) {
            results.push(Number(chunk));
          } else if (matches = chunk.match(STRING_REGEX)) {
            results.push(matches[2].replace(ESCAPE_REGEX, (m, escape, chr) => escape ? unescape(escape) : chr));
          } else {
            throw new Error(`Invalid Chalk template style argument: ${chunk} (in style '${name}')`);
          }
        }
        return results;
      }
      function parseStyle(style) {
        STYLE_REGEX.lastIndex = 0;
        const results = [];
        let matches;
        while ((matches = STYLE_REGEX.exec(style)) !== null) {
          const name = matches[1];
          if (matches[2]) {
            const args = parseArguments(name, matches[2]);
            results.push([name].concat(args));
          } else {
            results.push([name]);
          }
        }
        return results;
      }
      function buildStyle(chalk, styles) {
        const enabled = {};
        for (const layer of styles) {
          for (const style of layer.styles) {
            enabled[style[0]] = layer.inverse ? null : style.slice(1);
          }
        }
        let current = chalk;
        for (const styleName of Object.keys(enabled)) {
          if (Array.isArray(enabled[styleName])) {
            if (!(styleName in current)) {
              throw new Error(`Unknown Chalk style: ${styleName}`);
            }
            if (enabled[styleName].length > 0) {
              current = current[styleName].apply(current, enabled[styleName]);
            } else {
              current = current[styleName];
            }
          }
        }
        return current;
      }
      module.exports = (chalk, tmp) => {
        const styles = [];
        const chunks = [];
        let chunk = [];
        tmp.replace(TEMPLATE_REGEX, (m, escapeChar, inverse, style, close, chr) => {
          if (escapeChar) {
            chunk.push(unescape(escapeChar));
          } else if (style) {
            const str = chunk.join("");
            chunk = [];
            chunks.push(styles.length === 0 ? str : buildStyle(chalk, styles)(str));
            styles.push({ inverse, styles: parseStyle(style) });
          } else if (close) {
            if (styles.length === 0) {
              throw new Error("Found extraneous } in Chalk template literal");
            }
            chunks.push(buildStyle(chalk, styles)(chunk.join("")));
            chunk = [];
            styles.pop();
          } else {
            chunk.push(chr);
          }
        });
        chunks.push(chunk.join(""));
        if (styles.length > 0) {
          const errMsg = `Chalk template literal is missing ${styles.length} closing bracket${styles.length === 1 ? "" : "s"} (\`}\`)`;
          throw new Error(errMsg);
        }
        return chunks.join("");
      };
    }
  });

  // node_modules/@babel/highlight/node_modules/chalk/index.js
  var require_chalk2 = __commonJS({
    "node_modules/@babel/highlight/node_modules/chalk/index.js"(exports, module) {
      "use strict";
      var escapeStringRegexp = require_escape_string_regexp2();
      var ansiStyles = require_ansi_styles();
      var stdoutColor = require_browser2().stdout;
      var template = require_templates2();
      var isSimpleWindowsTerm = process.platform === "win32" && !(process.env.TERM || "").toLowerCase().startsWith("xterm");
      var levelMapping = ["ansi", "ansi", "ansi256", "ansi16m"];
      var skipModels = /* @__PURE__ */ new Set(["gray"]);
      var styles = /* @__PURE__ */ Object.create(null);
      function applyOptions(obj, options) {
        options = options || {};
        const scLevel = stdoutColor ? stdoutColor.level : 0;
        obj.level = options.level === void 0 ? scLevel : options.level;
        obj.enabled = "enabled" in options ? options.enabled : obj.level > 0;
      }
      function Chalk(options) {
        if (!this || !(this instanceof Chalk) || this.template) {
          const chalk = {};
          applyOptions(chalk, options);
          chalk.template = function() {
            const args = [].slice.call(arguments);
            return chalkTag.apply(null, [chalk.template].concat(args));
          };
          Object.setPrototypeOf(chalk, Chalk.prototype);
          Object.setPrototypeOf(chalk.template, chalk);
          chalk.template.constructor = Chalk;
          return chalk.template;
        }
        applyOptions(this, options);
      }
      if (isSimpleWindowsTerm) {
        ansiStyles.blue.open = "\x1B[94m";
      }
      for (const key of Object.keys(ansiStyles)) {
        ansiStyles[key].closeRe = new RegExp(escapeStringRegexp(ansiStyles[key].close), "g");
        styles[key] = {
          get() {
            const codes = ansiStyles[key];
            return build.call(this, this._styles ? this._styles.concat(codes) : [codes], this._empty, key);
          }
        };
      }
      styles.visible = {
        get() {
          return build.call(this, this._styles || [], true, "visible");
        }
      };
      ansiStyles.color.closeRe = new RegExp(escapeStringRegexp(ansiStyles.color.close), "g");
      for (const model of Object.keys(ansiStyles.color.ansi)) {
        if (skipModels.has(model)) {
          continue;
        }
        styles[model] = {
          get() {
            const level = this.level;
            return function() {
              const open = ansiStyles.color[levelMapping[level]][model].apply(null, arguments);
              const codes = {
                open,
                close: ansiStyles.color.close,
                closeRe: ansiStyles.color.closeRe
              };
              return build.call(this, this._styles ? this._styles.concat(codes) : [codes], this._empty, model);
            };
          }
        };
      }
      ansiStyles.bgColor.closeRe = new RegExp(escapeStringRegexp(ansiStyles.bgColor.close), "g");
      for (const model of Object.keys(ansiStyles.bgColor.ansi)) {
        if (skipModels.has(model)) {
          continue;
        }
        const bgModel = "bg" + model[0].toUpperCase() + model.slice(1);
        styles[bgModel] = {
          get() {
            const level = this.level;
            return function() {
              const open = ansiStyles.bgColor[levelMapping[level]][model].apply(null, arguments);
              const codes = {
                open,
                close: ansiStyles.bgColor.close,
                closeRe: ansiStyles.bgColor.closeRe
              };
              return build.call(this, this._styles ? this._styles.concat(codes) : [codes], this._empty, model);
            };
          }
        };
      }
      var proto = Object.defineProperties(() => {
      }, styles);
      function build(_styles, _empty, key) {
        const builder = function() {
          return applyStyle.apply(builder, arguments);
        };
        builder._styles = _styles;
        builder._empty = _empty;
        const self = this;
        Object.defineProperty(builder, "level", {
          enumerable: true,
          get() {
            return self.level;
          },
          set(level) {
            self.level = level;
          }
        });
        Object.defineProperty(builder, "enabled", {
          enumerable: true,
          get() {
            return self.enabled;
          },
          set(enabled) {
            self.enabled = enabled;
          }
        });
        builder.hasGrey = this.hasGrey || key === "gray" || key === "grey";
        builder.__proto__ = proto;
        return builder;
      }
      function applyStyle() {
        const args = arguments;
        const argsLen = args.length;
        let str = String(arguments[0]);
        if (argsLen === 0) {
          return "";
        }
        if (argsLen > 1) {
          for (let a = 1; a < argsLen; a++) {
            str += " " + args[a];
          }
        }
        if (!this.enabled || this.level <= 0 || !str) {
          return this._empty ? "" : str;
        }
        const originalDim = ansiStyles.dim.open;
        if (isSimpleWindowsTerm && this.hasGrey) {
          ansiStyles.dim.open = "";
        }
        for (const code of this._styles.slice().reverse()) {
          str = code.open + str.replace(code.closeRe, code.open) + code.close;
          str = str.replace(/\r?\n/g, `${code.close}$&${code.open}`);
        }
        ansiStyles.dim.open = originalDim;
        return str;
      }
      function chalkTag(chalk, strings) {
        if (!Array.isArray(strings)) {
          return [].slice.call(arguments, 1).join(" ");
        }
        const args = [].slice.call(arguments, 2);
        const parts = [strings.raw[0]];
        for (let i = 1; i < strings.length; i++) {
          parts.push(String(args[i - 1]).replace(/[{}\\]/g, "\\$&"));
          parts.push(String(strings.raw[i]));
        }
        return template(chalk, parts.join(""));
      }
      Object.defineProperties(Chalk.prototype, styles);
      module.exports = Chalk();
      module.exports.supportsColor = stdoutColor;
      module.exports.default = module.exports;
    }
  });

  // node_modules/@babel/highlight/lib/index.js
  var require_lib3 = __commonJS({
    "node_modules/@babel/highlight/lib/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.default = highlight;
      exports.getChalk = getChalk;
      exports.shouldHighlight = shouldHighlight;
      var _jsTokens = require_js_tokens();
      var _helperValidatorIdentifier = require_lib2();
      var _chalk = require_chalk2();
      var sometimesKeywords = /* @__PURE__ */ new Set(["as", "async", "from", "get", "of", "set"]);
      function getDefs(chalk) {
        return {
          keyword: chalk.cyan,
          capitalized: chalk.yellow,
          jsxIdentifier: chalk.yellow,
          punctuator: chalk.yellow,
          number: chalk.magenta,
          string: chalk.green,
          regex: chalk.magenta,
          comment: chalk.grey,
          invalid: chalk.white.bgRed.bold
        };
      }
      var NEWLINE = /\r\n|[\n\r\u2028\u2029]/;
      var BRACKET = /^[()[\]{}]$/;
      var tokenize;
      {
        const JSX_TAG = /^[a-z][\w-]*$/i;
        const getTokenType = function(token, offset, text) {
          if (token.type === "name") {
            if ((0, _helperValidatorIdentifier.isKeyword)(token.value) || (0, _helperValidatorIdentifier.isStrictReservedWord)(token.value, true) || sometimesKeywords.has(token.value)) {
              return "keyword";
            }
            if (JSX_TAG.test(token.value) && (text[offset - 1] === "<" || text.slice(offset - 2, offset) == "</")) {
              return "jsxIdentifier";
            }
            if (token.value[0] !== token.value[0].toLowerCase()) {
              return "capitalized";
            }
          }
          if (token.type === "punctuator" && BRACKET.test(token.value)) {
            return "bracket";
          }
          if (token.type === "invalid" && (token.value === "@" || token.value === "#")) {
            return "punctuator";
          }
          return token.type;
        };
        tokenize = function* (text) {
          let match;
          while (match = _jsTokens.default.exec(text)) {
            const token = _jsTokens.matchToToken(match);
            yield {
              type: getTokenType(token, match.index, text),
              value: token.value
            };
          }
        };
      }
      function highlightTokens(defs, text) {
        let highlighted = "";
        for (const {
          type,
          value
        } of tokenize(text)) {
          const colorize = defs[type];
          if (colorize) {
            highlighted += value.split(NEWLINE).map((str) => colorize(str)).join("\n");
          } else {
            highlighted += value;
          }
        }
        return highlighted;
      }
      function shouldHighlight(options) {
        return !!_chalk.supportsColor || options.forceColor;
      }
      function getChalk(options) {
        return options.forceColor ? new _chalk.constructor({
          enabled: true,
          level: 1
        }) : _chalk;
      }
      function highlight(code, options = {}) {
        if (code !== "" && shouldHighlight(options)) {
          const chalk = getChalk(options);
          const defs = getDefs(chalk);
          return highlightTokens(defs, code);
        } else {
          return code;
        }
      }
    }
  });

  // node_modules/@babel/code-frame/lib/index.js
  var require_lib4 = __commonJS({
    "node_modules/@babel/code-frame/lib/index.js"(exports) {
      "use strict";
      Object.defineProperty(exports, "__esModule", {
        value: true
      });
      exports.codeFrameColumns = codeFrameColumns2;
      exports.default = _default;
      var _highlight = require_lib3();
      var deprecationWarningShown = false;
      function getDefs(chalk) {
        return {
          gutter: chalk.grey,
          marker: chalk.red.bold,
          message: chalk.red.bold
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
        const chalk = (0, _highlight.getChalk)(opts);
        const defs = getDefs(chalk);
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
          return chalk.reset(frame);
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
  var assert = () => {
  };
  assert.ok = assert;
  assert.strictEqual = assert;
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
  };
  var UndefinedParserError = class extends Error {
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

  // src/main/normalize-options.js
  var import_vnopts = __toESM(require_lib(), 1);
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
      descriptor = import_vnopts.default.apiDescriptor;
    }
    const unknown = !passThrough ? (key, value, options2) => {
      const {
        _,
        ...schemas2
      } = options2.schemas;
      return import_vnopts.default.levenUnknownHandler(key, value, {
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
    const normalizer = new import_vnopts.default.Normalizer(schemas, {
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
      schemas.push(import_vnopts.default.AnySchema.create({
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
        schemas.push(import_vnopts.default.AliasSchema.create({
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
        SchemaConstructor = import_vnopts.default.IntegerSchema;
        if (isCLI) {
          parameters.preprocess = Number;
        }
        break;
      case "string":
        SchemaConstructor = import_vnopts.default.StringSchema;
        break;
      case "choice":
        SchemaConstructor = import_vnopts.default.ChoiceSchema;
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
        SchemaConstructor = import_vnopts.default.BooleanSchema;
        break;
      case "flag":
        SchemaConstructor = FlagSchema;
        parameters.flags = optionInfos.flatMap((optionInfo2) => [optionInfo2.alias, optionInfo2.description && optionInfo2.name, optionInfo2.oppositeDescription && `no-${optionInfo2.name}`].filter(Boolean));
        break;
      case "path":
        SchemaConstructor = import_vnopts.default.StringSchema;
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
    return optionInfo.array ? import_vnopts.default.ArraySchema.create({
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
  var import_code_frame = __toESM(require_lib4(), 1);
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
    if (printer.printComment && (!printer.willPrintOwnComments || !printer.willPrintOwnComments(path, options))) {
      doc = printComments(path, doc, options);
    }
    if (node === options.cursorNode) {
      doc = inheritLabel(doc, (doc2) => [cursor, doc2, cursor]);
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