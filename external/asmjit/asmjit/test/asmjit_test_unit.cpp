// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
#include <asmjit/x86.h>
#endif

#include "asmjitutils.h"
#include "broken.h"

using namespace asmjit;

static void dumpCpu(void) noexcept {
  const CpuInfo& cpu = CpuInfo::host();

  // CPU Information
  // ---------------

  INFO("CPU Info:");
  INFO("  Vendor                  : %s", cpu.vendor());
  INFO("  Brand                   : %s", cpu.brand());
  INFO("  Model ID                : %u", cpu.modelId());
  INFO("  Brand ID                : %u", cpu.brandId());
  INFO("  Family ID               : %u", cpu.familyId());
  INFO("  Stepping                : %u", cpu.stepping());
  INFO("  Processor Type          : %u", cpu.processorType());
  INFO("  Max logical Processors  : %u", cpu.maxLogicalProcessors());
  INFO("  Cache-Line Size         : %u", cpu.cacheLineSize());
  INFO("  HW-Thread Count         : %u", cpu.hwThreadCount());
  INFO("");

  // CPU Features
  // ------------

#ifndef ASMJIT_NO_LOGGING
  INFO("CPU Features:");
  CpuFeatures::Iterator it(cpu.features().iterator());
  while (it.hasNext()) {
    uint32_t featureId = uint32_t(it.next());
    StringTmp<64> featureString;
    Formatter::formatFeature(featureString, cpu.arch(), featureId);
    INFO("  %s\n", featureString.data());
  };
  INFO("");
#endif // !ASMJIT_NO_LOGGING
}

#define DUMP_TYPE(...) \
  INFO("  %-26s: %u", #__VA_ARGS__, uint32_t(sizeof(__VA_ARGS__)))

static void dumpSizeOf(void) noexcept {
  INFO("Size of C++ types:");
    DUMP_TYPE(int8_t);
    DUMP_TYPE(int16_t);
    DUMP_TYPE(int32_t);
    DUMP_TYPE(int64_t);
    DUMP_TYPE(int);
    DUMP_TYPE(long);
    DUMP_TYPE(size_t);
    DUMP_TYPE(intptr_t);
    DUMP_TYPE(float);
    DUMP_TYPE(double);
    DUMP_TYPE(void*);
  INFO("");

  INFO("Size of base classes:");
    DUMP_TYPE(BaseAssembler);
    DUMP_TYPE(BaseEmitter);
    DUMP_TYPE(CodeBuffer);
    DUMP_TYPE(CodeHolder);
    DUMP_TYPE(ConstPool);
    DUMP_TYPE(LabelEntry);
    DUMP_TYPE(RelocEntry);
    DUMP_TYPE(Section);
    DUMP_TYPE(String);
    DUMP_TYPE(Target);
    DUMP_TYPE(Zone);
    DUMP_TYPE(ZoneAllocator);
    DUMP_TYPE(ZoneBitVector);
    DUMP_TYPE(ZoneHashNode);
    DUMP_TYPE(ZoneHash<ZoneHashNode>);
    DUMP_TYPE(ZoneList<int>);
    DUMP_TYPE(ZoneVector<int>);
  INFO("");

  INFO("Size of operand classes:");
    DUMP_TYPE(Operand);
    DUMP_TYPE(BaseReg);
    DUMP_TYPE(BaseMem);
    DUMP_TYPE(Imm);
    DUMP_TYPE(Label);
  INFO("");

  INFO("Size of function classes:");
    DUMP_TYPE(CallConv);
    DUMP_TYPE(FuncFrame);
    DUMP_TYPE(FuncValue);
    DUMP_TYPE(FuncDetail);
    DUMP_TYPE(FuncSignature);
    DUMP_TYPE(FuncArgsAssignment);
  INFO("");

#if !defined(ASMJIT_NO_BUILDER)
  INFO("Size of builder classes:");
    DUMP_TYPE(BaseBuilder);
    DUMP_TYPE(BaseNode);
    DUMP_TYPE(InstNode);
    DUMP_TYPE(InstNodeWithOperands<InstNode::kBaseOpCapacity>);
    DUMP_TYPE(InstNodeWithOperands<InstNode::kFullOpCapacity>);
    DUMP_TYPE(AlignNode);
    DUMP_TYPE(LabelNode);
    DUMP_TYPE(EmbedDataNode);
    DUMP_TYPE(EmbedLabelNode);
    DUMP_TYPE(ConstPoolNode);
    DUMP_TYPE(CommentNode);
    DUMP_TYPE(SentinelNode);
  INFO("");
#endif

#if !defined(ASMJIT_NO_COMPILER)
  INFO("Size of compiler classes:");
    DUMP_TYPE(BaseCompiler);
    DUMP_TYPE(FuncNode);
    DUMP_TYPE(FuncRetNode);
    DUMP_TYPE(InvokeNode);
  INFO("");
#endif

#if !defined(ASMJIT_NO_X86)
  INFO("Size of x86-specific classes:");
    DUMP_TYPE(x86::Assembler);
    #if !defined(ASMJIT_NO_BUILDER)
    DUMP_TYPE(x86::Builder);
    #endif
    #if !defined(ASMJIT_NO_COMPILER)
    DUMP_TYPE(x86::Compiler);
    #endif
    DUMP_TYPE(x86::InstDB::InstInfo);
    DUMP_TYPE(x86::InstDB::CommonInfo);
    DUMP_TYPE(x86::InstDB::OpSignature);
    DUMP_TYPE(x86::InstDB::InstSignature);
  INFO("");
#endif
}

#undef DUMP_TYPE

static void onBeforeRun(void) noexcept {
  dumpCpu();
  dumpSizeOf();
}

int main(int argc, const char* argv[]) {
#if defined(ASMJIT_BUILD_DEBUG)
  const char buildType[] = "Debug";
#else
  const char buildType[] = "Release";
#endif

  INFO("AsmJit Unit-Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost),
    buildType
  );

  return BrokenAPI::run(argc, argv, onBeforeRun);
}
