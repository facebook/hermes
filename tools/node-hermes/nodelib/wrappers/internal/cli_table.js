'use strict';

var _primordials = primordials,
    ArrayPrototypeJoin = _primordials.ArrayPrototypeJoin,
    ArrayPrototypeMap = _primordials.ArrayPrototypeMap,
    MathCeil = _primordials.MathCeil,
    MathMax = _primordials.MathMax,
    MathMaxApply = _primordials.MathMaxApply,
    ObjectPrototypeHasOwnProperty = _primordials.ObjectPrototypeHasOwnProperty,
    StringPrototypeRepeat = _primordials.StringPrototypeRepeat;

var _require = require('internal/util/inspect'),
    getStringWidth = _require.getStringWidth; // The use of Unicode characters below is the only non-comment use of non-ASCII
// Unicode characters in Node.js built-in modules. If they are ever removed or
// rewritten with \u escapes, then a test will need to be (re-)added to Node.js
// core to verify that Unicode characters work in built-ins.
// Refs: https://github.com/nodejs/node/issues/10673


var tableChars = {
  /* eslint-disable node-core/non-ascii-character */
  middleMiddle: '─',
  rowMiddle: '┼',
  topRight: '┐',
  topLeft: '┌',
  leftMiddle: '├',
  topMiddle: '┬',
  bottomRight: '┘',
  bottomLeft: '└',
  bottomMiddle: '┴',
  rightMiddle: '┤',
  left: '│ ',
  right: ' │',
  middle: ' │ '
  /* eslint-enable node-core/non-ascii-character */

};

var renderRow = function renderRow(row, columnWidths) {
  var out = tableChars.left;

  for (var i = 0; i < row.length; i++) {
    var cell = row[i];
    var len = getStringWidth(cell);
    var needed = (columnWidths[i] - len) / 2; // round(needed) + ceil(needed) will always add up to the amount
    // of spaces we need while also left justifying the output.

    out += StringPrototypeRepeat(' ', needed) + cell + StringPrototypeRepeat(' ', MathCeil(needed));
    if (i !== row.length - 1) out += tableChars.middle;
  }

  out += tableChars.right;
  return out;
};

var table = function table(head, columns) {
  var rows = [];
  var columnWidths = ArrayPrototypeMap(head, function (h) {
    return getStringWidth(h);
  });
  var longestColumn = MathMaxApply(ArrayPrototypeMap(columns, function (a) {
    return a.length;
  }));
  for (var i = 0; i < head.length; i++) {
    var column = columns[i];

    for (var j = 0; j < longestColumn; j++) {
      if (rows[j] === undefined) rows[j] = [];
      var value = rows[j][i] = ObjectPrototypeHasOwnProperty(column, j) ? column[j] : '';
      var width = columnWidths[i] || 0;
      var counted = getStringWidth(value);
      columnWidths[i] = MathMax(width, counted);
    }
  }

  var divider = ArrayPrototypeMap(columnWidths, function (i) {
    return StringPrototypeRepeat(tableChars.middleMiddle, i + 2);
  });
  var result = tableChars.topLeft + ArrayPrototypeJoin(divider, tableChars.topMiddle) + tableChars.topRight + '\n' + renderRow(head, columnWidths) + '\n' + tableChars.leftMiddle + ArrayPrototypeJoin(divider, tableChars.rowMiddle) + tableChars.rightMiddle + '\n';

  for (var _i = 0, _rows = rows; _i < _rows.length; _i++) {
    var row = _rows[_i];
    result += "".concat(renderRow(row, columnWidths), "\n");
  }

  result += tableChars.bottomLeft + ArrayPrototypeJoin(divider, tableChars.bottomMiddle) + tableChars.bottomRight;
  return result;
};

module.exports = table;
