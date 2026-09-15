/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * NAPI addon implementing TextEncoderNApi — a proving-ground clone of
 * the JSI-based TextEncoder.  Uses only the standard NAPI C API surface.
 *
 * Exports:
 *   TextEncoderNApi        — constructor (must be called with `new`)
 *   detachArrayBuffer(ab)  — helper for testing detached-buffer checks
 *
 * Prototype members:
 *   encoding     — getter, always returns "utf-8"
 *   encode(str)  — returns a Uint8Array of the UTF-8 encoding
 *   encodeInto(source, destination) — encodes into an existing Uint8Array
 */

#include "hermes/napi/js_native_api.h"
#include "hermes/napi/node_api.h"

#include <cstring>
#include <string>
#include <vector>

/// Sentinel address used by napi_wrap / napi_unwrap to identify instances.
static int sentinel = 0;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Count how many UTF-16 code units correspond to \p nBytes of valid UTF-8
/// starting at \p utf8.
static size_t countUtf16Units(const char *utf8, size_t nBytes) {
  size_t units = 0;
  size_t i = 0;
  while (i < nBytes) {
    unsigned char c = static_cast<unsigned char>(utf8[i]);
    size_t seqLen;
    if (c < 0x80) {
      seqLen = 1; // ASCII — 1 UTF-16 unit
      units += 1;
    } else if ((c >> 5) == 0x06) {
      seqLen = 2; // 2-byte — 1 UTF-16 unit
      units += 1;
    } else if ((c >> 4) == 0x0E) {
      seqLen = 3; // 3-byte — 1 UTF-16 unit
      units += 1;
    } else {
      seqLen = 4; // 4-byte (supplementary) — 2 UTF-16 units
      units += 2;
    }
    i += seqLen;
  }
  return units;
}

/// Return the length of the UTF-8 sequence starting with byte \p c.
static size_t utf8SeqLen(unsigned char c) {
  if (c < 0x80)
    return 1;
  if ((c >> 5) == 0x06)
    return 2;
  if ((c >> 4) == 0x0E)
    return 3;
  return 4;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------

/// encoding getter — always returns "utf-8".
static napi_value getEncoding(napi_env env, napi_callback_info info) {
  napi_value result;
  napi_create_string_utf8(env, "utf-8", NAPI_AUTO_LENGTH, &result);
  return result;
}

/// Constructor callback for TextEncoderNApi.
static napi_value construct(napi_env env, napi_callback_info info) {
  napi_value newTarget;
  napi_get_new_target(env, info, &newTarget);
  if (!newTarget) {
    napi_throw_type_error(
        env,
        nullptr,
        "Class constructor TextEncoderNApi cannot be invoked without 'new'");
    return nullptr;
  }

  napi_value thisArg;
  napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);
  napi_wrap(env, thisArg, &sentinel, nullptr, nullptr, nullptr);
  return thisArg;
}

/// encode(input) — returns a Uint8Array of the UTF-8 encoding of input.
static napi_value encode(napi_env env, napi_callback_info info) {
  // Validate this.
  napi_value thisArg;
  size_t argc = 1;
  napi_value argv[1];
  napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr);

  void *wrapped = nullptr;
  napi_status st = napi_unwrap(env, thisArg, &wrapped);
  if (st != napi_ok || wrapped != &sentinel) {
    napi_throw_type_error(
        env,
        nullptr,
        "TextEncoderNApi.prototype.encode() called on non-TextEncoderNApi object");
    return nullptr;
  }

  // If no argument or undefined, return empty Uint8Array.
  if (argc == 0) {
    napi_value ab, result;
    void *data;
    napi_create_arraybuffer(env, 0, &data, &ab);
    napi_create_typedarray(env, napi_uint8_array, 0, ab, 0, &result);
    return result;
  }

  napi_valuetype argType;
  napi_typeof(env, argv[0], &argType);
  if (argType == napi_undefined) {
    napi_value ab, result;
    void *data;
    napi_create_arraybuffer(env, 0, &data, &ab);
    napi_create_typedarray(env, napi_uint8_array, 0, ab, 0, &result);
    return result;
  }

  // Coerce to string.
  napi_value strVal;
  napi_coerce_to_string(env, argv[0], &strVal);

  // Get UTF-8 length.
  size_t len = 0;
  napi_get_value_string_utf8(env, strVal, nullptr, 0, &len);

  if (len == 0) {
    napi_value ab, result;
    void *data;
    napi_create_arraybuffer(env, 0, &data, &ab);
    napi_create_typedarray(env, napi_uint8_array, 0, ab, 0, &result);
    return result;
  }

  // Copy UTF-8 into a temp buffer (napi writes a null terminator, so we
  // need len+1 bytes), then memcpy into the ArrayBuffer which is exactly len.
  std::vector<char> tmp(len + 1);
  size_t written = 0;
  napi_get_value_string_utf8(env, strVal, tmp.data(), len + 1, &written);

  napi_value ab;
  void *abData;
  napi_create_arraybuffer(env, written, &abData, &ab);
  std::memcpy(abData, tmp.data(), written);

  // Create Uint8Array wrapping the ArrayBuffer.
  napi_value result;
  napi_create_typedarray(env, napi_uint8_array, written, ab, 0, &result);
  return result;
}

/// encodeInto(source, destination) — encodes source into destination
/// Uint8Array and returns {read, written}.
static napi_value encodeInto(napi_env env, napi_callback_info info) {
  // Validate this.
  napi_value thisArg;
  size_t argc = 2;
  napi_value argv[2];
  napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr);

  void *wrapped = nullptr;
  napi_status st = napi_unwrap(env, thisArg, &wrapped);
  if (st != napi_ok || wrapped != &sentinel) {
    napi_throw_type_error(
        env,
        nullptr,
        "TextEncoderNApi.prototype.encodeInto() called on non-TextEncoderNApi object");
    return nullptr;
  }

  // Coerce source to string.
  napi_value strVal;
  napi_coerce_to_string(env, argv[0], &strVal);

  // Get destination typed array info.
  napi_typedarray_type taType;
  size_t destLen = 0;
  void *destData = nullptr;
  napi_value destAb;
  size_t destOffset = 0;
  st = napi_get_typedarray_info(
      env, argv[1], &taType, &destLen, &destData, &destAb, &destOffset);
  if (st != napi_ok) {
    napi_throw_type_error(
        env, nullptr, "The second argument should be a Uint8Array");
    return nullptr;
  }

  if (taType != napi_uint8_array) {
    napi_throw_type_error(
        env, nullptr, "The second argument should be a Uint8Array");
    return nullptr;
  }

  // Check for detached buffer.  When the typed array's underlying buffer is
  // detached, napi_get_typedarray_info returns data==nullptr and sets the
  // arraybuffer out-param to undefined, so we check data directly.
  if (!destData) {
    napi_throw_type_error(
        env,
        nullptr,
        "TextEncoderNApi.prototype.encodeInto called on a detached ArrayBuffer");
    return nullptr;
  }

  // Get UTF-8 of source.
  size_t utf8Len = 0;
  napi_get_value_string_utf8(env, strVal, nullptr, 0, &utf8Len);

  std::vector<char> utf8Buf(utf8Len + 1);
  size_t actualLen = 0;
  napi_get_value_string_utf8(
      env, strVal, utf8Buf.data(), utf8Len + 1, &actualLen);

  // Copy as many complete UTF-8 code points as fit into destination.
  size_t bytesWritten = 0;
  size_t bytesRead = 0;
  while (bytesRead < actualLen) {
    size_t seqLen = utf8SeqLen(static_cast<unsigned char>(utf8Buf[bytesRead]));
    if (bytesWritten + seqLen > destLen)
      break;
    std::memcpy(
        static_cast<char *>(destData) + bytesWritten,
        utf8Buf.data() + bytesRead,
        seqLen);
    bytesWritten += seqLen;
    bytesRead += seqLen;
  }

  // Count how many UTF-16 code units were consumed (JS string .length).
  size_t utf16Read = countUtf16Units(utf8Buf.data(), bytesRead);

  // Return {read, written}.
  napi_value resultObj;
  napi_create_object(env, &resultObj);

  napi_value readVal, writtenVal;
  napi_create_uint32(env, static_cast<uint32_t>(utf16Read), &readVal);
  napi_create_uint32(env, static_cast<uint32_t>(bytesWritten), &writtenVal);
  napi_set_named_property(env, resultObj, "read", readVal);
  napi_set_named_property(env, resultObj, "written", writtenVal);

  return resultObj;
}

/// detachArrayBuffer(ab) — test helper to detach an ArrayBuffer.
static napi_value detachArrayBufferFn(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  napi_detach_arraybuffer(env, argv[0]);
  return nullptr;
}

// ---------------------------------------------------------------------------
// Module init
// ---------------------------------------------------------------------------

NAPI_MODULE_INIT() {
  // Set up Symbol.toStringTag value.
  napi_value globalObj;
  napi_get_global(env, &globalObj);

  napi_value symbolObj;
  napi_get_named_property(env, globalObj, "Symbol", &symbolObj);
  napi_value toStringTagKey;
  napi_get_named_property(env, symbolObj, "toStringTag", &toStringTagKey);

  napi_value tagValue;
  napi_create_string_utf8(env, "TextEncoderNApi", NAPI_AUTO_LENGTH, &tagValue);

  // Define prototype properties.
  napi_property_descriptor props[] = {
      // encoding getter (enumerable + configurable)
      {"encoding",
       nullptr,
       nullptr,
       getEncoding,
       nullptr,
       nullptr,
       static_cast<napi_property_attributes>(
           napi_enumerable | napi_configurable),
       nullptr},
      // encode method
      {"encode",
       nullptr,
       encode,
       nullptr,
       nullptr,
       nullptr,
       napi_default_method,
       nullptr},
      // encodeInto method
      {"encodeInto",
       nullptr,
       encodeInto,
       nullptr,
       nullptr,
       nullptr,
       napi_default_method,
       nullptr},
      // Symbol.toStringTag
      {nullptr,
       toStringTagKey,
       nullptr,
       nullptr,
       nullptr,
       tagValue,
       napi_configurable,
       nullptr},
  };

  napi_value ctor;
  napi_define_class(
      env,
      "TextEncoderNApi",
      NAPI_AUTO_LENGTH,
      construct,
      nullptr,
      sizeof(props) / sizeof(props[0]),
      props,
      &ctor);

  napi_set_named_property(env, exports, "TextEncoderNApi", ctor);

  // Export test helper.
  napi_value detachFn;
  napi_create_function(
      env,
      "detachArrayBuffer",
      NAPI_AUTO_LENGTH,
      detachArrayBufferFn,
      nullptr,
      &detachFn);
  napi_set_named_property(env, exports, "detachArrayBuffer", detachFn);

  return exports;
}
