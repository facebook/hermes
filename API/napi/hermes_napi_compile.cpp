/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi_compile.h"
#include "hermes_napi_internal.h"

#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/Support/Buffer.h"

#include "llvh/Support/SHA1.h"
#include "llvh/Support/raw_ostream.h"

#include <cstdlib>
#include <cstring>
#include <string>

napi_status hermes_compile_to_bytecode(
    napi_env env,
    const uint8_t *source,
    size_t source_size,
    const char *source_url,
    const hermes_compile_flags *flags,
    uint8_t **bytecode_out,
    size_t *bytecode_size_out) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, source);
  CHECK_ARG(env, bytecode_out);
  CHECK_ARG(env, bytecode_size_out);

  using namespace hermes;
  using namespace hermes::vm;

  Runtime &runtime = env->runtime;

  // Wrap source in a Buffer. createBCProviderFromSrc requires
  // buffer.data()[buffer.size()] == '\0'.
  std::unique_ptr<Buffer> buffer;
  if (source_size > 0 && source[source_size - 1] == '\0') {
    // Null terminator included — use zero-copy wrapper.
    // The actual source text is source_size - 1 bytes; the byte at
    // source_size - 1 serves as the required past-the-end null.
    buffer = std::make_unique<Buffer>(source, source_size - 1);
  } else {
    // No null terminator — copy into a std::string (which guarantees
    // null termination).
    std::string copy(reinterpret_cast<const char *>(source), source_size);
    buffer = std::make_unique<StdStringBuffer>(std::move(copy));
  }

  // Read compile flags.
  hbc::CompileFlags compileFlags;
  compileFlags.format = EmitBundle;
  compileFlags.includeLibHermes = false;
  compileFlags.enableGenerator = true;
  compileFlags.enableES6BlockScoping = true;
  compileFlags.enableAsyncGenerators = true;
  if (flags && flags->struct_size >= sizeof(hermes_compile_flags)) {
    compileFlags.strict = flags->strict;
    compileFlags.emitAsyncBreakCheck = flags->emit_async_break_check;
    compileFlags.enableTS = flags->enable_ts;
  }

  bool optimize = flags && flags->struct_size >= sizeof(hermes_compile_flags) &&
      flags->optimize;

  // Compile source to bytecode.
  auto res = hbc::createBCProviderFromSrc(
      std::move(buffer),
      source_url ? source_url : "",
      /*sourceMap=*/"",
      compileFlags,
      /*topLevelFunctionName=*/"global",
      /*diagHandler=*/nullptr,
      /*diagContext=*/nullptr,
      optimize ? hbc::fullOptimizationPipeline : nullptr);
  if (!res.first) {
    (void)runtime.raiseSyntaxError(TwineChar16(res.second));
    return captureRuntimeException(env, napi_pending_exception);
  }

  assert(
      res.first->getBytecodeModule() &&
      "BCProviderFromSrc must have a bytecode module");

  // Compute source hash for the serialized bytecode header.
  size_t hashSize = (source_size > 0 && source[source_size - 1] == '\0')
      ? source_size - 1
      : source_size;
  auto sourceHash = llvh::SHA1::hash(llvh::makeArrayRef(source, hashSize));

  // Serialize to a string buffer.
  std::string bytecodeStr;
  llvh::raw_string_ostream os(bytecodeStr);
  BytecodeGenerationOptions opts(EmitBundle);
  opts.optimizationEnabled = optimize;
  hbc::serializeBytecodeModule(
      *res.first->getBytecodeModule(), sourceHash, os, opts);
  os.flush();

  // Copy to a malloc'd buffer for the caller.
  auto *out = static_cast<uint8_t *>(malloc(bytecodeStr.size()));
  if (!out) {
    (void)runtime.raiseRangeError("Out of memory allocating bytecode buffer");
    return captureRuntimeException(env, napi_pending_exception);
  }
  std::memcpy(out, bytecodeStr.data(), bytecodeStr.size());
  *bytecode_out = out;
  *bytecode_size_out = bytecodeStr.size();

  return napi_clear_last_error(env);
}

void hermes_free_bytecode(uint8_t *bytecode) {
  free(bytecode);
}
