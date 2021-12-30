/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings.h"
#include "hermes/hermes.h"

#include "uv.h"

using namespace facebook;

/// Takes a file path and returns a file descriptor representing the open file.
/// Called by js the following way:
/// fd = binding.open(path, flags, mode, FSReqCallback, ctx)
/// In the synchronous version, FSReqCallback will always be undefined.
/// Currently only the synchronous version is supported.
static jsi::Value open(
    RuntimeState &rs,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 5) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into synchronous open call.");
  }
  if (!args[0].isString() || !args[1].isNumber() || !args[2].isNumber()) {
    throw jsi::JSError(
        rt, "Incorrectly typed objects passed into synchronous open call.");
  }
  std::string filenameUTF8 = args[0].asString(rt).utf8(rt);

  llvh::SmallString<32> fullFileName = rs.resolvePath(filenameUTF8, "/");

  int flags = args[1].asNumber();
  int mode = args[2].asNumber();
  uv_fs_t openReq;
  int fd = uv_fs_open(
      rs.getLoop(), &openReq, fullFileName.c_str(), flags, mode, nullptr);
  if (fd < 0)
    throw jsi::JSError(
        rt,
        "OpenSync failed on file '" + filenameUTF8 + "' with errno " +
            std::to_string(errno) + ": " + std::strerror(errno));
  return jsi::Value(fd);
}

/// Closes the file descriptor passed in.
/// Called by js the following way:
/// binding.close(fd, undefined, ctx);
static jsi::Value close(
    RuntimeState &rs,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 3) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into synchronous close call.");
  }
  if (!args[0].isNumber()) {
    throw jsi::JSError(
        rt, "Incorrectly typed objects passed into synchronous close call.");
  }
  int fd = args[0].getNumber();
  uv_fs_t close_req;
  int closeRes = uv_fs_close(rs.getLoop(), &close_req, fd, nullptr);
  if (closeRes < 0)
    throw jsi::JSError(
        rt,
        "Close failed on fd " + std::to_string(fd) + " with errno " +
            std::to_string(errno) + ": " + std::strerror(errno));
  return jsi::Value::undefined();
}

/// Reads from an open file descriptor and stores the result in the buffer
/// object passed into the function. Returns the number of bytes read.
/// Called by js the following way:
/// bytesRead = fs.read(fd, buffer, offset, length, position, undefined, ctx)
static jsi::Value read(
    RuntimeState &rs,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 7) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into synchronous read call.");
  }
  if (!args[0].isNumber() || !args[1].isObject() ||
      !args[1]
           .getObject(rt)
           .getPropertyAsObject(rt, "buffer")
           .isArrayBuffer(rt) ||
      !args[2].isNumber() || !args[3].isNumber() || !args[4].isNumber()) {
    throw jsi::JSError(
        rt, "Incorrectly typed objects passed into synchronous open call.");
  }

  uint32_t fd = args[0].asNumber();
  jsi::ArrayBuffer arrayBuffer = args[1]
                                     .getObject(rt)
                                     .getPropertyAsObject(rt, "buffer")
                                     .getArrayBuffer(rt);
  char *bufferData = reinterpret_cast<char *>(arrayBuffer.data(rt));
  uint32_t offset = args[2].asNumber();
  uint32_t length = args[3].asNumber();
  int32_t position = args[4].asNumber();

  char *buf = bufferData + (int)offset;
  uv_buf_t uvBuf = uv_buf_init(buf, length);

  // Checks that the buffer is large enough to store file contents
  assert((size_t)length >= arrayBuffer.length(rt));

  uv_fs_t readReq;
  int bytesRead =
      uv_fs_read(rs.getLoop(), &readReq, (int)fd, &uvBuf, 1, position, nullptr);

  if (bytesRead < 0)
    throw jsi::JSError(
        rt,
        "Read failed on fd " + std::to_string(fd) + " with errno " +
            std::to_string(errno) + ": " + std::strerror(errno));

  return jsi::Value(bytesRead);
}

/// Returns information about the already opened file descriptor.
/// Called by js the following way:
/// fd = binding.fstat(fd, use_bigint, undefined, ctx)
static jsi::Value fstat(
    RuntimeState &rs,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 4) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into synchronous fstat call.");
  }
  uv_fs_t fstat_req;
  int fd = args[0].asNumber();
  int fstatRes = uv_fs_fstat(rs.getLoop(), &fstat_req, fd, nullptr);

  if (fstatRes < 0)
    throw jsi::JSError(
        rt,
        "Fstat failed on fd " + std::to_string(fd) + " with errno " +
            std::to_string(errno) + ": " + std::strerror(errno));

  uv_stat_t *statbuf = uv_fs_get_statbuf(&fstat_req);
  jsi::Object res{rt};

  // Missing properties: atime, mtime, ctime, birthtime because no datetime
  // support
  res.setProperty(rt, "dev", (double)statbuf->st_dev);
  res.setProperty(rt, "mode", (double)statbuf->st_mode);
  res.setProperty(rt, "nlink", (double)statbuf->st_nlink);
  res.setProperty(rt, "uid", (double)statbuf->st_uid);
  res.setProperty(rt, "gid", (double)statbuf->st_gid);
  res.setProperty(rt, "rdev", (double)statbuf->st_rdev);
  res.setProperty(rt, "blksize", (double)statbuf->st_blksize);
  res.setProperty(rt, "ino", (double)statbuf->st_size);
  res.setProperty(rt, "size", (double)statbuf->st_size);
  res.setProperty(rt, "blocks", (double)statbuf->st_blocks);

  return std::move(res);
}

/// Adds the 'fs' object as a property of internalBinding.
jsi::Value facebook::fsBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object fs{rt};

  rs.defineJSFunction(open, "open", 5, fs);
  rs.defineJSFunction(close, "close", 3, fs);
  rs.defineJSFunction(fstat, "fstat", 4, fs);
  rs.defineJSFunction(read, "read", 7, fs);

  rs.setInternalBindingProp("fs", std::move(fs));
  return rs.getInternalBindingProp("fs");
}
