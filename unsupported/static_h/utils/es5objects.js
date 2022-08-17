/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

var verbose = false;
var indirectEval = eval;
var global = indirectEval("this");

if (typeof print === "undefined")
    var print = console.log;

function check(objStr, propStr, type) {
    var x = objStr + "." + propStr;
    var obj = indirectEval(objStr);
    if (typeof obj === "undefined")
        print("Missing", objStr);
    else if (!obj.hasOwnProperty(propStr))
        print("Missing", x);
    else if (typeof obj[propStr] !== type)
        print("Wrong type of", x);
    else if (verbose)
        print("OK", x);
}

check("global", "NaN", "number");
check("global", "Infinity", "number");
check("global", "undefined", "undefined");
check("global", "eval", "function");
check("global", "parseInt", "function");
check("global", "parseFloat", "function");
check("global", "isNaN", "function");
check("global", "isFinite", "function");
check("global", "decodeURI", "function");
check("global", "decodeURIComponent", "function");
check("global", "encodeURI", "function");
check("global", "encodeURIComponent", "function");
check("global", "Object", "function");
check("global", "Function", "function");
check("global", "Array", "function");
check("global", "String", "function");
check("global", "Boolean", "function");
check("global", "Number", "function");
check("global", "Date", "function");
check("global", "RegExp", "function");
check("global", "Error", "function");
check("global", "EvalError", "function");
check("global", "TypeError", "function");
check("global", "URIError", "function");
check("global", "Math", "object");
check("global", "JSON", "object");
check("global", "escape", "function");
check("global", "unescape", "function");

// Object
check("global.Object", "getPrototypeOf", "function");
check("global.Object", "getOwnPropertyDescriptor", "function");
check("global.Object", "getOwnPropertyNames", "function");
check("global.Object", "create", "function");
check("global.Object", "defineProperty", "function");
check("global.Object", "defineProperties", "function");
check("global.Object", "seal", "function");
check("global.Object", "freeze", "function");
check("global.Object", "preventExtensions", "function");
check("global.Object", "isSealed", "function");
check("global.Object", "isFrozen", "function");
check("global.Object", "isExtensible", "function");
check("global.Object", "keys", "function");
check("global.Object", "prototype", "object");
check("global.Object.prototype", "constructor", "function");
check("global.Object.prototype", "toString", "function");
check("global.Object.prototype", "toLocaleString", "function");
check("global.Object.prototype", "valueOf", "function");
check("global.Object.prototype", "hasOwnProperty", "function");
check("global.Object.prototype", "isPrototypeOf", "function");
check("global.Object.prototype", "propertyIsEnumerable", "function");

// Function.
check("global", "Function", "function");
check("global.Function", "length", "number");
check("global.Function", "prototype", "function");
check("global.Function.prototype", "constructor", "function");
check("global.Function.prototype", "toString", "function");
check("global.Function.prototype", "apply", "function");
check("global.Function.prototype", "call", "function");
check("global.Function.prototype", "bind", "function");
global.testFunc = function() {}
check("global.testFunc", "length", "number");
check("global.testFunc", "prototype", "object");

// Array.
check("global", "Array", "function");
check("global.Array", "prototype", "object");
check("global.Array", "isArray", "function");
check("global.Array.prototype", "constructor", "function");
check("global.Array.prototype", "toString", "function");
check("global.Array.prototype", "toLocaleString", "function");
check("global.Array.prototype", "concat", "function");
check("global.Array.prototype", "join", "function");
check("global.Array.prototype", "pop", "function");
check("global.Array.prototype", "push", "function");
check("global.Array.prototype", "reverse", "function");
check("global.Array.prototype", "shift", "function");
check("global.Array.prototype", "slice", "function");
check("global.Array.prototype", "sort", "function");
check("global.Array.prototype", "splice", "function");
check("global.Array.prototype", "unshift", "function");
check("global.Array.prototype", "indexOf", "function");
check("global.Array.prototype", "lastIndexOf", "function");
check("global.Array.prototype", "every", "function");
check("global.Array.prototype", "some", "function");
check("global.Array.prototype", "forEach", "function");
check("global.Array.prototype", "map", "function");
check("global.Array.prototype", "filter", "function");
check("global.Array.prototype", "reduce", "function");
check("global.Array.prototype", "reduceRight", "function");
global.testArray = [];
check("global.testArray", "length", "number");

// String.
check("global", "String", "function");
check("global.String", "length", "number");
check("global.String", "prototype", "object");
check("global.String", "fromCharCode", "function");
check("global.String.prototype", "constructor", "function");
check("global.String.prototype", "toString", "function");
check("global.String.prototype", "valueOf", "function");
check("global.String.prototype", "charAt", "function");
check("global.String.prototype", "charCodeAt", "function");
check("global.String.prototype", "concat", "function");
check("global.String.prototype", "indexOf", "function");
check("global.String.prototype", "lastIndexOf", "function");
check("global.String.prototype", "localeCompare", "function");
check("global.String.prototype", "match", "function");
check("global.String.prototype", "replace", "function");
check("global.String.prototype", "search", "function");
check("global.String.prototype", "slice", "function");
check("global.String.prototype", "split", "function");
check("global.String.prototype", "substring", "function");
check("global.String.prototype", "toLowerCase", "function");
check("global.String.prototype", "toLocaleLowerCase", "function");
check("global.String.prototype", "toUpperCase", "function");
check("global.String.prototype", "toLocaleUpperCase", "function");
check("global.String.prototype", "trim", "function");
check("global.String.prototype", "substr", "function");

// Date.
check("global", "Date", "function");
check("global.Date", "length", "number");
check("global.Date", "prototype", "object");
check("global.Date", "parse", "function");
check("global.Date", "UTC", "function");
check("global.Date", "now", "function");
check("global.Date.prototype", "toString", "function");
check("global.Date.prototype", "toDateString", "function");
check("global.Date.prototype", "toTimeString", "function");
check("global.Date.prototype", "toLocaleString", "function");
check("global.Date.prototype", "toLocaleDateString", "function");
check("global.Date.prototype", "toLocaleTimeString", "function");
check("global.Date.prototype", "valueOf", "function");
check("global.Date.prototype", "getTime", "function");
check("global.Date.prototype", "getFullYear", "function");
check("global.Date.prototype", "getMonth", "function");
check("global.Date.prototype", "getDate", "function");
check("global.Date.prototype", "getDay", "function");
check("global.Date.prototype", "getHours", "function");
check("global.Date.prototype", "getMinutes", "function");
check("global.Date.prototype", "getSeconds", "function");
check("global.Date.prototype", "getMilliseconds", "function");
check("global.Date.prototype", "getUTCFullYear", "function");
check("global.Date.prototype", "getUTCMonth", "function");
check("global.Date.prototype", "getUTCDate", "function");
check("global.Date.prototype", "getUTCDay", "function");
check("global.Date.prototype", "getUTCHours", "function");
check("global.Date.prototype", "getUTCMinutes", "function");
check("global.Date.prototype", "getUTCSeconds", "function");
check("global.Date.prototype", "getUTCMilliseconds", "function");
check("global.Date.prototype", "getTimezoneOffset", "function");
check("global.Date.prototype", "setTime", "function");
check("global.Date.prototype", "setFullYear", "function");
check("global.Date.prototype", "setMonth", "function");
check("global.Date.prototype", "setDate", "function");
check("global.Date.prototype", "setHours", "function");
check("global.Date.prototype", "setMinutes", "function");
check("global.Date.prototype", "setSeconds", "function");
check("global.Date.prototype", "setMilliseconds", "function");
check("global.Date.prototype", "setUTCFullYear", "function");
check("global.Date.prototype", "setUTCMonth", "function");
check("global.Date.prototype", "setUTCDate", "function");
check("global.Date.prototype", "setUTCHours", "function");
check("global.Date.prototype", "setUTCMinutes", "function");
check("global.Date.prototype", "setUTCSeconds", "function");
check("global.Date.prototype", "setUTCMilliseconds", "function");
check("global.Date.prototype", "toUTCString", "function");
check("global.Date.prototype", "toISOString", "function");
check("global.Date.prototype", "toJSON", "function");
