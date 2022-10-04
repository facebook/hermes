'use strict';

var _primordials = primordials,
    MathFloor = _primordials.MathFloor,
    NumberIsInteger = _primordials.NumberIsInteger;

var ERR_INVALID_ARG_VALUE = require('internal/errors').codes.ERR_INVALID_ARG_VALUE;

function highWaterMarkFrom(options, isDuplex, duplexKey) {
  return options.highWaterMark != null ? options.highWaterMark : isDuplex ? options[duplexKey] : null;
}

function getDefaultHighWaterMark(objectMode) {
  return objectMode ? 16 : 16 * 1024;
}

function getHighWaterMark(state, options, duplexKey, isDuplex) {
  var hwm = highWaterMarkFrom(options, isDuplex, duplexKey);

  if (hwm != null) {
    if (!NumberIsInteger(hwm) || hwm < 0) {
      var name = isDuplex ? "options.".concat(duplexKey) : 'options.highWaterMark';
      throw new ERR_INVALID_ARG_VALUE(name, hwm);
    }

    return MathFloor(hwm);
  } // Default value


  return getDefaultHighWaterMark(state.objectMode);
}

module.exports = {
  getHighWaterMark: getHighWaterMark,
  getDefaultHighWaterMark: getDefaultHighWaterMark
};
