/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/CodeCoverageProfiler.h"

#include "hermes/VM/Callable.h"

#include <assert.h>

namespace hermes {
namespace vm {

/*static*/ std::shared_ptr<CodeCoverageProfiler>
CodeCoverageProfiler::getInstance() {
  static std::shared_ptr<CodeCoverageProfiler> instance(
      new CodeCoverageProfiler());
  return instance;
}

void CodeCoverageProfiler::markRoots(
    Runtime *runtime,
    SlotAcceptorWithNames &acceptor) {
  auto coverageInfoIter = coverageInfo_.find(runtime);
  if (LLVM_UNLIKELY(coverageInfoIter == coverageInfo_.end())) {
    return;
  }
  for (Domain *&domain : coverageInfoIter->second.domains) {
    acceptor.acceptPtr(domain);
  }
}

void CodeCoverageProfiler::markExecutedSlowPath(CodeBlock *codeBlock) {
  std::vector<bool> &moduleFuncMap =
      getModuleFuncMapRef(codeBlock->getRuntimeModule());

  const auto funcId = codeBlock->getFunctionID();
  assert(
      funcId < moduleFuncMap.size() &&
      "funcId is out of bound for moduleFuncMap.");
  moduleFuncMap[funcId] = true;
}

std::vector<CodeCoverageProfiler::FuncInfo>
CodeCoverageProfiler::getExecutedFunctions() {
  std::vector<CodeCoverageProfiler::FuncInfo> funcInfos;

  // Since Hermes only supports symbolication for one runtime
  // we return coverage information for the first runtime here.
  auto coverageInfoIter = coverageInfo_.begin();
  if (coverageInfoIter == coverageInfo_.end()) {
    return funcInfos;
  }
  for (auto &entry : coverageInfoIter->second.executedFuncBitsArrayMap) {
    auto *bcProvider = entry.first->getBytecode();
    const uint32_t moduleId = bcProvider->getCJSModuleOffset();
    const std::vector<bool> &moduleFuncBitsArray = entry.second;
    for (uint32_t i = 0; i < moduleFuncBitsArray.size(); ++i) {
      if (moduleFuncBitsArray[i]) {
        const uint32_t funcVirtualOffset =
            bcProvider->getVirtualOffsetForFunction(i);
        funcInfos.emplace_back(moduleId, funcVirtualOffset);
      }
    }
  }
  return funcInfos;
}

std::vector<bool> &CodeCoverageProfiler::getModuleFuncMapRef(
    RuntimeModule *module) {
  auto &runtimeCoverageInfo = coverageInfo_[module->getRuntime()];
  auto &funcBitsArrayMap = runtimeCoverageInfo.executedFuncBitsArrayMap;

  auto funcMapIter = funcBitsArrayMap.find(module);
  if (LLVM_LIKELY(funcMapIter != funcBitsArrayMap.end())) {
    return funcMapIter->second;
  }

  // For new module reigster its domain for marking.
  runtimeCoverageInfo.domains.insert(module->getDomainUnsafe());

  const uint32_t funcCount = module->getBytecode()->getFunctionCount();
  auto res = funcBitsArrayMap.insert(
      std::make_pair(module, std::vector<bool>(funcCount)));
  return res.first->second;
}

bool operator==(
    const CodeCoverageProfiler::FuncInfo &left,
    const CodeCoverageProfiler::FuncInfo &right) {
  return left.moduleId == right.moduleId &&
      left.funcVirtualOffset == right.funcVirtualOffset;
}

} // namespace vm
} // namespace hermes
