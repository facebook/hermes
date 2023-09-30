/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"

#include "hermes/AST/NativeContext.h"
#include "hermes/Utils/Dumper.h"

namespace hermes {

Context::Context(
    SourceErrorManager &sm,
    CodeGenerationSettings codeGenOpts,
    OptimizationSettings optimizationOpts,
    const NativeSettings *nativeSettings,
    std::unique_ptr<ResolutionTable> resolutionTable,
    std::vector<uint32_t> segments)
    : sm_(sm),
      resolutionTable_(std::move(resolutionTable)),
      segments_(std::move(segments)),
      codeGenerationSettings_(std::move(codeGenOpts)),
      optimizationSettings_(std::move(optimizationOpts)),
      nativeContext_(new NativeContext(
          nativeSettings ? *nativeSettings : NativeSettings())) {}

Context::Context(
    CodeGenerationSettings codeGenOpts,
    OptimizationSettings optimizationOpts,
    const NativeSettings *nativeSettings,
    std::unique_ptr<ResolutionTable> resolutionTable,
    std::vector<uint32_t> segments)
    : ownSm_(new SourceErrorManager()),
      sm_(*ownSm_),
      resolutionTable_(std::move(resolutionTable)),
      segments_(std::move(segments)),
      codeGenerationSettings_(std::move(codeGenOpts)),
      optimizationSettings_(std::move(optimizationOpts)),
      nativeContext_(new NativeContext(
          nativeSettings ? *nativeSettings : NativeSettings())) {}

Context::~Context() = default;

void Context::createPersistentIRNamer() {
  persistentIRNamer_ = std::make_unique<irdumper::Namer>(true);
}

void Context::clearPersistentIRNamer() {
  persistentIRNamer_.reset();
}

} // namespace hermes
