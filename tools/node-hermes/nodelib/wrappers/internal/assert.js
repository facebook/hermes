// @nolint
'use strict';

var error;

function lazyError() {
  if (!error) {
    error = require('internal/errors').codes.ERR_INTERNAL_ASSERTION;
  }

  return error;
}

function assert(value, message) {
  if (!value) {
    var ERR_INTERNAL_ASSERTION = lazyError();
    throw new ERR_INTERNAL_ASSERTION(message);
  }
}

function fail(message) {
  var ERR_INTERNAL_ASSERTION = lazyError();
  throw new ERR_INTERNAL_ASSERTION(message);
}

assert.fail = fail;
module.exports = assert;
