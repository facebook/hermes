/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// The RegExp library skips the JS-level call of RegExp.prototype.exec when
// exec is the builtin. Every observable effect of the call must remain: user
// replacements of exec, the exec lookups, lastIndex coercion and writes,
// the legacy statics, and the native exec frame in stack traces.

/// \return a printable form of \p v: strings quoted with non-ASCII escaped,
/// arrays bracketed, undefined and null by name.
function show(v) {
  if (v === undefined) return "undefined";
  if (v === null) return "null";
  if (typeof v === "string") {
    var out = '"';
    for (var i = 0; i < v.length; ++i) {
      var c = v.charCodeAt(i);
      out += c < 0x20 || c > 0x7e
          ? "\\u" + c.toString(16).padStart(4, "0") : v[i];
    }
    return out + '"';
  }
  if (Array.isArray(v)) return "[" + v.map(show).join(",") + "]";
  if (typeof v === "object") {
    return "{" + Object.keys(v).map(function (k) {
      return k + ":" + show(v[k]);
    }).join(",") + "}";
  }
  return String(v);
}

/// \return whether the stack trace \p stack has a frame for the native exec.
function hasExecFrame(stack) {
  return stack.indexOf("at exec (native)") >= 0;
}

/// Run \p body with RegExp.prototype.exec replaced by \p exec, restoring
/// the builtin afterwards.
function withProtoExec(exec, body) {
  var saved = RegExp.prototype.exec;
  RegExp.prototype.exec = exec;
  try {
    body();
  } finally {
    RegExp.prototype.exec = saved;
  }
}

function replacedExec(builtinExec) {
  print("replaced exec");
  var log = [];
  function counting(s) {
    log.push(this.source + "@" + this.lastIndex + ":" + s);
    return builtinExec.call(this, s);
  }
  withProtoExec(counting, function () {
    log = [];
    print(show("abab".match(/ab/g)), show(log));
    log = [];
    print(show("abab".replace(/ab/g, "x")), show(log));
    log = [];
    print(show("abab".replace(/ab/g, function (m) { return m.toUpperCase(); })),
          show(log));
    log = [];
    print("abab".search(/b/), show(log));
    log = [];
    print(show("abab".match(/b/)), show(log));
  });
}
// CHECK-LABEL: replaced exec
// CHECK-NEXT: ["ab","ab"] ["ab@0:abab","ab@2:abab","ab@4:abab"]
// CHECK-NEXT: "xx" ["ab@0:abab","ab@2:abab","ab@4:abab"]
// CHECK-NEXT: "ABAB" ["ab@0:abab","ab@2:abab","ab@4:abab"]
// CHECK-NEXT: 1 ["b@0:abab"]
// CHECK-NEXT: ["b"] ["b@0:abab"]

function ownAndSubclassExec(builtinExec) {
  print("own and subclass exec");
  var n = 0;
  var re = /a/g;
  re.exec = function (s) { ++n; return builtinExec.call(this, s); };
  print(show("aa".match(re)), show("aa".replace(re, "b")), "aa".search(re), n);
  class Sub extends RegExp {
    exec(s) { ++n; return super.exec(s); }
  }
  n = 0;
  var sub = new Sub("a", "g");
  print(show("aa".match(sub)), show("aa".replace(sub, "b")), "aa".search(sub),
        n);
}
// CHECK-LABEL: own and subclass exec
// CHECK-NEXT: ["a","a"] "bb" 0 7
// CHECK-NEXT: ["a","a"] "bb" 0 7

function execAccessor(builtinExec) {
  print("exec accessor");
  var gets = 0;
  Object.defineProperty(RegExp.prototype, "exec", {
    get: function () { ++gets; return builtinExec; },
    configurable: true,
  });
  try {
    print(show("abab".match(/ab/g)), gets);
    gets = 0;
    print(show("abab".replace(/ab/g, "x")), gets);
    gets = 0;
    print("abab".search(/b/), gets);
  } finally {
    Object.defineProperty(RegExp.prototype, "exec", {
      value: builtinExec,
      writable: true,
      configurable: true,
    });
  }
}
// CHECK-LABEL: exec accessor
// CHECK-NEXT: ["ab","ab"] 3
// CHECK-NEXT: "xx" 3
// CHECK-NEXT: 1 1

function proxyPrototype() {
  print("proxy prototype");
  var log = [];
  var proxy = new Proxy(RegExp.prototype, {
    get: function (target, key, receiver) {
      if (key === "exec") log.push("exec");
      return Reflect.get(target, key, receiver);
    },
  });
  var re = /ab/g;
  Object.setPrototypeOf(re, proxy);
  print(show(RegExp.prototype[Symbol.match].call(re, "abab")), log.length);
  log = [];
  print(show(RegExp.prototype[Symbol.replace].call(re, "abab", "x")),
        log.length);
  log = [];
  print(RegExp.prototype[Symbol.search].call(re, "abab"), log.length);
}
// CHECK-LABEL: proxy prototype
// CHECK-NEXT: ["ab","ab"] 3
// CHECK-NEXT: "xx" 3
// CHECK-NEXT: 0 1

function frozen() {
  print("frozen");
  var re = Object.freeze(/a/g);
  try {
    "aa".match(re);
  } catch (e) {
    print(e.name, hasExecFrame(e.stack));
  }
  try {
    "aa".replace(re, "b");
  } catch (e) {
    print(e.name, hasExecFrame(e.stack));
  }
}
// CHECK-LABEL: frozen
// CHECK-NEXT: TypeError false
// CHECK-NEXT: TypeError false

function lastIndexValueOf() {
  print("lastIndex valueOf");
  var n = 0;
  var re = /a/;
  re.lastIndex = { valueOf: function () { ++n; return 0; } };
  re.exec("a");
  re.test("a");
  "a".match(re);
  "a".replace(re, "b");
  "a".search(re);
  print(n, typeof re.lastIndex);
}
// CHECK-LABEL: lastIndex valueOf
// CHECK-NEXT: 4 object

function statics() {
  print("statics");
  function dump() {
    return [RegExp.$1, RegExp.lastMatch, RegExp.leftContext,
            RegExp.rightContext].join("|");
  }
  /(z)/.exec("z");
  /(\d)(\d)/.test("x12y");
  print(dump(), RegExp.$2);
  "a1b2c".match(/(\d)/g);
  print(dump());
  "xx3y".search(/(\d)/);
  print(dump());
  "p4q5r".replace(/(\d)/g, "-");
  print(dump());
  var seen = [];
  "p6q7r".replace(/(\d)/g, function (m) { seen.push(RegExp.$1); return m; });
  print(seen.join(","), dump());
}
// CHECK-LABEL: statics
// CHECK-NEXT: 1|12|x|y 2
// CHECK-NEXT: 2|2|a1b|c
// CHECK-NEXT: 3|3|xx|y
// CHECK-NEXT: 5|5|p4q|r
// CHECK-NEXT: 7,7 7|7|p6q|r

function emptyMatches() {
  print("empty matches");
  // Under /u the loop steps over the pair. The \uDE00 alternative matches
  // only a lone low surrogate, which a search started inside the pair sees:
  // so a loop that stepped by one code unit would match the pair (index
  // moved back by GetStringIndex) instead of looping on /(?:)/gu forever.
  var s = "\uD83D\uDE00x";
  print(show(s.match(/\uDE00|(?:)/gu)), show(s.match(/(?:)/g)));
  print(show(s.replace(/\uDE00|(?:)/gu, "-")), show(s.replace(/(?:)/g, "-")));
}
// CHECK-LABEL: empty matches
// CHECK-NEXT: ["","",""] ["","","",""]
// CHECK-NEXT: "-\ud83d\ude00-x-" "-\ud83d-\ude00-x-"

function stickyGlobal() {
  print("sticky global");
  print(show("aab".match(/a/gy)), show("baa".match(/a/gy)));
  print(show("aaba".replace(/a/gy, "x")), show("baa".replace(/a/gy, "x")));
}
// CHECK-LABEL: sticky global
// CHECK-NEXT: ["a","a"] null
// CHECK-NEXT: "xxba" "baa"

function replacements() {
  print("replacements");
  print(show("John Smith".replace(/(?<first>\w+)\s(?<last>\w+)/,
                                  "$<last>, $<first> [$1|$2|$&|$`|$']")));
  print(show("x-ab-y".replace(/(a)(z)?(b)/, "[$1|$2|$3|$&|$`|$']")));
  print(show("ab".replace(/(a)|(b)/g, "[$1|$2]")));
  "x-ab-y".replace(/(a)(z)?(b)/, function () {
    print(show(Array.prototype.slice.call(arguments)));
    return "";
  });
  "x-ab-y".replace(/(?<p>a)(?<q>z)?b/g, function () {
    print(show(Array.prototype.slice.call(arguments)));
    return "";
  });
}
// CHECK-LABEL: replacements
// CHECK-NEXT: "Smith, John [John|Smith|John Smith||]"
// CHECK-NEXT: "x-[a||b|ab|x-|-y]-y"
// CHECK-NEXT: "[a|][|b]"
// CHECK-NEXT: ["ab","a",undefined,"b",2,"x-ab-y"]
// CHECK-NEXT: ["ab","a",undefined,2,"x-ab-y",{p:"a",q:undefined}]

function hasIndices() {
  print("hasIndices");
  var m = "xab".match(/a(b)/d);
  print(show(m), m.index, show(m.indices));
}
// CHECK-LABEL: hasIndices
// CHECK-NEXT: ["ab","b"] 1 {{\[}}[1,3],[2,3]]

function unicodeGetter() {
  print("unicode getter");
  // @@match and @@replace advance past an empty match by Get(rx, "unicode"),
  // not by the regexp's own flag.
  var re = /(?:)/g;
  Object.defineProperty(re, "unicode", { value: true });
  var s = "\uD83D\uDE00x";
  print(show(s.match(re)), show(s.replace(re, "-")));
}
// CHECK-LABEL: unicode getter
// CHECK-NEXT: ["","",""] "-\ud83d\ude00-x-"

function reentrantCallback() {
  print("reentrant callback");
  // The callback runs after every exec of phase 1, and may use the same
  // regexp.
  var re = /\d/g;
  var out = "a1b2c".replace(re, function (d) {
    var inner = "7z8".replace(re, "q");
    return "<" + d + inner + re.lastIndex + RegExp.lastMatch + ">";
  });
  print(out, re.lastIndex, RegExp.lastMatch);
}
// CHECK-LABEL: reentrant callback
// CHECK-NEXT: a<1qzq08>b<2qzq08>c 0 8

function stacks() {
  print("stacks");
  var stack = "";
  var re = /a/;
  re.lastIndex = {
    valueOf: function () { stack = new Error().stack; return 0; },
  };
  "xa".match(re);
  print("match", hasExecFrame(stack));
  stack = "";
  "xa".replace(re, "b");
  print("replace", hasExecFrame(stack));
  stack = "";
  re.test("xa");
  print("test", hasExecFrame(stack));
  try {
    Object.freeze(/a/y)[Symbol.match]("a");
  } catch (e) {
    print("sticky", e.name, hasExecFrame(e.stack));
  }
  try {
    "b".match(/(){999999999}/g);
  } catch (e) {
    print("global match limit", e.name, hasExecFrame(e.stack));
  }
  try {
    "b".search(/(){999999999}/);
  } catch (e) {
    print("search limit", e.name, hasExecFrame(e.stack));
  }
}
// CHECK-LABEL: stacks
// CHECK-NEXT: match true
// CHECK-NEXT: replace true
// CHECK-NEXT: test false
// CHECK-NEXT: sticky TypeError true
// CHECK-NEXT: global match limit RangeError true
// CHECK-NEXT: search limit RangeError true

function main() {
  var builtinExec = RegExp.prototype.exec;
  replacedExec(builtinExec);
  ownAndSubclassExec(builtinExec);
  execAccessor(builtinExec);
  proxyPrototype();
  frozen();
  lastIndexValueOf();
  statics();
  emptyMatches();
  stickyGlobal();
  replacements();
  hasIndices();
  unicodeGetter();
  reentrantCallback();
  stacks();
}

main();
