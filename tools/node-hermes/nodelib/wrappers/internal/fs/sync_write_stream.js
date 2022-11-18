'use strict';

var _primordials = primordials,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    ReflectApply = _primordials.ReflectApply;

var _require = require('stream'),
    Writable = _require.Writable;

var _require2 = require('fs'),
    closeSync = _require2.closeSync,
    writeSync = _require2.writeSync;

function SyncWriteStream(fd, options) {
  ReflectApply(Writable, this, [{
    autoDestroy: true
  }]);
  options = options || {};
  this.fd = fd;
  this.readable = false;
  this.autoClose = options.autoClose === undefined ? true : options.autoClose;
}

ObjectSetPrototypeOf(SyncWriteStream.prototype, Writable.prototype);
ObjectSetPrototypeOf(SyncWriteStream, Writable);

SyncWriteStream.prototype._write = function (chunk, encoding, cb) {
  writeSync(this.fd, chunk, 0, chunk.length);
  cb();
  return true;
};

SyncWriteStream.prototype._destroy = function (err, cb) {
  if (this.fd === null) // already destroy()ed
    return cb(err);
  if (this.autoClose) closeSync(this.fd);
  this.fd = null;
  cb(err);
};

SyncWriteStream.prototype.destroySoon = SyncWriteStream.prototype.destroy;
module.exports = SyncWriteStream;
