/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// De-minifies a JS file by adding newlines after punctuation characters
// (;, {, }, (, ), ,) outside of and inside string literals, making diffs
// between two minified files human-readable despite variable name mangling.
//
// Usage: node expand-minified.js <file>

var c = require("fs").readFileSync(process.argv[2], "utf8");
var r = "", s = false, q = "", e = false, d = 0;
var BT = "`";
var SQ = "'";
for (var i = 0; i < c.length; i++) {
  var ch = c[i];
  if (e) { e = false; r += ch; continue; }
  if (ch === "\\") { e = true; r += ch; continue; }
  if (s) {
    r += ch;
    if (q === BT && ch === "$" && c[i+1] === "{") { d++; }
    else if (q === BT && d > 0 && ch === "}") { d--; }
    else if (ch === q && d === 0) { s = false; }
    else if (q !== BT || d === 0) {
      if (ch === ";" || ch === "{" || ch === "}" || ch === "," || ch === "(" || ch === ")") {
        r += "\n";
      }
    }
    continue;
  }
  if (ch === "\"" || ch === SQ || ch === BT) { s = true; q = ch; r += ch; continue; }
  r += ch;
  if (ch === ";" || ch === "{" || ch === "}" || ch === "," || ch === "(" || ch === ")") {
    r += "\n";
  }
}
process.stdout.write(r);
